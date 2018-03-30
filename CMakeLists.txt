cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(DwarfTherapist)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Qml Widgets)

include(CPack)
include(InstallRequiredSystemLibraries)
include_directories(src thirdparty/qtcolorpicker-2.6)

set(EXE_NAME "DwarfTherapist")
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DNOMINMAX)
    set(SOURCES ${SOURCES} DwarfTherapist.rc src/dfinstancewindows.cpp)
    set(LIBS user32 psapi)
elseif(UNIX)
    set(SOURCES ${SOURCES} src/dfinstancenix.cpp)
    if(APPLE)
        set(SOURCES ${SOURCES} DwarfTherapist.rc src/dfinstanceosx.mm)
        find_library(FOUNDATION_LIB Foundation)
        find_library(SECURITY_LIB Security)
        find_library(APPKIT_LIB AppKit)
        set(LIBS ${FOUNDATION_LIB} ${SECURITY_LIB} ${APPKIT_LIB})

        set(MACOSX_BUNDLE_GUI_IDENTIFIER "io.github.dwarf-therapist.DwarfTherapist")
    else() # Linux
        set(SOURCES ${SOURCES} src/dfinstancelinux.cpp)
        set(EXE_NAME "dwarftherapist")
    endif()
else()
    message(FATAL_ERROR "unsupported target")
endif()

# Portable build
option(BUILD_PORTABLE "Build as portable application (look for files in the application directory)" OFF)
if(BUILD_PORTABLE)
    add_definitions(-DBUILD_PORTABLE)
endif()

add_executable(${EXE_NAME} WIN32 MACOSX_BUNDLE  
    src/itemgenericsubtype.h src/itemtoolsubtype.h src/basedock.cpp
    src/aboutdialog.cpp src/activity.cpp src/activityevent.cpp src/attribute.cpp
    src/belief.cpp src/caste.cpp src/customcolor.cpp src/customprofession.cpp
    src/defaultfonts.cpp src/dfinstance.cpp src/basetreedock.cpp
    src/dwarfdetailsdock.cpp src/equipmentoverviewdock.cpp
    src/gridviewdock.cpp src/healthlegenddock.cpp
    src/informationdock.cpp src/preferencesdock.cpp
    src/skilllegenddock.cpp src/thoughtsdock.cpp src/dtstandarditem.cpp
    src/dwarf.cpp src/dwarfdetailswidget.cpp src/dwarfstats.cpp
    src/dwarftherapist.cpp src/emotion.cpp src/emotiongroup.cpp src/equipwarn.cpp src/flagarray.cpp
    src/fortressentity.cpp src/gamedatareader.cpp src/attributecolumn.cpp
    src/beliefcolumn.cpp src/cellcolors.cpp
    src/currentjobcolumn.cpp src/customprofessioncolumn.cpp
    src/equipmentcolumn.cpp src/flagcolumn.cpp
    src/gridview.cpp src/happinesscolumn.cpp
    src/healthcolumn.cpp src/highestmoodcolumn.cpp
    src/itemtypecolumn.cpp src/laborcolumn.cpp
    src/professioncolumn.cpp src/rolecolumn.cpp
    src/skillcolumn.cpp src/spacercolumn.cpp
    src/superlaborcolumn.cpp src/trainedcolumn.cpp
    src/traitcolumn.cpp src/unitkillscolumn.cpp
    src/viewcolumn.cpp src/viewcolumncolors.cpp
    src/viewcolumnset.cpp src/viewcolumnsetcolors.cpp
    src/vieweditordialog.cpp src/weaponcolumn.cpp
    src/gridviewdialog.cpp src/histfigure.cpp src/iconchooser.cpp
    src/importexportdialog.cpp src/item.cpp src/itemammo.cpp
    src/itemarmorsubtype.cpp src/iteminstrument.cpp src/itemsubtype.cpp src/itemtool.cpp
    src/itemuniform.cpp src/itemweaponsubtype.cpp src/laboroptimizer.cpp
    src/laboroptimizerplan.cpp src/languages.cpp src/main.cpp src/mainwindow.cpp
    src/material.cpp src/memorylayout.cpp src/dwarfmodel.cpp
    src/dwarfmodelproxy.cpp src/multilabor.cpp src/notificationwidget.cpp
    src/notifierwidget.cpp src/optimizereditor.cpp src/optionsmenu.cpp
    src/plant.cpp src/preference.cpp src/rolepreference.cpp
    src/races.cpp src/reaction.cpp src/role.cpp
    src/rolecalcbase.cpp src/roledialog.cpp src/rolestats.cpp src/rotatedheader.cpp
    src/scriptdialog.cpp src/selectparentlayoutdialog.cpp src/skill.cpp
    src/squad.cpp src/statetableview.cpp src/superlabor.cpp src/syndrome.cpp
    src/thought.cpp src/trait.cpp src/truncatingfilelogger.cpp src/uberdelegate.cpp
    src/uniform.cpp src/unitbelief.cpp src/unitemotion.cpp src/unithealth.cpp
    src/unitwound.cpp src/updater.cpp src/viewmanager.cpp src/word.cpp
    src/itemarmor.cpp src/itemdefuniform.h src/dwarfjob.h src/uniform.h
    src/contextmenuhelper.cpp src/cellcolordef.h src/mood.cpp src/labor.cpp
    src/subthoughttypes.cpp src/plandetail.h src/eventfilterlineedit.cpp
    src/adaptivecolorfactory.cpp src/preferencecolumn.cpp src/standardpaths.cpp
    thirdparty/qtcolorpicker-2.6/qtcolorpicker.cpp resources.qrc
    ${SOURCES})
target_link_libraries(${EXE_NAME} Qt5::Widgets Qt5::Qml ${LIBS})

if (APPLE)
    set_target_properties(${EXE_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/osx/MacOSXBundleInfo.plist.in)
endif()

if(UNIX AND NOT APPLE)
    # Installation is currently Linux-only
    install(TARGETS ${EXE_NAME}
            RUNTIME DESTINATION bin)
    if (BUILD_PORTABLE)
        install(DIRECTORY share/
                DESTINATION share)
        install(FILES LICENSE.txt README.rst
                DESTINATION share/doc)
    else()
        install(DIRECTORY share/
                DESTINATION share/dwarftherapist)
        install(FILES LICENSE.txt README.rst
                DESTINATION share/doc/dwarftherapist)
        # desktop file and icons
        install(DIRECTORY dist/xdg/
                DESTINATION share)
    endif()
endif()
