TEMPLATE = app
TARGET = DwarfTherapist
QT += network \
    script
CONFIG(debug, debug|release) { 
    message(Debug Mode)
    DESTDIR = bin/debug
    MOC_DIR = bin/debug
    UI_DIR = bin/debug
    RCC_DIR = bin/debug
    OBJECTS_DIR = bin/debug
}
else { 
    message(Release Mode)
    DESTDIR = bin/release
    MOC_DIR = bin/release
    UI_DIR = bin/release
    RCC_DIR = bin/release
    OBJECTS_DIR = bin/release
}
INCLUDEPATH += inc \
    inc/models \
    inc/grid_view \
    inc/docks \
    ui \
    thirdparty/qtcolorpicker-2.6
win32 { 
    message(Setting up for Windows)
    RC_FILE = DwarfTherapist.rc
    LIBS += -lpsapi
    HEADERS += inc/dfinstancewindows.h
    SOURCES += src/dfinstancewindows.cpp
}
else:macx { 
    message(Setting up for OSX)
    HEADERS += ./inc/dfinstanceosx.h
    SOURCES += ./src/dfinstanceosx.cpp
    ICON = hammer.ico
}
else:unix {
    message(Setting up for Linux)
    HEADERS += inc/dfinstancelinux.h
    SOURCES += src/dfinstancelinux.cpp

    message(Setting up for Linux Install)
    target.path = /usr/bin
    INSTALLS += target

    bin.path = /usr/bin
    bin.files += bin/release/DwarfTherapist
    bin.files += dist/dwarftherapist
    INSTALLS += bin

    application.path = /usr/share/applications
    application.files = dist/dwarftherapist.desktop
    INSTALLS += application

    doc.path = /usr/share/doc/dwarftherapist
    doc.extra = cp COPYRIGHT $(INSTALL_ROOT)/usr/share/doc/dwarftherapist/copyright; cp README $(INSTALL_ROOT)/usr/share/doc/dwarftherapist/README.Debian
    INSTALLS += doc

    icon.path = /usr/share/pixmaps
    icon.files += img/dwarftherapist.png
    icon.files += img/dwarftherapist.xpm
    INSTALLS += icon

    share.path = /usr/share/dwarftherapist
    share.extra = mkdir -p $(INSTALL_ROOT)/usr/share/dwarftherapist/etc; \
        mkdir -p $(INSTALL_ROOT)/usr/share/dwarftherapist/etc/memory_layouts; \
        mkdir -p $(INSTALL_ROOT)/usr/share/dwarftherapist/etc/memory_layouts/linux; \
        cp etc/game_data.ini $(INSTALL_ROOT)/usr/share/dwarftherapist/etc; \
        cp etc/memory_layouts/linux/* $(INSTALL_ROOT)/usr/share/dwarftherapist/etc/memory_layouts/linux

    INSTALLS += share
}

# Translation files
TRANSLATIONS += ./dwarftherapist_en.ts
HEADERS += inc/win_structs.h \
    inc/viewmanager.h \
    inc/version.h \
    inc/utils.h \
    inc/uberdelegate.h \
    inc/truncatingfilelogger.h \
    inc/translationvectorsearchjob.h \
    inc/trait.h \
    inc/stonevectorsearchjob.h \
    inc/statetableview.h \
    inc/squad.h \
    inc/skill.h \
    inc/scriptdialog.h \
    inc/scannerthread.h \
    inc/scannerjob.h \
    inc/scanner.h \
    inc/rotatedheader.h \
    inc/profession.h \
    inc/optionsmenu.h \
    inc/nullterminatedstringsearchjob.h \
    inc/militarypreference.h \
    inc/memorysegment.h \
    inc/memorylayout.h \
    inc/mainwindow.h \
    inc/labor.h \
    inc/importexportdialog.h \
    inc/gridviewdialog.h \
    inc/gamedatareader.h \
    inc/dwarftherapist.h \
    inc/dwarfjob.h \
    inc/dwarfdetailswidget.h \
    inc/dwarf.h \
    inc/dfinstance.h \
    inc/defines.h \
    inc/customprofession.h \
    inc/customcolor.h \
    inc/cp437codec.h \
    inc/aboutdialog.h \
    inc/models/dwarfmodelproxy.h \
    inc/models/dwarfmodel.h \
    inc/grid_view/viewcolumnset.h \
    inc/grid_view/viewcolumn.h \
    inc/grid_view/traitcolumn.h \
    inc/grid_view/spacercolumn.h \
    inc/grid_view/skillcolumn.h \
    inc/grid_view/militarypreferencecolumn.h \
    inc/grid_view/laborcolumn.h \
    inc/grid_view/currentjobcolumn.h \
    inc/grid_view/happinesscolumn.h \
    inc/grid_view/gridview.h \
    inc/grid_view/columntypes.h \
    inc/grid_view/attributecolumn.h \
    inc/docks/skilllegenddock.h \
    inc/docks/gridviewdock.h \
    inc/docks/dwarfdetailsdock.h \
    thirdparty/qtcolorpicker-2.6/qtcolorpicker.h \
    inc/vectorsearchjob.h \
    inc/dwarfraceindexsearchjob.h \
    inc/creaturevectorsearchjob.h \
    inc/positionvectorsearchjob.h \
    inc/stdstringsearchjob.h \
    inc/selectparentlayoutdialog.h \
    inc/layoutcreator.h \
    inc/narrowingvectorsearchjob.h \
    inc/squadvectorsearchjob.h \
    inc/word.h
SOURCES += src/viewmanager.cpp \
    src/uberdelegate.cpp \
    src/truncatingfilelogger.cpp \
    src/statetableview.cpp \
    src/squad.cpp \
    src/skill.cpp \
    src/scriptdialog.cpp \
    src/scannerjob.cpp \
    src/scanner.cpp \
    src/rotatedheader.cpp \
    src/optionsmenu.cpp \
    src/memorylayout.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/importexportdialog.cpp \
    src/gridviewdialog.cpp \
    src/gamedatareader.cpp \
    src/dwarftherapist.cpp \
    src/dwarfdetailswidget.cpp \
    src/dwarf.cpp \
    src/dfinstance.cpp \
    src/customprofession.cpp \
    src/customcolor.cpp \
    src/aboutdialog.cpp \
    src/models/dwarfmodelproxy.cpp \
    src/models/dwarfmodel.cpp \
    src/grid_view/viewcolumnset.cpp \
    src/grid_view/viewcolumn.cpp \
    src/grid_view/traitcolumn.cpp \
    src/grid_view/spacercolumn.cpp \
    src/grid_view/skillcolumn.cpp \
    src/grid_view/militarypreferencecolumn.cpp \
    src/grid_view/laborcolumn.cpp \
    src/grid_view/currentjobcolumn.cpp \
    src/grid_view/happinesscolumn.cpp \
    src/grid_view/gridview.cpp \
    src/grid_view/attributecolumn.cpp \
    src/docks/skilllegenddock.cpp \
    src/docks/gridviewdock.cpp \
    src/docks/dwarfdetailsdock.cpp \
    thirdparty/qtcolorpicker-2.6/qtcolorpicker.cpp \
    src/selectparentlayoutdialog.cpp \
    src/layoutcreator.cpp \
    src/word.cpp
FORMS += ui/scriptdialog.ui \
    ui/scannerdialog.ui \
    ui/pendingchanges.ui \
    ui/optionsmenu.ui \
    ui/mainwindow.ui \
    ui/importexportdialog.ui \
    ui/gridviewdock.ui \
    ui/gridviewdialog.ui \
    ui/dwarfdetailswidget.ui \
    ui/dwarfdetailsdock.ui \
    ui/customprofession.ui \
    ui/columneditdialog.ui \
    ui/about.ui \
    ui/selectparentlayoutdialog.ui
RESOURCES += images.qrc
