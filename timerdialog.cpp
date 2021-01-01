#include "timerdialog.hpp"

#include <QHBoxLayout>
#include <QDebug>
#include <QGraphicsDropShadowEffect>
#include <QAbstractItemView>

TimerDialog::TimerDialog(const QVector<GoogleCalendar::Calendar>& a_cals, int cal_index, QWidget * parent) : QDialog(parent)
{
    // overall structure
    QVBoxLayout * layout = new QVBoxLayout(this);

    QWidget * eventNameWidget = new QWidget(this);
    QWidget * eventDecriptionWidget = new QWidget(this);
    QWidget * calendarComboBoxWidget = new QWidget(this);
    QWidget * exitButtons = new QWidget(this);

    layout->addWidget(eventNameWidget);
    layout->addWidget(eventDecriptionWidget);
    layout->addWidget(calendarComboBoxWidget);
    layout->addWidget(exitButtons);

    setLayout(layout);
    setFixedWidth(parentWidget()->width() * 0.8);

    // sub-layouts:

    // name
    QVBoxLayout *eventNameLayout = new QVBoxLayout(eventNameWidget);

    eventName = new QLabel("Default Event Name:", eventNameWidget);
    eventNameEdit = new QLineEdit(eventNameWidget);

    eventNameLayout->addWidget(eventName);
    eventNameLayout->addWidget(eventNameEdit);

    // decription
    QVBoxLayout *eventDescriptionLayout = new QVBoxLayout(eventDecriptionWidget);

    eventDescription = new QLabel("Default Event Description:", eventDecriptionWidget);
    eventDescriptionEdit = new QTextEdit(eventDecriptionWidget);

    eventDescriptionLayout->addWidget(eventDescription);
    eventDescriptionLayout->addWidget(eventDescriptionEdit);

    // calendar combo box
    QVBoxLayout *calendarComboBoxLayout = new QVBoxLayout(calendarComboBoxWidget);

    calendarName = new QLabel("Default Event Calendar:", calendarComboBoxWidget);
    calendarComboBox = new QComboBox(calendarComboBoxWidget);

    for (int c = 0; c<a_cals.size(); ++c) {
        calendarComboBox->addItem(a_cals[c].name);
    }
    calendarComboBox->setCurrentIndex(cal_index);

    calendarComboBoxLayout->addWidget(calendarName);
    calendarComboBoxLayout->addWidget(calendarComboBox);

    // exitButtons
    QHBoxLayout *exitButtons_layout = new QHBoxLayout(exitButtons);

    cancelButton = new QPushButton("Cancel", exitButtons);
    saveButton = new QPushButton("Save", exitButtons);

    setStyleSheet("QPushButton {"
                  "color: rgb(255,255,255);"
                  "border-radius: 1em;"
                  "padding: " + QString::number(parentWidget()->width() * 0.025) + "px;"
                  "}"
                  "font: bold " + QString::number(parentWidget()->height() * 0.025) + "px;");

    cancelButton->setStyleSheet("background-color: black;");
    saveButton->setStyleSheet("background-color: rgb(0, 144, 0);");


    connect(cancelButton, &QPushButton::clicked, [this](){
        this->done(0);
    });
    connect(saveButton, &QPushButton::clicked, [this](){
        this->done(1);
    });

    exitButtons_layout->addWidget(cancelButton);
    exitButtons_layout->addWidget(saveButton);


/*
    setStyleSheet("background-color: white;"
                  "border-radius: 30px;"
                  "border-color: black; border-width: 2px; border-style: outset;"
                  "padding: 5px;");
*/
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    //setAttribute(Qt::WA_TranslucentBackground);
}
