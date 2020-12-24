#ifndef SCROLLABLETABLEWIDGET_H
#define SCROLLABLETABLEWIDGET_H

#include <QTableWidget>
#include <QTimer>
#include <QTime>

class ScrollableTableWidget : public QTableWidget
{
public:
    ScrollableTableWidget(QWidget * parent = nullptr);

    void addWidget(QWidget * widget);
    int widgetCount();
    void clear();

    void resetScrolled();
    bool getScrolled();

private:
    bool viewportEvent(QEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    void scrollEvent(QEvent* event);

private:
    int m_widgetCount;
    bool scrolled; // saved to know if any scrolling happened since last reset

    QTimer drag_timer;
    double drag_velocity_start;
    QTime drag_time_start;
};

#endif // SCROLLABLETABLEWIDGET_H
