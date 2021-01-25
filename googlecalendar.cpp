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

GoogleCalendar::GoogleCalendar(const QString& credentials_file) : m_settings("Calefficient", "GoogleCalendar")
{
    readCrendentials(credentials_file);

    QString token_str = m_settings.value("token").toString();
    QString refreshToken_str = m_settings.value("refreshToken").toString();
    QDateTime expirationDate = m_settings.value("expirationDate").toDateTime();

    if(!isSignedIn())
    {
        // not needed for refreshing the token, just for grant
        m_replyHandler->setCallbackText("<h1> Logged in succesfully! Go back and enjoy Calefficient ;) </h1>\
                                        <img src=\"http://caenrigen.tech/Calefficient/Logo-512.png\" alt=\"Calefficient Logo\">");
        setScope(
                    /*https://www.googleapis.com/auth/calendar.events.readonly \ - calendar.events replaces it with writing permissions as well*/
                    "https://www.googleapis.com/auth/calendar.events \
                    https://www.googleapis.com/auth/calendar.readonly \
                    https://www.googleapis.com/auth/calendar.settings.readonly \
                    https://www.googleapis.com/auth/userinfo.email"
                    );
    }


    if(token_str != "")
        setToken(token_str);
    if(refreshToken_str != "")
        setRefreshToken(refreshToken_str);
    if(expirationDate.isValid())
        m_expirationDate = expirationDate;

    qDebug() << "START";
    qDebug() << token_str;
    qDebug() << refreshToken_str;
    qDebug() << expirationDate;

    connect(this, &QOAuth2AuthorizationCodeFlow::tokenChanged, [this](){
        qDebug() << "tokenChanged!!!!" ;
        qDebug() << "Token changed: " << token() << "\n\n\n";
        m_settings.setValue("token", token());
    });
    connect(this, &QOAuth2AuthorizationCodeFlow::refreshTokenChanged, [this](){
        qDebug() << "refreshTokenChanged!!!!" ;
        qDebug() << "Refresh token changed: " << refreshToken() << "\n\n\n";
        m_settings.setValue("refreshToken", refreshToken());
    });
    connect(this, &QOAuth2AuthorizationCodeFlow::expirationAtChanged, [this](){
        qDebug() << "expirationAtChanged!!!!" ;
        qDebug() << "Date changed: " << expirationAt() << "\n\n\n";
        m_settings.setValue("expirationDate", expirationAt());
        m_expirationDate = expirationAt();
    });
}

GoogleCalendar::~GoogleCalendar()
{
    delete m_replyHandler;
}

QDebug operator<<(QDebug dbg, const GoogleCalendar::Calendar& c){
    dbg.nospace() << "Calendar " << c.name << " [" << c.id << "]:\nColor = " << c.color_hex
                  << "; isSelected = " << c.isSelected << "; isPrimary = " << c.isPrimary
                  << "; timeZone = " << c.timeZone << "\n";
    return dbg.maybeSpace();
}

QDebug operator<<(QDebug dbg, const GoogleCalendar::Event& e){
    dbg.nospace() << "Event " << e.name << " [" << e.id << "] from Calendar " << e.calendar->name << ":\nStart = " << e.start
                  << "; End = " << e.end << "; Created = " << e.created << "; Updated = " << e.updated
                  << "\nURL = " << e.htmlLink << "; Description = " << e.description
                  << "; CreatorId = " << e.creatorId << "; organizerId = " << e.organizerId << "\n";
    return dbg.maybeSpace();
}

QVector<GoogleCalendar::Calendar> GoogleCalendar::getOwnedCalendarList()
{
    QVector<Calendar> calendars;

    Request request;
    request.url = "https://www.googleapis.com/calendar/v3/users/me/calendarList";
    request.parameters["showHidden"] = true;
    auto reply = request_EventLoop(request);
    if(reply == nullptr)
        return calendars;

    QString response = reply->readAll();
    QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
    QJsonObject object = json.object();
    auto items = object["items"].toArray();

    for(int i =0; i<items.size(); ++i){
        auto item = items[i].toObject();

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

        calendars.push_back(calendar);
    }

    // sort by primary, then selected, then name
    std::sort(calendars.begin(), calendars.end(), [](const Calendar& c1, const Calendar &c2){
        if (c1.isPrimary)
            return true;
        if (c2.isPrimary)
            return false;
        if(c1.isSelected && !c2.isSelected)
            return true;
        if(!c1.isSelected && c2.isSelected)
            return false;
        return c1.name < c2.name;
    });

    // reply = get_EventLoop("https://www.googleapis.com/oauth2/v1/userinfo", options);
    // qDebug() << reply->readAll();

    return calendars;
}

QVector<GoogleCalendar::Event> GoogleCalendar::getCalendarEvents(const GoogleCalendar::Calendar &cal, const QDateTime &start, const QDateTime &end, const QString& key)
{
    return getMultipleCalendarEvents({&cal}, start, end, key)[0];
}

QVector<QVector<GoogleCalendar::Event>> GoogleCalendar::getMultipleCalendarEvents(
        const QVector<const GoogleCalendar::Calendar*> &cals,
        const QDateTime &start,
        const QDateTime &end,
        const QString& key)
{
    QVector<QVector<GoogleCalendar::Event>> events;
    QVector<Request> requests;

    static const int MAX_RESULTS = 250; // 2500 is maximum, 250 default

    for(auto *cal: cals)
    {
        Q_ASSERT(cal->id != "");
        Q_ASSERT(end > start);

        events.push_back(QVector<GoogleCalendar::Event>());

        // https://developers.google.com/calendar/v3/reference/events/list
        Request request;
        request.url = "https://www.googleapis.com/calendar/v3/calendars/" + cal->id + "/events";
        //options["calendarId"] = cal.id;
        request.parameters["orderBy"] = "startTime";
        if(key!="")
            request.parameters["q"] = key;
        request.parameters["singleEvents"] = true;
        request.parameters["timeMin"] = QDateTimeToRFC3339Format(start);
        request.parameters["timeMax"] = QDateTimeToRFC3339Format(end);
        request.parameters["maxResults"] = MAX_RESULTS;
        requests.push_back(request);
    }

    auto replies = request_MultipleEventLoop(requests);

    for(int c = 0; c < cals.size(); ++c)
    {
        const Calendar *cal = cals[c];
        QNetworkReply* reply = replies[c];

        if(reply == nullptr)
            return events;

        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        auto items = object["items"].toArray();

        for(int i = 0; i < items.size(); ++i){
            auto item = items[i].toObject();

            Event event;
            UpdateEventFromJsonObject(event, item);
            event.calendar = cal;
            events[c].push_back(event);
        }
        //qDebug() << items.size();

        if(items.size() == MAX_RESULTS){
            // start at last event's start, to ensure any event between event.start and event.end is also consider
            // but then discard the same event that was drawn twice (assumes return comes ordered in time)
            QVector<GoogleCalendar::Event> extra_events = getCalendarEvents(*cal, events[c].back().start, end, key);
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

bool GoogleCalendar::moveEvent(GoogleCalendar::Event &event, const GoogleCalendar::Calendar &cal)
{
    Q_ASSERT(event.calendar != nullptr);

    Request request;
    request.url = "https://www.googleapis.com/calendar/v3/calendars/" + event.calendar->id + "/events/" + event.id + "/move";
    request.parameters["destination"] = cal.id;
    request.type = POST;

    QNetworkReply* reply = request_EventLoop(request);

    // qDebug() << reply->readAll();

    bool success = (reply->error() == QNetworkReply::NoError);

    if(success){
        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        UpdateEventFromJsonObject(event, object);
        Q_ASSERT(cal.id == event.organizerId);
        event.calendar = &cal;
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

    bool success = (reply->error() == QNetworkReply::NoError);
    return success;
}

QNetworkReply* GoogleCalendar::request_EventLoop(const Request &request){
    return request_MultipleEventLoop({request})[0];
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
        success = false;
    }
    else if(m_expirationDate < QDateTime::currentDateTime()){
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
    } else
        return success;

    return success;
}

void GoogleCalendar::readCrendentials(const QString& filename)
{
    QFile file;
    file.setFileName(filename);
    file.open(QIODevice::ReadOnly);
    QString val = file.readAll();

    m_credentials = QJsonDocument::fromJson(val.toUtf8());
    const auto object = m_credentials.object();
    const auto settingsObject = object["installed"].toObject();
    const auto clientId = settingsObject["client_id"].toString();
    //const auto projectId = settingsObject["project_id"].toString();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    //const QUrl auth_provider_x509_cert_url(settingsObject["auth_provider_x509_cert_url"].toString());

    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
    const auto port = static_cast<quint16>(redirectUri.port()); // Get the port

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
        int hour = timezone.mid(1, 2).toInt();
        int minute = timezone.right(2).toInt();
        int offsetInSecs = sign * (hour * 3600 + minute * 60);
        date.setOffsetFromUtc(offsetInSecs);
    }

    return date.toUTC();
}

void GoogleCalendar::UpdateEventFromJsonObject(Event &event, const QJsonObject &item)
{
    event.id = item["id"].toString();
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

    bool success = reply->error() == QNetworkReply::NoError;

    if(success){
        QString response = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject object = json.object();
        UpdateEventFromJsonObject(event, object);
    }

    return success;
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
    timeoutTimer.start(3000);

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
