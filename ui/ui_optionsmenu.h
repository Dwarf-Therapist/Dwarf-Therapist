/********************************************************************************
** Form generated from reading ui file 'optionsmenu.ui'
**
** Created: Sun Aug 2 10:41:39 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_OPTIONSMENU_H
#define UI_OPTIONSMENU_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpinBox>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_OptionsMenu
{
public:
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_6;
    QGroupBox *group_general_colors;
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout;
    QCheckBox *cb_read_dwarves_on_startup;
    QCheckBox *cb_auto_contrast;
    QCheckBox *cb_show_aggregates;
    QCheckBox *cb_single_click_labor_changes;
    QCheckBox *cb_show_toolbar_text;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_3;
    QLabel *lbl_current_font;
    QPushButton *btn_change_font;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label_2;
    QSpinBox *sb_cell_size;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *sb_cell_padding;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *btn_restore_defaults;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OptionsMenu)
    {
        if (OptionsMenu->objectName().isEmpty())
            OptionsMenu->setObjectName(QString::fromUtf8("OptionsMenu"));
        OptionsMenu->resize(321, 328);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/color_wheel.png")), QIcon::Normal, QIcon::Off);
        OptionsMenu->setWindowIcon(icon);
        verticalLayout_4 = new QVBoxLayout(OptionsMenu);
        verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
        horizontalLayout_6 = new QHBoxLayout();
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        group_general_colors = new QGroupBox(OptionsMenu);
        group_general_colors->setObjectName(QString::fromUtf8("group_general_colors"));
        group_general_colors->setStyleSheet(QString::fromUtf8("QtColorPicker {\n"
"text-align: left;\n"
"}"));

        horizontalLayout_6->addWidget(group_general_colors);

        verticalLayout_3 = new QVBoxLayout();
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox_2 = new QGroupBox(OptionsMenu);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout = new QVBoxLayout(groupBox_2);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        cb_read_dwarves_on_startup = new QCheckBox(groupBox_2);
        cb_read_dwarves_on_startup->setObjectName(QString::fromUtf8("cb_read_dwarves_on_startup"));

        verticalLayout->addWidget(cb_read_dwarves_on_startup);

        cb_auto_contrast = new QCheckBox(groupBox_2);
        cb_auto_contrast->setObjectName(QString::fromUtf8("cb_auto_contrast"));

        verticalLayout->addWidget(cb_auto_contrast);

        cb_show_aggregates = new QCheckBox(groupBox_2);
        cb_show_aggregates->setObjectName(QString::fromUtf8("cb_show_aggregates"));

        verticalLayout->addWidget(cb_show_aggregates);

        cb_single_click_labor_changes = new QCheckBox(groupBox_2);
        cb_single_click_labor_changes->setObjectName(QString::fromUtf8("cb_single_click_labor_changes"));

        verticalLayout->addWidget(cb_single_click_labor_changes);

        cb_show_toolbar_text = new QCheckBox(groupBox_2);
        cb_show_toolbar_text->setObjectName(QString::fromUtf8("cb_show_toolbar_text"));

        verticalLayout->addWidget(cb_show_toolbar_text);


        verticalLayout_3->addWidget(groupBox_2);

        groupBox = new QGroupBox(OptionsMenu);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout_2 = new QVBoxLayout(groupBox);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(groupBox);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        QFont font;
        font.setBold(false);
        font.setWeight(50);
        label_3->setFont(font);

        horizontalLayout_4->addWidget(label_3);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        lbl_current_font = new QLabel(groupBox);
        lbl_current_font->setObjectName(QString::fromUtf8("lbl_current_font"));

        horizontalLayout_3->addWidget(lbl_current_font);

        btn_change_font = new QPushButton(groupBox);
        btn_change_font->setObjectName(QString::fromUtf8("btn_change_font"));

        horizontalLayout_3->addWidget(btn_change_font);


        horizontalLayout_4->addLayout(horizontalLayout_3);


        verticalLayout_2->addLayout(horizontalLayout_4);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label_2 = new QLabel(groupBox);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setFont(font);

        horizontalLayout_2->addWidget(label_2);

        sb_cell_size = new QSpinBox(groupBox);
        sb_cell_size->setObjectName(QString::fromUtf8("sb_cell_size"));
        sb_cell_size->setMinimum(4);
        sb_cell_size->setMaximum(64);
        sb_cell_size->setValue(16);

        horizontalLayout_2->addWidget(sb_cell_size);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));
        label->setFont(font);

        horizontalLayout->addWidget(label);

        sb_cell_padding = new QSpinBox(groupBox);
        sb_cell_padding->setObjectName(QString::fromUtf8("sb_cell_padding"));
        sb_cell_padding->setMaximum(6);

        horizontalLayout->addWidget(sb_cell_padding);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout_3->addWidget(groupBox);


        horizontalLayout_6->addLayout(verticalLayout_3);


        verticalLayout_4->addLayout(horizontalLayout_6);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        btn_restore_defaults = new QPushButton(OptionsMenu);
        btn_restore_defaults->setObjectName(QString::fromUtf8("btn_restore_defaults"));

        horizontalLayout_5->addWidget(btn_restore_defaults);

        buttonBox = new QDialogButtonBox(OptionsMenu);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_5->addWidget(buttonBox);


        verticalLayout_4->addLayout(horizontalLayout_5);

#ifndef QT_NO_SHORTCUT
        label_2->setBuddy(sb_cell_size);
        label->setBuddy(sb_cell_padding);
#endif // QT_NO_SHORTCUT

        retranslateUi(OptionsMenu);
        QObject::connect(buttonBox, SIGNAL(accepted()), OptionsMenu, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), OptionsMenu, SLOT(reject()));

        QMetaObject::connectSlotsByName(OptionsMenu);
    } // setupUi

    void retranslateUi(QDialog *OptionsMenu)
    {
        OptionsMenu->setWindowTitle(QApplication::translate("OptionsMenu", "Options", 0, QApplication::UnicodeUTF8));
        group_general_colors->setTitle(QApplication::translate("OptionsMenu", "Grid Colors", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("OptionsMenu", "Main Options", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cb_read_dwarves_on_startup->setToolTip(QApplication::translate("OptionsMenu", "When checked, DwarfTherapist will attemp to connect and load all dwarves from a running copy of Dwarf Fortress.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_read_dwarves_on_startup->setStatusTip(QApplication::translate("OptionsMenu", "When checked, DwarfTherapist will attemp to connect and load all dwarves from a running copy of Dwarf Fortress.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        cb_read_dwarves_on_startup->setWhatsThis(QApplication::translate("OptionsMenu", "When checked, DwarfTherapist will attemp to connect and load all dwarves from a running copy of Dwarf Fortress.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        cb_read_dwarves_on_startup->setText(QApplication::translate("OptionsMenu", "Load Dwarves on Launch", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cb_auto_contrast->setToolTip(QApplication::translate("OptionsMenu", "When checked, skill blocks on grid cells will choose the highest contrast color available. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_auto_contrast->setStatusTip(QApplication::translate("OptionsMenu", "When checked, skill blocks on grid cells will choose the highest contrast color available. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        cb_auto_contrast->setWhatsThis(QApplication::translate("OptionsMenu", "When checked, skill blocks on grid cells will choose the highest contrast color available. ", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        cb_auto_contrast->setText(QApplication::translate("OptionsMenu", "Auto Contrast Skill Display", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cb_show_aggregates->setToolTip(QApplication::translate("OptionsMenu", "When checked, and sorting by some criteria, labors will show an aggregate cell above a group. This cell can be toggled to enable/disable a labor for an entire group at once.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_show_aggregates->setStatusTip(QApplication::translate("OptionsMenu", "When checked, and sorting by some criteria, labors will show an aggregate cell above a group. This cell can be toggled to enable/disable a labor for an entire group at once.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        cb_show_aggregates->setWhatsThis(QApplication::translate("OptionsMenu", "When checked, and sorting by some criteria, labors will show an aggregate cell above a group. This cell can be toggled to enable/disable a labor for an entire group at once.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        cb_show_aggregates->setText(QApplication::translate("OptionsMenu", "Display Labor Group Cells", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cb_single_click_labor_changes->setToolTip(QApplication::translate("OptionsMenu", "When checked, labors can be toggled with a single left-click of the mouse. Otherwise they must be double-clicked", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_single_click_labor_changes->setStatusTip(QApplication::translate("OptionsMenu", "When checked, labors can be toggled with a single left-click of the mouse. Otherwise they must be double-clicked", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        cb_single_click_labor_changes->setWhatsThis(QApplication::translate("OptionsMenu", "When checked, labors can be toggled with a single left-click of the mouse. Otherwise they must be double-clicked", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        cb_single_click_labor_changes->setText(QApplication::translate("OptionsMenu", "Single Click Labor Changes", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        cb_show_toolbar_text->setToolTip(QApplication::translate("OptionsMenu", "When checked, show a text description for each icon on the toolbar", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        cb_show_toolbar_text->setStatusTip(QApplication::translate("OptionsMenu", "When checked, show a text description for each icon on the toolbar", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        cb_show_toolbar_text->setWhatsThis(QApplication::translate("OptionsMenu", "When checked, show a text description for each icon on the toolbar", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        cb_show_toolbar_text->setText(QApplication::translate("OptionsMenu", "Show Toolbar Text", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("OptionsMenu", "Grid Options", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("OptionsMenu", "Font", 0, QApplication::UnicodeUTF8));
        lbl_current_font->setText(QApplication::translate("OptionsMenu", "CURRENT_FONT", 0, QApplication::UnicodeUTF8));
        btn_change_font->setText(QApplication::translate("OptionsMenu", "Change Font...", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("OptionsMenu", "Cell Size", 0, QApplication::UnicodeUTF8));
        sb_cell_size->setSuffix(QApplication::translate("OptionsMenu", "px", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("OptionsMenu", "Cell Padding", 0, QApplication::UnicodeUTF8));
        sb_cell_padding->setSuffix(QApplication::translate("OptionsMenu", "px", 0, QApplication::UnicodeUTF8));
        btn_restore_defaults->setText(QApplication::translate("OptionsMenu", "Restore Defaults", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(OptionsMenu);
    } // retranslateUi

};

namespace Ui {
    class OptionsMenu: public Ui_OptionsMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSMENU_H
