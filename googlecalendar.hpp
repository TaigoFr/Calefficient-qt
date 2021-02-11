#ifndef GOOGLEOAUTH2_HPP
#define GOOGLEOAUTH2_HPP

#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>

#include <QJsonDocument>
#include <QSettings>
#include <QColor>
#include <QDataStream>
#include <QTimer>

class GoogleCalendar : public QOAuth2AuthorizationCodeFlow
{
    Q_OBJECT

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

      friend QDebug operator<<(QDebug dbg, const Calendar* c);

      bool operator!=(const Calendar& other) const;
      bool operator==(const Calendar& other) const;

      // functions to allow writing to QSettings as a CustomType
      friend QDataStream &operator<<(QDataStream &out, const Calendar &c);
      friend QDataStream &operator>>(QDataStream &in, Calendar &c);
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

private:
    GoogleCalendar(const QString& credentials);
    ~GoogleCalendar();

public:
    static void setCredentials(const QString& credentials);
    static GoogleCalendar& getInstance();

    static bool isOnline();
    bool isSignedIn();

    Calendar* getCalendarById(const QString &id);
    QVector<Calendar*>& getOwnedCalendarList();
    QVector<Event> getCalendarEvents(Calendar* cal,
                                     const QDateTime &start,
                                     const QDateTime &end,
                                     const QDateTime& minUpdateDate = QDateTime(),
                                     const QString &key = "");
    QVector<QVector<Event>> getMultipleCalendarEvents(const QVector<Calendar*>& cal,
                                                      const QDateTime &start,
                                                      const QDateTime &end,
                                                      const QDateTime& minUpdateDate = QDateTime(),
                                                      const QString &key = "");
    bool createEvent(Event& event);
    bool moveEvent(Event& event, const Calendar* cal);
    bool updateEvent(Event& event);
    bool deleteEvent(Event& event);

    bool checkForUpdates();

    void deleteTokens();
    void deleteSettings();

signals:
    void signedIn();
    void calendarsUpdated(const QVector<QString> deleted_ids, QVector<const Calendar*> created, QVector<const Calendar*> updated);
    void eventsUpdated(QVector<QVector<Event>> events, QVector<QVector<QString>> deleted_ids);

private:
    QVector<Calendar> getUpdatedOwnedCalendarList();
    QNetworkReply* request_EventLoop(const Request& request);
    QVector<QNetworkReply*> request_MultipleEventLoop(const QVector<Request>& requests);
    bool checkAuthentication();
    void readCrendentials(const QString& filename);

    QString QDateTimeToRFC3339Format(const QDateTime&);
    QDateTime QDateTimeFromRFC3339Format(const QString&);
    void updateEventFromJsonObject(Event &event, const QJsonObject &item);

    bool createOrUpdateEvent(Event& event, bool update);

    void sortCalendars();

    bool checkForCalendarUpdates();
    bool checkForEventUpdates();
    void setCalendarsInSettings();
    void getCalendarsFromSettings();

private:
    QJsonDocument m_credentials;
    QOAuthHttpServerReplyHandler* m_replyHandler;

    QSettings m_settings;
    QDateTime m_expirationDate;
    QDateTime m_last_date_checked;

    QVector<Calendar*> m_calendars;

    static GoogleCalendar* instance;
    QTimer updateTimer;
};


Q_DECLARE_METATYPE(GoogleCalendar::Calendar)


#endif // GOOGLEOAUTH2_HPP
