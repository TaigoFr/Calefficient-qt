#include "timereditpage.hpp"

#include <QHBoxLayout>
#include <QDebug>
//#include <QGraphicsDropShadowEffect>
#include <QAbstractItemView>
#include <QPushButton>

TimerEditPage::TimerEditPage(QWidget * parent)
    : QWidget(parent), edit_button(nullptr)
    //, colorPickerDialog(new QColorDialog(this))
{
    // overall structure
    QVBoxLayout * layout = new QVBoxLayout(this);

    QWidget * eventNameWidget = new QWidget(this);
    QWidget * eventDecriptionWidget = new QWidget(this);
    QWidget * calendarComboBoxWidget = new QWidget(this);
    //QWidget * colorPickerWidget = new QWidget(this);
    QWidget * exitButtons = new QWidget(this);

    layout->addWidget(calendarComboBoxWidget);
    layout->addWidget(eventNameWidget);
    layout->addWidget(eventDecriptionWidget);
    //layout->addWidget(colorPickerWidget);
    layout->addWidget(exitButtons);

    setLayout(layout);

    // sub-layouts:

    // calendar combo box
    QVBoxLayout *calendarComboBoxLayout = new QVBoxLayout(calendarComboBoxWidget);

    QLabel *calendarName = new QLabel("Default Event Calendar:", calendarComboBoxWidget);
    calendarComboBox = new QComboBox(calendarComboBoxWidget);

    calendarComboBoxLayout->addWidget(calendarName);
    calendarComboBoxLayout->addWidget(calendarComboBox);

    // name
    QVBoxLayout *eventNameLayout = new QVBoxLayout(eventNameWidget);

    QLabel *eventName = new QLabel("Default Event Name:", eventNameWidget);
    eventNameEdit = new QLineEdit(eventNameWidget);

    eventNameLayout->addWidget(eventName);
    eventNameLayout->addWidget(eventNameEdit);

    // decription
    QVBoxLayout *eventDescriptionLayout = new QVBoxLayout(eventDecriptionWidget);

    QLabel *eventDescription = new QLabel("Default Event Description:", eventDecriptionWidget);
    eventDescriptionEdit = new QTextEdit(eventDecriptionWidget);

    eventDescriptionLayout->addWidget(eventDescription);
    eventDescriptionLayout->addWidget(eventDescriptionEdit);

    /*
    // color picker
    QHBoxLayout *colorPickerLayout = new QHBoxLayout(colorPickerWidget);

    QLabel *colorPicker = new QLabel("Button Color:", colorPickerWidget);
    colorPickerButton = new QPushButton("", colorPickerWidget);


    connect(colorPickerButton, &QPushButton::clicked, [this](){
        if(edit_button!=nullptr){
            QColor color = colorPickerDialog->getColor(edit_button->getData().calendar.color_hex, this);
            if(color.isValid()){
                colorPickerDialog->setCurrentColor(color);
                updateStyle(parentWidget()->size());
            }
        }
    });

    colorPicker->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    colorPickerButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    colorPickerLayout->addWidget(colorPicker);
    colorPickerLayout->addWidget(colorPickerButton, 0, Qt::AlignRight);
    */

    // exitButtons
    QHBoxLayout *exitButtons_layout = new QHBoxLayout(exitButtons);

    QPushButton *cancelButton = new QPushButton("Cancel", exitButtons);
    QPushButton *saveButton = new QPushButton("Save", exitButtons);

    cancelButton->setStyleSheet("background-color: black;");
    saveButton->setStyleSheet("background-color: rgb(0, 144, 0);");

    connect(cancelButton, &QPushButton::clicked, [this](){
        emit done(0);
    });
    connect(saveButton, &QPushButton::clicked, [this](){
        if(edit_button != nullptr)
        {
            TimerButton::Data data;
            data.name = eventNameEdit->text();
            data.description = eventDescriptionEdit->toPlainText();
            data.calendar = calendars[calendarComboBox->currentIndex()];
            //data.color = colorPickerDialog->currentColor();
            edit_button->setData(data);
        }

        emit done(1);
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

void TimerEditPage::setCalendars(const QVector<GoogleCalendar::Calendar> &a_cals)
{
    calendars = a_cals;

    calendarComboBox->clear();
    for (int c = 0; c<a_cals.size(); ++c) {
        calendarComboBox->addItem(a_cals[c].name);
        calendarComboBox->setItemData(c, QBrush(QColor(a_cals[c].color_hex)), Qt::BackgroundRole);
    }

    void (QComboBox:: *itemChangeSignal)(int) = &QComboBox::currentIndexChanged;
    connect(calendarComboBox, itemChangeSignal, [this](int index){
        QString color = calendars[index].color_hex;
        calendarComboBox->setStyleSheet("background-color: " + color + ";");
        //colorPickerDialog->setCurrentColor(QColor(color));
        //updateStyle(parentWidget()->size());
    });
}

void TimerEditPage::setEditButton(TimerButton *button)
{
    edit_button = button;

    const TimerButton::Data& data = edit_button->getData();

    eventNameEdit->setText(data.name);
    eventDescriptionEdit->setText(data.description);
    calendarComboBox->setCurrentText(data.calendar.name);

    if(data.calendar.name == calendars[0].name) // otherwise signal would not be emitted
        emit calendarComboBox->currentIndexChanged(0);

    //colorPickerDialog->setCurrentColor(data.color);
    //updateStyle(parentWidget()->size());
}

void TimerEditPage::updateStyle(const QSize &size)
{
    setStyleSheet("QWidget {"
                  "font: " + QString::number((int)(size.height() * 0.025)) + "px;"
                  "}"
                  "QPushButton {"
                  "color: rgb(255,255,255);"
                  "border-radius: 1em;"
                  "padding: " + QString::number((int)(size.width() * 0.035)) + "px;"
                  "}");
/*
    if(edit_button != nullptr)
        colorPickerButton->setStyleSheet("background-color: " + colorPickerDialog->currentColor().name() + ";"
                                         "border: " + QString::number((int)(size.height() * 0.005)) + "px solid black;"
                                         "height: " + QString::number((int)(size.height() * 0.005)) + "px;");

    //colorPickerDialog->resize(size);
*/
}
