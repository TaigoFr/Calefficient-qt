#include "googlecalendar.hpp"

#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QNetworkReply>
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>
#include <QMutex>

#include <algorithm> // std::sort

GoogleCalendar *GoogleCalendar::instance = nullptr;

GoogleCalendar::GoogleCalendar(const QString& credentials_file) : m_settings("Calefficient", "GoogleCalendar")
{
    qRegisterMetaTypeStreamOperators<QVector<Calendar>>("QVector<Calendar>");

    readCrendentials(credentials_file);

    QString token_str = m_settings.value("token").toString();
    QString refreshToken_str = m_settings.value("refreshToken").toString();
    QDateTime expirationDate = m_settings.value("expirationDate").toDateTime();
    QDateTime last_date_checked = m_settings.value("last_date_checked").toDateTime();
    getCalendarsFromSettings();

    if(token_str != "")
        setToken(token_str);
    if(refreshToken_str != "")
        setRefreshToken(refreshToken_str);
    if(expirationDate.isValid())
        m_expirationDate = expirationDate;

    if(last_date_checked.isValid())
        m_last_date_checked = last_date_checked;
    else
        m_last_date_checked = QDateTime::currentDateTimeUtc();

    qDebug() << "START";
    qDebug() << token_str;
    qDebug() << refreshToken_str;
    qDebug() << expirationDate;
    qDebug() << m_last_date_checked;

    // not needed for refreshing the token, just for grant
    {
        m_replyHandler->setCallbackText("<h1> Logged in succesfully! Go back and enjoy Calefficient ;) </h1>\
                                        <img src=\"http://caenrigen.tech/Calefficient/Logo-512.png\" alt=\"Calefficient Logo\">");
        setScope(
                    "https://www.googleapis.com/auth/calendar.events \
                    https://www.googleapis.com/auth/calendar.readonly \
                    https://www.googleapis.com/auth/calendar.settings.readonly \
                    https://www.googleapis.com/auth/userinfo.email"
                    );
    }

    connect(this, &QOAuth2AuthorizationCodeFlow::tokenChanged, [this](){
        qDebug() << "tokenChanged!!!!" ;
        qDebug() << "Token changed: " << token() << "\n\n\n";
        m_settings.setValue("token", token());
        m_settings.sync();
    });
    connect(this, &QOAuth2AuthorizationCodeFlow::refreshTokenChanged, [this](){
        qDebug() << "refreshTokenChanged!!!!" ;
        qDebug() << "Refresh token changed: " << refreshToken() << "\n\n\n";
        m_settings.setValue("refreshToken", refreshToken());
        m_settings.sync();
    });
    connect(this, &QOAuth2AuthorizationCodeFlow::expirationAtChanged, [this](){
        qDebug() << "expirationAtChanged!!!!" ;
        qDebug() << "Date changed: " << expirationAt() << "\n\n\n";
        m_expirationDate = expirationAt().toUTC();
        m_settings.setValue("expirationDate", m_expirationDate);
        m_settings.sync();
    });
    connect(this, &QOAuth2AuthorizationCodeFlow::granted, [this](){
        qDebug() << "GRANTED!";
        emit signedIn();
    });

    //updateTimer.start(5000);
    //connect(&updateTimer, &QTimer::timeout, this, &GoogleCalendar::checkForUpdates);
}

GoogleCalendar::~GoogleCalendar()
{
    delete m_replyHandler;

    for(Calendar* cal: m_calendars)
        delete cal;
}

void GoogleCalendar::setCredentials(const QString &credentials)
{
    instance = new GoogleCalendar(credentials);
}

GoogleCalendar &GoogleCalendar::getInstance()
{
    return *instance;
}

QDebug operator<<(QDebug dbg, const GoogleCalendar::Calendar* c){
    dbg.nospace() << "Calendar " << c->name << " [" << c->id << "]:\nColor = " << c->color_hex
                  << "; isSelected = " << c->isSelected << "; isPrimary = " << c->isPrimary
                  << "; timeZone = " << c->timeZone << "\n";
    return dbg.maybeSpace();
}

bool GoogleCalendar::Calendar::operator!=(const GoogleCalendar::Calendar& other) const
{ return !(*this == other); }
bool GoogleCalendar::Calendar::operator==(const GoogleCalendar::Calendar& other) const{
    assert(id == other.id);
    return (name == other.name &&
            color_hex == other.color_hex &&
            isSelected == other.isSelected &&
            isPrimary == other.isPrimary &&
            timeZone == other.timeZone);
}

// functions to allow writing to QSettings as a CustomType
QDataStream& operator<<(QDataStream& out, const GoogleCalendar::Calendar& c) {
    out << c.name << c.color_hex << c.id << c.isSelected << c.isPrimary << c.timeZone;
    return out;
}

QDataStream& operator>>(QDataStream& in, GoogleCalendar::Calendar& c) {
    in >> c.name >> c.color_hex >> c.id >> c.isSelected >> c.isPrimary >> c.timeZone;
    return in;
}

QDebug operator<<(QDebug dbg, const GoogleCalendar::Event& e){
    dbg.nospace() << "Event " << e.name << " [" << e.id << "] from Calendar " << e.calendar->name << ":\nStart = " << e.start
                  << "; End = " << e.end << "; Created = " << e.created << "; Updated = " << e.updated
                  << "\nURL = " << e.htmlLink << "; Description = " << e.description
                  << "; CreatorId = " << e.creatorId << "; organizerId = " << e.organizerId << "\n";
    return dbg.maybeSpace();
}

QVector<GoogleCalendar::Calendar*>& GoogleCalendar::getOwnedCalendarList()
{
    static QMutex mutex;
    mutex.lock();
    if(m_calendars.size() == 0){
        auto updated_calendars = getUpdatedOwnedCalendarList();
        for(Calendar &cal : updated_calendars)
            m_calendars.push_back(new Calendar(cal));
        sortCalendars();
        setCalendarsInSettings();
    }
    mutex.unlock();

    return m_calendars;
}


QVector<GoogleCalendar::Calendar> GoogleCalendar::getUpdatedOwnedCalendarList()
{
    QVector<Calendar> updated_calendars;

    Request request;
    request.url = "https://www.googleapis.com/calendar/v3/users/me/calendarList";
    request.parameters["showHidden"] = true;
    QNetworkReply* reply = request_EventLoop(request);
    if(reply == nullptr || reply->error() != QNetworkReply::NoError)
        return updated_calendars;

    QString response = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
    QJsonObject object = json.object();
    QJsonArray items = object["items"].toArray();

    for(int i =0; i<items.size(); ++i){
        QJsonObject item = items[i].toObject();

        // skip if not owner
        if(item["accessRole"] != "owner")
            continue;

        Calendar calendar;
        calendar.name = item["summary"].toString();
        calendar.color_hex = item["backgroundColor"].toString();
        calendar.id = item["id"].toString();
        calendar.isSelected = item["selected"].toBool();
        calendar.isPrimary = false;
        // in principle only the primary calendar has this, and has it set to true
        if(item.contains("primary"))
            calendar.isPrimary = item["primary"].toBool();
        calendar.timeZone = item["timeZone"].toString();

        updated_calendars.push_back(calendar);
    }

    // reply = get_EventLoop("https://www.googleapis.com/oauth2/v1/userinfo", options);
    // qDebug() << reply->readAll();

    return updated_calendars;
}

QVector<GoogleCalendar::Event> GoogleCalendar::getCalendarEvents(GoogleCalendar::Calendar *cal,
                                                                 const QDateTime &start,
                                                                 const QDateTime &end,
                                                                 const QDateTime& minUpdateDate,
                                                                 const QString& key)
{
    QVector<QVector<GoogleCalendar::Event>> list = getMultipleCalendarEvents({cal}, start, end, minUpdateDate, key);
    return list[0];
}

QVector<QVector<GoogleCalendar::Event>> GoogleCalendar::getMultipleCalendarEvents(
        const QVector<GoogleCalendar::Calendar*> &cals,
        const QDateTime &start,
        const QDateTime &end,
        const QDateTime& minUpdateDate,
        const QString& key)
{
    QVector<QVector<GoogleCalendar::Event>> events;
    QVector<Request> requests;

    static const int MAX_RESULTS = 250; // 2500 is maximum, 250 default

    for(const GoogleCalendar::Calendar *cal: cals)
    {
        Q_ASSERT(cal->id != "");

        events.push_back(QVector<GoogleCalendar::Event>());

        // https://developers.google.com/calendar/v3/reference/events/list
        Request request;
        request.url = "https://www.googleapis.com/calendar/v3/calendars/" + cal->id + "/events";
        //options["calendarId"] = cal.id;
        request.parameters["orderBy"] = "startTime";
        if(key!="")
            request.parameters["q"] = key;
        request.parameters["singleEvents"] = true;
        if(start.isValid())
            request.parameters["timeMin"] = QDateTimeToRFC3339Format(start);
        if(end.isValid())
            request.parameters["timeMax"] = QDateTimeToRFC3339Format(end);
        request.parameters["maxResults"] = MAX_RESULTS;

        if(minUpdateDate.isValid()){
            request.parameters["updatedMin"] = QDateTimeToRFC3339Format(minUpdateDate);
            // request.parameters["showDeleted"] = true; // already default when 'updateMin' is provided
        }

        requests.push_back(request);
    }

    QVector<QNetworkReply*> replies = request_MultipleEventLoop(requests);

    for(int c = 0; c < cals.size(); ++c)
    {
        QNetworkReply* reply = replies[c];
        if(reply == nullptr || reply->error() != QNetworkReply::NoError)
            continue;

        Calendar *cal = cals[c];

        if(reply == nullptr)
            return events;

        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        QJsonArray items = object["items"].toArray();

        for(int i = 0; i < items.size(); ++i){
            QJsonObject item = items[i].toObject();

            Event event;
            updateEventFromJsonObject(event, item);
            event.calendar = cal;
            events[c].push_back(event);
        }
        //qDebug() << items.size();

        if(items.size() == MAX_RESULTS){
            // start at last event's start, to ensure any event between event.start and event.end is also consider
            // but then discard the same event that was drawn twice (assumes return comes ordered in time)
            QVector<GoogleCalendar::Event> extra_events = getCalendarEvents(cal, events[c].back().start, end, minUpdateDate, key);
            extra_events.removeFirst();
            events[c].append(extra_events);
        }
    }

    return events;
}

bool GoogleCalendar::createEvent(GoogleCalendar::Event &event)
{
    bool update = false;
    return createOrUpdateEvent(event, update);
}

bool GoogleCalendar::moveEvent(GoogleCalendar::Event &event, const GoogleCalendar::Calendar *cal)
{
    Q_ASSERT(event.calendar != nullptr);

    Request request;
    request.url = "https://www.googleapis.com/calendar/v3/calendars/" + event.calendar->id + "/events/" + event.id + "/move";
    request.parameters["destination"] = cal->id;
    request.type = POST;

    QNetworkReply* reply = request_EventLoop(request);

    // qDebug() << reply->readAll();

    bool success = (reply != nullptr && reply->error() == QNetworkReply::NoError);

    if(success){
        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        updateEventFromJsonObject(event, object);
        Q_ASSERT(cal->id == event.organizerId);
        event.calendar = cal;
    }

    return success;
}

bool GoogleCalendar::updateEvent(GoogleCalendar::Event &event)
{
    bool update = true;
    return createOrUpdateEvent(event, update);
}

bool GoogleCalendar::deleteEvent(GoogleCalendar::Event &event)
{
    Q_ASSERT(event.id != "");

    Request request;
    request.url = "https://www.googleapis.com/calendar/v3/calendars/" + event.calendar->id + "/events/" + event.id;
    request.type = DELETE;
    QNetworkReply* reply = request_EventLoop(request);

    // qDebug() << reply->readAll();

    bool success = (reply != nullptr && reply->error() == QNetworkReply::NoError);
    return success;
}

bool GoogleCalendar::checkForUpdates()
{
    qDebug() << "HERE";
    return checkForCalendarUpdates() || checkForEventUpdates();
}


bool GoogleCalendar::checkForEventUpdates()
{
    QVector<QVector<Event>> new_events = getMultipleCalendarEvents(
                m_calendars, QDateTime() /*start*/, QDateTime() /*end*/, m_last_date_checked);


    m_last_date_checked = QDateTime::currentDateTimeUtc();
    m_settings.setValue("last_date_checked", m_last_date_checked);
    m_settings.sync();

    QVector<QVector<QString>> deleted_ids(new_events.size());
    QVector<QVector<Event>> updated_events(new_events.size());

    bool new_exist = false;
    for(int i = 0; i < new_events.size(); ++i){
        for(Event& event: new_events[i]){
            new_exist = true;
            if(event.start.isValid())
                updated_events[i].push_back(event);
            else
                deleted_ids[i].push_back(event.id);
        }
    }

    if(new_exist){
        qDebug() << new_events;

        emit eventsUpdated(updated_events, deleted_ids);
        return true;
    }

    return false;
}

void GoogleCalendar::setCalendarsInSettings()
{
    QVector<Calendar> store;
    for(Calendar *cal: m_calendars)
        store.push_back(*cal);
    m_settings.setValue("calendars", QVariant::fromValue(store));
    m_settings.sync(); // force
}

void GoogleCalendar::getCalendarsFromSettings()
{
    QVector<Calendar> read = m_settings.value("calendars").value<QVector<Calendar>>();
    for(Calendar &cal: read)
        m_calendars.push_back(new Calendar(cal));
    qDebug() << m_calendars;
}


bool GoogleCalendar::checkForCalendarUpdates()
{
    QVector<Calendar> updated_calendars = getUpdatedOwnedCalendarList();

    QVector<QString> deleted_ids;
    QVector<const Calendar*> created, updated;

    QMap<QString, int> dict;
    for(Calendar &cal : updated_calendars)
        dict[cal.id] = -1;

    for(int i = 0; i < m_calendars.size(); ++i){
        Calendar *cal = m_calendars[i];
        if(dict.find(cal->id) == dict.end()){
            deleted_ids.push_back(cal->id);
            m_calendars.remove(i);
            delete cal;
            --i; // calendars shift after remove
        }
        else{
            dict[cal->id] = i;
        }
    }

    for(int i = 0; i < updated_calendars.size(); ++i){
        const Calendar &cal = updated_calendars[i];
        int idx = dict[cal.id];
        if(idx == -1){
            Calendar* new_cal = new Calendar(cal);
            m_calendars.push_back(new_cal);
            created.push_back(new_cal);
        }
        else{
            if(cal != (*m_calendars[idx])){
                *m_calendars[idx] = cal;
                updated.push_back(m_calendars[idx]);
            }
        }
    }

    if(created.size() || updated.size())
        sortCalendars();

    if(deleted_ids.size() || created.size() || updated.size()){
        qDebug() << "deleted_ids" << deleted_ids;
        qDebug() << "created" << created;
        qDebug() << "updated" << updated;
        setCalendarsInSettings();
        emit calendarsUpdated(deleted_ids, created, updated);
        return true;
    }

    return false;
}

QNetworkReply* GoogleCalendar::request_EventLoop(const Request &request)
{
    QVector<QNetworkReply*> list = request_MultipleEventLoop({request});
    return list[0];
}

QVector<QNetworkReply*> GoogleCalendar::request_MultipleEventLoop(const QVector<Request>& requests)
{
    //get(QUrl("https://www.googleapis.com/plus/v1/people/me"));
    if(!checkAuthentication())
        return QVector<QNetworkReply*>(requests.size(), nullptr);

    QEventLoop loop;
    QTimer timer;
    QMutex mutex;
    int finished_requests = 0;

    int total = requests.size();
    QVector<QNetworkReply*> replies(total);
    for(int i=0; i<total; ++i){
        const Request &request = requests[i];
        // when we change the pointer QNetworkReply* we want it to be changed in 'replies' directly
        QNetworkReply **reply = &replies[i];
        switch(request.type){
        case GET:
            Q_ASSERT(request.format == VARIANT_MAP);
            *reply = get(QUrl(request.url), request.parameters);
            break;
        case PUT:
            if(request.format == VARIANT_MAP)
                *reply = put(QUrl(request.url), request.parameters);
            else
                *reply = put(QUrl(request.url), QJsonDocument::fromVariant(request.parameters).toJson());
            break;
        case POST:
            if(request.format == VARIANT_MAP)
                *reply = post(QUrl(request.url), request.parameters);
            else
                *reply = post(QUrl(request.url), QJsonDocument::fromVariant(request.parameters).toJson());
            break;
        case DELETE:
            Q_ASSERT(request.format == VARIANT_MAP);
            *reply = deleteResource(QUrl(request.url), request.parameters);
            break;
        default:
            qDebug("Option not implemented");
            exit(1);
        }

        connect(*reply, &QNetworkReply::finished, [&finished_requests, reply, i, request, &mutex, &loop, total](){
            qDebug() << "REQUEST" + (total>1 ? " " + QString::number(i) : "") + " type" << request.type << "FINISHED. Error?" << ((*reply)->error() != QNetworkReply::NoError);
            if((*reply)->error() != QNetworkReply::NoError)
                qDebug() << (*reply)->error();

            // lock increase of variable to prevent race-conditions when multiple requests finish at the same time
            mutex.lock();
            ++finished_requests;
            mutex.unlock();

            if(finished_requests == total)
                loop.quit();
        });

    }

    connect(&timer, &QTimer::timeout, [](){
        qDebug() << "CALENDAR REQUEST NOT RECEIVED IN TIME!!!!!!";
    });

    timer.start(1000 * 5 * total); // 5 secs
    loop.exec();

    return replies;
}

bool GoogleCalendar::checkAuthentication()
{
    bool success = true;

    if(!isSignedIn()){
        grant();
        qDebug() << "Are you sure you want to get here?";
        success = false;
    }
    else if(m_expirationDate < QDateTime::currentDateTimeUtc()){
        if(isOnline()){
            QEventLoop loop;
            QTimer timer;
            refreshAccessToken();

            connect(&timer, &QTimer::timeout, [&success, &loop, this](){
                qDebug() << "NOT REFRESHED IN TIME!!!!!!";
                success = false;
                // something very wrong happened with refresh token, delete and force grant again
                deleteTokens();
                loop.quit();
            });
            connect(this, &QOAuth2AuthorizationCodeFlow::tokenChanged, &loop, &QEventLoop::quit);

            timer.start(1000 * 5); // 5 secs
            loop.exec();
        }
        else
            success = false;
    }

    return success;
}

void GoogleCalendar::readCrendentials(const QString& filename)
{
    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly);
    QString val = file.readAll();

    m_credentials = QJsonDocument::fromJson(val.toUtf8());
    const QJsonObject object = m_credentials.object();
    const QJsonObject settingsObject = object["installed"].toObject();
    const QString clientId = settingsObject["client_id"].toString();
    //const QString projectId = settingsObject["project_id"].toString();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    //const QUrl auth_provider_x509_cert_url(settingsObject["auth_provider_x509_cert_url"].toString());

    const QJsonArray redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
    const quint16 port = redirectUri.port(); // Get the port

    m_replyHandler = new QOAuthHttpServerReplyHandler(port, this);

    setReplyHandler(m_replyHandler);
    setAuthorizationUrl(authUri); // not needed for refreshing the token, just for grant
    setClientIdentifier(clientId);
    setAccessTokenUrl(tokenUri);
    //google->setClientIdentifierSharedKey(clientSecret);
}

QString GoogleCalendar::QDateTimeToRFC3339Format(const QDateTime & date)
{
    return date.toUTC().toString("yyyy-MM-ddThh:mm:ssZ");
}

QDateTime GoogleCalendar::QDateTimeFromRFC3339Format(const QString &str)
{
    // 4 cases:
    // 2020-11-24T13:15:00Z
    // 2020-11-24T15:42:51.000Z
    // 2020-11-30T20:00:00+01:00
    // 2020-11-30T20:00:00.000+01:00

    bool hasMiliseconds = (str.indexOf('.') != -1);
    bool hasTimeZone = (str[str.size()-1] != 'Z');

    QString str_date = str.left(str.size() - (hasTimeZone ? 6 : 1));
    QDateTime date;
    if(hasMiliseconds){
        date = QDateTime::fromString(str_date, "yyyy-MM-ddThh:mm:ss.zzz");
    }
    else{
        date = QDateTime::fromString(str_date, "yyyy-MM-ddThh:mm:ss");
    }
    date.setOffsetFromUtc(0); // force UTC

    if(hasTimeZone){
        QString timezone = str.right(6);
        int sign = (timezone[0] == '+' ? 1 : -1);
        int hour = timezone.midRef(1, 2).toInt();
        int minute = timezone.rightRef(2).toInt();
        int offsetInSecs = sign * (hour * 3600 + minute * 60);
        date.setOffsetFromUtc(offsetInSecs);
    }

    return date.toUTC();
}

void GoogleCalendar::updateEventFromJsonObject(Event &event, const QJsonObject &item)
{
    event.id = item["id"].toString();
    if(item.contains("status")){
        QString status = item["status"].toString();
        if(status == "cancelled")
            return;
    }

    event.htmlLink = item["htmlLink"].toString();

    QJsonObject start = item["start"].toObject();
    QJsonObject end = item["end"].toObject();
    if(start.contains("date")){
        event.start = QDateTime::fromString(start["date"].toString(), "yyyy-MM-dd");
        event.start.setOffsetFromUtc(0);
    }
    else
        event.start = QDateTimeFromRFC3339Format(start["dateTime"].toString());

    if(end.contains("date")){
        event.end = QDateTime::fromString(end["date"].toString(), "yyyy-MM-dd");
        event.end.setOffsetFromUtc(0);
    }
    else
        event.end = QDateTimeFromRFC3339Format(end["dateTime"].toString());

    event.created = QDateTimeFromRFC3339Format(item["created"].toString());
    event.updated = QDateTimeFromRFC3339Format(item["updated"].toString());
    event.name = item["summary"].toString();
    event.creatorId = item["creator"].toObject()["email"].toString();
    event.organizerId = item["organizer"].toObject()["email"].toString();

    if(item.contains("description"))
        event.description = item["description"].toString();
}

bool GoogleCalendar::createOrUpdateEvent(GoogleCalendar::Event &event, bool update)
{
    Q_ASSERT(event.calendar != nullptr);

    Request request;

    QVariantMap endMap;
    endMap["dateTime"] = QDateTimeToRFC3339Format(event.end);
    request.parameters["end"] = endMap;
    QVariantMap startMap;
    startMap["dateTime"] = QDateTimeToRFC3339Format(event.start);
    request.parameters["start"] = startMap;

    request.parameters["summary"] = event.name;
    request.parameters["description"] = event.description;

    QNetworkReply* reply;
    if(update){
        Q_ASSERT(event.id != "");

        request.url = "https://www.googleapis.com/calendar/v3/calendars/" + event.calendar->id + "/events/" + event.id;
        request.type = PUT;
        request.format = REQUEST_BODY;
        reply = request_EventLoop(request);
    }
    else{
        request.url = "https://www.googleapis.com/calendar/v3/calendars/" + event.calendar->id + "/events";
        request.type = POST;
        request.format = REQUEST_BODY;
        reply = request_EventLoop(request);
    }

    // qDebug() << reply->readAll();

    bool success = (reply != nullptr && reply->error() == QNetworkReply::NoError);

    if(success){
        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        updateEventFromJsonObject(event, object);
    }

    return success;
}

void GoogleCalendar::sortCalendars()
{
    // sort by primary, then selected, then name
    std::sort(m_calendars.begin(), m_calendars.end(), [](const Calendar* c1, const Calendar *c2){
        if(c1->isPrimary)
            return true;
        if(c2->isPrimary)
            return false;
        if(c1->isSelected && !c2->isSelected)
            return true;
        if(!c1->isSelected && c2->isSelected)
            return false;
        return c1->name < c2->name;
    });
}

// https://stackoverflow.com/questions/13779789/monitoring-internet-connection-status
bool GoogleCalendar::isOnline()
{
    bool retVal = false;
    QNetworkAccessManager nam;
    QNetworkRequest req(QUrl("http://www.google.com"));
    QNetworkReply* reply = nam.get(req);
    QEventLoop loop;
    QTimer timeoutTimer;

    connect(&timeoutTimer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));

    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(1000);

    loop.exec();

    if (reply->bytesAvailable())
    {
        retVal = true;
    }

    return retVal;
}

bool GoogleCalendar::isSignedIn()
{
    return token() != "";
}

void GoogleCalendar::deleteTokens()
{
    setToken("");
    setRefreshToken("");
    m_settings.remove("expirationDate");
    m_expirationDate = QDateTime();
}

void GoogleCalendar::deleteSettings()
{
    deleteTokens();
    m_settings.clear();
    m_last_date_checked = QDateTime();
    for(Calendar* cal: m_calendars)
        delete cal;
    m_calendars.clear();
}
