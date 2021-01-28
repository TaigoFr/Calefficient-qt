#ifndef TIMERTABLE_H
#define TIMERTABLE_H

#include "scrollabletablewidget.hpp"
#include "timerbutton.hpp"

#include <QPushButton>
#include <QTimer>
#include <QToolButton>

class TimerTable : public ScrollableTableWidget
{
    Q_OBJECT

public:
    TimerTable(QWidget * parent = nullptr);

    void updateStyle();
    void clear();

    void saveButtonOnEdit(const TimerButton::Data&);

signals:
    void buttonClicked(TimerButton*);
    void buttonLongPressed(TimerButton*);
    void buttonCreated();
    void plusButtonClicked();

private:
    void setButtons();
    void setupButton(QAbstractButton *button);
    TimerButton* addButton(const GoogleCalendar::Calendar*);
    QToolButton* makePlusButton();

private:
    QTimer update_timer;
    TimerButton* active_timer_button;
    TimerButton* button_on_edit;
};

#endif // TIMERTABLE_H
