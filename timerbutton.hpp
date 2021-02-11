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
        QDateTime start_time;

        Data(const GoogleCalendar::Calendar * cal = nullptr);

        // functions to allow writing to QSettings as a CustomType
        friend QDataStream &operator<<(QDataStream &out, const Data &d);
        friend QDataStream &operator>>(QDataStream &in, Data &d);
    };

    TimerButton(const GoogleCalendar::Calendar* a_cal = nullptr, QWidget* parent = nullptr);
    TimerButton(const Data& a_data, QWidget* parent = nullptr);

    void start();
    void updateText();
    void reset();
    void setName(QString name);

    const Data &getData();
    void setData(const Data&);

    QDateTime getStart();
    float getElapsedTime();

signals:
    void longPressed();

private:
    bool event(QEvent *event) override;

private:
    Data data;
    QString name_formatted;

    static const QString timeFormat;
};

Q_DECLARE_METATYPE(TimerButton::Data)

#endif // TIMERBUTTON_HPP
