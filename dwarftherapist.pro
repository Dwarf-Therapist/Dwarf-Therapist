TEMPLATE = app
TARGET = DwarfTherapist
CONFIG += debug_and_release
DESTDIR = ./bin/debug
QT += network
INCLUDEPATH += $$(LIBCOLORPICKER)/src  \
    $$(LIBQXT)/QxtGui \
    $$(LIBQXT)/QxtCore \
    ./inc \
    ./inc/models \
    ./inc/grid_view \
    ./inc/docks \
    ./ui 

win32 {
    DEFINES += _WINDOWS QT_LARGEFILE_SUPPORT QT_DLL QT_NETWORK_LIB
    INCLUDEPATH += $$(QTDIR)/mkspecs/win32-msvc2008
    HEADERS += ./inc/dfinstancewindows.h
    SOURCES += ./src/dfinstancewindows.cpp
    LIBS += -lpsapi
}
unix {
	DEFINES += _LINUX
    INCLUDEPATH += $$(QTDIR)/mkspecs/linux-g++
	HEADERS += ./inc/dfinstancelinux.h
	SOURCES += ./src/dfinstancelinux.cpp
}
LIBS += -L"$$(LIBCOLORPICKER)/lib" \
    -lQxtCore \
    -lQxtGui \
    -lQtSolutions_ColorPicker-2.6

DEPENDPATH += .
MOC_DIR += bin/debug
OBJECTS_DIR += bin/debug
UI_DIR += ./ui
RCC_DIR += ./bin/debug

#Include file(s)
include(DwarfTherapist.pri)

#Translation files
TRANSLATIONS += ./dwarftherapist_en.ts

#Windows resource file
win32:RC_FILE = DwarfTherapist.rc
