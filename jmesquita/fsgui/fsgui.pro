# -------------------------------------------------
# Project created by QtCreator 2009-01-18T13:57:52
# -------------------------------------------------
TARGET = fs_gui
TEMPLATE = app
SOURCES += main.cpp \
    fs_gui.cpp \
    server_manager.cpp
INCLUDEPATH += ../../../libs/esl/src/include/
HEADERS += fs_gui.h \
    server_manager.h
FORMS += fs_gui.ui \
    server_manager.ui
LIBS += -L../../../libs/esl \
    -lesl
