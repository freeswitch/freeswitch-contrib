#include <QtGui/QApplication>
#include "fs_gui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Cfsgui w;
    w.show();
    return a.exec();
}
