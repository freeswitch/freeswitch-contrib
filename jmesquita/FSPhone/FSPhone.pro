# #####################################
# version check qt
# #####################################
contains(QT_VERSION, ^4\.[0-4]\..*) {
    message("Cannot build FsGui with Qt version $$QT_VERSION.")
    error("Use at least Qt 4.5.")
}

QT += xml
TARGET = fsphone
TEMPLATE = app
INCLUDEPATH = ../../../src/include \
    ../../../libs/apr/include \
    ../../../libs/libteletone/src
LIBS = -L../../../.libs \
    -lfreeswitch \
    -lm \
    -lcrypt \
    -lrt

# This is here to comply with the default freeswitch installation
QMAKE_LFLAGS += -Wl,-rpath,/usr/local/freeswitch/lib
SOURCES += main.cpp \
    mainwindow.cpp \
    fshost.cpp
HEADERS += mainwindow.h \
    fshost.h
FORMS += mainwindow.ui
