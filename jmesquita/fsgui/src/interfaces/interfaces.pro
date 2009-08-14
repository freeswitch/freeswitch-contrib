######################################
# version check qt
######################################
contains(QT_VERSION, ^4\.[0-4]\..*) {
    message("Cannot build FsGui with Qt version $$QT_VERSION.")
    error("Use at least Qt 4.5.")
}
######################################

#####################################
# Generates the version of FsGui
# based on SVN Revision
#####################################
SVN_REV = $$system(svnversion -n .)
isEmpty ( SVN_REV ) {
    SVN_REV = \\"Unknown or SVN not present\\"
}
DEFINES += 'FSGUI_SVN_VERSION=\\"$$SVN_REV\\"'
######################################

#####################################
# ESL must be compiled
#####################################
!exists ( ../../../../../libs/esl/libesl.a ) {
    error("lib esl must be compiled before FsGui")
}
#####################################


TEMPLATE = lib
CONFIG += staticlib
VERSION = 0.1
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
