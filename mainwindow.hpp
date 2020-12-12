#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QTimer>

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
    void setMainFlow();
    void setSignInPage();
#if USE_INTERNAL_BROWSER
    void setAuthenticationPage();
#endif
    void resizeEvent(QResizeEvent* event);

private:
    Ui::MainWindow *ui;
    GoogleCalendar google;

    QStackedWidget flowPages;

    QTimer update_timer;
    TimerButton* active_timer_button;

#if USE_INTERNAL_BROWSER
    QWebEngineView webView;
#endif
};
#endif // MAINWINDOW_H
