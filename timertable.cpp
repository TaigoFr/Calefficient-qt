#include "timertable.h"

#include <QScrollBar>

TimerTable::TimerTable(QWidget * parent) : ScrollableTableWidget(parent), active_timer_button(nullptr)
{
    connect(&update_timer, &QTimer::timeout, [=](){
        if(active_timer_button != nullptr)
            active_timer_button->updateText();
        else
            update_timer.stop();
    });
}

void TimerTable::addButton(TimerButton *button)
{
    addWidget(button);

    button->installEventFilter(this);

    connect(button, &QPushButton::pressed, [this](){
        resetScrolled();
    });

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
    });
}

void TimerTable::updateStyle(int width, int height)
{

    qDebug() << "UPDATINGGGGGGGGGGGGGG";

    int spacing = width * 0.02;
    int button_width = width * 0.44;
    //int iconSize = widthPercentage(0.05);

    setStyleSheet("QTableWidget::item { padding: " + QString::number(spacing) + "px }");

    for(int w=0; w<widgetCount(); ++w){
        int r = w / columnCount();
        int c = w % columnCount();
        TimerButton* button = static_cast<TimerButton*>(cellWidget(r,c));
        QColor color(button->getCalendar().color_hex);
        button->setStyleSheet(
                                "background-color: " + color.name() + ";"
                                //"background-color: rgb(" + QString::number(128+100*std::sin(300*i)) + "," + QString::number(128+100*std::sin(200*i)) + "," + QString::number(128+100*std::sin(400*i)) + ");"
                                  "color: rgb(255,255,255);"
                                  "font: bold " + QString::number(height * 0.025) + "px;"
                                  //"padding: 0.5em;"
                                  "border-radius: 1em;"
                                  "min-width: " + QString::number(button_width) + "px;"
                                  "min-height: " + QString::number(button_width) + "px;"
                                  "margin: " + QString::number(spacing) + "px;"
                                //"background-position: center bottom;"
                                //"background-repeat: no-repeat;"
                                //"background-origin: content;"
                                //"background-image: url(\":/resources/images/play_icon.png\");"
                                                                          );
    }

    resizeColumnsToContents();
    resizeRowsToContents();
}


void TimerTable::clear()
{
    active_timer_button = nullptr;
    ScrollableTableWidget::clear();
}
