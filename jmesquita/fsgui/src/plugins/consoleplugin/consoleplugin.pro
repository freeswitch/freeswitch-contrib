TEMPLATE = lib
TARGET = ../../../bin/plugins/consoleplugin
CONFIG += plugin
INCLUDEPATH = ../../fsgui/ \
    ../../interfaces \
    ../../../../../../libs/esl/src/include/
HEADERS = consoleplugin.h \
    sortfilterproxymodel.h \
    consoletabwidget.h
SOURCES = consoleplugin.cpp \
    sortfilterproxymodel.cpp \
    consoletabwidget.cpp
FORMS += centralWidget.ui \
    console_settings.ui \
    consoletabwidget.ui
LIBS += -L \
    ../../interfaces \
    -linterfaces \
    -L../../../../../../libs/esl/ \
    -lesl
RESOURCES += ../../resources/resources.qrc
