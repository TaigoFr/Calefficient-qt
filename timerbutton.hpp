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
    struct Data{
        QString name;
        QString description;
        GoogleCalendar::Calendar calendar;
        //QColor color;
    };

    TimerButton(const GoogleCalendar::Calendar& a_cal, QWidget* parent = nullptr);
    TimerButton(const Data& a_data, QWidget* parent = nullptr);

    void start();
    void updateText();
    void reset();
    void setName(QString name);

    const Data &getData();
    void setData(const Data&);

private:
    Data data;
    QString name_formatted;
    QDateTime time;
    QDateTime display_timer;
};

#endif // TIMERBUTTON_HPP
