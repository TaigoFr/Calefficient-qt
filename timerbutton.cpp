#include "timerbutton.hpp"

#include <QEvent>
#include <QGestureEvent>
#include <QGesture>

const QString TimerButton::timeFormat = "hh:mm:ss";
//const QString TimerButton::timeFormat = "hh:mm:ss,zzz";

TimerButton::TimerButton(const GoogleCalendar::Calendar* a_cal, QWidget* parent)
    : TimerButton(Data(a_cal), parent)
{}

TimerButton::TimerButton(const Data& a_data, QWidget* parent)
    : QPushButton(parent), data(a_data)
{
    reset();
    //this->setText(name + "\n\n" + display_timer.toString(timeFormat));

    grabGesture(Qt::TapAndHoldGesture);
}

void TimerButton::start(){
    data.start_time = QDateTime::currentDateTime();
    updateText();
}

void TimerButton::updateText(){
    QDateTime display_timer = QDateTime::fromMSecsSinceEpoch(getElapsedTime()).toTimeSpec(Qt::TimeSpec::UTC);
    this->setText(name_formatted + "\n\n" + display_timer.toString(timeFormat));
}

void TimerButton::reset(){
    setName((data.name == "" && data.calendar != nullptr) ? data.calendar->name : data.name);
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
            name_formatted += list[s].midRef(i,max_characters_per_line);
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

QDateTime TimerButton::getStart()
{
    return data.start_time;
}

float TimerButton::getElapsedTime()
{
    return data.start_time.msecsTo(QDateTime::currentDateTime());
}

bool TimerButton::event(QEvent *event)
{
    if (event->type() == QEvent::Gesture){
        QGestureEvent * gestureEvent = static_cast<QGestureEvent*>(event);
        QGesture *gesture = gestureEvent->gesture(Qt::TapAndHoldGesture);
        if (gesture){
            QTapAndHoldGesture* tapAndHold = static_cast<QTapAndHoldGesture *>(gesture);
            if(tapAndHold->state() == Qt::GestureFinished)
                emit longPressed();
        }
    }

    return QPushButton::event(event);
}

TimerButton::Data::Data(const GoogleCalendar::Calendar *cal)
{
    calendar = cal;
}

// functions to allow writing to QSettings as a CustomType
QDataStream& operator<<(QDataStream& out, const TimerButton::Data& d) {
    out << d.name << d.description << d.calendar->id << d.start_time;
    return out;
}

QDataStream& operator>>(QDataStream& in, TimerButton::Data& d) {
    QString cal_id;
    in >> d.name >> d.description >> cal_id >> d.start_time;
    d.calendar = GoogleCalendar::getInstance().getCalendarById(cal_id);
    return in;
}
