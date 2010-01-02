#include <QtGui/QApplication>
#include <QSplashScreen>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("FreeSWITCH");
    QCoreApplication::setOrganizationDomain("freeswitch.org");
    QCoreApplication::setApplicationName("FSPhone");

    QPixmap image(":/images/splash.png");
    QSplashScreen *splash = new QSplashScreen(image);
    splash->show();
    splash->showMessage("Loading, please wait...", Qt::AlignRight|Qt::AlignBottom, Qt::blue);

    QObject::connect(&g_FSHost, SIGNAL(ready()), splash, SLOT(close()));
    MainWindow w;    
    QObject::connect(&g_FSHost, SIGNAL(ready()), &w, SLOT(show()));
    g_FSHost.start();
    return a.exec();
}
