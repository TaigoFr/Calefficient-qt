#ifndef TIMERTABLE_H
#define TIMERTABLE_H

#include "scrollabletablewidget.h"
#include "timerbutton.hpp"

#include <QPushButton>
#include <QTimer>

class TimerTable : public ScrollableTableWidget
{
public:
    TimerTable(const QSize& size = QSize(0, 0), QWidget * parent = nullptr);

    void addButton(TimerButton * button);

    void updateStyle();

    void clear();

private:
    QTimer update_timer;
    TimerButton* active_timer_button;
};

#endif // TIMERTABLE_H
