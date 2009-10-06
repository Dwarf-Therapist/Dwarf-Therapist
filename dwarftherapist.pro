TEMPLATE = app
TARGET = DwarfTherapist
CONFIG += debug_and_release
CONFIG:debug {
	DESTDIR = ./bin/debug
}
CONFIG:release {
	DESTDIR = ./bin/release
}
QT += network
INCLUDEPATH += ./thirdparty/qtcolorpicker-2.6 \
	./thirdparty/libqxt-0.5.0 \
    ./inc \
    ./inc/models \
    ./inc/grid_view \
    ./inc/docks \
    ./ui 

#Include file(s)
include(dwarftherapist.pri)

win32 {
	message(Setting up for Windows)
    DEFINES += _WINDOWS QT_LARGEFILE_SUPPORT QT_DLL QT_NETWORK_LIB
    INCLUDEPATH += $$(QTDIR)/mkspecs/win32-msvc2008
    HEADERS += ./inc/dfinstancewindows.h
    SOURCES += ./src/dfinstancewindows.cpp
    LIBS += -lpsapi
}
linux-g++ {
	message(Setting up for Linux)
	DEFINES += _LINUX
    INCLUDEPATH += $$(QTDIR)/mkspecs/linux-g++
	HEADERS += ./inc/dfinstancelinux.h
	SOURCES += ./src/dfinstancelinux.cpp
}
macx {
	message(Setting up for OSX)
	DEFINES += _OSX BUILD_QXT
	INCLUDEPATH += $$(QTDIR)/mkspecs/macx-xcode
	HEADERS += ./inc/dfinstanceosx.h
	SOURCES += ./src/dfinstanceosx.cpp
}

DEPENDPATH += .
MOC_DIR += bin/debug
OBJECTS_DIR += bin/debug
UI_DIR += ./ui
RCC_DIR += ./bin/debug

#Translation files
TRANSLATIONS += ./dwarftherapist_en.ts

#Windows resource file
win32:RC_FILE = DwarfTherapist.rc
