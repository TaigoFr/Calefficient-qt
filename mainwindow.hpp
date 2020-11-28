#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>

#include "googlecalendar.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void setMainFlow();
    void setSignInPage();
    void setAuthenticationPage();

private:
    Ui::MainWindow *ui;
    GoogleCalendar google;

    QStackedWidget flowPages;
};
#endif // MAINWINDOW_H
