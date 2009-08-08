TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH = ../fsgui/ \
    ../../../../../libs/esl/src/include/
HEADERS = interfaces.h \
    settingsdialog.h \
    servermanager.h \
    eslconnection.h
SOURCES = settingsdialog.cpp \
    servermanager.cpp \
    eslconnection.cpp
FORMS += settingsdialog.ui \
    servermanager.ui
LIBS+=-L../../../../../libs/esl/ \
    -lesl
RESOURCES += ../resources/resources.qrc
