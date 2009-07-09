TARGET = DwarfTherapist
VERSION = 0.0.1
MOC_DIR = tmp
UI_DIR = ui
TEMPLATE = app
INCLUDEPATH = inc
OBJECTS_DIR = tmp

SOURCES += src\main.cpp \
    src\mainwindow.cpp \
    src\dfinstance.cpp \
    src\dwarf.cpp

HEADERS += inc\mainwindow.h \
    inc\dfinstance.h \
    inc\win_structs.h \
    inc\dwarf.h

FORMS += ui\mainwindow.ui

LIBS += -lpsapi
