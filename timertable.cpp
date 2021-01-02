#include "timertable.hpp"
#include "timereditpage.hpp"

#include <QScrollBar>
#include <QToolButton>

TimerTable::TimerTable(QWidget * parent) : ScrollableTableWidget(parent), active_timer_button(nullptr)
{
    connect(&update_timer, &QTimer::timeout, [=](){
        if(active_timer_button != nullptr)
            active_timer_button->updateText();
        else
            update_timer.stop();
    });

    //setDragDropMode(QAbstractItemView::InternalMove);
}

void TimerTable::setButtons(const QVector<GoogleCalendar::Calendar> &a_cals)
{
    clear();
    setColumnCount(a_cals.size() <= 3 ? 1 : 2);

    int repeat = 1;

    //QVector<TimerButton*> buttons(a_cals.size());
    for (int r=0; r<repeat ; ++r) {
        for(int i = 0; i < a_cals.size(); ++i){
            addButton(a_cals, i);
            //timersTable.addWidget(new QLabel(QString::number(i)));
        }
    }

    QToolButton * plus = new QToolButton(this);
    plus->setIcon(QIcon(":/resources/images/plus.png"));
    //QPushButton *plus = new QPushButton("+", this);
    setupButton(plus);
}

void TimerTable::addButton(const QVector<GoogleCalendar::Calendar>& a_cals, int cal_index)
{
    TimerButton *button = new TimerButton(a_cals[cal_index], this);
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
}

void TimerTable::updateStyle(const QSize& window_size)
{
    if(window_size.width() == 0)
        return;

    double spacing_perc = 0.03;
    int spacing = window_size.width() * spacing_perc;
    int button_width = window_size.width() * 0.5;
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
            button->setIconSize(QSize(button_width * 0.4, button_width * 0.4));
        }
        else{
            TimerButton* button = static_cast<TimerButton*>(widget);
            //background_color = button->getData().color;
            background_color.setNamedColor(button->getData().calendar.color_hex);
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
        widget->setFixedSize(QSize(button_width, button_width));
    }

    //resizeColumnsToContents();
    resizeRowsToContents();
}


void TimerTable::clear()
{
    active_timer_button = nullptr;
    ScrollableTableWidget::clear();
}

void TimerTable::setupButton(QAbstractButton *button)
{
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    addWidget(button);

    button->installEventFilter(this);

    connect(button, &QAbstractButton::pressed, [this](){
        resetScrolled();
    });
}
