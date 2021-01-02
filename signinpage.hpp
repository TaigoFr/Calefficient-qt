#ifndef SIGNINPAGE_HPP
#define SIGNINPAGE_HPP

#include <QWidget>

#include <QLabel>
#include <QPushButton>
#include <QPixmap>
#include <QSpacerItem>

class SignInPage : public QWidget
{
    Q_OBJECT

public:
    SignInPage(QWidget* parent = nullptr);

    void updateStyle(const QSize& size);

signals:
    void signInResquested();

private:
    QLabel* name;
    QLabel* logo;
    QString colour;
    QSpacerItem* spacer;

    QPixmap img;
};

#endif // SIGNINPAGE_HPP
