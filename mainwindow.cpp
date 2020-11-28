#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

#include <QWebEngineView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , google(":/private/Calefficient_client_secret.json")
{
    ui->setupUi(this);

    QVBoxLayout *l = new QVBoxLayout();
    ui->centralwidget->setLayout(l);
    l->addWidget(&flowPages);
    //ui->centralwidget->layout()->addWidget(view);

    setMainFlow();
    setSignInPage();
    setAuthenticationPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::setMainFlow()
{
    QWidget* widget = new QWidget(&flowPages);
    QVBoxLayout*l = new QVBoxLayout();
    widget->setLayout(l);

    QPushButton *button = new QPushButton("Get Calendar List", widget);
    connect(button, &QPushButton::pressed, [this](){
        qDebug() << google.getCalendarList();
        flowPages.setCurrentIndex(1);
    });
    l->addWidget(button);

    flowPages.addWidget(widget);
}


void MainWindow::setSignInPage()
{
    QWidget* widget = new QWidget(&flowPages);
    QVBoxLayout*l = new QVBoxLayout();
    widget->setLayout(l);

    QPushButton *button = new QPushButton("Get Calendar List 2", widget);
    connect(button, &QPushButton::pressed, [this](){
        //qDebug() << google.getCalendarList();
        flowPages.setCurrentIndex(2);
    });
    l->addWidget(button);

    flowPages.addWidget(widget);

    /*
    QWebEngineView *view = new QWebEngineView(this);
    view->load(QUrl("http://qt-project.org/"));
    //view->show();
    */
}

void MainWindow::setAuthenticationPage()
{
    QWidget* widget = new QWidget(&flowPages);
    QVBoxLayout*l = new QVBoxLayout();
    widget->setLayout(l);

    QWebEngineView *view = new QWebEngineView(this);
    view->load(QUrl("http://qt-project.org/"));

    l->addWidget(view);

    flowPages.addWidget(widget);
}

