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

    void updateStyle();
    TimerButton::Data getData() const;
    void setData(const TimerButton::Data&);

signals:
    void done(int success);

private:
    void setCalendars();

private:
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
