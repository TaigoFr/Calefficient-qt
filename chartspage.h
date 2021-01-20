#ifndef CHARTSPAGE_H
#define CHARTSPAGE_H

#include <QWidget>

#include "googlecalendar.hpp"

class ChartsPage: public QWidget
{
    //Q_OBJECT

public:
    struct TagSettings {
        bool active;
        float target;
        bool weekOrDay;
        QString name;
        QString rename;
    };

    struct CalendarSettings : public TagSettings{
        CalendarSettings(const GoogleCalendar::Calendar* cal = nullptr);
        const GoogleCalendar::Calendar *calendar;
        QVector<TagSettings> tags;
    };

    typedef QVector<CalendarSettings> Profile;

    struct AnalysisSettings{
        QDateTime start, end;
        Profile *profile = nullptr;
    };

    struct AnalysisResults{
        AnalysisResults(const Profile&);
        QVector<float> sumCalendars;
        QVector<QVector<float>> sumTags;
        float totalEmptyTime;
    };

    ChartsPage(GoogleCalendar &a_google, QWidget * parent = nullptr);

    AnalysisResults sumCalendars(const AnalysisSettings &analysis);

private:
    GoogleCalendar &google;

private:
    QVector<const GoogleCalendar::Calendar*> getActiveCalendars(const Profile& profile);
    int findTagInString(const QString& str, const QVector<TagSettings>& calendar_tags);
};

#endif // CHARTSPAGE_H
