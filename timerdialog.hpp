#ifndef TIMERDIALOG_H
#define TIMERDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>

#include "googlecalendar.hpp"

//#include <QPainter>

class TimerDialog: public QDialog
{
public:
    TimerDialog(const QVector<GoogleCalendar::Calendar>& a_cals, int cal_index, QWidget * parent = nullptr);

private:
    QLabel *eventName;
    QLineEdit *eventNameEdit;

    QLabel *eventDescription;
    QTextEdit *eventDescriptionEdit;

    QLabel *calendarName;
    QComboBox *calendarComboBox;

    QPushButton * cancelButton, *saveButton;

/*
    void paintEvent(QPaintEvent *)
    {
        QPainter painter(this);
        painter.drawImage(QRectF(0, 0, 500, 500), QImage(":/resources/images/transparent.png"));
    }
*/

};

#endif // TIMERDIALOG_H
