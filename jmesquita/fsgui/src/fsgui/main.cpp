#include <QtGui/QApplication>
#include "appmanager.h"

int main(int argc, char *argv[])
{
    //Q_INIT_RESOURCE(sdi);
    QApplication a(argc, argv);
    AppManager w;
    return a.exec();
}
