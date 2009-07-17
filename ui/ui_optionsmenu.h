/********************************************************************************
** Form generated from reading ui file 'optionsmenu.ui'
**
** Created: Fri Jul 17 15:17:51 2009
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
#include <QtGui/QVBoxLayout>
#include "qtcolorpicker.h"

QT_BEGIN_NAMESPACE

class Ui_OptionsMenu
{
public:
    QVBoxLayout *verticalLayout_3;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QCheckBox *checkBox;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout;
    QtColorPicker *clr_active_labor;
    QLabel *label;
    QHBoxLayout *horizontalLayout_2;
    QtColorPicker *clr_dirty_border;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_3;
    QtColorPicker *clr_cursor;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout_4;
    QtColorPicker *clr_active_group;
    QLabel *label_4;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OptionsMenu)
    {
        if (OptionsMenu->objectName().isEmpty())
            OptionsMenu->setObjectName(QString::fromUtf8("OptionsMenu"));
        OptionsMenu->resize(640, 480);
        verticalLayout_3 = new QVBoxLayout(OptionsMenu);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        groupBox = new QGroupBox(OptionsMenu);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        checkBox = new QCheckBox(groupBox);
        checkBox->setObjectName(QString::fromUtf8("checkBox"));

        verticalLayout->addWidget(checkBox);


        verticalLayout_3->addWidget(groupBox);

        groupBox_2 = new QGroupBox(OptionsMenu);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        verticalLayout_2 = new QVBoxLayout(groupBox_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        clr_active_labor = new QtColorPicker(groupBox_2);
        clr_active_labor->setObjectName(QString::fromUtf8("clr_active_labor"));
        clr_active_labor->setAutoDefault(false);
        clr_active_labor->setProperty("colorDialog", QVariant(true));

        horizontalLayout->addWidget(clr_active_labor);

        label = new QLabel(groupBox_2);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);


        verticalLayout_2->addLayout(horizontalLayout);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        clr_dirty_border = new QtColorPicker(groupBox_2);
        clr_dirty_border->setObjectName(QString::fromUtf8("clr_dirty_border"));

        horizontalLayout_2->addWidget(clr_dirty_border);

        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        clr_cursor = new QtColorPicker(groupBox_2);
        clr_cursor->setObjectName(QString::fromUtf8("clr_cursor"));

        horizontalLayout_3->addWidget(clr_cursor);

        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_3->addWidget(label_3);


        verticalLayout_2->addLayout(horizontalLayout_3);

        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        clr_active_group = new QtColorPicker(groupBox_2);
        clr_active_group->setObjectName(QString::fromUtf8("clr_active_group"));

        horizontalLayout_4->addWidget(clr_active_group);

        label_4 = new QLabel(groupBox_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_4->addWidget(label_4);


        verticalLayout_2->addLayout(horizontalLayout_4);


        verticalLayout_3->addWidget(groupBox_2);

        buttonBox = new QDialogButtonBox(OptionsMenu);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        verticalLayout_3->addWidget(buttonBox);

        verticalLayout_3->setStretch(1, 3);
#ifndef QT_NO_SHORTCUT
        label->setBuddy(clr_active_labor);
        label_2->setBuddy(clr_dirty_border);
        label_3->setBuddy(clr_cursor);
        label_4->setBuddy(clr_active_group);
#endif // QT_NO_SHORTCUT

        retranslateUi(OptionsMenu);
        QObject::connect(buttonBox, SIGNAL(accepted()), OptionsMenu, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), OptionsMenu, SLOT(reject()));

        QMetaObject::connectSlotsByName(OptionsMenu);
    } // setupUi

    void retranslateUi(QDialog *OptionsMenu)
    {
        OptionsMenu->setWindowTitle(QApplication::translate("OptionsMenu", "Options", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("OptionsMenu", "Behavior", 0, QApplication::UnicodeUTF8));
        checkBox->setText(QApplication::translate("OptionsMenu", "Write Changes Immediately", 0, QApplication::UnicodeUTF8));
        groupBox_2->setTitle(QApplication::translate("OptionsMenu", "Grid Colors", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        clr_active_labor->setToolTip(QApplication::translate("OptionsMenu", "Background color for an individual labor square when it is enabled", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        clr_active_labor->setStatusTip(QApplication::translate("OptionsMenu", "Background color for an individual labor square when it is enabled", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        clr_active_labor->setWhatsThis(QApplication::translate("OptionsMenu", "Background color for an individual labor square when it is enabled", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        clr_active_labor->setText(QApplication::translate("OptionsMenu", "Black", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("OptionsMenu", "Active Labor Background", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        clr_dirty_border->setToolTip(QApplication::translate("OptionsMenu", "Color of the border for labor squares that have not yet been written to the game", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        clr_dirty_border->setStatusTip(QApplication::translate("OptionsMenu", "Color of the border for labor squares that have not yet been written to the game", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        clr_dirty_border->setWhatsThis(QApplication::translate("OptionsMenu", "Color of the border for labor squares that have not yet been written to the game", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        label_2->setText(QApplication::translate("OptionsMenu", "Dirty Border", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        clr_cursor->setToolTip(QApplication::translate("OptionsMenu", "Color of the \"X\" cursor on the grid", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        clr_cursor->setStatusTip(QApplication::translate("OptionsMenu", "Color of the \"X\" cursor on the grid", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        clr_cursor->setWhatsThis(QApplication::translate("OptionsMenu", "Color of the \"X\" cursor on the grid", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        label_3->setText(QApplication::translate("OptionsMenu", "Cursor Color", 0, QApplication::UnicodeUTF8));
#ifndef QT_NO_TOOLTIP
        clr_active_group->setToolTip(QApplication::translate("OptionsMenu", "Only used with \"group by\". The background color of a group of labors, if all members have this labor active.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_TOOLTIP
#ifndef QT_NO_STATUSTIP
        clr_active_group->setStatusTip(QApplication::translate("OptionsMenu", "Only used with \"group by\". The background color of a group of labors, if all members have this labor active.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_STATUSTIP
#ifndef QT_NO_WHATSTHIS
        clr_active_group->setWhatsThis(QApplication::translate("OptionsMenu", "Only used with \"group by\". The background color of a group of labors, if all members have this labor active.", 0, QApplication::UnicodeUTF8));
#endif // QT_NO_WHATSTHIS
        label_4->setText(QApplication::translate("OptionsMenu", "Active Group", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(OptionsMenu);
    } // retranslateUi

};

namespace Ui {
    class OptionsMenu: public Ui_OptionsMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSMENU_H
