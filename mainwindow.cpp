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
#include <QVBoxLayout>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings("Calefficient", "OAuth2");
    QString token = settings.value("token").toString();
    QString refreshToken = settings.value("refreshToken").toString();
    qDebug() << "START";
    qDebug() << token;
    qDebug() << refreshToken;

    google = new QOAuth2AuthorizationCodeFlow;

    if(token!=""){
        google->setToken(token);
        google->setRefreshToken(refreshToken);

        auto reply = google->get(QUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList"));

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug() << "REQUEST FINISHED. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });

        return;
    }


    //google->setScope("email");
    google->setScope("https://www.googleapis.com/auth/calendar.readonly");
    connect(google, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);

    QFile file;
    file.setFileName(":/private/Calefficient_client_secret.json");
    file.open(QIODevice::ReadOnly);
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
        qDebug() << google->token();
        qDebug() << google->refreshToken();
    });

    replyHandler->setCallbackText("<h1> Logged in succesfully! Go back and enjoy Calefficient ;) </h1>\
                                  <img src=\"http://caenrigen.tech/Calefficient/Logo-512.png\" alt=\"Calefficient Logo\">");

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

    connect(button, SIGNAL(pressed()), this, SLOT(googleGrant()));
}

MainWindow::~MainWindow()
{
    delete ui;
    delete google;
}

void MainWindow::googleGrant()
{
    connect(google, &QOAuth2AuthorizationCodeFlow::granted, [=](){
        qDebug() << __FUNCTION__ << __LINE__ << "Access Granted!";
        qDebug() << "Writing tokens:";
        qDebug() << "Token: " << google->token();
        qDebug() << "Refresh Token: " << google->refreshToken();

        QSettings settings("Calefficient", "OAuth2");
        settings.setValue("token", google->token());
        settings.setValue("refreshToken", google->refreshToken());

        //auto reply = google->get(QUrl("https://www.googleapis.com/plus/v1/people/me"));
        auto reply = google->get(QUrl("https://www.googleapis.com/calendar/v3/users/me/calendarList"));

        connect(reply, &QNetworkReply::finished, [reply](){
            qDebug() << "REQUEST FINISHED. Error? " << (reply->error() != QNetworkReply::NoError);
            qDebug() << reply->readAll();
        });
    });


    google->grant();

}
