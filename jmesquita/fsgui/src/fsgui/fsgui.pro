# version check qt
contains(QT_VERSION, ^4\.[0-4]\..*) { 
    message("Cannot build FsGui with Qt version $$QT_VERSION.")
    error("Use at least Qt 4.5.")
}
QT += network
TARGET = ../../bin/fsgui
TEMPLATE = app
INCLUDEPATH = ../interfaces \
    ../../../../../libs/esl/src/include/
DEFINES += 'FSGUI_SVN_VERSION=\\"$(shell svnversion -n .)\\"'
SOURCES += main.cpp \
    appmanager.cpp
HEADERS += appmanager.h
LIBS += -L../interfaces \
    -linterfaces \
    -L../../../../../libs/esl/ \
    -lesl
FORMS += config_plugin_widget.ui
RESOURCES += ../resources/resources.qrc
