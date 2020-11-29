#ifndef GOOGLEOAUTH2_HPP
#define GOOGLEOAUTH2_HPP

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>

#include <QJsonDocument>
#include <QSettings>

class GoogleCalendar : public QOAuth2AuthorizationCodeFlow
{
public:
    GoogleCalendar(const QString& credentials);
    ~GoogleCalendar();
    QString getCalendarList();
    static bool isOnline();
    bool isSignedIn();

    void deleteTokens();

private:
    QNetworkReply* get_EventLoop(const QString& url);
    bool checkAuthentication();
    void readCrendentials(const QString& filename);

private:
    QJsonDocument m_credentials;
    QOAuthHttpServerReplyHandler* m_replyHandler;

    QSettings m_settings;
    QDateTime m_expirationDate;
};

#endif // GOOGLEOAUTH2_HPP
