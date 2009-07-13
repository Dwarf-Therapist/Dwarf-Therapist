/********************************************************************************
** Form generated from reading ui file 'mainwindow.ui'
**
** Created: Sun Jul 12 21:10:31 2009
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
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QSlider>
#include <QtGui/QStatusBar>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include <statetableview.h>

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
    QWidget *main_widget;
    QVBoxLayout *verticalLayout_2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *le_filter;
    QPushButton *btn_filter;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSlider *horizontalSlider;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lbl_x_size;
    QLabel *label_3;
    QLabel *lbl_y_size;
    StateTableView *stv;
    QMenuBar *menuBar;
    QMenu *menu_File;
    QMenu *menu_Help;
    QMenu *menuOptions;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(769, 543);
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
        main_widget = new QWidget(MainWindow);
        main_widget->setObjectName(QString::fromUtf8("main_widget"));
        verticalLayout_2 = new QVBoxLayout(main_widget);
        verticalLayout_2->setSpacing(6);
        verticalLayout_2->setMargin(11);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        tabWidget = new QTabWidget(main_widget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        verticalLayout = new QVBoxLayout(tab);
        verticalLayout->setSpacing(6);
        verticalLayout->setMargin(11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(tab);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        le_filter = new QLineEdit(tab);
        le_filter->setObjectName(QString::fromUtf8("le_filter"));
        le_filter->setEnabled(false);

        horizontalLayout->addWidget(le_filter);

        btn_filter = new QPushButton(tab);
        btn_filter->setObjectName(QString::fromUtf8("btn_filter"));
        btn_filter->setEnabled(false);

        horizontalLayout->addWidget(btn_filter);


        verticalLayout->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(tab);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        horizontalSlider = new QSlider(tab);
        horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
        horizontalSlider->setMinimum(8);
        horizontalSlider->setMaximum(160);
        horizontalSlider->setSingleStep(1);
        horizontalSlider->setPageStep(48);
        horizontalSlider->setValue(16);
        horizontalSlider->setOrientation(Qt::Horizontal);
        horizontalSlider->setTickPosition(QSlider::NoTicks);
        horizontalSlider->setTickInterval(8);

        horizontalLayout_2->addWidget(horizontalSlider);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setSpacing(6);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        lbl_x_size = new QLabel(tab);
        lbl_x_size->setObjectName(QString::fromUtf8("lbl_x_size"));

        horizontalLayout_3->addWidget(lbl_x_size);

        label_3 = new QLabel(tab);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);

        lbl_y_size = new QLabel(tab);
        lbl_y_size->setObjectName(QString::fromUtf8("lbl_y_size"));

        horizontalLayout_3->addWidget(lbl_y_size);


        horizontalLayout_2->addLayout(horizontalLayout_3);


        verticalLayout->addLayout(horizontalLayout_2);

        stv = new StateTableView(tab);
        stv->setObjectName(QString::fromUtf8("stv"));
        QFont font;
        font.setPointSize(8);
        stv->setFont(font);
        stv->setMidLineWidth(2);
        stv->setEditTriggers(QAbstractItemView::NoEditTriggers);
        stv->setAlternatingRowColors(true);
        stv->setSelectionMode(QAbstractItemView::SingleSelection);
        stv->setSelectionBehavior(QAbstractItemView::SelectRows);
        stv->setGridStyle(Qt::SolidLine);
        stv->setSortingEnabled(true);
        stv->setCornerButtonEnabled(false);
        stv->horizontalHeader()->setVisible(false);
        stv->horizontalHeader()->setDefaultSectionSize(16);
        stv->horizontalHeader()->setProperty("showSortIndicator", QVariant(true));
        stv->verticalHeader()->setVisible(false);
        stv->verticalHeader()->setDefaultSectionSize(16);

        verticalLayout->addWidget(stv);

        QIcon icon5;
        icon5.addPixmap(QPixmap(QString::fromUtf8(":/img/application_view_icons.png")), QIcon::Normal, QIcon::Off);
        tabWidget->addTab(tab, icon5, QString());

        verticalLayout_2->addWidget(tabWidget);

        MainWindow->setCentralWidget(main_widget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 769, 22));
        menu_File = new QMenu(menuBar);
        menu_File->setObjectName(QString::fromUtf8("menu_File"));
        menu_File->setTearOffEnabled(false);
        menu_Help = new QMenu(menuBar);
        menu_Help->setObjectName(QString::fromUtf8("menu_Help"));
        menu_Help->setTearOffEnabled(false);
        menuOptions = new QMenu(menuBar);
        menuOptions->setObjectName(QString::fromUtf8("menuOptions"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        mainToolBar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        MainWindow->addToolBar(Qt::BottomToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);
#ifndef QT_NO_SHORTCUT
        label->setBuddy(le_filter);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(stv, le_filter);
        QWidget::setTabOrder(le_filter, btn_filter);
        QWidget::setTabOrder(btn_filter, tabWidget);

        menuBar->addAction(menu_File->menuAction());
        menuBar->addAction(menuOptions->menuAction());
        menuBar->addAction(menu_Help->menuAction());
        menu_File->addAction(act_connect_to_DF);
        menu_File->addAction(act_read_dwarves);
        menu_File->addAction(act_scan_memory);
        menu_File->addSeparator();
        menu_File->addAction(act_exit);
        menu_Help->addAction(act_about);
        menuOptions->addAction(action_apply_skill_changes_immedietly);
        mainToolBar->addAction(act_connect_to_DF);
        mainToolBar->addAction(act_read_dwarves);
        mainToolBar->addAction(act_scan_memory);
        mainToolBar->addAction(act_exit);

        retranslateUi(MainWindow);
        QObject::connect(act_connect_to_DF, SIGNAL(triggered()), MainWindow, SLOT(connect_to_df()));
        QObject::connect(act_exit, SIGNAL(triggered()), MainWindow, SLOT(close()));
        QObject::connect(act_read_dwarves, SIGNAL(triggered()), MainWindow, SLOT(read_dwarves()));
        QObject::connect(act_scan_memory, SIGNAL(triggered()), MainWindow, SLOT(scan_memory()));
        QObject::connect(btn_filter, SIGNAL(clicked()), MainWindow, SLOT(filter_dwarves()));
        QObject::connect(le_filter, SIGNAL(textChanged(QString)), stv, SLOT(filter_dwarves(QString)));
        QObject::connect(horizontalSlider, SIGNAL(sliderMoved(int)), stv, SLOT(set_grid_size(int)));
        QObject::connect(horizontalSlider, SIGNAL(sliderMoved(int)), lbl_x_size, SLOT(setNum(int)));
        QObject::connect(horizontalSlider, SIGNAL(sliderMoved(int)), lbl_y_size, SLOT(setNum(int)));

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        act_connect_to_DF->setText(QApplication::translate("MainWindow", "&Connect To DF", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_connect_to_DF->setToolTip(QApplication::translate("MainWindow", "Attempt connecting to a running copy of Dwarf Fortress", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_connect_to_DF->setShortcut(QApplication::translate("MainWindow", "Ctrl+C", 0, QApplication::UnicodeUTF8));
        act_exit->setText(QApplication::translate("MainWindow", "E&xit", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_exit->setToolTip(QApplication::translate("MainWindow", "Close Dwarf Therapist", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_about->setText(QApplication::translate("MainWindow", "&About", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_about->setToolTip(QApplication::translate("MainWindow", "Exactly what you think...", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_read_dwarves->setText(QApplication::translate("MainWindow", "Read Dwarves", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_read_dwarves->setToolTip(QApplication::translate("MainWindow", "Read the stuff...", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_read_dwarves->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0, QApplication::UnicodeUTF8));
        act_scan_memory->setText(QApplication::translate("MainWindow", "Scan Memory", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        act_scan_memory->setToolTip(QApplication::translate("MainWindow", "Map out the RAM of Dwarf Fortress looking for known values", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        act_scan_memory->setShortcut(QApplication::translate("MainWindow", "Ctrl+M", 0, QApplication::UnicodeUTF8));
        action_apply_skill_changes_immedietly->setText(QApplication::translate("MainWindow", "Apply Skill Toggles Immedietly", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        action_apply_skill_changes_immedietly->setToolTip(QApplication::translate("MainWindow", "When set, skill changes are written to the game immedietly as opposed to buffering up.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
        label->setText(QApplication::translate("MainWindow", "Filter Dwarves", 0, QApplication::UnicodeUTF8));
        btn_filter->setText(QApplication::translate("MainWindow", "Go", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Resize Grid", 0, QApplication::UnicodeUTF8));
        lbl_x_size->setText(QApplication::translate("MainWindow", "16", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "x", 0, QApplication::UnicodeUTF8));
        lbl_y_size->setText(QApplication::translate("MainWindow", "16", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tab), QApplication::translate("MainWindow", "Career Advisor", 0, QApplication::UnicodeUTF8));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0, QApplication::UnicodeUTF8));
        menu_Help->setTitle(QApplication::translate("MainWindow", "&Help", 0, QApplication::UnicodeUTF8));
        menuOptions->setTitle(QApplication::translate("MainWindow", "Options", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
