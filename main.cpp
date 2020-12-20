#include "mainwindow.hpp"

#include <QApplication>
#include <QDesktopWidget>
#include <QSizePolicy>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    int height = QDesktopWidget().height() * 0.79;
    int width = (height * 8.5) / 18;
    w.setFixedSize(QSize(width, height));

    w.show();
    return a.exec();
}
