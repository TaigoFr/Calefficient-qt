#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "timerbutton.hpp"

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

// REVER OS CONNECTS TODOS E GARANTIR QUE O QT NÃO ESTÁ A ACUMULAR CONNECTS (POR ENTRAR VÁRIAS VEZES NA MESMA FUNÇÃO)
// ADD CONST WHEREVER POSSIBLE

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

    // font for everything to use
    flowPages.setStyleSheet("font-family: \"Roboto\";");

    // MAIN
    flowPages.addWidget(makeMainFlow(&flowPages));

    // SIGNIN
    flowPages.addWidget(makeSignInPage(&flowPages));

    /*auto calendars = google.getOwnedCalendarLists();
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

    flowPages.setCurrentIndex(google.isSignedIn() ? MAIN : SIGNIN);
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

void MainWindow::enterEvent(QEvent *event)
{
    //static int i = 1;
    //qDebug() << "enterEvent!!!!!!!!!!!!!!!!!" << i++;
    //qDebug() << event->type();
    QWidget::enterEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    //static int i = 1;
    //qDebug() << "EVENT " << i++;
    //qDebug() << event->type();
    /*
    if(event->type() == QEvent::HoverMove){
        QHoverEvent *hover = static_cast<QHoverEvent*>(event);
        qDebug() << hover->oldPos();
        qDebug() << hover->pos();
        int delta = hover->pos().y() - hover->oldPos().y();
        if(abs(delta) < 100){
            tableWidget.verticalScrollBar()->setValue(tableWidget.verticalScrollBar()->value() - delta);
        }

    }
    */
    return QWidget::event(event);
}

QWidget* MainWindow::makeMainFlow(QWidget* parent)
{
    mainTabs.setParent(parent);

    // TIMERS
    QString timers_id = "&1";
    mainTabs.addTab(makeTimersPage(&mainTabs), QIcon(QPixmap(":/resources/images/timer_icon.png")), timers_id);
    mainTabs.setTabToolTip(0, "Tooltip");
    mainTabs.addTab(new QWidget(&mainTabs), QIcon(QPixmap(":/resources/images/stats_icon.png")), "&2");
    mainTabs.addTab(new QWidget(&mainTabs), QIcon(QPixmap(":/resources/images/settings_icon.png")), "&3");

    // update Timers page when change
    connect(&mainTabs, &QTabWidget::currentChanged, [=](int index){
        if(mainTabs.tabText(index) == timers_id)
            updateTimersPage();
    });

    connect(&flowPages, &QStackedWidget::currentChanged, [=](int index){
        // also emit signal of 'Changed to new tab' when moving to the tabs page
        if(index==MAIN)
            emit mainTabs.currentChanged(0);
    });

    // COSTUMIZE TABS
    mainTabs.setTabPosition(QTabWidget::South);
    //mainTabs.setMovable(true);
    mainTabs.setTabShape(QTabWidget::TabShape::Rounded);
    mainTabs.setUsesScrollButtons(false);

    // onResize
    connect(this, &MainWindow::onResize, [=](){
        int iconSize = widthPercentage(0.09);

        mainTabs.setIconSize(QSize(iconSize,iconSize));

        int nTabs = mainTabs.count();
        int width = widthPercentage(1./nTabs);
        mainTabs.setStyleSheet("QTabBar::tab { height: 3em; width: " + QString::number((width+iconSize)/2) + "px; "
                               //"               background-color: " + colourDef + ";"
                               "color: transparent; "
                               "               border: none; "
                               "               padding-left: " + QString::number((width-iconSize)/2) + "px;"
                                                                                                       " }"
                               //"QTabBar::tab:selected { background-color: " + colourSelected + " }"
                               );
    });

    return &mainTabs;
}

QWidget* MainWindow::makeTimersPage(QWidget * parent)
{
    timersTable.setParent(parent);

    // onResize (needed because during app startup, window size is not yet defined
    connect(this, &MainWindow::onResize, [=](){
        timersTable.setWindowSize(this->size());
        timersTable.updateStyle();
    });

    return &timersTable;
}

QWidget* MainWindow::makeSignInPage(QWidget* parent)
{
    QWidget* widget = new QWidget(parent);
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

    QString colour = "rgb(0, 144, 0);";

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
                          "color: " + colour +
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
    connect(this, &MainWindow::onResize, [=](){
        // int size = std::min(this->width(), this->height());
        int fontSize = heightPercentage(0.02);

        widget->setStyleSheet("background-color: " + colour +
                              "font: bold " + QString::number(fontSize) + "px;");

        // QPixmap pix = logo->pixmap(Qt::ReturnByValueConstant());
        logo->setPixmap(img.scaledToWidth(heightPercentage(0.25)));

        name->setStyleSheet("margin-top: " + QString::number(heightPercentage(0.05)) + ";"
                                                                                       "font: bold " + QString::number(fontSize * 2) + "px;"
                                                                                                                                       "color: white");

        spacer->changeSize(0, heightPercentage(0.3));
    });

    // add to layout
    l->addWidget(logo);
    l->addWidget(name);
    l->addSpacerItem(spacer);
    l->addWidget(button);

    return widget;
}

void MainWindow::updateTimersPage()
{
    if(!google.isSignedIn())
        return;

    auto calendars = google.getOwnedCalendarList();
    //QVector<int> calendars(100);

    timersTable.clear();
    timersTable.setColumnCount(calendars.size() <= 3 ? 1 : 2);

    int repeat = 1;

    QVector<TimerButton*> buttons(calendars.size());
    for (int r=0; r<repeat ; ++r) {
        for(int i = 0; i < calendars.size(); ++i){
            TimerButton *button = new TimerButton(calendars[i], &timersTable);
            buttons[i] = button;
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

            timersTable.addButton(button);
        }
    }

    timersTable.updateStyle();
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

