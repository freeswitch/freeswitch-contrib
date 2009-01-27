# -------------------------------------------------
# Project created by QtCreator 2009-01-18T13:57:52
# -------------------------------------------------
TARGET = fs_gui
TEMPLATE = app

SOURCES += main.cpp \
    fsgui.cpp \
    conn_event_handler.cpp \
    serverlogin.cpp

INCLUDEPATH += ../../../../libs/esl/src/include

HEADERS += fsgui.h \
    conn_event_handler.h \
    serverlogin.h \
    esl.h \
    esl_threadmutex.h \
    esl_config.h

FORMS += fsgui.ui \
    serverlogin.ui

LIBS += -L../../../../libs/esl -lesl
