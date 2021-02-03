#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "timerbutton.hpp"
#include "scrollabletablewidget.hpp"

#include "settingspage.hpp"
#include "signinpage.hpp"
#include "chartspage.hpp"

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
// check all lambda's: do we really need '[=]' (to pass the whole environment)

#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    #if USE_INTERNAL_BROWSER
    , webView(this)
    #endif
{
    GoogleCalendar::setCredentials(":/private/Calefficient_client_secret.json");
    //GoogleCalendar::getInstance().deleteSettings();

    ui->setupUi(this);

    flowPages = new QStackedWidget(this);

    QVBoxLayout *l = new QVBoxLayout();
    ui->centralwidget->setLayout(l);
    l->addWidget(flowPages);
    l->setContentsMargins(0, 0, 0, 0);


    // font for everything to use
    flowPages->setStyleSheet("font-family: \"Roboto\"; border: 0px;");

    // SIGNIN
    flowPages->addWidget(makeSignInPage(flowPages));

    // MAIN
    flowPages->addWidget(makeMainTabs(flowPages));

    // TIMER_EDIT
    flowPages->addWidget(makeTimerEditPage(flowPages));

#if USE_INTERNAL_BROWSER
    setAuthenticationPage();    // WEB
#endif

    flowPages->setCurrentIndex(GoogleCalendar::getInstance().isSignedIn() ? MAIN_TABS : SIGNIN_PAGE);
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

QWidget* MainWindow::makeMainTabs(QWidget* parent)
{
    mainTabs = new QTabWidget(parent);

    // TIMERSx
    mainTabs->addTab(makeTimersPage(mainTabs), QIcon(":/resources/images/timer_icon.png"), "&" + QString::number(TIMERS_PAGE));
    mainTabs->setTabToolTip(0, "Tooltip");
    mainTabs->addTab(makeChartsPage(mainTabs), QIcon(":/resources/images/stats_icon.png"), "&" + QString::number(CHARTS_PAGE));
    mainTabs->addTab(makeSettingsPage(mainTabs), QIcon(":/resources/images/settings_icon.png"), "&" + QString::number(SETTINGS_PAGE));

    // COSTUMIZE TABS
    mainTabs->setTabPosition(QTabWidget::South);
    //mainTabs.setMovable(true);
    mainTabs->setTabShape(QTabWidget::TabShape::Rounded);
    mainTabs->setUsesScrollButtons(false);

    // onResize
    connect(this, &MainWindow::onResize, [=](){
        int iconSize = width() * 0.09;

        mainTabs->setIconSize(QSize(iconSize,iconSize));

        int nTabs = mainTabs->count();
        int width = this->width() * 1./nTabs;
        mainTabs->setStyleSheet("QTabBar::tab { height: 3em; width: " + QString::number((width+iconSize)/2) + "px; "
                               //"               background-color: " + colourDef + ";"
                               "color: transparent; "
                               "               border: none; "
                               "               padding-left: " + QString::number((width-iconSize)/2) + "px;"
                                                                                                       " }"
                               //"QTabBar::tab:selected { background-color: " + colourSelected + " }"
                               );
    });

    return mainTabs;
}

QWidget* MainWindow::makeTimersPage(QWidget * parent)
{
    timersTable = new TimerTable(parent);

    // onResize (needed because during app startup, window size is not yet defined
    connect(this, &MainWindow::onResize, timersTable, &TimerTable::updateStyle);

    connect(timersTable, &TimerTable::buttonLongPressed, [this](TimerButton* button){
        timerEditPage->setData(button->getData());
        flowPages->setCurrentIndex(TIMER_EDIT_PAGE);
    });
    connect(timersTable, &TimerTable::plusButtonClicked, [this](){
        timerEditPage->setData(TimerButton::Data());
        flowPages->setCurrentIndex(TIMER_EDIT_PAGE);
    });

    return timersTable;
}

QWidget* MainWindow::makeSignInPage(QWidget* parent)
{
    SignInPage *signin_widget = new SignInPage(parent);

    connect(&GoogleCalendar::getInstance(), &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            [](const QUrl &url){
        //qDebug() << url;
#if USE_INTERNAL_BROWSER
        webView.load(url);
        flowPages.setCurrentIndex(WEB);
#else
        QDesktopServices::openUrl(url);
#endif
    });

    connect(&GoogleCalendar::getInstance(), &GoogleCalendar::signedIn, [this](){
        qDebug() << "SIGNED IN!";
        flowPages->setCurrentIndex(MAIN_TABS);
        mainTabs->setCurrentIndex(TIMERS_PAGE);
    });

    connect(signin_widget, &SignInPage::signInResquested, [](){
        if(!GoogleCalendar::getInstance().isSignedIn()){
            qDebug() << "REQUEST GRANT!";
            GoogleCalendar::getInstance().grant();
        }
        else
        {
            qDebug() << "ALREADY SIGNED IN!";
            emit GoogleCalendar::getInstance().granted();
        }
    });

    connect(this, &MainWindow::onResize, signin_widget, &SignInPage::updateStyle);

    return signin_widget;
}

QWidget *MainWindow::makeChartsPage(QWidget *parent)
{
    ChartsPage* charts_widget = new ChartsPage(parent);

    connect(mainTabs, &QTabWidget::currentChanged, [=](int index){
        if(index == CHARTS_PAGE)
        {
            ChartsPage::AnalysisSettings analysis;
            analysis.start = QDateTime::fromString("2020-12-01T00:00:00", Qt::ISODate).toUTC();
            analysis.end = QDateTime::fromString("2021-01-02T23:00:00", Qt::ISODate).toUTC();

            ChartsPage::Profile profile;

            QVector<GoogleCalendar::Calendar*> &calendars = GoogleCalendar::getInstance().getOwnedCalendarList();
            for(GoogleCalendar::Calendar *calendar: calendars){
                ChartsPage::CalendarSettings cal_settings(calendar);
                ChartsPage::TagSettings tag_settings;
                tag_settings.active = true;
                tag_settings.name = "Calefficient";

                cal_settings.active = true;
                cal_settings.tags.push_back(tag_settings);
                profile.push_back(cal_settings);
            }
            analysis.profile = &profile;

            ChartsPage::AnalysisResults res = charts_widget->runAnalysis(analysis);
            charts_widget->showChartAnalysis(res);
        }
    });

    return charts_widget;
}


QWidget* MainWindow::makeSettingsPage(QWidget * parent)
{
    SettingsPage* settings_widget = new SettingsPage(parent);

    connect(this, &MainWindow::onResize, settings_widget, &SettingsPage::updateStyle);

    return settings_widget;
}

QWidget *MainWindow::makeTimerEditPage(QWidget *parent)
{
    timerEditPage = new TimerEditPage(parent);

    connect(timerEditPage, &TimerEditPage::done, [this](int sucess){
        if(sucess)
            timersTable->saveButtonOnEdit(timerEditPage->getData());
        flowPages->setCurrentIndex(MAIN_TABS);
    });

    connect(this, &MainWindow::onResize, timerEditPage, &TimerEditPage::updateStyle);

    return timerEditPage;
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

