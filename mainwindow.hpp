#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QTabWidget>

#include "googlecalendar.hpp"
#include "timertable.hpp"
#include "timereditpage.hpp"

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
        SIGNIN_PAGE
        , MAIN_TABS
        , TIMER_EDIT_PAGE
#if USE_INTERNAL_BROWSER
        , WEB
#endif
    };

    enum{
        TIMERS_PAGE,
        CHARTS_PAGE,
        SETTINGS_PAGE
    };

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void onResize();

private:
    QWidget* makeMainTabs(QWidget* parent);
    QWidget* makeTimersPage(QWidget* parent);
    QWidget* makeSignInPage(QWidget* parent);
    QWidget* makeChartsPage(QWidget* parent);
    QWidget* makeSettingsPage(QWidget * parent);
    QWidget* makeTimerEditPage(QWidget* parent);
#if USE_INTERNAL_BROWSER
    void setAuthenticationPage();
#endif

    void resizeEvent(QResizeEvent* event);

    void enterEvent(QEvent * event);
    bool event(QEvent* event);

private:
    Ui::MainWindow *ui;

    QStackedWidget *flowPages;
    QTabWidget *mainTabs;

    TimerTable *timersTable;
    TimerEditPage *timerEditPage;

#if USE_INTERNAL_BROWSER
    QWebEngineView webView;
#endif
};
#endif // MAINWINDOW_H
