TARGET = DwarfTherapist
VERSION = 0.0.1
MOC_DIR = tmp
UI_DIR = ui
TEMPLATE = app
INCLUDEPATH += inc \
	inc\models

SOURCES += src\main.cpp \
    src\mainwindow.cpp \
    src\dfinstance.cpp \
    src\dwarf.cpp \
    src\models\dwarfmodel.cpp

HEADERS += inc\mainwindow.h \
    inc\dfinstance.h \
    inc\win_structs.h \
    inc\dwarf.h \
    inc\models\dwarfmodel.h

FORMS += ui\mainwindow.ui

LIBS += -lpsapi
