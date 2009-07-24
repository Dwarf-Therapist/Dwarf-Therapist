/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Fri Jul 24 12:06:36 2009
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
#include <QtGui/QListWidget>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QTreeWidget>
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
    QAction *act_show_pending_labors_window;
    QAction *act_show_custom_professions_window;
    QAction *act_show_main_toolbar;
    QAction *act_options;
    QAction *act_import_existing_professions;
    QAction *act_keyboard_grid_focus;
    QAction *act_single_click_labor_changes;
    QAction *act_go_forums;
    QAction *act_go_project_home;
    QAction *act_go_new_issue;
    QAction *act_go_donate;
    QWidget *main_widget;
    QVBoxLayout *verticalLayout;
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
    QMenu *menuProfessions;
    QMenu *menuWindows;
    QMenu *menuDocks;
    QMenu *menuSettings;
    QToolBar *main_toolbar;
    QStatusBar *statusBar;
    QDockWidget *dock_pending_jobs_list;
    QWidget *dockWidgetContents;
    QVBoxLayout *verticalLayout_2;
    QTreeWidget *tree_pending;
    QHBoxLayout *horizontalLayout_4;
    QPushButton *btn_expand_all_pending;
    QPushButton *btn_collapse_all_pending;
    QHBoxLayout *horizontalLayout_3;
    QPushButton *btn_commit;
    QPushButton *btn_clear;
    QDockWidget *dock_custom_professions;
    QWidget *dockWidgetContents_2;
    QVBoxLayout *verticalLayout_3;
    QListWidget *list_custom_professions;
    QPushButton *btn_new_custom_profession;
    QPushButton *btn_import_professions;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1129, 747);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/hammer.png")), QIcon::Normal, QIcon::Off);
        MainWindow->setWindowIcon(icon);
        act_connect_to_DF = new QAction(MainWindow);
        act_connect_to_DF->setObjectName(QString::fromUtf8("act_connect_to_DF"));
        QIcon icon1;
        icon1.addPixmap(QPixmap(QString::fromUtf8(":/img/connect.png")), QIcon::Normal, QIcon::Off);
        act_connect_to_DF->setIcon(icon1);
        act_exit = new QAction(MainWindow);
        act_exit->setObjectName(QString::fromUtf8("act_exit"));
        QIcon icon2;
        icon2.addPixmap(QPixmap(QString::fromUtf8(":/img/door_out.png")), QIcon::Normal, QIcon::Off);
        act_exit->setIcon(icon2);
        act_about = new QAction(MainWindow);
        act_about->setObjectName(QString::fromUtf8("act_about"));
        QIcon icon3;
        icon3.addPixmap(QPixmap(QString::fromUtf8(":/img/help.png")), QIcon::Normal, QIcon::Off);
        act_about->setIcon(icon3);
        act_read_dwarves = new QAction(MainWindow);
        act_read_dwarves->setObjectName(QString::fromUtf8("act_read_dwarves"));
        QIcon icon4;
        icon4.addPixmap(QPixmap(QString::fromUtf8(":/img/drive_go.png")), QIcon::Normal, QIcon::Off);
        act_read_dwarves->setIcon(icon4);
        act_scan_memory = new QAction(MainWindow);
        act_scan_memory->setObjectName(QString::fromUtf8("act_scan_memory"));
        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/img/drive_magnify.png")), QIcon::Normal, QIcon::Off);
        act_scan_memory->setIcon(icon5);
        action_apply_skill_changes_immedietly = new QAction(MainWindow);
        action_apply_skill_changes_immedietly->setObjectName(QString::fromUtf8("action_apply_skill_changes_immedietly"));
        action_apply_skill_changes_immedietly->setCheckable(true);
        action_apply_skill_changes_immedietly->setChecked(true);
        act_expand_all = new QAction(MainWindow);
        act_expand_all->setObjectName(QString::fromUtf8("act_expand_all"));
        act_expand_all->setEnabled(false);
        QIcon icon6;
        icon6.addPixmap(QPixmap(QString::fromUtf8(":/img/arrow_out.png")), QIcon::Normal, QIcon::Off);
        act_expand_all->setIcon(icon6);
        act_collapse_all = new QAction(MainWindow);
        act_collapse_all->setObjectName(QString::fromUtf8("act_collapse_all"));
        act_collapse_all->setEnabled(false);
        QIcon icon7;
        icon7.addPixmap(QPixmap(QString::fromUtf8(":/img/arrow_in.png")), QIcon::Normal, QIcon::Off);
        act_collapse_all->setIcon(icon7);
        act_show_toolbutton_text = new QAction(MainWindow);
        act_show_toolbutton_text->setObjectName(QString::fromUtf8("act_show_toolbutton_text"));
        act_show_toolbutton_text->setCheckable(true);
        act_show_toolbutton_text->setChecked(true);
        QIcon icon8;
        icon8.addPixmap(QPixmap(QString::fromUtf8(":/img/textfield_add.png")), QIcon::Normal, QIcon::Off);
        act_show_toolbutton_text->setIcon(icon8);
        act_add_custom_profession = new QAction(MainWindow);
        act_add_custom_profession->setObjectName(QString::fromUtf8("act_add_custom_profession"));
        QIcon icon9;
        icon9.addPixmap(QPixmap(QString::fromUtf8(":/img/page_white_add.png")), QIcon::Normal, QIcon::Off);
        act_add_custom_profession->setIcon(icon9);
        act_clear_pending_changes = new QAction(MainWindow);
        act_clear_pending_changes->setObjectName(QString::fromUtf8("act_clear_pending_changes"));
        act_clear_pending_changes->setEnabled(false);
        QIcon icon10;
        icon10.addPixmap(QPixmap(QString::fromUtf8(":/img/table_delete.png")), QIcon::Normal, QIcon::Off);
        act_clear_pending_changes->setIcon(icon10);
        act_commit_pending_changes = new QAction(MainWindow);
        act_commit_pending_changes->setObjectName(QString::fromUtf8("act_commit_pending_changes"));
        act_commit_pending_changes->setEnabled(false);
        QIcon icon11;
        icon11.addPixmap(QPixmap(QString::fromUtf8(":/img/table_go.png")), QIcon::Normal, QIcon::Off);
        act_commit_pending_changes->setIcon(icon11);
        act_show_pending_labors_window = new QAction(MainWindow);
        act_show_pending_labors_window->setObjectName(QString::fromUtf8("act_show_pending_labors_window"));
        act_show_pending_labors_window->setCheckable(true);
        act_show_pending_labors_window->setChecked(true);
        QIcon icon12;
        icon12.addPixmap(QPixmap(QString::fromUtf8(":/img/layout_content.png")), QIcon::Normal, QIcon::Off);
        act_show_pending_labors_window->setIcon(icon12);
        act_show_custom_professions_window = new QAction(MainWindow);
        act_show_custom_professions_window->setObjectName(QString::fromUtf8("act_show_custom_professions_window"));
        act_show_custom_professions_window->setCheckable(true);
        act_show_custom_professions_window->setChecked(true);
        act_show_custom_professions_window->setIcon(icon12);
        act_show_main_toolbar = new QAction(MainWindow);
        act_show_main_toolbar->setObjectName(QString::fromUtf8("act_show_main_toolbar"));
        act_show_main_toolbar->setCheckable(false);
        act_show_main_toolbar->setIcon(icon12);
        act_options = new QAction(MainWindow);
        act_options->setObjectName(QString::fromUtf8("act_options"));
        QIcon icon13;
        icon13.addPixmap(QPixmap(QString::fromUtf8(":/img/color_wheel.png")), QIcon::Normal, QIcon::Off);
        act_options->setIcon(icon13);
        act_import_existing_professions = new QAction(MainWindow);
        act_import_existing_professions->setObjectName(QString::fromUtf8("act_import_existing_professions"));
        QIcon icon14;
        icon14.addPixmap(QPixmap(QString::fromUtf8(":/img/application_get.png")), QIcon::Normal, QIcon::Off);
        act_import_existing_professions->setIcon(icon14);
        act_keyboard_grid_focus = new QAction(MainWindow);
        act_keyboard_grid_focus->setObjectName(QString::fromUtf8("act_keyboard_grid_focus"));
        act_keyboard_grid_focus->setCheckable(true);
        act_single_click_labor_changes = new QAction(MainWindow);
        act_single_click_labor_changes->setObjectName(QString::fromUtf8("act_single_click_labor_changes"));
        act_single_click_labor_changes->setCheckable(true);
        act_go_forums = new QAction(MainWindow);
        act_go_forums->setObjectName(QString::fromUtf8("act_go_forums"));
        QIcon icon15;
        icon15.addPixmap(QPixmap(QString::fromUtf8(":/img/world_go.png")), QIcon::Normal, QIcon::Off);
        act_go_forums->setIcon(icon15);
        act_go_project_home = new QAction(MainWindow);
        act_go_project_home->setObjectName(QString::fromUtf8("act_go_project_home"));
        act_go_project_home->setIcon(icon15);
        act_go_new_issue = new QAction(MainWindow);
        act_go_new_issue->setObjectName(QString::fromUtf8("act_go_new_issue"));
        act_go_new_issue->setIcon(icon15);
        act_go_donate = new QAction(MainWindow);
        act_go_donate->setObjectName(QString::fromUtf8("act_go_donate"));
        QIcon icon16;
        icon16.addPixmap(QPixmap(QString::fromUtf8(":/img/heart.png")), QIcon::Normal, QIcon::Off);
        act_go_donate->setIcon(icon16);
        main_widget = new QWidget(MainWindow);
        main_widget->setObjectName(QString::fromUtf8("main_widget"));
        verticalLayout = new QVBoxLayout(main_widget);
        verticalLayout->setSpacing(6);
        verticalLayout->setMargin(11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
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
        QFont font;
        font.setPointSize(20);
        lbl_pending_changes->setFont(font);

        horizontalLayout_2->addWidget(lbl_pending_changes);

        label_2 = new QLabel(main_widget);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);


        verticalLayout->addLayout(horizontalLayout_2);

        stv = new StateTableView(main_widget);
        stv->setObjectName(QString::fromUtf8("stv"));
        QFont font1;
        font1.setPointSize(8);
        stv->setFont(font1);
        stv->setMouseTracking(true);
        stv->setContextMenuPolicy(Qt::CustomContextMenu);
        stv->setEditTriggers(QAbstractItemView::NoEditTriggers);
        stv->setTabKeyNavigation(true);
        stv->setProperty("showDropIndicator", QVariant(false));
        stv->setAlternatingRowColors(false);
        stv->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
        menuBar->setGeometry(QRect(0, 0, 1129, 20));
        menuBar->setFont(font1);
        menu_File = new QMenu(menuBar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menu_File->setTearOffEnabled(false);
        menu_Help = new QMenu(menuBar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        menu_Help->setTearOffEnabled(false);
        menuProfessions = new QMenu(menuBar);
        menuProfessions->setObjectName(QString::fromUtf8("menuProfessions"));
        menuWindows = new QMenu(menuBar);
        menuWindows->setObjectName(QString::fromUtf8("menuWindows"));
        menuDocks = new QMenu(menuWindows);
        menuDocks->setObjectName(QString::fromUtf8("menuDocks"));
        menuSettings = new QMenu(menuBar);
        menuSettings->setObjectName(QString::fromUtf8("menuSettings"));
        MainWindow->setMenuBar(menuBar);
        main_toolbar = new QToolBar(MainWindow);
        main_toolbar->setObjectName(QString::fromUtf8("main_toolbar"));
        main_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        MainWindow->addToolBar(Qt::TopToolBarArea, main_toolbar);
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
        tree_pending = new QTreeWidget(dockWidgetContents);
        tree_pending->setObjectName(QString::fromUtf8("tree_pending"));
        tree_pending->setFont(font1);
        tree_pending->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tree_pending->setAlternatingRowColors(true);
        tree_pending->setUniformRowHeights(true);
        tree_pending->setSortingEnabled(true);
        tree_pending->setAllColumnsShowFocus(true);

        verticalLayout_2->addWidget(tree_pending);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setSpacing(6);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        btn_expand_all_pending = new QPushButton(dockWidgetContents);
        btn_expand_all_pending->setObjectName(QString::fromUtf8("btn_expand_all_pending"));
        btn_expand_all_pending->setIcon(icon6);

        horizontalLayout_4->addWidget(btn_expand_all_pending);

        btn_collapse_all_pending = new QPushButton(dockWidgetContents);
        btn_collapse_all_pending->setObjectName(QString::fromUtf8("btn_collapse_all_pending"));
        btn_collapse_all_pending->setIcon(icon7);

        horizontalLayout_4->addWidget(btn_collapse_all_pending);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        btn_commit = new QPushButton(dockWidgetContents);
        btn_commit->setObjectName(QString::fromUtf8("btn_commit"));
        btn_commit->setEnabled(false);
        btn_commit->setIcon(icon11);

        horizontalLayout_3->addWidget(btn_commit);

        btn_clear = new QPushButton(dockWidgetContents);
        btn_clear->setObjectName(QString::fromUtf8("btn_clear"));
        btn_clear->setEnabled(false);
        btn_clear->setIcon(icon10);

        horizontalLayout_3->addWidget(btn_clear);


        verticalLayout_2->addLayout(horizontalLayout_3);

        dock_pending_jobs_list->setWidget(dockWidgetContents);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dock_pending_jobs_list);
        dock_custom_professions = new QDockWidget(MainWindow);
        dock_custom_professions->setObjectName(QString::fromUtf8("dock_custom_professions"));
        dockWidgetContents_2 = new QWidget();
        dockWidgetContents_2->setObjectName(QString::fromUtf8("dockWidgetContents_2"));
        verticalLayout_3 = new QVBoxLayout(dockWidgetContents_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setMargin(11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        list_custom_professions = new QListWidget(dockWidgetContents_2);
        list_custom_professions->setObjectName(QString::fromUtf8("list_custom_professions"));
        list_custom_professions->setContextMenuPolicy(Qt::CustomContextMenu);

        verticalLayout_3->addWidget(list_custom_professions);

        btn_new_custom_profession = new QPushButton(dockWidgetContents_2);
        btn_new_custom_profession->setObjectName(QString::fromUtf8("btn_new_custom_profession"));
        btn_new_custom_profession->setIcon(icon9);

        verticalLayout_3->addWidget(btn_new_custom_profession);

        btn_import_professions = new QPushButton(dockWidgetContents_2);
        btn_import_professions->setObjectName(QString::fromUtf8("btn_import_professions"));
        btn_import_professions->setEnabled(false);
        btn_import_professions->setIcon(icon14);

        verticalLayout_3->addWidget(btn_import_professions);

        dock_custom_professions->setWidget(dockWidgetContents_2);
        MainWindow->addDockWidget(static_cast<Qt::DockWidgetArea>(2), dock_custom_professions);
#ifndef QT_NO_SHORTCUT
        lbl_group_by->setBuddy(cb_group_by);
#endif // QT_NO_SHORTCUT

        menuBar->addAction(menu_File->menuAction());
        menuBar->addAction(menuProfessions->menuAction());
        menuBar->addAction(menuSettings->menuAction());
        menuBar->addAction(menuWindows->menuAction());
        menuBar->addAction(menu_Help->menuAction());
        menu_File->addAction(act_connect_to_DF);
        menu_File->addAction(act_read_dwarves);
        menu_File->addSeparator();
        menu_File->addAction(act_commit_pending_changes);
        menu_File->addAction(act_clear_pending_changes);
        menu_File->addSeparator();
        menu_File->addAction(act_exit);
        menu_Help->addAction(act_go_project_home);
        menu_Help->addAction(act_go_forums);
        menu_Help->addAction(act_go_new_issue);
        menu_Help->addAction(act_go_donate);
        menu_Help->addSeparator();
        menu_Help->addAction(act_about);
        menuProfessions->addAction(act_add_custom_profession);
        menuProfessions->addAction(act_import_existing_professions);
        menuWindows->addAction(menuDocks->menuAction());
        menuWindows->addAction(act_show_main_toolbar);
        menuDocks->addAction(act_show_pending_labors_window);
        menuDocks->addAction(act_show_custom_professions_window);
        menuSettings->addAction(act_show_toolbutton_text);
        menuSettings->addAction(act_keyboard_grid_focus);
        menuSettings->addAction(act_single_click_labor_changes);
        menuSettings->addAction(act_options);
        main_toolbar->addAction(act_connect_to_DF);
        main_toolbar->addAction(act_read_dwarves);
        main_toolbar->addAction(act_add_custom_profession);
        main_toolbar->addSeparator();
        main_toolbar->addAction(act_expand_all);
        main_toolbar->addAction(act_collapse_all);
        main_toolbar->addSeparator();
        main_toolbar->addAction(act_clear_pending_changes);
        main_toolbar->addAction(act_commit_pending_changes);
        main_toolbar->addSeparator();
        main_toolbar->addAction(act_exit);

        retranslateUi(MainWindow);
        QObject::connect(act_connect_to_DF, SIGNAL(triggered()), MainWindow, SLOT(connect_to_df()));
        QObject::connect(act_exit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(act_read_dwarves, SIGNAL(triggered()), MainWindow, SLOT(read_dwarves()));
        QObject::connect(act_scan_memory, SIGNAL(triggered()), MainWindow, SLOT(scan_memory()));
        QObject::connect(act_expand_all, SIGNAL(triggered()), stv, SLOT(expandAll()));
        QObject::connect(act_collapse_all, SIGNAL(triggered()), stv, SLOT(collapseAll()));
        QObject::connect(act_show_toolbutton_text, SIGNAL(toggled(bool)), MainWindow, SLOT(show_toolbutton_text(bool)));
        QObject::connect(cb_group_by, SIGNAL(currentIndexChanged(int)), MainWindow, SLOT(set_group_by(int)));
        QObject::connect(act_about, SIGNAL(triggered()), MainWindow, SLOT(show_about()));
        QObject::connect(act_add_custom_profession, SIGNAL(triggered()), MainWindow, SLOT(add_custom_profession()));
        QObject::connect(act_show_pending_labors_window, SIGNAL(triggered(bool)), dock_pending_jobs_list, SLOT(setVisible(bool)));
        QObject::connect(dock_pending_jobs_list, SIGNAL(visibilityChanged(bool)), act_show_pending_labors_window, SLOT(setChecked(bool)));
        QObject::connect(btn_commit, SIGNAL(clicked()), act_commit_pending_changes, SLOT(trigger()));
        QObject::connect(btn_clear, SIGNAL(clicked()), act_clear_pending_changes, SLOT(trigger()));
        QObject::connect(dock_custom_professions, SIGNAL(visibilityChanged(bool)), act_show_custom_professions_window, SLOT(setChecked(bool)));
        QObject::connect(act_show_custom_professions_window, SIGNAL(triggered(bool)), dock_custom_professions, SLOT(setVisible(bool)));
        QObject::connect(act_show_main_toolbar, SIGNAL(triggered()), main_toolbar, SLOT(show()));
        QObject::connect(act_options, SIGNAL(triggered()), MainWindow, SLOT(open_options_menu()));
        QObject::connect(tree_pending, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), stv, SLOT(jump_to_dwarf(QTreeWidgetItem*,QTreeWidgetItem*)));
        QObject::connect(btn_collapse_all_pending, SIGNAL(clicked()), tree_pending, SLOT(collapseAll()));
        QObject::connect(btn_expand_all_pending, SIGNAL(clicked()), tree_pending, SLOT(expandAll()));
        QObject::connect(btn_new_custom_profession, SIGNAL(clicked()), act_add_custom_profession, SLOT(trigger()));
        QObject::connect(btn_import_professions, SIGNAL(clicked()), act_import_existing_professions, SLOT(trigger()));
        QObject::connect(act_import_existing_professions, SIGNAL(triggered()), MainWindow, SLOT(import_existing_professions()));
        QObject::connect(act_keyboard_grid_focus, SIGNAL(triggered(bool)), MainWindow, SLOT(set_allow_grid_focus(bool)));
        QObject::connect(act_single_click_labor_changes, SIGNAL(triggered(bool)), MainWindow, SLOT(set_single_click_labor_changes(bool)));
        QObject::connect(act_go_donate, SIGNAL(triggered()), MainWindow, SLOT(go_to_donate()));
        QObject::connect(act_go_forums, SIGNAL(triggered()), MainWindow, SLOT(go_to_forums()));
        QObject::connect(act_go_new_issue, SIGNAL(triggered()), MainWindow, SLOT(go_to_new_issue()));
        QObject::connect(act_go_project_home, SIGNAL(triggered()), MainWindow, SLOT(go_to_project_home()));

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
        act_show_pending_labors_window->setText(QApplication::translate("MainWindow", "Show Pending Labors Window", 0, QApplication::UnicodeUTF8));
        act_show_custom_professions_window->setText(QApplication::translate("MainWindow", "Show Custom Professions Window", 0, QApplication::UnicodeUTF8));
        act_show_main_toolbar->setText(QApplication::translate("MainWindow", "Show Main Toolbar", 0, QApplication::UnicodeUTF8));
        act_options->setText(QApplication::translate("MainWindow", "Color Settings...", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_options->setToolTip(QApplication::translate("MainWindow", "Open the color options menu...", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_import_existing_professions->setText(QApplication::translate("MainWindow", "Import Existing Professions", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_import_existing_professions->setToolTip(QApplication::translate("MainWindow", "Import all custom professions from the loaded game, and create new labor templates for them", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_keyboard_grid_focus->setText(QApplication::translate("MainWindow", "Enable Keyboard Grid Focus", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_keyboard_grid_focus->setToolTip(QApplication::translate("MainWindow", "When checked it will show an \"X\" over the currently focused labor cell. You can then use the arrow keys to navigate the grid much like in DF. Unchecked disables the focus \"X\"", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_single_click_labor_changes->setText(QApplication::translate("MainWindow", "Single Click Labor Changes", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_single_click_labor_changes->setToolTip(QApplication::translate("MainWindow", "When set you will be able to toggle skills in the main grid with a single left-click of the mouse. When unchecked, you will have to double-click", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_go_forums->setText(QApplication::translate("MainWindow", "Discussion Forums", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_go_forums->setToolTip(QApplication::translate("MainWindow", "Launch a browser to Dwarf Therapist's discussion forums", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_go_project_home->setText(QApplication::translate("MainWindow", "Project Homepage", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_go_project_home->setToolTip(QApplication::translate("MainWindow", "Launch a browser to Dwarf Therapist's project homepage", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_go_new_issue->setText(QApplication::translate("MainWindow", "Request Feature / Report Bug", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_go_new_issue->setToolTip(QApplication::translate("MainWindow", "Report a bug or request a feature online", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_go_donate->setText(QApplication::translate("MainWindow", "Donate", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_go_donate->setToolTip(QApplication::translate("MainWindow", "Your donations make the world a better place!", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
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
        menuProfessions->setTitle(QApplication::translate("MainWindow", "Professions", 0, QApplication::UnicodeUTF8));
        menuWindows->setTitle(QApplication::translate("MainWindow", "Windows", 0, QApplication::UnicodeUTF8));
        menuDocks->setTitle(QApplication::translate("MainWindow", "Docks", 0, QApplication::UnicodeUTF8));
        menuSettings->setTitle(QApplication::translate("MainWindow", "Settings", 0, QApplication::UnicodeUTF8));
        main_toolbar->setWindowTitle(QApplication::translate("MainWindow", "Main Toolbar", 0, QApplication::UnicodeUTF8));
        dock_pending_jobs_list->setWindowTitle(QApplication::translate("MainWindow", "Pending Labor Changes", 0, QApplication::UnicodeUTF8));
        QTreeWidgetItem *___qtreewidgetitem = tree_pending->headerItem();
        ___qtreewidgetitem->setText(0, QApplication::translate("MainWindow", "Name", 0, QApplication::UnicodeUTF8));
        btn_expand_all_pending->setText(QApplication::translate("MainWindow", "Expand All", 0, QApplication::UnicodeUTF8));
        btn_collapse_all_pending->setText(QApplication::translate("MainWindow", "Collapse All", 0, QApplication::UnicodeUTF8));
        btn_commit->setText(QApplication::translate("MainWindow", "Commit Changes", 0, QApplication::UnicodeUTF8));
        btn_clear->setText(QApplication::translate("MainWindow", "Clear Changes", 0, QApplication::UnicodeUTF8));
        dock_custom_professions->setWindowTitle(QApplication::translate("MainWindow", "Custom Professions", 0, QApplication::UnicodeUTF8));
        btn_new_custom_profession->setText(QApplication::translate("MainWindow", "Add New Custom Profession", 0, QApplication::UnicodeUTF8));
        btn_import_professions->setText(QApplication::translate("MainWindow", "Import Custom Professions", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
