#ifndef GOOGLEOAUTH2_HPP
#define GOOGLEOAUTH2_HPP

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>

#include <QJsonDocument>
#include <QSettings>
#include <QColor>

class GoogleCalendar : public QOAuth2AuthorizationCodeFlow
{
    enum RequestType{
        GET,
        PUT,
        POST,
        DELETE
    };
    enum RequestFormat{
        VARIANT_MAP,
        REQUEST_BODY
    };
    struct Request{
        QString url;
        QVariantMap parameters = QVariantMap();
        RequestType type = GET;
        RequestFormat format = VARIANT_MAP;
    };

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
        QString htmlLink;
        QDateTime start, end;
        QDateTime created, updated;
        QString name, description;
        QString creatorId, organizerId;

        const Calendar *calendar = nullptr;

        friend QDebug operator<<(QDebug dbg, const Event& e);
    };

public:
    GoogleCalendar(const QString& credentials);
    ~GoogleCalendar();
    static bool isOnline();
    bool isSignedIn();

    QVector<Calendar> getOwnedCalendarList();
    QVector<Event> getCalendarEvents(const Calendar& cal, const QDateTime &start, const QDateTime &end, const QString &key = "");
    QVector<QVector<Event>> getMultipleCalendarEvents(const QVector<const Calendar*>& cal, const QDateTime &start, const QDateTime &end, const QString &key = "");
    bool createEvent(Event& event);
    bool moveEvent(Event& event, const Calendar& cal);
    bool updateEvent(Event& event);
    bool deleteEvent(Event& event);

    void deleteTokens();

private:
    QNetworkReply* request_EventLoop(const Request& request);
    QVector<QNetworkReply*> request_MultipleEventLoop(const QVector<Request>& requests);
    bool checkAuthentication();
    void readCrendentials(const QString& filename);

    QString QDateTimeToRFC3339Format(const QDateTime&);
    QDateTime QDateTimeFromRFC3339Format(const QString&);
    void UpdateEventFromJsonObject(Event &event, const QJsonObject &item);

    bool createOrUpdateEvent(Event& event, bool update);

private:
    QJsonDocument m_credentials;
    QOAuthHttpServerReplyHandler* m_replyHandler;

    QSettings m_settings;
    QDateTime m_expirationDate;
};

#endif // GOOGLEOAUTH2_HPP
