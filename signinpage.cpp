#include "signinpage.hpp"

#include <QVBoxLayout>
#include <QDebug>

SignInPage::SignInPage(QWidget* parent) : QWidget(parent), colour("rgb(0, 144, 0);")
{
    setAttribute(Qt::WA_StyledBackground, true);

    QVBoxLayout*l = new QVBoxLayout(this);
    l->setAlignment(Qt::AlignCenter);
    //setLayout(l);

    // LOGO
    logo = new QLabel(this);
    img = QPixmap(":/resources/images/calefficient_logo_1024_transparent.png");
    logo->setAlignment(Qt::AlignCenter);

    // NAME
    name = new QLabel("Calefficient", this);
    name->setAlignment(Qt::AlignCenter);

    // SPACER
    spacer = new QSpacerItem(0, 0);

    // BUTTON
    QPushButton *button = new QPushButton("Sign In to Google Calendar", this);
    button->setStyleSheet("background-color: rgb(255, 255, 255);"
                          "color: " + colour +
                          "padding: 0.5em;"
                          "border-radius: 0.5em;");

    button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);

    // add to layout
    l->addWidget(logo);
    l->addWidget(name);
    l->addSpacerItem(spacer);
    l->addWidget(button);

    connect(button, &QPushButton::clicked, [this](){ emit signInResquested(); });
}

void SignInPage::updateStyle(const QSize &size)
{
    // int size = std::min(this->width(), this->height());
    int fontSize = size.height() * 0.02;

    setStyleSheet("background-color: " + colour +
                  "font: bold " + QString::number(fontSize) + "px;");

    // QPixmap pix = logo->pixmap(Qt::ReturnByValueConstant());
    logo->setPixmap(img.scaledToWidth(size.height() * 0.25));

    name->setStyleSheet("margin-top: " + QString::number(size.height() * 0.05) + ";"
                        "font: bold " + QString::number(fontSize * 2) + "px;"
                        "color: white;");

    spacer->changeSize(0, size.height() * 0.3);
}
