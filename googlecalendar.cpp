#include "googlecalendar.hpp"
#include <QDesktopServices>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QNetworkReply>
#include <QDateTime>
#include <QEventLoop>
#include <QTimer>

GoogleCalendar::GoogleCalendar(const QString& credentials_file) : m_settings("Calefficient", "GoogleCalendar")
{
    readCrendentials(credentials_file);

    QString token_str = m_settings.value("token").toString();
    QString refreshToken_str = m_settings.value("refreshToken").toString();
    QDateTime expirationDate = m_settings.value("expirationDate").toDateTime();

    if(isSignedIn())
    {
        // not needed for refreshing the token, just for grant
        m_replyHandler->setCallbackText("<h1> Logged in succesfully! Go back and enjoy Calefficient ;) </h1>\
                                        <img src=\"http://caenrigen.tech/Calefficient/Logo-512.png\" alt=\"Calefficient Logo\">");
        setScope("https://www.googleapis.com/auth/calendar.readonly");
        connect(this, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
                &QDesktopServices::openUrl);
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

QString GoogleCalendar::getCalendarList()
{
    auto reply = get_EventLoop("https://www.googleapis.com/calendar/v3/users/me/calendarList");
    if(reply != nullptr)
        return reply->readAll();
    else
        return "";
}

QNetworkReply* GoogleCalendar::get_EventLoop(const QString &url)
{
    //get(QUrl("https://www.googleapis.com/plus/v1/people/me"));
    if(!checkAuthentication()) return nullptr;

    QEventLoop loop;
    QTimer timer;

    auto reply = get(QUrl(url));

    connect(reply, &QNetworkReply::finished, [reply, &loop](){
        qDebug() << "REQUEST FINISHED. Error? " << (reply->error() != QNetworkReply::NoError);
        if((reply->error() != QNetworkReply::NoError))
            qDebug() << reply->error();
        loop.quit();
    });

    connect(&timer, &QTimer::timeout, [](){
        qDebug() << "CALENDAR REQUEST NOT RECIEVED IN TIME!!!!!!";
    });

    timer.start(1000 * 5); // 5 secs
    loop.exec();

    return reply;
}

bool GoogleCalendar::checkAuthentication()
{
    bool success = true;

    if(token() == ""){
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
                m_settings.remove("token");
                m_settings.remove("refreshToken");
                m_settings.remove("expirationDate");
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
    return token()=="";
}
