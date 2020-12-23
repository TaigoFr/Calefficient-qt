#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include <QLabel>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QDebug>
#include <QScrollBar>

#include <QScroller>
//#include <QEasingCurve>
#include <QHeaderView>
#include <QMouseEvent>

#include <cmath>
#include "flickcharm.h"

#include <QLineEdit>

#if USE_INTERNAL_BROWSER
#include <QWebEngineView>
#include <QWebEngineProfile>
#else
#include <QDesktopServices>
#endif

// REVER OS CONNECTS TODOS E GARANTIR QUE O QT NÃO ESTÁ A ACUMULAR CONNECTS (POR ENTRAR VÁRIAS VEZES NA MESMA FUNÇÃO)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , google(":/private/Calefficient_client_secret.json")
    , active_timer_button(nullptr)
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
    static int i = 1;
    qDebug() << "enterEvent!!!!!!!!!!!!!!!!!" << i++;
    qDebug() << event->type();
    QWidget::enterEvent(event);
}

bool MainWindow::event(QEvent *event)
{
    static int i = 1;
    qDebug() << "EVENT " << i++;
    qDebug() << event->type();
    if(event->type() == QEvent::HoverMove){
        QHoverEvent *hover = static_cast<QHoverEvent*>(event);
        qDebug() << hover->oldPos();
        qDebug() << hover->pos();
        int delta = hover->pos().y() - hover->oldPos().y();
        if(abs(delta) < 100){
            tableWidget.verticalScrollBar()->setValue(tableWidget.verticalScrollBar()->value() - delta);
        }

    }
    return QWidget::event(event);
}

QWidget* MainWindow::makeMainFlow(QWidget* parent)
{
    mainTabs.setParent(parent);

    // TIMERS
    mainTabs.addTab(makeTimersPage(&mainTabs), QIcon(QPixmap(":/resources/images/timer_icon.png")), "&1");
    mainTabs.setTabToolTip(0, "Tooltip");
    mainTabs.addTab(new QWidget(&mainTabs), QIcon(QPixmap(":/resources/images/stats_icon.png")), "&2");
    mainTabs.addTab(new QWidget(&mainTabs), QIcon(QPixmap(":/resources/images/settings_icon.png")), "&3");

    // update Timers page when change
    connect(&mainTabs, &QTabWidget::currentChanged, [=](int index){
        if(index==0)
            updateTimersPage();
    });

    connect(&flowPages, &QStackedWidget::currentChanged, [=](int index){
        // also emit signal of 'Changed to new tab' when moving to the tabs page
        if(index==MAIN)
            emit mainTabs.currentChanged(0);
    });

    // COSTUMIZE TABS
    mainTabs.setTabPosition(QTabWidget::South);
    mainTabs.setMovable(true);
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

    tableWidget.setParent(parent);
    tableWidget.setFrameShape(QFrame::NoFrame);
    tableWidget.horizontalHeader()->hide();
    tableWidget.verticalHeader()->hide();

    //QScroller::grabGesture(tableWidget.viewport(), QScroller::LeftMouseButtonGesture);
    tableWidget.setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    tableWidget.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    // disable selection of cells
    tableWidget.setSelectionMode(QAbstractItemView::NoSelection);

    // hide scrollBarsw
    tableWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    tableWidget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // hide grid
    tableWidget.setShowGrid(false);

    tableWidget.setAutoScroll(false);

    setAttribute(Qt::WA_Hover, true);
    tableWidget.setAttribute(Qt::WA_Hover, true);
    tableWidget.verticalScrollBar()->setAttribute(Qt::WA_Hover, true);

    return &tableWidget;
    /*

    scrollArea.setParent(parent);
    scrollArea.setContentsMargins(0,0,0,0);
    scrollArea.setFrameShape(QFrame::NoFrame);

    QHBoxLayout* l = new QHBoxLayout();

    scrollArea.setLayout(l);
    scrollArea.setWidgetResizable(true);

    QWidget * widget = new QWidget;
    widget->setLayout(&grid_layout);
    scrollArea.setWidget(widget);

    int spacing = widthPercentage(0.02);
    grid_layout.setContentsMargins(spacing, spacing, spacing, spacing);
    grid_layout.setSpacing(spacing);

    // allow touch scroll for Mobile devices
    //QScroller::grabGesture(scrollArea->viewport(), QScroller::TouchGesture);

    QScroller *scroller = QScroller::scroller(scrollArea.viewport());

    QScrollerProperties prop = scroller->scrollerProperties();

    //prop.setScrollMetric(QScrollerProperties::MousePressEventDelay, 0.001);
    prop.setScrollMetric(QScrollerProperties::DragStartDistance, 0.001);
    //prop.setScrollMetric(QScrollerProperties::DragVelocitySmoothingFactor, 1.);
    //prop.setScrollMetric(QScrollerProperties::ScrollingCurve, QEasingCurve(QEasingCurve::Linear));
    //prop.setScrollMetric(QScrollerProperties::DecelerationFactor, 1.);
    //prop.setScrollMetric(QScrollerProperties::MinimumVelocity, 0.);
    //prop.setScrollMetric(QScrollerProperties::MaximumVelocity, 100.);
    //prop.setScrollMetric(QScrollerProperties::SnapPositionRatio, 0.0);
    //prop.setScrollMetric(QScrollerProperties::SnapTime, 0.9);
    //prop.setScrollMetric(QScrollerProperties::OvershootDragResistanceFactor, 0.99);
    //prop.setScrollMetric(QScrollerProperties::OvershootDragDistanceFactor, 0.99);
    //prop.setScrollMetric(QScrollerProperties::OvershootScrollDistanceFactor, 0.99);
    //prop.setScrollMetric(QScrollerProperties::OvershootScrollTime, 10.);
    //prop.setScrollMetric(QScrollerProperties::VerticalOvershootPolicy, QScrollerProperties::OvershootWhenScrollable);
    //prop.setScrollMetric(QScrollerProperties::FrameRate, 120);

    scroller->setScrollerProperties(prop);
    scroller->grabGesture(scrollArea.viewport(), QScroller::LeftMouseButtonGesture);


    //FlickCharm *flickCharm = new FlickCharm;
    //flickCharm->activateOn(&scrollArea);
    //this->installEventFilter(flickCharm);


    // hide scrollBars
    scrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scrollArea.horizontalScrollBar()->setSingleStep(1);

    return &scrollArea;
*/
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

    //grid_layout.removeAll(true);
    //grid_layout.setColumns(calendars.size() <= 3 ? 1 : 2);
    tableWidget.setColumnCount(calendars.size() <= 3 ? 1 : 2);
    tableWidget.setRowCount((calendars.size()+1) / tableWidget.columnCount());

    QVector<TimerButton*> buttons(calendars.size());
    //QVector<QLineEdit*> buttons(calendars.size());
    int r=0, c=0;
    for(int i = 0; i < calendars.size(); ++i){
        TimerButton *button = new TimerButton(calendars[i].name, &tableWidget);
        //TimerButton *button = new TimerButton("Button " + QString::number(i), &tableWidget);
        //TimerButton *button = new TimerButton(calendars[i].name, grid_layout.parentWidget());
        //TimerButton *button = new TimerButton("Button " + QString::number(i), grid_layout.parentWidget());
        //QLineEdit* button = new QLineEdit("text " + QString::number(i));
        buttons[i] = button;
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        //grid_layout.addWidget(button);

        tableWidget.setCellWidget(r,c,button);
        ++c;
        if(c==tableWidget.columnCount()){
            c = 0;
            ++r;
        }

    }


    for(int i = 0; i < buttons.size(); ++i){
        connect(buttons[i], &QPushButton::pressed, [this](){
            this->scrollValue = this->scrollArea.verticalScrollBar()->value();
        });

        connect(buttons[i], &QPushButton::clicked, [=](){
            if(this->scrollArea.verticalScrollBar()->value() != this->scrollValue)
                return;
            //scrollA
            //qDebug() << google.getCalendarList();
            //flowPages.setCurrentIndex(SIGNIN);
            //foreach(auto b, buttons)
                //b->reset();
            //if(active_timer_button != nullptr && active_timer_button != buttons[i])
            if(active_timer_button != nullptr)
               active_timer_button->reset();

            if(active_timer_button == buttons[i]){
                buttons[i]->reset();
                active_timer_button = nullptr;
            } else {
                buttons[i]->start();
                active_timer_button = buttons[i];
                update_timer.start(1000); // every 1000ms
            }
        });
    }


    connect(&update_timer, &QTimer::timeout, [=](){
        if(active_timer_button != nullptr){
            active_timer_button->updateText();
            update_timer.start(1000); // every 1000ms
        }
    });

    // onResize (needed because during app startup, window size is not yet defined
    connect(this, &MainWindow::onResize, [=](){
        int spacing = widthPercentage(0.02);
        int width = widthPercentage(0.44);
        //int iconSize = widthPercentage(0.05);

        tableWidget.setStyleSheet("QTableWidget::item { padding: " + QString::number(spacing) + "px }");
        for(int i = 0; i < calendars.size(); ++i){
            QColor color(calendars[i].color_hex);
            //color = color.darker(120);
            //color.setHsl(color.hue(), color.hslSaturation()*1.1, color.lightness());
            buttons[i]->setStyleSheet(
                                    "background-color: " + calendars[i].color_hex + ";"
                                    //"background-color: " + QColor(calendars[i].color_hex).darker(110).name() + ";"
                                    //"background-color: " + color.name() + ";"
                                    //"background-color: rgb(" + QString::number(128+100*std::sin(300*i)) + "," + QString::number(128+100*std::sin(200*i)) + "," + QString::number(128+100*std::sin(400*i)) + ");"
                                      "color: rgb(255,255,255);"
                                      "font: bold " + QString::number(heightPercentage(0.025)) + "px;"
                                      //"padding: 0.5em;"
                                      "border-radius: 1em;"
                                      "min-width: " + QString::number(width) + "px;"
                                      "min-height: " + QString::number(width) + "px;"
                                      "margin: " + QString::number(spacing) + "px;"
                                    //"background-position: center bottom;"
                                    //"background-repeat: no-repeat;"
                                    //"background-origin: content;"
                                    //"background-image: url(\":/resources/images/play_icon.png\");"
                                                                              );
        }

        tableWidget.resizeColumnsToContents();
        tableWidget.resizeRowsToContents();
    });
    // needs to be forced to update styleSheet when tabs are changed
    emit onResize();
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

