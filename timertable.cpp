#include "timertable.hpp"
#include "timereditpage.hpp"

#include <QScrollBar>

TimerTable::TimerTable(QWidget * parent) : ScrollableTableWidget(parent), active_timer_button(nullptr), button_on_edit(nullptr)
{
    connect(&update_timer, &QTimer::timeout, [=](){
        if(active_timer_button != nullptr)
            active_timer_button->updateText();
        else
            update_timer.stop();
    });

    //setDragDropMode(QAbstractItemView::InternalMove);

    connect(&GoogleCalendar::getInstance(), &GoogleCalendar::signedIn, [this](){
        qDebug() << "WRONG";
        setButtons();
    });

    if(GoogleCalendar::getInstance().isSignedIn())
        setButtons();
}

void TimerTable::setButtons()
{
    qDebug() << "HERE1";
    QVector<GoogleCalendar::Calendar> &calendars = GoogleCalendar::getInstance().getOwnedCalendarList();

    qDebug() << "HERE2";
    clear();
    setColumnCount(calendars.size() <= 3 ? 1 : 2);

    int repeat = 1;

    //QVector<TimerButton*> buttons(a_cals.size());
    for (int r=0; r<repeat ; ++r)
        for(auto &calendar : calendars)
            addButton(&calendar);

    qDebug() << "HERE3";
    setupButton(makePlusButton());
    qDebug() << "HERE4";
    updateStyle();
    qDebug() << "HERE5";
}

TimerButton* TimerTable::addButton(const GoogleCalendar::Calendar* a_cal)
{
    TimerButton *button = new TimerButton(a_cal, this);
    setupButton(button);

    connect(button, &QPushButton::clicked, [=](){
        if(getScrolled())
            return;

        if(active_timer_button != nullptr)
            active_timer_button->reset();

        if(active_timer_button == button){
            button->reset();
            active_timer_button = nullptr;
        } else {
            button->start();
            active_timer_button = button;
            update_timer.start(1000); // every 1000ms
        }

        emit buttonClicked(button);
    });
    connect(button, &TimerButton::longPressed, [button, this](){
        button_on_edit = button;
        emit buttonLongPressed(button);
    });

    return button;
}

QToolButton *TimerTable::makePlusButton()
{
    QToolButton * plus = new QToolButton(this);
    plus->setIcon(QIcon(":/resources/images/plus.png"));

    connect(plus, &QToolButton::clicked, [=](){
        if(getScrolled())
            return;

        button_on_edit = nullptr;
        emit plusButtonClicked(); // to call TimerEditPage
    });

    return plus;
}

void TimerTable::updateStyle()
{
    QSize window_size = window()->size();

    double spacing_perc = 0.03;
    int spacing = window_size.width() * spacing_perc;
    int button_height = window_size.width() * 0.5;
    int button_width = button_height * (columnCount() == 1 ? 2. : 1.);
    //int iconSize = widthPercentage(0.05);

    //setStyleSheet("QTableWidget::item { padding: " + QString::number(spacing) + "px }");

    for(int w=0; w<widgetCount(); ++w){
        int r = w / columnCount();
        int c = w % columnCount();

        QWidget* widget = cellWidget(r,c);
        QColor background_color;
        if(w==widgetCount()-1){ // plus button
            background_color.setRgb(230, 230, 230);
            QToolButton* button = static_cast<QToolButton*>(widget);
            button->setIconSize(QSize(button_height * 0.4, button_height * 0.4));
        }
        else{
            TimerButton* button = static_cast<TimerButton*>(widget);
            //background_color = button->getData().color;
            const GoogleCalendar::Calendar* calendar = button->getData().calendar;
            if(calendar != nullptr)
                background_color.setNamedColor(calendar->color_hex);
        }
        widget->setStyleSheet("background-color: " + background_color.name() + ";"
                              //"background-color: rgb(" + QString::number(128+100*std::sin(300*i)) + "," + QString::number(128+100*std::sin(200*i)) + "," + QString::number(128+100*std::sin(400*i)) + ");"
                              "color: white;"
                              "font: bold " + QString::number((int)(window_size.height() * 0.025)) + "px;"
                              //"padding: 5em;"
                              "border-radius: 1em;"
                              //"width: " + QString::number(button_width) + "px;"
                              //"height: " + QString::number(button_width) + "px;"
                              "margin-left: " + QString::number(c==0 ? spacing : spacing/2) + "px;"
                              "margin-right: " + QString::number(c==columnCount()-1 ? spacing : spacing/2) + "px;"
                              "margin-top: " + QString::number(r==0 ? spacing : spacing/2) + "px;"
                              "margin-bottom: " + QString::number(r==rowCount()-1 ? spacing : spacing/2) + "px;"
                              //"background-repeat: no-repeat;"
                              //"background-origin: content;"
                              //"background-image: url(\":/resources/images/play_icon.png\");"
                              );
        widget->setFixedSize(QSize(button_width, button_height));
    }

    //resizeColumnsToContents();
    resizeRowsToContents();
}


void TimerTable::clear()
{
    active_timer_button = nullptr;
    ScrollableTableWidget::clear();
}

void TimerTable::saveButtonOnEdit(const TimerButton::Data &data)
{
    if(button_on_edit == nullptr){
        int plus_index = widgetCount()-1;
        int r = plus_index / columnCount();
        int c = plus_index % columnCount();
        QWidget* plus = cellWidget(r, c);
        removeItem(r, c);

        button_on_edit = addButton(data.calendar);

        // decided to make new instead of re-using,
        // because QTableWidget doesn't delete it when removing from grid,
        // but takes ownership of it, so re-using it was giving errors
        setupButton(makePlusButton());
        delete plus; // delete old, that is still tied to QTableWidget
    }
    else{
        if(button_on_edit->getData().calendar->id != data.calendar->id)
            button_on_edit->reset();
    }

    button_on_edit->setData(data);
}

void TimerTable::setupButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    addWidget(button);
    emit buttonCreated();

    //button->installEventFilter(this);

    connect(button, &QAbstractButton::pressed, [this](){
        resetScrolled();
    });
}
