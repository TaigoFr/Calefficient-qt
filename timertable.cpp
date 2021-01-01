#include "timertable.hpp"
#include "timerdialog.hpp"

#include <QScrollBar>

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

    //QToolButton * plus = new QToolButton(&timersTable);
    //plus->setIcon(QIcon(QPixmap(":/resources/images/plus.png")));
    //timersTable.addWidget(plus);
}

void TimerTable::addButton(const QVector<GoogleCalendar::Calendar>& a_cals, int cal_index)
{
    TimerButton *button = new TimerButton(a_cals[cal_index], this);
    //buttons[i] = button;
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

        TimerDialog *dialog = new TimerDialog(a_cals, cal_index, this);
        int success = dialog->exec();

        if(success == QDialog::Accepted){
            // read dialog
        }
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
        TimerButton* button = static_cast<TimerButton*>(cellWidget(r,c));
        QColor color(button->getCalendar().color_hex);
        button->setStyleSheet("QPushButton {"
                              "background-color: " + color.name() + ";"
                              //"background-color: rgb(" + QString::number(128+100*std::sin(300*i)) + "," + QString::number(128+100*std::sin(200*i)) + "," + QString::number(128+100*std::sin(400*i)) + ");"
                              "color: rgb(255,255,255);"
                              "font: bold " + QString::number(window_size.height() * 0.025) + "px;"
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
                              "}"
                                                                          );
        button->setFixedSize(QSize(button_width, button_width));
    }

    //resizeColumnsToContents();
    resizeRowsToContents();
}


void TimerTable::clear()
{
    active_timer_button = nullptr;
    ScrollableTableWidget::clear();
}
