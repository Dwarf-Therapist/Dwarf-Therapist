TARGET = DwarfTherapist
VERSION = 0.1.0
UI_DIR = ui
TEMPLATE = app
INCLUDEPATH += inc \
    inc\models
SOURCES += src\main.cpp \
    src\mainwindow.cpp \
    src\dfinstance.cpp \
    src\dwarf.cpp \
    src\models\dwarfmodel.cpp \
    src/skill.cpp
HEADERS += inc\mainwindow.h \
    inc\dfinstance.h \
    inc\win_structs.h \
    inc\dwarf.h \
    inc\models\dwarfmodel.h \
    inc/utils.h \
    inc/skill.h
FORMS += ui\mainwindow.ui
LIBS += -lpsapi
RESOURCES += images.qrc
