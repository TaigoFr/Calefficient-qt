#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "googlecalendar.hpp"

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

private:
    void setMainFlow();
    void setSignInPage();
#if USE_INTERNAL_BROWSER
    void setAuthenticationPage();
#endif

private:
    Ui::MainWindow *ui;
    GoogleCalendar google;

    QStackedWidget flowPages;

#if USE_INTERNAL_BROWSER
    QWebEngineView webView;
#endif
};
#endif // MAINWINDOW_H
