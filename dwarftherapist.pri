# color picker dependency
COLOR_PICKER_HEADERS += ./thirdparty/qtcolorpicker-2.6/qtcolorpicker.h 
COLOR_PICKER_SOURCES += ./thirdparty/qtcolorpicker-2.6/qtcolorpicker.cpp

# libqxt dependency
LIBQXT_HEADERS += ./thirdparty/libqxt-0.5.0/qxtlogstream.h \
	./thirdparty/libqxt-0.5.0/qxtlogger.h \
	./thirdparty/libqxt-0.5.0/qxtlogger_p.h 
LIBQXT_SOURCES += ./thirdparty/libqxt-0.5.0/qxtlogstream.cpp \
	./thirdparty/libqxt-0.5.0/qxtlogger.cpp \
	./thirdparty/libqxt-0.5.0/qxtloggerengine.cpp \
	./thirdparty/libqxt-0.5.0/logengines/qxtabstractiologgerengine.cpp \
	./thirdparty/libqxt-0.5.0/logengines/qxtabstractfileloggerengine.cpp \
	./thirdparty/libqxt-0.5.0/logengines/qxtbasicstdloggerengine.cpp

#Header files
HEADERS += $$COLOR_PICKER_HEADERS \
	$$LIBQXT_HEADERS \
	./inc/customcolor.h \
    ./inc/models/dwarfmodel.h \
    ./inc/models/dwarfmodelproxy.h \
    ./inc/mainwindow.h \
    ./inc/docks/dwarfdetailsdock.h \
    ./inc/docks/gridviewdock.h \
    ./inc/docks/skilllegenddock.h \
    ./inc/docks/viewcolumnsetdock.h \
    ./inc/aboutdialog.h \
    ./inc/customprofessionsexportdialog.h \
    ./inc/gridviewdialog.h \
    ./inc/optionsmenu.h \
    ./inc/viewcolumnsetdialog.h \
    ./inc/cp437codec.h \
    ./inc/defines.h \
    ./inc/dwarftherapist.h \
    ./inc/gamedatareader.h \
    ./inc/memorylayout.h \
    ./inc/truncatingfilelogger.h \
    ./inc/utils.h \
    ./inc/version.h \
    ./inc/win_structs.h \
    ./inc/customprofession.h \
    ./inc/dfinstance.h \
    ./inc/dwarf.h \
    ./inc/labor.h \
    ./inc/skill.h \
    ./inc/grid_view/columntypes.h \
    ./inc/grid_view/happinesscolumn.h \
    ./inc/grid_view/laborcolumn.h \
    ./inc/grid_view/skillcolumn.h \
    ./inc/grid_view/spacercolumn.h \
    ./inc/grid_view/viewcolumn.h \
    ./inc/grid_view/gridview.h \
    ./inc/statetableview.h \
    ./inc/grid_view/viewcolumnset.h \
    ./inc/viewmanager.h \
    ./inc/rotatedheader.h \
    ./inc/uberdelegate.h \
    ./inc/scannerthread.h \
    ./inc/scanner.h \
    ./inc/scannerjob.h \
    ./inc/translationvectorsearchjob.h \
    ./inc/nullterminatedstringsearchjob.h \
    ./inc/stonevectorsearchjob.h \
    ./inc/memorysegment.h \

#Source files
SOURCES += $$COLOR_PICKER_SOURCES \
	$$LIBQXT_SOURCES \
	./src/customcolor.cpp \
    ./src/models/dwarfmodel.cpp \
    ./src/models/dwarfmodelproxy.cpp \
    ./src/mainwindow.cpp \
    ./src/docks/dwarfdetailsdock.cpp \
    ./src/docks/gridviewdock.cpp \
    ./src/docks/skilllegenddock.cpp \
    ./src/docks/viewcolumnsetdock.cpp \
    ./src/aboutdialog.cpp \
    ./src/customprofessionsexportdialog.cpp \
    ./src/gridviewdialog.cpp \
    ./src/optionsmenu.cpp \
    ./src/viewcolumnsetdialog.cpp \
    ./src/dwarftherapist.cpp \
    ./src/gamedatareader.cpp \
    ./src/main.cpp \
    ./src/memorylayout.cpp \
    ./src/customprofession.cpp \
    ./src/dfinstance.cpp \
    ./src/dwarf.cpp \
    ./src/grid_view/happinesscolumn.cpp \
    ./src/grid_view/laborcolumn.cpp \
    ./src/grid_view/skillcolumn.cpp \
    ./src/grid_view/spacercolumn.cpp \
    ./src/grid_view/viewcolumn.cpp \
    ./src/grid_view/gridview.cpp \
    ./src/statetableview.cpp \
    ./src/grid_view/viewcolumnset.cpp \
    ./src/viewmanager.cpp \
    ./src/rotatedheader.cpp \
    ./src/uberdelegate.cpp \
    ./src/scanner.cpp \
    ./src/scannerjob.cpp \
    ./src/skill.cpp

#Forms
FORMS += ./ui/about.ui \
    ./ui/columneditdialog.ui \
    ./ui/customprofession.ui \
    ./ui/customprofessionsexportdialog.ui \
    ./ui/dwarfdetailsdock.ui \
    ./ui/gridviewdialog.ui \
    ./ui/gridviewdock.ui \
    ./ui/mainwindow.ui \
    ./ui/optionsmenu.ui \
    ./ui/pendingchanges.ui \
    ./ui/scannerdialog.ui \
    ./ui/viewcolumnsetdialog.ui \
    ./ui/viewcolumnsetdock.ui

#Resource file(s)
RESOURCES += ./images.qrc
