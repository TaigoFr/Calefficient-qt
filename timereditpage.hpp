#ifndef TimerEditPage_H
#define TimerEditPage_H

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
//#include <QColorDialog>

#include "googlecalendar.hpp"
#include "timerbutton.hpp"

//#include <QPainter>

class TimerEditPage: public QWidget
{
    Q_OBJECT

public:
    TimerEditPage(QWidget * parent = nullptr);

    void setCalendars(const QVector<GoogleCalendar::Calendar>& a_cals);
    void setEditButton(TimerButton *button);

    void updateStyle(const QSize& size);

signals:
    void done(int success);

private:
    QVector<GoogleCalendar::Calendar> calendars;
    TimerButton* edit_button;

    QLineEdit *eventNameEdit;
    QTextEdit *eventDescriptionEdit;
    QComboBox *calendarComboBox;
    //QPushButton *colorPickerButton;
    //QColorDialog *colorPickerDialog;

/*
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.drawImage(QRectF(0, 0, 500, 500), QImage(":/resources/images/transparent.png"));
    }
*/

};

#endif // TimerEditPage_H
