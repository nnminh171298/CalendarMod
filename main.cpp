#include "mainwindow.h"
#include <QApplication>
#include <QFile>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile file(":/files/style.css");
    if(file.open(QIODevice::ReadOnly))
        qApp->setStyleSheet(file.readAll());
    MainWindow w;
    QRect screenrect = a.primaryScreen()->geometry();
    w.move(screenrect.left(), screenrect.top());
    w.show();

    return a.exec();
}
