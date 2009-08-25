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

#####################################
# Let user change the plugin location
# upon compilation
#####################################
isEmpty ( PLUGINDIR ) {
    PLUGINDIR = $${PWD}/../../bin/plugins
}
DEFINES += FSGUI_PLUGINDIR='\\"$${PLUGINDIR}\\"'
######################################

macx:ICON=../resources/mac_icon.icns

QT         += network
TARGET      = fsgui
DESTDIR     = ../../bin/
TEMPLATE    = app
INCLUDEPATH = ../interfaces \
            ../../../../../libs/esl/src/include/
SOURCES    += main.cpp \
            appmanager.cpp
HEADERS    += appmanager.h
LIBS       += -L../interfaces \
            -linterfaces \
            -L../../../../../libs/esl/ \
            -lesl
FORMS      += config_plugin_widget.ui
RESOURCES  += ../resources/resources.qrc
