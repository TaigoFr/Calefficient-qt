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
    TimerButton(const QString& a_name, QWidget* parent = nullptr);

    void start();
    void updateText();
    void reset();

private:
    QString name;
    QDateTime time;
    QDateTime display_timer;
};

#endif // TIMERBUTTON_HPP
