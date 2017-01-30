#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("Gamepad Launcher");
    MainWindow w;
    //w.show();

    return a.exec();
}
