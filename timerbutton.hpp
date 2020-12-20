#ifndef TIMERBUTTON_HPP
#define TIMERBUTTON_HPP

#include <QDateTime>
#include <QPushButton>
#include <QDebug>
#include <QTimeZone>

class TimerButton : public QPushButton
{
    static const QString timeFormat;

public:
    TimerButton(QWidget* parent = nullptr) : QPushButton(parent)
    {
        reset();
        this->setText(display_timer.toString(timeFormat));
    }

    void start(){
        time = QDateTime::currentDateTime();
    }

    void update(){
        display_timer = display_timer.fromMSecsSinceEpoch(time.msecsTo(QDateTime::currentDateTime())).toTimeSpec(Qt::TimeSpec::UTC);
        updateText();
    }

    void reset(){
        display_timer = QDateTime::fromMSecsSinceEpoch(0).toTimeSpec(Qt::TimeSpec::UTC);
        updateText();
    }

private:
    void updateText(){
        this->setText(display_timer.toString(timeFormat));
    }

private:
    QDateTime time;
    QDateTime display_timer;
};

#endif // TIMERBUTTON_HPP
