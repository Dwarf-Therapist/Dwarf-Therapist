/********************************************************************************
** Form generated from reading ui file 'optionsmenu.ui'
**
** Created: Fri Jul 31 21:31:33 2009
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
    QVBoxLayout *verticalLayout_2;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpinBox *sb_cell_padding;
    QGroupBox *group_general_colors;
    QHBoxLayout *horizontalLayout_5;
    QPushButton *btn_restore_defaults;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OptionsMenu)
    {
        if (OptionsMenu->objectName().isEmpty())
            OptionsMenu->setObjectName(QString::fromUtf8("OptionsMenu"));
        OptionsMenu->resize(344, 334);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/color_wheel.png")), QIcon::Normal, QIcon::Off);
        OptionsMenu->setWindowIcon(icon);
        verticalLayout_2 = new QVBoxLayout(OptionsMenu);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        groupBox = new QGroupBox(OptionsMenu);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(groupBox);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        sb_cell_padding = new QSpinBox(groupBox);
        sb_cell_padding->setObjectName(QString::fromUtf8("sb_cell_padding"));
        sb_cell_padding->setMaximum(6);

        horizontalLayout->addWidget(sb_cell_padding);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addWidget(groupBox);

        group_general_colors = new QGroupBox(OptionsMenu);
        group_general_colors->setObjectName(QString::fromUtf8("group_general_colors"));
        group_general_colors->setStyleSheet(QString::fromUtf8("QtColorPicker {\n"
"text-align: left;\n"
"}"));

        verticalLayout_2->addWidget(group_general_colors);

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


        verticalLayout_2->addLayout(horizontalLayout_5);

#ifndef QT_NO_SHORTCUT
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
        groupBox->setTitle(QApplication::translate("OptionsMenu", "Grid Options", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("OptionsMenu", "Cell Padding", 0, QApplication::UnicodeUTF8));
        sb_cell_padding->setSuffix(QApplication::translate("OptionsMenu", "px", 0, QApplication::UnicodeUTF8));
        group_general_colors->setTitle(QApplication::translate("OptionsMenu", "Grid Colors", 0, QApplication::UnicodeUTF8));
        btn_restore_defaults->setText(QApplication::translate("OptionsMenu", "Restore Default Colors", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(OptionsMenu);
    } // retranslateUi

};

namespace Ui {
    class OptionsMenu: public Ui_OptionsMenu {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OPTIONSMENU_H
