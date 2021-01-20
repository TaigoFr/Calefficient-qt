#include "chartspage.h"

#include <QtConcurrent>
#include <QSettings>
#include <QDateTime>

#define MSECS_PER_HOUR (1000 * 60 * 60)

ChartsPage::ChartsPage(GoogleCalendar &a_google, QWidget *parent): QWidget(parent), google(a_google)
{

}

ChartsPage::CalendarSettings::CalendarSettings(const GoogleCalendar::Calendar* cal) {
    calendar = cal;
    if(calendar != nullptr)
        name = calendar->name;
}

ChartsPage::AnalysisResults::AnalysisResults(const Profile& profile) {
    int num_calendars = profile.size();
    sumCalendars.fill(0., num_calendars);
    sumTags.resize(num_calendars);

    for(int i = 0; i < num_calendars; ++i)
        sumTags[i].fill(0., profile[i].tags.size());

    totalEmptyTime = 0.;
}

ChartsPage::AnalysisResults ChartsPage::sumCalendars(const ChartsPage::AnalysisSettings &analysis)
{
    const Profile& profile = *analysis.profile;
    QVector<QVector<GoogleCalendar::Event>> events =
            google.getMultipleCalendarEvents(getActiveCalendars(profile), analysis.start, analysis.end);

    int num_calendars = profile.size();

    // For each Calendar what is the index of the event we are in at the current timestamp
    QVector<int> indices(num_calendars, 0);

    AnalysisResults results(profile);
    QSettings settings ("Calefficient", "Settings");

    QVector<QDateTime> empty_list;
    bool ignore_24h = settings.value("ignore_24h_events").toBool();
    bool warm_empty_slots = settings.value("warn_empty_slots").toBool();
    bool calculate_empty_slots = settings.value("calculate_empty_slots").toBool();

    // current timestamp we're in
    QDateTime start = analysis.start;

    //iterate until we reach the final data
    while( start < analysis.end ){
        /* 'next' corresponds to the last date that a given calendar can access
        (the earliest start time of the current events from calendar with higher priority,
        such that we know we cant move forward in the 'current' lower priority calendar)
        */
        QDateTime next = analysis.end;
        bool isBlank = true;
        //loop through calendars in priority order
        for(int i = 0; i < num_calendars; ++i){

            // if we hit 'next', set 'repeat' to 'true' and come back to 1st calendar
            bool repeat = false;
            const GoogleCalendar::Event* event;
            /* Loop through events of the calendar until
             (1) events finish for this calendar
             (2) there is a blank space (so that we can check if other lower priority calendars fill that space)
             (3) if we reach 'next', the start of an event of a higher priority calendar
            */
            while(indices[i] < events[i].size()){
                event = &events[i][indices[i]];

                //case (2)
                if(event->start > start )
                    break;

                //event is old, we're already past that
                if( event->end <= start ){
                    ++indices[i];
                    continue;
                }

                // event is bigger than 24h, skip
                if(ignore_24h && start.msecsTo(event->end) >= 24 * MSECS_PER_HOUR){
                    ++indices[i];
                    continue;
                }

                isBlank = false;

                //add to the sum only the part that is relevant without overlap
                QDateTime end = std::min(event->end, next);
                float duration = start.msecsTo(end);
                start = end;

                if(profile[i].tags.size() > 0){
                    int where = findTagInString(event->name, profile[i].tags);
                    if(where>=0)
                        results.sumTags[i][where] += duration;
                    else
                        results.sumCalendars[i] += duration;
                }
                else
                    results.sumCalendars[i] += duration;

                //case (3)
                if(end == next){
                    repeat = true;
                    break;
                }

                ++indices[i];
            }

            //in case (3), since we reached a higher priority calendar, one must go back to the beginning of the for loop
            if( repeat ) break;
            //set the right 'next' for the next 'for' loop iteration
            if( indices[i] < events[i].size() ) next = std::min(next, event->start);
        }
        if(isBlank && calculate_empty_slots){
            if(warm_empty_slots)
                empty_list.push_back(start);

            // even if user doesn't want to calculate empty Slots, 'sumCalendars' does it anyway. The overhead is just summing them in this line, which amounts to nothing
            results.totalEmptyTime += start.msecsTo(next);
            start = next; // leave blank space alone and continue
        }
    }

    //convert to hours
    for(int i = 0; i < num_calendars; ++i){
        results.sumCalendars[i] /= MSECS_PER_HOUR;
        for(int t = 0; t < profile[i].tags.size(); ++t)
            results.sumTags[i][t] /= MSECS_PER_HOUR;
    }

    results.totalEmptyTime /= MSECS_PER_HOUR;

    return results;
}

QVector<const GoogleCalendar::Calendar*> ChartsPage::getActiveCalendars(const ChartsPage::Profile &profile)
{
    QVector<const GoogleCalendar::Calendar*> new_profile;
    for(auto &calendar: profile)
        if(calendar.active)
            new_profile.push_back(calendar.calendar);

    return new_profile;
}

int ChartsPage::findTagInString(const QString &str, const QVector<ChartsPage::TagSettings> &calendar_tags)
{
    int t = 0;
    for(; t < calendar_tags.size(); ++t)
        if(str.indexOf(calendar_tags[t].name) != -1)
            break;

    if(t == calendar_tags.size())
        return -1;
    return t;
}
