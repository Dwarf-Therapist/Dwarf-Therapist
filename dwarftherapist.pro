TEMPLATE = app
TARGET = DwarfTherapist
lessThan(QT_MAJOR_VERSION, 5) {
    message(Setting up for Qt 4)
    QT += script
}
else {
    message(Setting up for Qt 5)
    QT += qml widgets
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

build_pass {
    win32 {
        message(Setting up for Windows)
        RC_FILE = DwarfTherapist.rc
        LIBS += -luser32
        LIBS += -lpsapi
        HEADERS += inc/dfinstancewindows.h
        SOURCES += src/dfinstancewindows.cpp

        DEFINES += NOMINMAX

        PWD = $$replace(PWD, /, \\)

        check_dirs.path = $$DESTDIR
        check_dirs.extra = if not exist \"$$DESTDIR\\share\\memory_layouts\\windows\" mkdir \"$$DESTDIR\\share\\memory_layouts\\windows\";

        copy_mem_layouts.path = $$DESTDIR
        copy_mem_layouts.extra = copy /Y \"$$PWD\\share\\memory_layouts\\windows\\*\" \".\\$$DESTDIR\\share\\memory_layouts\\windows\";

        INSTALLS += check_dirs
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

        share.path = Contents/MacOS/share
        QMAKE_BUNDLE_DATA += share

        memory_layouts.path = Contents/MacOS/share/memory_layouts/osx
        memory_layouts.files += $$files(share/memory_layouts/osx/*)
        QMAKE_BUNDLE_DATA += memory_layouts
    }
    else:unix {
        message(Setting up for Linux)
        isEmpty(PREFIX) {
            PREFIX = /usr/local
        }
        HEADERS += inc/dfinstancelinux.h
        SOURCES += src/dfinstancelinux.cpp

        target.path = $$PREFIX/bin
        INSTALLS += target

        bin.path = $$PREFIX/bin
        bin.files += dist/dwarftherapist
        INSTALLS += bin

        application.path = $$PREFIX/share/applications
        application.files += dist/dwarftherapist.desktop
        INSTALLS += application

        doc.path = $$PREFIX/share/doc/dwarftherapist
        doc.files += LICENSE.txt
        doc.files += README.rst
        system("printf 'Checking for pdflatex... '; if ! command -v pdflatex; then echo 'not found'; exit 1; fi") {
            manual.depends = "$$PWD/doc/Dwarf Therapist.tex" $$PWD/doc/images/*
            manual.commands = [ -d doc ] || mkdir doc;
            manual.commands += TEXINPUTS=".:$$PWD/doc/images:" pdflatex -output-directory=doc \"$<\"
            manual.target = "doc/Dwarf Therapist.pdf"
            QMAKE_EXTRA_TARGETS += manual
            POST_TARGETDEPS += "$$manual.target"
            doc.files += "$$manual.target"
        }
        INSTALLS += doc

        icon.path = $$PREFIX/share/pixmaps
        icon.files += img/dwarftherapist.png
        icon.files += img/dwarftherapist.xpm
        INSTALLS += icon

        memory_layouts.path = $$PREFIX/share/dwarftherapist/memory_layouts/linux
        memory_layouts.files += $$files(share/memory_layouts/linux/*)
        INSTALLS += memory_layouts
    }

    unix {
        HEADERS += inc/dfinstancenix.h
        SOURCES += src/dfinstancenix.cpp
    }
}

# Translation files
TRANSLATIONS += ./dwarftherapist_en.ts
HEADERS += inc/viewmanager.h \
    inc/version.h \
    inc/utils.h \
    inc/uberdelegate.h \
    inc/truncatingfilelogger.h \
    inc/trait.h \
    inc/statetableview.h \
    inc/squad.h \
    inc/skill.h \
    inc/scriptdialog.h \
    inc/rotatedheader.h \
    inc/profession.h \
    inc/optionsmenu.h \
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
    inc/selectparentlayoutdialog.h \
    inc/word.h \
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
    inc/itemweaponsubtype.h \
    inc/cp437codec.h \
    inc/rolestats.h \
    inc/contextmenuhelper.h \
    inc/belief.h \
    inc/unitbelief.h \
    inc/superlabor.h \
    inc/grid_view/superlaborcolumn.h \
    inc/grid_view/customprofessioncolumn.h \
    inc/multilabor.h \
    inc/eventfilterlineedit.h \
    inc/grid_view/beliefcolumn.h \
    inc/histfigure.h \
    inc/grid_view/unitkillscolumn.h \
    inc/dtstandarditem.h \
    inc/docks/informationdock.h \
    inc/docks/equipmentoverviewdock.h \
    inc/rolecalcminmax.h \
    inc/rolecalcrecenter.h \
    inc/rolecalcbase.h \
    inc/itemsubtype.h \
    inc/itemtoolsubtype.h \
    inc/itemgenericsubtype.h \
    inc/unitemotion.h \
    inc/emotion.h \
    inc/emotiongroup.h \
    inc/docks/basetreedock.h \
    inc/subthoughttypes.h \
    inc/mood.h \
    inc/grid_view/cellcolors.h \
    inc/grid_view/vieweditordialog.h \
    inc/grid_view/viewcolumncolors.h \
    inc/grid_view/viewcolumnsetcolors.h \
    inc/grid_view/cellcolordef.h
SOURCES += src/viewmanager.cpp \
    src/uberdelegate.cpp \
    src/truncatingfilelogger.cpp \
    src/statetableview.cpp \
    src/squad.cpp \
    src/skill.cpp \
    src/scriptdialog.cpp \
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
    src/word.cpp \
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
    src/rolestats.cpp \
    src/belief.cpp \
    src/grid_view/superlaborcolumn.cpp \
    src/superlabor.cpp \
    src/multilabor.cpp \
    src/grid_view/customprofessioncolumn.cpp \
    src/syndrome.cpp \
    src/grid_view/beliefcolumn.cpp \
    src/unitbelief.cpp \
    src/histfigure.cpp \
    src/grid_view/unitkillscolumn.cpp \
    src/dtstandarditem.cpp \
    src/docks/informationdock.cpp \
    src/docks/equipmentoverviewdock.cpp \
    src/rolecalcbase.cpp \
    src/itemarmorsubtype.cpp \
    src/flagarray.cpp \
    src/unitemotion.cpp \
    src/emotion.cpp \
    src/docks/basetreedock.cpp \
    src/grid_view/vieweditordialog.cpp \
    src/grid_view/cellcolors.cpp \
    src/grid_view/viewcolumncolors.cpp \
    src/grid_view/viewcolumnsetcolors.cpp
FORMS += ui/scriptdialog.ui \
    ui/pendingchanges.ui \
    ui/optionsmenu.ui \
    ui/mainwindow.ui \
    ui/importexportdialog.ui \
    ui/gridviewdock.ui \
    ui/gridviewdialog.ui \
    ui/dwarfdetailswidget.ui \
    ui/dwarfdetailsdock.ui \
    ui/customprofession.ui \
    ui/about.ui \
    ui/selectparentlayoutdialog.ui \
    ui/roledialog.ui \
    ui/optimizereditor.ui \
    ui/superlabor.ui \
    ui/vieweditor.ui
RESOURCES += \
    resources.qrc
