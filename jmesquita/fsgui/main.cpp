#include <QtGui/QApplication>
#include "fsgui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FSGui w;
    w.show();
    return a.exec();
}
