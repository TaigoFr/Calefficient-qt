#ifndef TIMERBUTTON_HPP
#define TIMERBUTTON_HPP

#include <QDateTime>
#include <QPushButton>
#include <QDebug>
#include <QTimeZone>

#include "googlecalendar.hpp"

class TimerButton : public QPushButton
{
    Q_OBJECT

public:
    struct Data{
        QString name;
        QString description;
        const GoogleCalendar::Calendar *calendar;
        //QColor color;
        Data(const GoogleCalendar::Calendar * cal = nullptr);
    };

    TimerButton(const GoogleCalendar::Calendar* a_cal = nullptr, QWidget* parent = nullptr);
    TimerButton(const Data& a_data, QWidget* parent = nullptr);

    void start();
    void updateText();
    void reset();
    void setName(QString name);

    const Data &getData();
    void setData(const Data&);

signals:
    void longPressed();

private:
    bool event(QEvent *event) override;

private:
    Data data;
    QString name_formatted;
    QDateTime time;
    QDateTime display_timer;

    static const QString timeFormat;
};

#endif // TIMERBUTTON_HPP
