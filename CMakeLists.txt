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

find_package(Qt5 REQUIRED COMPONENTS Qml Widgets Concurrent)

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
    src/aboutdialog.cpp
    src/activity.cpp
    src/activityevent.cpp
    src/adaptivecolorfactory.cpp
    src/attributecolumn.cpp
    src/attribute.cpp
    src/basedock.cpp
    src/basetreedock.cpp
    src/beliefcolumn.cpp
    src/belief.cpp
    src/caste.cpp
    src/cellcolordef.h
    src/cellcolors.cpp
    src/contextmenuhelper.cpp
    src/currentjobcolumn.cpp
    src/customcolor.cpp
    src/customprofessioncolumn.cpp
    src/customprofession.cpp
    src/defaultfonts.cpp
    src/defaultroleweight.cpp
    src/dfinstance.cpp
    src/dtstandarditem.cpp
    src/dwarf.cpp
    src/dwarfdetailsdock.cpp
    src/dwarfdetailswidget.cpp
    src/dwarfjob.h
    src/dwarfmodel.cpp
    src/dwarfmodelproxy.cpp
    src/dwarfstats.cpp
    src/dwarftherapist.cpp
    src/emotion.cpp
    src/emotiongroup.cpp
    src/equipmentcolumn.cpp
    src/equipmentoverviewdock.cpp
    src/equipwarn.cpp
    src/eventfilterlineedit.cpp
    src/flagarray.cpp
    src/flagcolumn.cpp
    src/focuscolumn.cpp
    src/fortressentity.cpp
    src/gamedatareader.cpp
    src/gridview.cpp
    src/gridviewdialog.cpp
    src/gridviewdock.cpp
    src/happinesscolumn.cpp
    src/healthcolumn.cpp
    src/healthlegenddock.cpp
    src/highestmoodcolumn.cpp
    src/histfigure.cpp
    src/iconchooser.cpp
    src/importexportdialog.cpp
    src/informationdock.cpp
    src/itemammo.cpp
    src/itemarmor.cpp
    src/itemarmorsubtype.cpp
    src/item.cpp
    src/itemdefuniform.h
    src/iteminstrument.cpp
    src/itemsubtype.cpp
    src/itemtool.cpp
    src/itemtoolsubtype.cpp
    src/itemtypecolumn.cpp
    src/itemuniform.cpp
    src/itemweaponsubtype.cpp
    src/laborcolumn.cpp
    src/labor.cpp
    src/laboroptimizer.cpp
    src/laboroptimizerplan.cpp
    src/languages.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/material.cpp
    src/memorylayout.cpp
    src/mood.cpp
    src/multilabor.cpp
    src/needcolumn.cpp
    src/needsdock.cpp
    src/notificationwidget.cpp
    src/notifierwidget.cpp
    src/optimizereditor.cpp
    src/optionsmenu.cpp
    src/plandetail.h
    src/plant.cpp
    src/preferencecolumn.cpp
    src/preference.cpp
    src/preferencepickerdialog.cpp
    src/preferencesdock.cpp
    src/professioncolumn.cpp
    src/races.cpp
    src/reaction.cpp
    src/rolecalcbase.cpp
    src/rolecolumn.cpp
    src/role.cpp
    src/roledialog.cpp
    src/rolemodel.cpp
    src/rolepreference.cpp
    src/rolepreferencemodel.cpp
    src/rolestats.cpp
    src/rotatedheader.cpp
    src/scriptdialog.cpp
    src/searchfiltertreeview.cpp
    src/selectparentlayoutdialog.cpp
    src/skillcolumn.cpp
    src/skill.cpp
    src/skilllegenddock.cpp
    src/sortfilterproxymodel.cpp
    src/spacercolumn.cpp
    src/squad.cpp
    src/standardpaths.cpp
    src/statetableview.cpp
    src/subthoughttypes.cpp
    src/superlaborcolumn.cpp
    src/superlabor.cpp
    src/syndrome.cpp
    src/thought.cpp
    src/thoughtsdock.cpp
    src/trainedcolumn.cpp
    src/traitcolumn.cpp
    src/trait.cpp
    src/truncatingfilelogger.cpp
    src/uberdelegate.cpp
    src/uniform.cpp
    src/uniform.h
    src/unitbelief.cpp
    src/unitemotion.cpp
    src/unithealth.cpp
    src/unitkillscolumn.cpp
    src/unitneed.cpp
    src/unitwound.cpp
    src/updater.cpp
    src/viewcolumncolors.cpp
    src/viewcolumn.cpp
    src/viewcolumnsetcolors.cpp
    src/viewcolumnset.cpp
    src/vieweditordialog.cpp
    src/viewmanager.cpp
    src/weaponcolumn.cpp
    src/word.cpp
    thirdparty/qtcolorpicker-2.6/qtcolorpicker.cpp
    resources.qrc
    ${SOURCES})
target_link_libraries(${EXE_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Qml Qt5::Network Qt5::Concurrent ${LIBS})

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
