/********************************************************************************
** Form generated from reading ui file 'optionsmenu.ui'
**
** Created: Tue Jul 21 16:11:45 2009
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
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_OptionsMenu
{
public:
    QVBoxLayout *verticalLayout;
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
        verticalLayout = new QVBoxLayout(OptionsMenu);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        group_general_colors = new QGroupBox(OptionsMenu);
        group_general_colors->setObjectName(QString::fromUtf8("group_general_colors"));
        group_general_colors->setStyleSheet(QString::fromUtf8("QtColorPicker {\n"
"text-align: left;\n"
"}"));

        verticalLayout->addWidget(group_general_colors);

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


        verticalLayout->addLayout(horizontalLayout_5);

        verticalLayout->setStretch(0, 10);

        retranslateUi(OptionsMenu);
        QObject::connect(buttonBox, SIGNAL(accepted()), OptionsMenu, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), OptionsMenu, SLOT(reject()));

        QMetaObject::connectSlotsByName(OptionsMenu);
    } // setupUi

    void retranslateUi(QDialog *OptionsMenu)
    {
        OptionsMenu->setWindowTitle(QApplication::translate("OptionsMenu", "Options", 0, QApplication::UnicodeUTF8));
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
