#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "autoarrangedgridlayout.hpp"

#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDebug>

#if USE_INTERNAL_BROWSER
#include <QWebEngineView>
#include <QWebEngineProfile>
#else
#include <QDesktopServices>
#endif

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , google(":/private/Calefficient_client_secret.json")
    #if USE_INTERNAL_BROWSER
    , webView(this)
    #endif
{
    // google.deleteTokens();

    ui->setupUi(this);

    QVBoxLayout *l = new QVBoxLayout();
    ui->centralwidget->setLayout(l);
    l->addWidget(&flowPages);
    l->setContentsMargins(0, 0, 0, 0);

    setMainFlow();              // MAIN
    setSignInPage();            // SIGNIN

    /*auto calendars = google.getOwnedCalendarList();
    GoogleCalendar::Event event;
    event.start = QDateTime::fromString("2020-12-12T00:00:00", Qt::ISODate);
    event.end = QDateTime::fromString("2020-12-12T12:00:00", Qt::ISODate);
    event.name = "It worked!";
    event.description = "I am a description";
    event.calendar = &calendars[0];
    google.createEvent(event);*/

#if USE_INTERNAL_BROWSER
    setAuthenticationPage();    // WEB
#endif

    flowPages.setCurrentIndex(SIGNIN);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   emit onResize();
}

void MainWindow::setMainFlow()
{
    QWidget* widget = new QWidget(&flowPages);
    AutoArrangedGridLayout *grid_layout = new AutoArrangedGridLayout(widget);
    grid_layout->setColumns(2);
    grid_layout->setContentsMargins(10, 0, 10, 0);
    grid_layout->setSpacing(0);

    QVector<TimerButton*> buttons;
    for(int i = 0; i < 20; ++i){
        TimerButton *button = new TimerButton(widget);
        buttons.push_back(button);
        //button->setMinimumHeight();
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        grid_layout->addWidget(button);
    }
    for(int i = 0; i < buttons.size(); ++i){
        connect(buttons[i], &QPushButton::pressed, [=](){
            //qDebug() << google.getCalendarList();
            //flowPages.setCurrentIndex(SIGNIN);
            foreach(auto b, buttons)
                b->reset();

            if(active_timer_button == buttons[i]){
                buttons[i]->reset();
                active_timer_button = nullptr;
            } else {
                buttons[i]->start();
                active_timer_button = buttons[i];
                update_timer.start(20);
            }
        });
    }

    flowPages.addWidget(widget);

    connect(&update_timer, &QTimer::timeout, [=](){
        if(active_timer_button != nullptr){
            active_timer_button->update();
            update_timer.start(20);
        }
    });
}

void MainWindow::setSignInPage()
{
    QWidget* widget = new QWidget(&flowPages);
    QVBoxLayout*l = new QVBoxLayout();
    l->setAlignment(Qt::AlignCenter);
    widget->setLayout(l);

    connect(&google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            [](const QUrl &url){
        //qDebug() << url;
#if USE_INTERNAL_BROWSER
        webView.load(url);
        flowPages.setCurrentIndex(WEB);
#else
        QDesktopServices::openUrl(url);
#endif
    });

    connect(&google, &QOAuth2AuthorizationCodeFlow::granted, [this](){
        qDebug() << "GRANTED!";
        flowPages.setCurrentIndex(MAIN);
    });

    // LOGO
    QLabel* logo = new QLabel(widget);
    QPixmap img = QPixmap(":/resources/images/calefficient_logo_1024_transparent.png");
    logo->setAlignment(Qt::AlignCenter);

    // NAME
    QLabel* name = new QLabel("Calefficient", widget);
    name->setAlignment(Qt::AlignCenter);

    // SPACER
    QSpacerItem* spacer = new QSpacerItem(0, 0);

    // BUTTON
    QPushButton *button = new QPushButton("Sign In to Google Calendar", widget);
    button->setStyleSheet("background-color: rgb(255, 255, 255);"
                          "color: rgb(0, 144, 0);"
                          "padding: 0.5em;"
                          "border-radius: 0.5em;");

    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    connect(button, &QPushButton::clicked, [this](){
        if(!google.isSignedIn()){
            qDebug() << "REQUEST GRANT!";
            google.grant();
        }
        else
        {
            qDebug() << "ALREADY SIGNED IN!";
            emit google.granted();
        }
    });

    // onResize
    connect(this, &MainWindow::onResize, [widget, logo, img, name, spacer, this](){
        // int size = std::min(this->width(), this->height());
        int fontSize = (14 * this->height()) / 600;

        widget->setStyleSheet("background-color: rgb(0, 144, 0);"
                              "font: bold " + QString::number(fontSize) + "px;"
                              "font-family: \"Roboto\";");

        // QPixmap pix = logo->pixmap(Qt::ReturnByValueConstant());
        logo->setPixmap(img.scaledToWidth(this->height() * 0.25));

        name->setStyleSheet("margin-top: " + QString::number(this->height() * 0.05) + ";"
                            "font: bold " + QString::number(fontSize * 2) + "px;"
                            "color: white");

        spacer->changeSize(0, this->height() * 0.3);
    });

    // add to layout
    l->addWidget(logo);
    l->addWidget(name);
    l->addSpacerItem(spacer);
    l->addWidget(button);

    flowPages.addWidget(widget);
}

#if USE_INTERNAL_BROWSER
void MainWindow::setAuthenticationPage()
{
    QWidget* widget = new QWidget(&flowPages);
    QVBoxLayout*l = new QVBoxLayout();
    widget->setLayout(l);
    l->addWidget(&webView);

    // https://github.com/qutebrowser/qutebrowser/issues/5182
    // https://stackoverflow.com/questions/30875699/how-to-set-user-agent-in-qtwebengine-qml-application
    webView.page()->profile()->setHttpUserAgent("Mozilla/5.0 (X11; Linux x86_64; rv:57.0) Gecko/20100101 Firefox/57.0");

    flowPages.addWidget(widget);
}
#endif

