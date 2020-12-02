#ifndef GOOGLEOAUTH2_HPP
#define GOOGLEOAUTH2_HPP

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>

#include <QJsonDocument>
#include <QSettings>
#include <QColor>

class GoogleCalendar : public QOAuth2AuthorizationCodeFlow
{
public:
    struct Calendar{
      QString name;
      QString color_hex;
      QString id;
      bool isSelected;
      bool isPrimary;
      QString timeZone;

      friend QDebug operator<<(QDebug dbg, const Calendar& c);
    };

    struct Event{
        QString id;
    };

public:
    GoogleCalendar(const QString& credentials);
    ~GoogleCalendar();
    static bool isOnline();
    bool isSignedIn();

    QVector<Calendar> getOwnedCalendarList();
    QVector<Event> getEvents(const Calendar& cal, const QDateTime &start, const QDateTime &end);
    Calendar getDefaultCalendar();
    QString getUsername();

    void deleteTokens();

private:
    QNetworkReply* get_EventLoop(const QString& url, const QVariantMap& parameters = QVariantMap());
    bool checkAuthentication();
    void readCrendentials(const QString& filename);

private:
    QJsonDocument m_credentials;
    QOAuthHttpServerReplyHandler* m_replyHandler;

    QSettings m_settings;
    QDateTime m_expirationDate;
};

#endif // GOOGLEOAUTH2_HPP
