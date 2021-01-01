#ifndef TIMERTABLE_H
#define TIMERTABLE_H

#include "scrollabletablewidget.hpp"
#include "timerbutton.hpp"

#include <QPushButton>
#include <QTimer>

class TimerTable : public ScrollableTableWidget
{
public:
    TimerTable(QWidget * parent = nullptr);

    void setButtons(const QVector<GoogleCalendar::Calendar>& a_cals);

    void updateStyle(const QSize& window_size);

    void clear();

private:
    void addButton(const QVector<GoogleCalendar::Calendar>& a_cals, int cal_index);

private:
    QTimer update_timer;
    TimerButton* active_timer_button;
};

#endif // TIMERTABLE_H
