#include "scrollabletablewidget.h"

#include <QDebug>

#include <QHeaderView>
#include <QScrollBar>
#include <QEvent>
#include <QMouseEvent>
#include <QTouchEvent>

#include <cmath>

#define DRAG_DT 10. // ms
#define MIN_VELOCITY 0.1 // pixel.ms^-1


ScrollableTableWidget::ScrollableTableWidget(QWidget * parent): m_widgetCount(0), drag_velocity_start(0.), drag_time_start(QTime::currentTime())
{
    resetScrolled();

    setParent(parent);
    setFrameShape(QFrame::NoFrame);

    //setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // hide table headers
    horizontalHeader()->hide();
    verticalHeader()->hide();

    // disable selection of cells
    setSelectionMode(QAbstractItemView::NoSelection);

    // hide scrollBars
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // hide grid
    setShowGrid(false);

    //setAutoScroll(false);

    connect(&drag_timer, &QTimer::timeout, [=](){
        double velocity = drag_velocity_start *
                (1. - std::min(1., std::pow(drag_time_start.msecsTo(QTime::currentTime())/(1000.) , 2.)));

        if(abs(velocity) < MIN_VELOCITY){
            drag_timer.stop();
        }
        else{
            double scroll_value = DRAG_DT * velocity;
            verticalScrollBar()->setValue(verticalScrollBar()->value() - scroll_value);
            qDebug() << "NEW VEL/VAL: " << velocity << "/" << scroll_value;
        }
    });
}

void ScrollableTableWidget::addWidget(QWidget *widget)
{
    if(columnCount() <= 0)
        setColumnCount(1);

    if(widgetCount() == columnCount() * rowCount())
        setRowCount(rowCount()+1);

    setCellWidget(rowCount()-1, widgetCount() % columnCount(), widget);
    ++m_widgetCount;
}

int ScrollableTableWidget::widgetCount()
{
    return m_widgetCount;
}

void ScrollableTableWidget::clear()
{
    for (int w=0; w<widgetCount(); ++w){
        int r = w / columnCount();
        int c = w % columnCount();
        auto *widget = cellWidget(r, c);
        removeCellWidget(r,c);
        delete widget;
    }

    this->QTableWidget::clear();
    setColumnCount(0);
    setRowCount(0);
    m_widgetCount = 0;
}

void ScrollableTableWidget::resetScrolled()
{
    scrolled = false;
}

bool ScrollableTableWidget::getScrolled()
{
    return scrolled;
}



bool ScrollableTableWidget::viewportEvent(QEvent *event)
{
    //static int i = 1;
    if(event->type() != QEvent::Paint && event->type() != QEvent::HoverMove && event->type() != QEvent::Move){
        //qDebug() << "EVENT " << i++;
        //qDebug() << event->type();
    }

    scrollEvent(event);

    return QTableWidget::viewportEvent(event);
}

bool ScrollableTableWidget::eventFilter(QObject *obj, QEvent *event)
{
    //static int i = 1;
    if(event->type() != QEvent::Paint && event->type() != QEvent::HoverMove && event->type() != QEvent::Move){
        //qDebug() << "eventFilter " << i++;
        //qDebug() << event->type();
    }

    scrollEvent(event);

    return QTableWidget::eventFilter(obj, event);
}

void ScrollableTableWidget::scrollEvent(QEvent *event)
{
    static int oldBarPos;

    // velocity statistics
    static const int saveCount = 3;
    static int savedPositions[saveCount];
    static QTime savedTimes[saveCount];
    static int posIndex = 0;

    if(event->type() == QEvent::MouseButtonPress){
        oldBarPos = verticalScrollBar()->value();
        posIndex = 0;
        savedPositions[posIndex % saveCount] = static_cast<QMouseEvent*>(event)->globalY();
        savedTimes[posIndex % saveCount] = QTime::currentTime();
        ++posIndex;
    }
    else if(event->type() == QEvent::TouchBegin){
        oldBarPos = verticalScrollBar()->value();
        savedPositions[posIndex % saveCount] = static_cast<QTouchEvent*>(event)->touchPoints().at(0).pos().y();
        savedTimes[posIndex % saveCount] = QTime::currentTime();
        ++posIndex;
    }
    else if(event->type() == QEvent::MouseMove){
        scrolled = true;

        int newPos = static_cast<QMouseEvent*>(event)->globalY();
        int newBarPos = oldBarPos - (newPos - savedPositions[(posIndex-1) % saveCount]);
        //qDebug() << "MOVING to" << newBarPos << " (OLD POS = " << oldPos << "| NEW POS = " << newPos << "| oldBarPos = " << oldBarPos << ")";
        verticalScrollBar()->setValue(newBarPos);

        oldBarPos = newBarPos;
        savedPositions[posIndex % saveCount] = newPos;
        savedTimes[posIndex % saveCount] = QTime::currentTime();
        ++posIndex;
    }
    else if(event->type() == QEvent::MouseButtonRelease){
        qDebug() << "Num points = " << posIndex;
        if(posIndex >= saveCount){
            drag_time_start = savedTimes[(posIndex-1) % saveCount];
            drag_velocity_start = ((double)(savedPositions[(posIndex-1) % saveCount] - savedPositions[posIndex % saveCount])) /
                    savedTimes[posIndex % saveCount].msecsTo(drag_time_start);
            qDebug() << "VELOCITY";
            qDebug() << drag_velocity_start << " (" << savedPositions[(posIndex-1) % saveCount] << " | " << savedPositions[posIndex % saveCount] << ")("
                     << drag_time_start << " | " << savedTimes[posIndex % saveCount] << ")";
            drag_timer.start(DRAG_DT);
        }
    }
}
