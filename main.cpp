#include "mainwindow.hpp"

#include <QApplication>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;

    int height = QDesktopWidget().height() * 0.8;
    int width = (height * 9) / 16;
    w.setFixedSize(width, height);

    w.show();
    return a.exec();
}
