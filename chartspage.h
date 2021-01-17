#ifndef CHARTSPAGE_H
#define CHARTSPAGE_H

#include <QWidget>

#include "googlecalendar.hpp"

class ChartsPage: public QWidget
{
    //Q_OBJECT

public:
    struct CalendarSettings{
        bool active;
        float target;
        bool weekOrDay;
        QString rename;
        const GoogleCalendar::Calendar *calendar = nullptr;
    };

    typedef QVector<CalendarSettings> Profile;

    struct AnalysisSettings{
        QDateTime start, end;
        Profile *profile = nullptr;
    };

    ChartsPage(GoogleCalendar &a_google, QWidget * parent = nullptr);

    QVector<float> sumCalendars(const AnalysisSettings &analysis);

private:
    GoogleCalendar &google;

    QVector<const GoogleCalendar::Calendar*> getActiveCalendars(const Profile& profile);
};

#endif // CHARTSPAGE_H
