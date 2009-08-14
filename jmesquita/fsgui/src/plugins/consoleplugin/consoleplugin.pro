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
!win32 {
    SVN_REV = $$system(svnversion -n .)
    isEmpty ( SVN_REV ) {
    SVN_REV = \\"Unknown or SVN not present\\"
    }
    DEFINES += 'FSGUI_SVN_VERSION=\\"$$SVN_REV\\"'
}
win32 {
    INCLUDEPATH += ../../../../../src/include/
}
######################################

TEMPLATE    = lib
TARGET      = consoleplugin
DESTDIR     = ../../../bin/plugins/
CONFIG     += plugin
INCLUDEPATH = ../../fsgui/ \
            ../../interfaces \
            ../../../../../../libs/esl/src/include/
HEADERS     = consoleplugin.h \
            sortfilterproxymodel.h \
            consoletabwidget.h
SOURCES     = consoleplugin.cpp \
            sortfilterproxymodel.cpp \
            consoletabwidget.cpp
FORMS     += centralWidget.ui \
            console_settings.ui \
            consoletabwidget.ui
LIBS      += -L../../interfaces \
            -linterfaces \
            -L../../../../../../libs/esl/ \
            -lesl
RESOURCES += ../../resources/resources.qrc
