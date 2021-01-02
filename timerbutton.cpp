#include "timerbutton.hpp"

const QString TimerButton::timeFormat = "hh:mm:ss";
//const QString TimerButton::timeFormat = "hh:mm:ss,zzz";

TimerButton::TimerButton(const GoogleCalendar::Calendar& a_cal, QWidget* parent)
    : TimerButton({"","",a_cal /*, QColor(a_cal.color_hex)*/}, parent)
{}

TimerButton::TimerButton(const Data& a_data, QWidget* parent)
    : QPushButton(parent), data(a_data)
{
    reset();
    //this->setText(name + "\n\n" + display_timer.toString(timeFormat));
}

void TimerButton::start(){
    time = QDateTime::currentDateTime();
    updateText();
}

void TimerButton::updateText(){
    display_timer = display_timer.fromMSecsSinceEpoch(time.msecsTo(QDateTime::currentDateTime())).toTimeSpec(Qt::TimeSpec::UTC);
    this->setText(name_formatted + "\n\n" +display_timer.toString(timeFormat));
}

void TimerButton::reset(){
    setName(data.name == "" ? data.calendar.name : data.name);
    display_timer = QDateTime::fromMSecsSinceEpoch(0).toTimeSpec(Qt::TimeSpec::UTC);
    this->setText(name_formatted);
    //updateText();
}

void TimerButton::setName(QString name)
{
    static int max_characters_per_line = 11;
    static int max_length = max_characters_per_line * 5;
    name_formatted = "";

    if(name.size() >= max_length)
        name = name.left(max_length-3) + "...";

    QStringList list = name.split(QRegExp("\\s+"), Qt::KeepEmptyParts);
    for(int s=0; s<list.size(); ++s){
        for(int i=0; i<list[s].size(); i+=max_characters_per_line){
            name_formatted += list[s].mid(i,max_characters_per_line);
            if(i+max_characters_per_line<list[s].size())
                name_formatted += "\n";
        }
        if(s<list.size()-1)
            name_formatted += "\n";
    }
}

const TimerButton::Data &TimerButton::getData()
{
    return data;
}

void TimerButton::setData(const TimerButton::Data & a_data)
{
    data = a_data;
    reset();
}
