#include "chartspage.h"

#include <QtConcurrent>

ChartsPage::ChartsPage(GoogleCalendar &a_google, QWidget *parent): QWidget(parent), google(a_google)
{

}

QVector<float> ChartsPage::sumCalendars(const ChartsPage::AnalysisSettings &analysis)
{
    QVector<QVector<GoogleCalendar::Event>> events =
            google.getMultipleCalendarEvents(getActiveCalendars(*analysis.profile),analysis.start, analysis.end);

    qDebug() << events;

    return {0};
}

QVector<const GoogleCalendar::Calendar*> ChartsPage::getActiveCalendars(const ChartsPage::Profile &profile)
{
    QVector<const GoogleCalendar::Calendar*> new_profile;
    for(auto &calendar: profile)
        if(calendar.active)
            new_profile.push_back(calendar.calendar);

    return new_profile;
}
