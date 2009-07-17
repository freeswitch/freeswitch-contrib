DESTDIR = ./bin/
message(The project will be installed in $$DESTDIR)
TARGET = fs_gui
TEMPLATE = app
MOC_DIR = ./build/
OBJECTS_DIR = ./build/
UI_DIR = ./build/
SOURCES += src/main.cpp \
    src/fs_gui.cpp \
    src/server_manager.cpp \
    src/esl_connection.cpp \
    src/consolepage.cpp \
    src/preferencesdialog.cpp
INCLUDEPATH += ../../../libs/esl/src/include \
    src/include
HEADERS += src/include/fs_gui.h \
    src/include/server_manager.h \
    src/include/esl_connection.h \
    src/include/consolepage.h \
    src/include/global_defines.h \
    src/include/preferencesdialog.h
FORMS += resources/fs_gui.ui \
    resources/server_manager.ui \
    resources/consolepage.ui \
    resources/preferencesdialog.ui
LIBS += -L../../../libs/esl \
    -lesl
RESOURCES += resources/images.qrc
