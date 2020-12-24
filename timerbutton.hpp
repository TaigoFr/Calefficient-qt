#ifndef TIMERBUTTON_HPP
#define TIMERBUTTON_HPP

#include <QDateTime>
#include <QPushButton>
#include <QDebug>
#include <QTimeZone>

#include "googlecalendar.hpp"

class TimerButton : public QPushButton
{
    static const QString timeFormat;

public:
    TimerButton(const GoogleCalendar::Calendar& a_cal, QWidget* parent = nullptr);

    void start();
    void updateText();
    void reset();

    const GoogleCalendar::Calendar &getCalendar();

private:
    GoogleCalendar::Calendar calendar;
    QString name;
    QDateTime time;
    QDateTime display_timer;
};

#endif // TIMERBUTTON_HPP
