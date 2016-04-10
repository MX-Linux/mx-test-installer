#include "mainwindow.h"
#include <QApplication>
#include <unistd.h>
#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTran;
    qtTran.load(QString("qt_") + QLocale::system().name());
    a.installTranslator(&qtTran);

    QTranslator appTran;
    appTran.load(QString("mx-test-installer_") + QLocale::system().name(), "/usr/share/mx-test-installer/locale");
    a.installTranslator(&appTran);

    MainWindow w;
    w.show();

    return a.exec();
}
