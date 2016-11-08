cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(DwarfTherapist)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Qml Widgets)

include(CPack)
include(InstallRequiredSystemLibraries)
include_directories(src src/models src/grid_view src/docks thirdparty/qtcolorpicker-2.6)

if(WIN32)
    set(SOURCES ${SOURCES} DwarfTherapist.rc src/dfinstancewindows.cpp)
    set(LIBS user32 psapi)
elseif(UNIX)
    set(SOURCES ${SOURCES} src/dfinstancenix.cpp)
    if(APPLE)
        set(SOURCES ${SOURCES} DwarfTherapist.rc src/dfinstanceosx.mm)
    else()
        set(SOURCES ${SOURCES} src/dfinstancelinux.cpp)

        #    system("printf 'Checking for pdflatex... '; if ! command -v pdflatex; then echo 'not found'; exit 1; fi") {
        #        manual.depends = "$$PWD/doc/Dwarf Therapist.tex" $$PWD/doc/images/*
        #        manual.commands = [ -d doc ] || mkdir doc;
        #        manual.commands += TEXINPUTS=".:$$PWD/doc/images:" pdflatex -output-directory=doc "$<"
        #        manual.target = "doc/Dwarf Therapist.pdf"
        #        QMAKE_EXTRA_TARGETS += manual
        #        POST_TARGETDEPS += "$$manual.target"
        #        doc.files += "$$manual.target"
        #    }
    endif()
else()
    message(FATAL_ERROR "unsupported target")
endif()

add_executable(DwarfTherapist WIN32 MACOSX_BUNDLE  
    src/itemgenericsubtype.h src/itemtoolsubtype.h src/docks/basedock.cpp
    src/aboutdialog.cpp src/activity.cpp src/activityevent.cpp src/attribute.cpp
    src/belief.cpp src/caste.cpp src/customcolor.cpp src/customprofession.cpp
    src/defaultfonts.cpp src/dfinstance.cpp src/docks/basetreedock.cpp
    src/docks/dwarfdetailsdock.cpp src/docks/equipmentoverviewdock.cpp
    src/docks/gridviewdock.cpp src/docks/healthlegenddock.cpp
    src/docks/informationdock.cpp src/docks/preferencesdock.cpp
    src/docks/skilllegenddock.cpp src/docks/thoughtsdock.cpp src/dtstandarditem.cpp
    src/dwarf.cpp src/dwarfdetailswidget.cpp src/dwarfstats.cpp
    src/dwarftherapist.cpp src/emotion.cpp src/emotiongroup.cpp src/equipwarn.cpp src/flagarray.cpp
    src/fortressentity.cpp src/gamedatareader.cpp src/grid_view/attributecolumn.cpp
    src/grid_view/beliefcolumn.cpp src/grid_view/cellcolors.cpp
    src/grid_view/currentjobcolumn.cpp src/grid_view/customprofessioncolumn.cpp
    src/grid_view/equipmentcolumn.cpp src/grid_view/flagcolumn.cpp
    src/grid_view/gridview.cpp src/grid_view/happinesscolumn.cpp
    src/grid_view/healthcolumn.cpp src/grid_view/highestmoodcolumn.cpp
    src/grid_view/itemtypecolumn.cpp src/grid_view/laborcolumn.cpp
    src/grid_view/professioncolumn.cpp src/grid_view/rolecolumn.cpp
    src/grid_view/skillcolumn.cpp src/grid_view/spacercolumn.cpp
    src/grid_view/superlaborcolumn.cpp src/grid_view/trainedcolumn.cpp
    src/grid_view/traitcolumn.cpp src/grid_view/unitkillscolumn.cpp
    src/grid_view/viewcolumn.cpp src/grid_view/viewcolumncolors.cpp
    src/grid_view/viewcolumnset.cpp src/grid_view/viewcolumnsetcolors.cpp
    src/grid_view/vieweditordialog.cpp src/grid_view/weaponcolumn.cpp
    src/gridviewdialog.cpp src/histfigure.cpp src/iconchooser.cpp
    src/importexportdialog.cpp src/item.cpp src/itemammo.cpp
    src/itemarmorsubtype.cpp src/iteminstrument.cpp src/itemsubtype.cpp src/itemtool.cpp
    src/itemuniform.cpp src/itemweaponsubtype.cpp src/laboroptimizer.cpp
    src/laboroptimizerplan.cpp src/languages.cpp src/main.cpp src/mainwindow.cpp
    src/material.cpp src/memorylayout.cpp src/models/dwarfmodel.cpp
    src/models/dwarfmodelproxy.cpp src/multilabor.cpp src/notificationwidget.cpp
    src/notifierwidget.cpp src/optimizereditor.cpp src/optionsmenu.cpp
    src/plant.cpp src/preference.cpp src/races.cpp src/reaction.cpp src/role.cpp
    src/rolecalcbase.cpp src/roledialog.cpp src/rolestats.cpp src/rotatedheader.cpp
    src/scriptdialog.cpp src/selectparentlayoutdialog.cpp src/skill.cpp
    src/squad.cpp src/statetableview.cpp src/superlabor.cpp src/syndrome.cpp
    src/thought.cpp src/trait.cpp src/truncatingfilelogger.cpp src/uberdelegate.cpp
    src/uniform.cpp src/unitbelief.cpp src/unitemotion.cpp src/unithealth.cpp
    src/unitwound.cpp src/updater.cpp src/viewmanager.cpp src/word.cpp
    src/itemarmor.cpp src/itemdefuniform.h src/dwarfjob.h src/uniform.h src/contextmenuhelper.cpp src/grid_view/cellcolordef.h src/mood.cpp src/labor.cpp src/subthoughttypes.cpp src/plandetail.h src/eventfilterlineedit.cpp src/roleaspect.h
    thirdparty/qtcolorpicker-2.6/qtcolorpicker.cpp resources.qrc
    ${SOURCES})
target_compile_features(DwarfTherapist PRIVATE cxx_generalized_initializers)
target_link_libraries(DwarfTherapist Qt5::Widgets Qt5::Qml)
