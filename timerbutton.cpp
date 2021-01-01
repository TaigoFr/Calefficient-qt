#include "timerbutton.hpp"

const QString TimerButton::timeFormat = "hh:mm:ss";
//const QString TimerButton::timeFormat = "hh:mm:ss,zzz";

TimerButton::TimerButton(const GoogleCalendar::Calendar& a_cal, QWidget* parent) : QPushButton(parent), calendar(a_cal)
{
    static int max_characters = 11;
    name = "";

    QStringList list = calendar.name.split(QRegExp("\\s+"), Qt::KeepEmptyParts);
    for(int s=0; s<list.size(); ++s){
        for(int i=0; i<list[s].size(); i+=max_characters){
            name += list[s].mid(i,max_characters);
            if(i+max_characters<list[s].size())
                name += "\n";
        }
        if(s<list.size()-1)
            name += "\n";
    }

    reset();
    //this->setText(name + "\n\n" + display_timer.toString(timeFormat));


}

void TimerButton::start(){
    time = QDateTime::currentDateTime();
    updateText();
}

void TimerButton::updateText(){
    display_timer = display_timer.fromMSecsSinceEpoch(time.msecsTo(QDateTime::currentDateTime())).toTimeSpec(Qt::TimeSpec::UTC);
    this->setText(name + "\n\n" +display_timer.toString(timeFormat));
}

void TimerButton::reset(){
    display_timer = QDateTime::fromMSecsSinceEpoch(0).toTimeSpec(Qt::TimeSpec::UTC);
    this->setText(name);
    //updateText();
}

const GoogleCalendar::Calendar &TimerButton::getCalendar()
{
    return calendar;
}
