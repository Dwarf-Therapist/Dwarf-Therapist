TEMPLATE = app
TARGET = DwarfTherapist
DESTDIR = ./bin/debug
QT += network
CONFIG += debug
win32:DEFINES += _WINDOWS QT_LARGEFILE_SUPPORT QT_DLL QT_NETWORK_LIB
INCLUDEPATH += $$(LIBCOLORPICKER)/src  \
    $$(LIBQXT)/include/Qxt/QxtGui \
    $$(LIBQXT)/include/Qxt/QxtCore \
    ./inc \
    ./inc/models \
    ./bin/debug \
    ./bin/release \
    ./ui \
    .

win32 {
    INCLUDEPATH += $$(QTDIR)/mkspecs/win32-msvc2008
}
LIBS += -$${LIBCOLORPICKER} \
    -$${LIBQXT}/lib \
    -lpsapi \
    -lQxtCored \
    -lQxtGuid

DEPENDPATH += .
MOC_DIR += bin/debug
OBJECTS_DIR += bin/debug
UI_DIR += ./ui
RCC_DIR += ./bing/debug

#Include file(s)
include(DwarfTherapist.pri)

#Translation files
TRANSLATIONS += ./dwarftherapist_en.ts

#Windows resource file
win32:RC_FILE = DwarfTherapist.rc
