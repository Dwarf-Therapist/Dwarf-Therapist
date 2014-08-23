TEMPLATE = app
TARGET = DwarfTherapist
QT += concurrent widgets
lessThan(QT_MAJOR_VERSION, 5) {
    QT += script
}
else {
    QT += qml
}
CONFIG += debug_and_release \
    warn_on

QMAKE_CXXFLAGS += $$(CXXFLAGS)
QMAKE_LFLAGS += $$(LDFLAGS)

INCLUDEPATH += inc \
    inc$${DIR_SEPARATOR}models \
    inc$${DIR_SEPARATOR}grid_view \
    inc$${DIR_SEPARATOR}docks \
    ui \
    thirdparty/qtcolorpicker-2.6

Release:DESTDIR = release
Release:OBJECTS_DIR = release/.obj
Release:MOC_DIR = release/.moc
Release:UI_DIR = release/.ui

Debug:DESTDIR = debug
Debug:OBJECTS_DIR = debug/.obj
Debug:MOC_DIR = debug/.moc
Debug:UI_DIR = debug/.ui

win32 {
    message(Setting up for Windows)
    RC_FILE = DwarfTherapist.rc
    LIBS += -luser32
    LIBS += -lpsapi
    HEADERS += inc/dfinstancewindows.h
    SOURCES += src/dfinstancewindows.cpp    

    DEFINES += NOMINMAX

     #setup_files.path = $$DESTDIR
    #setup_files.extra = ROBOCOPY /MIR "etc" ".\\$$DESTDIR\\etc";

    check_log.path = $$DESTDIR
    check_log.extra = if not exist $$DESTDIR\\log mkdir "$$DESTDIR\\log";

    check_dirs.path = $$DESTDIR
    check_dirs.extra = if not exist $$DESTDIR\\etc\\memory_layouts\\windows mkdir "$$DESTDIR\\etc\\memory_layouts\\windows";

    copy_game_data.path = $$DESTDIR
    copy_game_data.extra = copy /Y "etc\\game_data.ini" ".\\$$DESTDIR\\etc";

    copy_mem_layouts.path = $$DESTDIR
    copy_mem_layouts.extra = copy /Y "etc\\memory_layouts\\windows\\*" ".\\$$DESTDIR\\etc\\memory_layouts\\windows";

    INSTALLS += check_log
    INSTALLS += check_dirs
    INSTALLS += copy_game_data
    INSTALLS += copy_mem_layouts
}
else:macx {
    message(Setting up for OSX)
    HEADERS += ./inc/dfinstanceosx.h
    OBJECTIVE_SOURCES += ./src/dfinstanceosx.mm
    ICON = hammer.icns
    LIBS += -framework Cocoa
    LIBS += -framework Carbon
    LIBS += -framework Security
    LIBS += -framework SecurityFoundation
    LIBS += -framework Foundation
    LIBS += -framework ApplicationServices
    LIBS += -framework Accelerate

    log.path = Contents/MacOS/log
    QMAKE_BUNDLE_DATA += log

    etc.path = Contents/MacOS/etc
    etc.files += etc/game_data.ini
    QMAKE_BUNDLE_DATA += etc

    layouts.path = Contents/MacOS/etc/memory_layouts/osx
    layouts.files += etc/memory_layouts/osx/v0.40.04_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.05_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.06_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.07_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.08_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.09_osx.ini
    layouts.files += etc/memory_layouts/osx/v0.40.10_osx.ini
    QMAKE_BUNDLE_DATA += layouts
}
else:unix {
    message(Setting up for Linux)
    HEADERS += inc/dfinstancelinux.h
    SOURCES += src/dfinstancelinux.cpp

    target.path = /usr/bin
    INSTALLS += target

    bin.path = /usr/bin
    bin.files += dist/dwarftherapist
    INSTALLS += bin

    bin_mod.path = /usr/bin
    bin_mod.extra = chmod +x $(INSTALL_ROOT)/usr/bin/dwarftherapist
    bin_mod.depends = install_bin
    INSTALLS += bin_mod

    application.path = /usr/share/applications
    application.files += dist/dwarftherapist.desktop
    INSTALLS += application

    doc.path = /usr/share/doc/dwarftherapist
    doc.files += LICENSE.txt
    doc.files += README.md
    INSTALLS += doc

    icon.path = /usr/share/pixmaps
    icon.files += img/dwarftherapist.png
    icon.files += img/dwarftherapist.xpm
    INSTALLS += icon

    memory_layouts.path = /usr/share/dwarftherapist/etc/memory_layouts/linux
    memory_layouts.files += etc/memory_layouts/linux/*
    INSTALLS += memory_layouts

    game_data.path = /usr/share/dwarftherapist/etc
    game_data.files += etc/game_data.ini
    INSTALLS += game_data
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
    inc/aboutdialog.h \
    inc/models/dwarfmodelproxy.h \
    inc/models/dwarfmodel.h \
    inc/grid_view/viewcolumnset.h \
    inc/grid_view/viewcolumn.h \
    inc/grid_view/traitcolumn.h \
    inc/grid_view/spacercolumn.h \
    inc/grid_view/skillcolumn.h \
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
    inc/word.h \
    inc/raws/rawnode.h \
    inc/raws/rawobject.h \
    inc/raws/rawreader.h \
    inc/raws/rawobjectlist.h \
    inc/attribute.h \
    inc/grid_view/flagcolumn.h \
    inc/grid_view/rolecolumn.h \
    inc/role.h \
    inc/dwarfstats.h \
    inc/global_enums.h \
    inc/grid_view/weaponcolumn.h \
    inc/roledialog.h \
    inc/reaction.h \
    inc/races.h \
    inc/languages.h \    
    inc/caste.h \    
    inc/fortressentity.h \
    inc/material.h \
    inc/plant.h \
    inc/docks/preferencesdock.h \
    inc/item.h \
    inc/laboroptimizer.h \
    inc/optimizereditor.h \
    inc/laboroptimizerplan.h \
    inc/preference.h \
    inc/flagarray.h \
    inc/plandetail.h \
    inc/roleaspect.h \
    inc/grid_view/professioncolumn.h \
    inc/sortabletableitems.h \
    inc/iconchooser.h \
    inc/grid_view/highestmoodcolumn.h \
    inc/thought.h \
    inc/docks/thoughtsdock.h \
    inc/grid_view/trainedcolumn.h \
    inc/unithealth.h \
    inc/unitwound.h \
    inc/healthinfo.h \
    inc/grid_view/healthcolumn.h \
    inc/docks/healthlegenddock.h \
    inc/bodypart.h \
    inc/bodypartlayer.h \
    inc/healthcategory.h \
    inc/bodypartdamage.h \
    inc/currentyearsearchjob.h \
    inc/defaultfonts.h \
    inc/docks/basedock.h \
    inc/syndrome.h \
    inc/itemweapon.h \
    inc/itemarmor.h \
    inc/grid_view/equipmentcolumn.h \
    inc/grid_view/itemtypecolumn.h \
    inc/itemdefuniform.h \
    inc/uniform.h \
    inc/itemammo.h \
    inc/itemarmorsubtype.h \
    inc/itemammosubtype.h \
    inc/itemweaponsubtype.h \
    inc/cp437codec.h \
    inc/rolestats.h \
    inc/ecdf.h \
    inc/contextmenuhelper.h \
    inc/belief.h \
    inc/unitbelief.h \
    inc/superlabor.h \
    inc/grid_view/superlaborcolumn.h \
    inc/grid_view/customprofessioncolumn.h \
    inc/multilabor.h \
    inc/eventfilterlineedit.h
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
    src/word.cpp \
    src/raws/rawreader.cpp \
    src/grid_view/flagcolumn.cpp \
    src/grid_view/rolecolumn.cpp \
    src/dwarfstats.cpp \
    src/role.cpp \
    src/attribute.cpp \
    src/grid_view/weaponcolumn.cpp \
    src/roledialog.cpp \
    src/races.cpp \
    src/languages.cpp \    
    src/caste.cpp \
    src/reaction.cpp \
    src/fortressentity.cpp \
    src/material.cpp \
    src/plant.cpp \
    src/docks/preferencesdock.cpp \
    src/laboroptimizer.cpp \
    src/optimizereditor.cpp \
    src/laboroptimizerplan.cpp \
    src/preference.cpp \
    src/grid_view/professioncolumn.cpp \
    src/iconchooser.cpp \
    src/grid_view/highestmoodcolumn.cpp \
    src/trait.cpp \
    src/docks/thoughtsdock.cpp \
    src/thought.cpp \
    src/grid_view/trainedcolumn.cpp \
    src/unithealth.cpp \
    src/unitwound.cpp \
    src/grid_view/healthcolumn.cpp \
    src/docks/healthlegenddock.cpp \
    src/defaultfonts.cpp \
    src/grid_view/equipmentcolumn.cpp \
    src/grid_view/itemtypecolumn.cpp \
    src/item.cpp \
    src/uniform.cpp \
    src/itemweaponsubtype.cpp \
    src/ecdf.cpp \
    src/rolestats.cpp \
    src/belief.cpp \
    src/grid_view/superlaborcolumn.cpp \
    src/superlabor.cpp \
    src/multilabor.cpp \
    src/grid_view/customprofessioncolumn.cpp
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
    ui/selectparentlayoutdialog.ui \
    ui/roledialog.ui \
    ui/optimizereditor.ui \
    ui/superlabor.ui
RESOURCES += images.qrc
