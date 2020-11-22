#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QOAuthHttpServerReplyHandler>
#include <QNetworkReply>
#include <QTimer>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    google = new QOAuth2AuthorizationCodeFlow;
    //google->setScope("email");
    google->setScope("https://www.googleapis.com/auth/calendar.readonly");
    connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
        &QDesktopServices::openUrl);

    QFile file;
    file.setFileName(":/Calefficient_client_secret.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString val = file.readAll();

    QJsonDocument document = QJsonDocument::fromJson(val.toUtf8());
    const auto object = document.object();
    const auto settingsObject = object["installed"].toObject();
    const auto clientId = settingsObject["client_id"].toString();
    const auto projectId = settingsObject["project_id"].toString();
    const QUrl authUri(settingsObject["auth_uri"].toString());
    const QUrl tokenUri(settingsObject["token_uri"].toString());
    const QUrl auth_provider_x509_cert_url(settingsObject["auth_provider_x509_cert_url"].toString());

    const auto redirectUris = settingsObject["redirect_uris"].toArray();
    const QUrl redirectUri(redirectUris[0].toString()); // Get the first URI
    const auto port = static_cast<quint16>(redirectUri.port()); // Get the port

    google->setAuthorizationUrl(authUri);
    google->setClientIdentifier(clientId);
    google->setAccessTokenUrl(tokenUri);
    //google->setClientIdentifierSharedKey(clientSecret);

    auto replyHandler = new QOAuthHttpServerReplyHandler(port, this);
    google->setReplyHandler(replyHandler);

    // tmp, not sure if necessary, probably can use google->token
    connect(replyHandler, &QOAuthHttpServerReplyHandler::tokensReceived, [=](){
        qDebug() << __FUNCTION__ << __LINE__ << "Token Received!";
    });

    replyHandler->setCallbackText("<h1> Logged in succesfully! Go back and enjoy Calefficient ;) </h1>\
                                <img src=\"http://caenrigen.tech/Calefficient/Logo-512.png\" alt=\"Calefficient Logo\">");

    connect(google, &QOAuth2AuthorizationCodeFlow::granted, [=](){
        qDebug() << __FUNCTION__ << __LINE__ << "Access Granted!";

        //auto reply = google->get(QUrl("https://www.googleapis.com/plus/v1/people/me"));
        auto reply = google->get(QUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList"));

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug() << "REQUEST FINISHED. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    });



    // https://www.qt.io/blog/2017/01/25/connecting-qt-application-google-services-using-oauth-2-0
    // https://stackoverflow.com/questions/62296641/unable-to-implement-google-sign-in-using-qoauth2authorizationcodeflow
    // https://stackoverflow.com/questions/52181208/why-am-i-getting-server-replied-forbidden-from-google-drive-rest-api

    // remove permissions at:
    // https://myaccount.google.com/permissions

    // remember to use QOAuthHttpServerReplyHandler::tokensReceived signal and store access_token (what for?)


    QPushButton *button = new QPushButton("Authenticate");
    QVBoxLayout *l = new QVBoxLayout();
    ui->centralwidget->setLayout(l);
    ui->centralwidget->layout()->addWidget(button);
    //button->setText(tr("something"));
    //button->setText(redirectUri.toEncoded());
    //setCentralWidget(button);

    connect(button, SIGNAL(pressed()), this, SLOT(googleGrant()));

/*
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(showTime()));
    timer->start(1000);
*/

}

MainWindow::~MainWindow()
{
    delete ui;
    delete google;
}

void MainWindow::googleGrant()
{
    //QEventLoop *loop = new QEventLoop();
    //connect(google, &QOAuth2AuthorizationCodeFlow::granted, loop, &QEventLoop::quit);

    google->grant();
/*
    //loop->exec();

    qDebug() << __FUNCTION__ << __LINE__ << "Access Granted!";

    //auto reply = google->get(QUrl("https://www.googleapis.com/plus/v1/people/me"));
    auto reply = google->get(QUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList"));

    connect(reply, &QNetworkReply::finished, [reply](){
        qDebug() << "REQUEST FINISHED. Error? " << (reply->error() != QNetworkReply::NoError);
        qDebug() << reply->readAll();
    });
    */
}

void MainWindow::showTime()
{
    static int i = 0;
    qDebug() << i++;
}

