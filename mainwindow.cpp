#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "timerbutton.hpp"
#include "scrollabletablewidget.hpp"

#include "settingspage.hpp"
#include "signinpage.hpp"

#include <QLabel>
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

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , google(":/private/Calefficient_client_secret.json")
    , flowPages(this)
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
    if(event->type()==QEvent::KeyPress){
        QKeyEvent * key = static_cast<QKeyEvent*>(event);
        qDebug() << key->key();
        qDebug() << key->text();
    }
    */
    return QWidget::event(event);
}

QWidget* MainWindow::makeMainFlow(QWidget* parent)
{
    mainTabs.setParent(parent);

    // TIMERS
    QString timers_id = "&1";
    mainTabs.addTab(makeTimersPage(&mainTabs), QIcon(":/resources/images/timer_icon.png"), timers_id);
    mainTabs.setTabToolTip(0, "Tooltip");
    mainTabs.addTab(new QWidget(&mainTabs), QIcon(":/resources/images/stats_icon.png"), "&2");
    mainTabs.addTab(makeSettingsPage(&mainTabs), QIcon(":/resources/images/settings_icon.png"), "&3");

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
        int iconSize = width() * 0.09;

        mainTabs.setIconSize(QSize(iconSize,iconSize));

        int nTabs = mainTabs.count();
        int width = this->width() * 1./nTabs;
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
        timersTable.updateStyle(this->size());
    });

    return &timersTable;
}

QWidget* MainWindow::makeSignInPage(QWidget* parent)
{
    SignInPage * signin_widget = new SignInPage(parent);

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

    connect(signin_widget, &SignInPage::signInResquested, [this](){
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
        signin_widget->updateStyle(size());
    });

    return signin_widget;
}


QWidget* MainWindow::makeSettingsPage(QWidget * parent)
{
    SettingsPage* settings_widget = new SettingsPage(parent);

    connect(&mainTabs, &QTabWidget::currentChanged, [=](int index){
        if(mainTabs.tabText(index) == "&3")
            settings_widget->updateStyle(size());
    });

    return settings_widget;
}

void MainWindow::updateTimersPage()
{
    if(!google.isSignedIn())
        return;

    auto calendars = google.getOwnedCalendarList();
    timersTable.setButtons(calendars);
    //QVector<int> calendars(100);

    timersTable.updateStyle(this->size());
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

