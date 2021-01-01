#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "timerbutton.hpp"
#include "scrollabletablewidget.hpp"

#include "collapsible.h"

#include <QLabel>
#include <QListWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDebug>
#include <QToolButton>

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
        timersTable.updateStyle(this->size());
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


QWidget* MainWindow::makeSettingsPage(QWidget * parent)
{
    /*
    listWidget->setStyleSheet("font: 30px");
    //listWidget->setDragDropMode(QAbstractItemView::InternalMove);
    //listWidget->setAcceptDrops(true);
    //listWidget->setDragEnabled(true);
    */

    ScrollableTableWidget* settings_widget = new ScrollableTableWidget(parent);
    settings_widget->setColumnCount(1);

    Collapsible* priorities_section = new Collapsible(settings_widget);
    QLabel* priorities_section_header_label = new QLabel("Priorities", priorities_section);
    QVBoxLayout* priorities_section_content_layout = new QVBoxLayout(priorities_section);
    QListWidget *priorities_section_listWidget = new QListWidget(priorities_section);
    QListWidgetItem* item1 = new QListWidgetItem("1", priorities_section_listWidget);
    QListWidgetItem* item2 = new QListWidgetItem("2", priorities_section_listWidget);
    QListWidgetItem* item3 = new QListWidgetItem("3", priorities_section_listWidget);
    priorities_section_listWidget->addItem(item1);
    priorities_section_listWidget->addItem(item2);
    priorities_section_listWidget->addItem(item3);
    priorities_section_listWidget->setFrameShape(QFrame::NoFrame);
    priorities_section_content_layout->setContentsMargins(0, 0, 0, 0);

    connect(priorities_section, &Collapsible::toggled, [=](){
        settings_widget->resizeRowsToContents();
    });

    priorities_section_content_layout->addWidget(priorities_section_listWidget);
    priorities_section->setHeader(priorities_section_header_label);
    priorities_section->setContentLayout(priorities_section_content_layout);

    QWidget* week_day_widget = new QWidget(settings_widget);
    QHBoxLayout* week_day_layout = new QHBoxLayout(week_day_widget);
    week_day_widget->setLayout(week_day_layout);
    week_day_layout->addWidget(new QLabel("Week starting day"));
    QSpacerItem* week_day_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    week_day_layout->addSpacerItem(week_day_spacer);
    QComboBox* week_day_combobox = new QComboBox(week_day_widget);
    week_day_combobox->addItems(QStringList() << "Saturday" << "Sunday" << "Monday");
    //week_day_combobox->setStyleSheet("QComboBox { border: 1px solid black; border-radius: 3px;  }"
    //                                 "QComboBox::drop-down { border-top-right-radius: 3px; border-bottom-right-radius: 3px; }"
    //                                 "QComboBox::down-arrow { image: url(:/resources/images/down_arrow.png); width: 12; height: 12; }");
    week_day_layout->addWidget(week_day_combobox);

    QWidget* auto_update_widget = new QWidget(settings_widget);
    QHBoxLayout* auto_update_layout = new QHBoxLayout(auto_update_widget);
    auto_update_widget->setLayout(auto_update_layout);
    auto_update_layout->addWidget(new QLabel("Auto update"));
    QSpacerItem* auto_update_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    auto_update_layout->addSpacerItem(auto_update_spacer);
    QCheckBox* auto_update_checkbox = new QCheckBox(auto_update_widget);
    auto_update_checkbox->setCheckState(Qt::CheckState::Unchecked);
    auto_update_layout->addWidget(auto_update_checkbox);

    QWidget* calculate_empty_slots_widget = new QWidget(settings_widget);
    QHBoxLayout* calculate_empty_slots_layout = new QHBoxLayout(calculate_empty_slots_widget);
    calculate_empty_slots_widget->setLayout(calculate_empty_slots_layout);
    calculate_empty_slots_layout->addWidget(new QLabel("Calculate empty slots"));
    QSpacerItem* calculate_empty_slots_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    calculate_empty_slots_layout->addSpacerItem(calculate_empty_slots_spacer);
    QCheckBox* calculate_empty_slots_checkbox = new QCheckBox(calculate_empty_slots_widget);
    calculate_empty_slots_checkbox->setCheckState(Qt::CheckState::Unchecked);
    calculate_empty_slots_layout->addWidget(calculate_empty_slots_checkbox);

    QWidget* warn_empty_slots_widget = new QWidget(settings_widget);
    QHBoxLayout* warn_empty_slots_layout = new QHBoxLayout(warn_empty_slots_widget);
    warn_empty_slots_widget->setLayout(warn_empty_slots_layout);
    warn_empty_slots_layout->addWidget(new QLabel("Warn empty slots"));
    QSpacerItem* warn_empty_slots_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    warn_empty_slots_layout->addSpacerItem(warn_empty_slots_spacer);
    QCheckBox* warn_empty_slots_checkbox = new QCheckBox(warn_empty_slots_widget);
    warn_empty_slots_checkbox->setCheckState(Qt::CheckState::Unchecked);
    warn_empty_slots_layout->addWidget(warn_empty_slots_checkbox);

    QWidget* ignore_24h_events_widget = new QWidget(settings_widget);
    QHBoxLayout* ignore_24h_events_layout = new QHBoxLayout(ignore_24h_events_widget);
    ignore_24h_events_widget->setLayout(ignore_24h_events_layout);
    ignore_24h_events_layout->addWidget(new QLabel("Ignore 24h+ events"));
    QSpacerItem* ignore_24h_events_spacer = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Ignored);
    ignore_24h_events_layout->addSpacerItem(ignore_24h_events_spacer);
    QCheckBox* ignore_24h_events_checkbox = new QCheckBox(ignore_24h_events_widget);
    ignore_24h_events_checkbox->setCheckState(Qt::CheckState::Unchecked);
    ignore_24h_events_layout->addWidget(ignore_24h_events_checkbox);

    QPushButton* reset_app_button = new QPushButton("Delete all data");
    reset_app_button->setStyleSheet("QPushButton { background-color:red; color: white; }");

    settings_widget->addWidget(priorities_section);
    settings_widget->addWidget(week_day_widget);
    settings_widget->addWidget(auto_update_widget);
    settings_widget->addWidget(calculate_empty_slots_widget);
    settings_widget->addWidget(warn_empty_slots_widget);
    settings_widget->addWidget(ignore_24h_events_widget);
    settings_widget->addWidget(reset_app_button);

    connect(&mainTabs, &QTabWidget::currentChanged, [=](int index){
        if(mainTabs.tabText(index) == "&3"){
            settings_widget->setStyleSheet("font: " + QString::number((int)(this->height() * 0.025)) + "px;");
            settings_widget->resizeRowsToContents();
        }
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

