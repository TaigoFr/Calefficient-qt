#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QTimer>
#include <QTabWidget>

#include "googlecalendar.hpp"
#include "timerbutton.hpp"

#define USE_INTERNAL_BROWSER false

#if USE_INTERNAL_BROWSER
#include <QWebEngineView>
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum{
        MAIN
        , SIGNIN
        , TIMERS
#if USE_INTERNAL_BROWSER
        , WEB
#endif
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void onResize();

private:
    QWidget* makeMainFlow(QWidget* parent);
    QWidget* makeTimersPage(QWidget* parent);
    QWidget* makeSignInPage(QWidget* parent);
#if USE_INTERNAL_BROWSER
    void setAuthenticationPage();
#endif
    void resizeEvent(QResizeEvent* event);

private:
    Ui::MainWindow *ui;
    GoogleCalendar google;

    QStackedWidget flowPages;
    QTabWidget mainTabs;

    QTimer update_timer;
    TimerButton* active_timer_button;

#if USE_INTERNAL_BROWSER
    QWebEngineView webView;
#endif
};
#endif // MAINWINDOW_H
