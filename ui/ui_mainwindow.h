/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Thu Jul 16 20:31:26 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDockWidget>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "statetableview.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *act_connect_to_DF;
    QAction *act_exit;
    QAction *act_about;
    QAction *act_read_dwarves;
    QAction *act_scan_memory;
    QAction *action_apply_skill_changes_immedietly;
    QAction *act_expand_all;
    QAction *act_collapse_all;
    QAction *act_show_toolbutton_text;
    QAction *act_add_custom_profession;
    QAction *act_clear_pending_changes;
    QAction *act_commit_pending_changes;
    QAction *act_list_pending_changes;
    QAction *act_show_pending_labors_window;
    QWidget *main_widget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *le_filter;
    QPushButton *btn_filter;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lbl_group_by;
    QComboBox *cb_group_by;
    QSpacerItem *horizontalSpacer;
    QLabel *lbl_pending_changes;
    QLabel *label_2;
    StateTableView *stv;
    QMenuBar *menuBar;
    QMenu *menu_File;
    QMenu *menu_Help;
    QMenu *menuOptions;
    QMenu *menuProfessions;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QDockWidget *dock_pending_jobs_list;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QListWidget *list_pending;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(848, 577);
        act_connect_to_DF = new QAction(MainWindow);
        act_connect_to_DF->setObjectName(QString::fromUtf8("act_connect_to_DF"));
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/connect.png")), QIcon::Normal, QIcon::Off);
        act_connect_to_DF->setIcon(icon);
        act_exit = new QAction(MainWindow);
        act_exit->setObjectName(QString::fromUtf8("act_exit"));
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/img/door_out.png")), QIcon::Normal, QIcon::Off);
        act_exit->setIcon(icon1);
        act_about = new QAction(MainWindow);
        act_about->setObjectName(QString::fromUtf8("act_about"));
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/img/help.png")), QIcon::Normal, QIcon::Off);
        act_about->setIcon(icon2);
        act_read_dwarves = new QAction(MainWindow);
        act_read_dwarves->setObjectName(QString::fromUtf8("act_read_dwarves"));
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/img/drive_go.png")), QIcon::Normal, QIcon::Off);
        act_read_dwarves->setIcon(icon3);
        act_scan_memory = new QAction(MainWindow);
        act_scan_memory->setObjectName(QString::fromUtf8("act_scan_memory"));
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/img/drive_magnify.png")), QIcon::Normal, QIcon::Off);
        act_scan_memory->setIcon(icon4);
        action_apply_skill_changes_immedietly = new QAction(MainWindow);
        action_apply_skill_changes_immedietly->setObjectName(QString::fromUtf8("action_apply_skill_changes_immedietly"));
        action_apply_skill_changes_immedietly->setCheckable(true);
        action_apply_skill_changes_immedietly->setChecked(true);
        act_expand_all = new QAction(MainWindow);
        act_expand_all->setObjectName(QString::fromUtf8("act_expand_all"));
        act_expand_all->setEnabled(false);
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/img/arrow_out.png")), QIcon::Normal, QIcon::Off);
        act_expand_all->setIcon(icon5);
        act_collapse_all = new QAction(MainWindow);
        act_collapse_all->setObjectName(QString::fromUtf8("act_collapse_all"));
        act_collapse_all->setEnabled(false);
        QIcon icon6;
        icon6.addPixmap(QPixmap(QString::fromUtf8(":/img/arrow_in.png")), QIcon::Normal, QIcon::Off);
        act_collapse_all->setIcon(icon6);
        act_show_toolbutton_text = new QAction(MainWindow);
        act_show_toolbutton_text->setObjectName(QString::fromUtf8("act_show_toolbutton_text"));
        act_show_toolbutton_text->setCheckable(true);
        act_show_toolbutton_text->setChecked(true);
        act_add_custom_profession = new QAction(MainWindow);
        act_add_custom_profession->setObjectName(QString::fromUtf8("act_add_custom_profession"));
        act_clear_pending_changes = new QAction(MainWindow);
        act_clear_pending_changes->setObjectName(QString::fromUtf8("act_clear_pending_changes"));
        act_clear_pending_changes->setEnabled(false);
        QIcon icon7;
        icon7.addPixmap(QPixmap(QString::fromUtf8(":/img/table_delete.png")), QIcon::Normal, QIcon::Off);
        act_clear_pending_changes->setIcon(icon7);
        act_commit_pending_changes = new QAction(MainWindow);
        act_commit_pending_changes->setObjectName(QString::fromUtf8("act_commit_pending_changes"));
        act_commit_pending_changes->setEnabled(false);
        QIcon icon8;
        icon8.addPixmap(QPixmap(QString::fromUtf8(":/img/table_go.png")), QIcon::Normal, QIcon::Off);
        act_commit_pending_changes->setIcon(icon8);
        act_list_pending_changes = new QAction(MainWindow);
        act_list_pending_changes->setObjectName(QString::fromUtf8("act_list_pending_changes"));
        act_list_pending_changes->setEnabled(false);
        QIcon icon9;
        icon9.addPixmap(QPixmap(QString::fromUtf8(":/img/table.png")), QIcon::Normal, QIcon::Off);
        act_list_pending_changes->setIcon(icon9);
        act_show_pending_labors_window = new QAction(MainWindow);
        act_show_pending_labors_window->setObjectName(QString::fromUtf8("act_show_pending_labors_window"));
        act_show_pending_labors_window->setCheckable(true);
        act_show_pending_labors_window->setChecked(true);
        main_widget = new QWidget(MainWindow);
        main_widget->setObjectName(QString::fromUtf8("main_widget"));
        verticalLayout = new QVBoxLayout(main_widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setMargin(11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(main_widget);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        le_filter = new QLineEdit(main_widget);
        le_filter->setObjectName(QString::fromUtf8("le_filter"));
        le_filter->setEnabled(false);

        horizontalLayout->addWidget(le_filter);

        btn_filter = new QPushButton(main_widget);
        btn_filter->setObjectName(QString::fromUtf8("btn_filter"));
        btn_filter->setEnabled(false);

        horizontalLayout->addWidget(btn_filter);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        lbl_group_by = new QLabel(main_widget);
        lbl_group_by->setObjectName(QString::fromUtf8("lbl_group_by"));

        horizontalLayout_2->addWidget(lbl_group_by);

        cb_group_by = new QComboBox(main_widget);
        cb_group_by->setObjectName(QString::fromUtf8("cb_group_by"));
        cb_group_by->setEnabled(false);

        horizontalLayout_2->addWidget(cb_group_by);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        lbl_pending_changes = new QLabel(main_widget);
        lbl_pending_changes->setObjectName(QString::fromUtf8("lbl_pending_changes"));

        horizontalLayout_2->addWidget(lbl_pending_changes);

        label_2 = new QLabel(main_widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);


        verticalLayout->addLayout(horizontalLayout_2);

        stv = new StateTableView(main_widget);
        stv->setObjectName(QString::fromUtf8("stv"));
        QFont font;
        font.setPointSize(8);
        stv->setFont(font);
        stv->setMouseTracking(true);
        stv->setEditTriggers(QAbstractItemView::NoEditTriggers);
        stv->setTabKeyNavigation(true);
        stv->setProperty("showDropIndicator", QVariant(false));
        stv->setAlternatingRowColors(false);
        stv->setSelectionMode(QAbstractItemView::SingleSelection);
        stv->setSelectionBehavior(QAbstractItemView::SelectRows);
        stv->setIndentation(12);
        stv->setUniformRowHeights(true);
        stv->setSortingEnabled(false);
        stv->setAnimated(false);
        stv->setAllColumnsShowFocus(false);
        stv->setHeaderHidden(false);
        stv->header()->setVisible(true);
        stv->header()->setDefaultSectionSize(16);
        stv->header()->setHighlightSections(true);
        stv->header()->setMinimumSectionSize(16);
        stv->header()->setStretchLastSection(false);

        verticalLayout->addWidget(stv);

        MainWindow->setCentralWidget(main_widget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 848, 20));
        menuBar->setFont(font);
        menu_File = new QMenu(menuBar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menu_File->setTearOffEnabled(false);
        menu_Help = new QMenu(menuBar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        menu_Help->setTearOffEnabled(false);
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        menuProfessions = new QMenu(menuBar);
        menuProfessions->setObjectName(QString::fromUtf8("menuProfessions"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
        dock_pending_jobs_list = new QDockWidget(MainWindow);
        dock_pending_jobs_list->setObjectName(QString::fromUtf8("dock_pending_jobs_list"));
        dockWidgetContents = new QWidget();
        dockWidgetContents->setObjectName(QString::fromUtf8("dockWidgetContents"));
        verticalLayout_2 = new QVBoxLayout(dockWidgetContents);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setMargin(11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        list_pending = new QListWidget(dockWidgetContents);
        list_pending->setObjectName(QString::fromUtf8("list_pending"));

        verticalLayout_2->addWidget(list_pending);

        dock_pending_jobs_list->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dock_pending_jobs_list);
#ifndef QT_NO_SHORTCUT
        label->setBuddy(le_filter);
        lbl_group_by->setBuddy(cb_group_by);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(le_filter, btn_filter);

        menuBar->addAction(menu_File->menuAction());
        menuBar->addAction(menuProfessions->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menu_Help->menuAction());
        menu_File->addAction(act_connect_to_DF);
        menu_File->addAction(act_read_dwarves);
        menu_File->addAction(act_scan_memory);
        menu_File->addSeparator();
        menu_File->addAction(act_list_pending_changes);
        menu_File->addAction(act_commit_pending_changes);
        menu_File->addAction(act_clear_pending_changes);
        menu_File->addSeparator();
        menu_File->addAction(act_exit);
        menu_Help->addAction(act_about);
        menuOptions->addAction(action_apply_skill_changes_immedietly);
        menuOptions->addAction(act_show_toolbutton_text);
        menuOptions->addAction(act_show_pending_labors_window);
        menuProfessions->addAction(act_add_custom_profession);
        mainToolBar->addAction(act_connect_to_DF);
        mainToolBar->addAction(act_read_dwarves);
        mainToolBar->addAction(act_add_custom_profession);
        mainToolBar->addSeparator();
        mainToolBar->addAction(act_expand_all);
        mainToolBar->addAction(act_collapse_all);
        mainToolBar->addSeparator();
        mainToolBar->addAction(act_clear_pending_changes);
        mainToolBar->addAction(act_commit_pending_changes);
        mainToolBar->addAction(act_list_pending_changes);
        mainToolBar->addSeparator();
        mainToolBar->addAction(act_exit);

        retranslateUi(MainWindow);
        QObject::connect(act_connect_to_DF, SIGNAL(triggered()), MainWindow, SLOT(connect_to_df()));
        QObject::connect(act_exit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(act_read_dwarves, SIGNAL(triggered()), MainWindow, SLOT(read_dwarves()));
        QObject::connect(act_scan_memory, SIGNAL(triggered()), MainWindow, SLOT(scan_memory()));
        QObject::connect(btn_filter, SIGNAL(clicked()), MainWindow, SLOT(filter_dwarves()));
        QObject::connect(act_expand_all, SIGNAL(triggered()), stv, SLOT(expandAll()));
        QObject::connect(act_collapse_all, SIGNAL(triggered()), stv, SLOT(collapseAll()));
        QObject::connect(act_show_toolbutton_text, SIGNAL(toggled(bool)), MainWindow, SLOT(show_toolbutton_text(bool)));
        QObject::connect(cb_group_by, SIGNAL(currentIndexChanged(int)), MainWindow, SLOT(set_group_by(int)));
        QObject::connect(act_about, SIGNAL(triggered()), MainWindow, SLOT(show_about()));
        QObject::connect(act_add_custom_profession, SIGNAL(triggered()), MainWindow, SLOT(add_custom_profession()));
        QObject::connect(act_show_pending_labors_window, SIGNAL(triggered(bool)), dock_pending_jobs_list, SLOT(setVisible(bool)));
        QObject::connect(dock_pending_jobs_list, SIGNAL(visibilityChanged(bool)), act_show_pending_labors_window, SLOT(setChecked(bool)));
        QObject::connect(list_pending, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), stv, SLOT(jump_to_dwarf(QListWidgetItem*,QListWidgetItem*)));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        act_connect_to_DF->setText(QApplication::translate("MainWindow", "&Connect To DF", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_connect_to_DF->setToolTip(QApplication::translate("MainWindow", "Attempt connecting to a running copy of Dwarf Fortress", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        act_connect_to_DF->setStatusTip(QApplication::translate("MainWindow", "Attempt connecting to a running copy of Dwarf Fortress", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        act_connect_to_DF->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        act_exit->setText(QApplication::translate("MainWindow", "E&xit", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_exit->setToolTip(QApplication::translate("MainWindow", "Close Dwarf Therapist", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        act_exit->setStatusTip(QApplication::translate("MainWindow", "Close Dwarf Therapist", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        act_about->setText(QApplication::translate("MainWindow", "&About", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_about->setToolTip(QApplication::translate("MainWindow", "Exactly what you think...", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_read_dwarves->setText(QApplication::translate("MainWindow", "Read Dwarves", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_read_dwarves->setToolTip(QApplication::translate("MainWindow", "Attempt to load all Dwarves from DF", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        act_read_dwarves->setStatusTip(QApplication::translate("MainWindow", "Attempt to load all Dwarves from DF", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        act_read_dwarves->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        act_scan_memory->setText(QApplication::translate("MainWindow", "Scan Memory", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_scan_memory->setToolTip(QApplication::translate("MainWindow", "Map out the RAM of Dwarf Fortress looking for known values", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        act_scan_memory->setStatusTip(QApplication::translate("MainWindow", "Map out the RAM of Dwarf Fortress looking for known values", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        act_scan_memory->setShortcut(QApplication::translate("MainWindow", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        action_apply_skill_changes_immedietly->setText(QApplication::translate("MainWindow", "Apply Skill Toggles Immedietly", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        action_apply_skill_changes_immedietly->setToolTip(QApplication::translate("MainWindow", "When set, skill changes are written to the game immedietly as opposed to buffering up.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        action_apply_skill_changes_immedietly->setStatusTip(QApplication::translate("MainWindow", "When set, skill changes are written to the game immedietly as opposed to buffering up", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        act_expand_all->setText(QApplication::translate("MainWindow", "Expand All", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_expand_all->setToolTip(QApplication::translate("MainWindow", "Expand all groups", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_expand_all->setShortcut(QApplication::translate("MainWindow", "Ctrl+Right", 0, QApplication::UnicodeUTF8));
        act_collapse_all->setText(QApplication::translate("MainWindow", "Collapse All", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_collapse_all->setToolTip(QApplication::translate("MainWindow", "Collapse all groups", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_collapse_all->setShortcut(QApplication::translate("MainWindow", "Ctrl+Left", 0, QApplication::UnicodeUTF8));
        act_show_toolbutton_text->setText(QApplication::translate("MainWindow", "Show Toolbutton Text", 0, QApplication::UnicodeUTF8));
        act_add_custom_profession->setText(QApplication::translate("MainWindow", "Add Custom Profession", 0, QApplication::UnicodeUTF8));
        act_clear_pending_changes->setText(QApplication::translate("MainWindow", "Clear Pending Changes", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_clear_pending_changes->setToolTip(QApplication::translate("MainWindow", "Abort all uncomitted changes", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_commit_pending_changes->setText(QApplication::translate("MainWindow", "Commit Pending Changes", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_commit_pending_changes->setToolTip(QApplication::translate("MainWindow", "Write all uncomitted changes to DF", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_list_pending_changes->setText(QApplication::translate("MainWindow", "List Pending Changes", 0, QApplication::UnicodeUTF8));
        act_show_pending_labors_window->setText(QApplication::translate("MainWindow", "Show Pending Labors Window", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Filter Dwarves", 0, QApplication::UnicodeUTF8));
        btn_filter->setText(QApplication::translate("MainWindow", "Go", 0, QApplication::UnicodeUTF8));
        lbl_group_by->setText(QApplication::translate("MainWindow", "Group By", 0, QApplication::UnicodeUTF8));
        cb_group_by->clear();
        cb_group_by->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "Nothing", 0, QApplication::UnicodeUTF8)
        );
#ifndef QT_NO_TOOLTIP
        cb_group_by->setToolTip(QApplication::translate("MainWindow", "Change how dwarves are grouped in the table", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_group_by->setStatusTip(QApplication::translate("MainWindow", "Change how dwarves are grouped in the table", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
        lbl_pending_changes->setText(QApplication::translate("MainWindow", "0", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Pending Changes", 0, QApplication::UnicodeUTF8));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0, QApplication::UnicodeUTF8));
        menu_Help->setTitle(QApplication::translate("MainWindow", "&Help", 0, QApplication::UnicodeUTF8));
        menuOptions->setTitle(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
        menuProfessions->setTitle(QApplication::translate("MainWindow", "Professions", 0, QApplication::UnicodeUTF8));
        dock_pending_jobs_list->setWindowTitle(QApplication::translate("MainWindow", "Pending Labor Changes", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
