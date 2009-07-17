/********************************************************************************
** Form generated from reading ui file 'customprofession.ui'
**
** Created: Thu Jul 16 20:31:26 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_CUSTOMPROFESSION_H
#define UI_CUSTOMPROFESSION_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QDialogButtonBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <qxtlistwidget.h>

QT_BEGIN_NAMESPACE

class Ui_CustomProfessionEditor
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *name_edit;
    QxtListWidget *labor_list;
    QHBoxLayout *horizontalLayout_2;
    QLabel *lbl_skill_count;
    QLabel *label_2;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *CustomProfessionEditor)
    {
        if (CustomProfessionEditor->objectName().isEmpty())
            CustomProfessionEditor->setObjectName(QString::fromUtf8("CustomProfessionEditor"));
        CustomProfessionEditor->resize(268, 365);
        CustomProfessionEditor->setSizeGripEnabled(true);
        verticalLayout = new QVBoxLayout(CustomProfessionEditor);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(CustomProfessionEditor);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        name_edit = new QLineEdit(CustomProfessionEditor);
        name_edit->setObjectName(QString::fromUtf8("name_edit"));

        horizontalLayout->addWidget(name_edit);

        horizontalLayout->setStretch(0, 1);
        horizontalLayout->setStretch(1, 2);

        verticalLayout->addLayout(horizontalLayout);

        labor_list = new QxtListWidget(CustomProfessionEditor);
        labor_list->setObjectName(QString::fromUtf8("labor_list"));
        labor_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
        labor_list->setSelectionMode(QAbstractItemView::NoSelection);
        labor_list->setUniformItemSizes(true);
        labor_list->setSortingEnabled(false);

        verticalLayout->addWidget(labor_list);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        lbl_skill_count = new QLabel(CustomProfessionEditor);
        lbl_skill_count->setObjectName(QString::fromUtf8("lbl_skill_count"));

        horizontalLayout_2->addWidget(lbl_skill_count);

        label_2 = new QLabel(CustomProfessionEditor);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_2->addWidget(label_2);

        horizontalLayout_2->setStretch(0, 1);
        horizontalLayout_2->setStretch(1, 100);

        verticalLayout->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer);

        buttonBox = new QDialogButtonBox(CustomProfessionEditor);
        buttonBox->setObjectName(QString::fromUtf8("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        horizontalLayout_3->addWidget(buttonBox);


        verticalLayout->addLayout(horizontalLayout_3);

#ifndef QT_NO_SHORTCUT
        label->setBuddy(name_edit);
#endif // QT_NO_SHORTCUT
        QWidget::setTabOrder(name_edit, labor_list);
        QWidget::setTabOrder(labor_list, buttonBox);

        retranslateUi(CustomProfessionEditor);
        QObject::connect(buttonBox, SIGNAL(rejected()), CustomProfessionEditor, SLOT(reject()));

        QMetaObject::connectSlotsByName(CustomProfessionEditor);
    } // setupUi

    void retranslateUi(QDialog *CustomProfessionEditor)
    {
        CustomProfessionEditor->setWindowTitle(QApplication::translate("CustomProfessionEditor", "Dialog", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("CustomProfessionEditor", "Profession Name", 0, QApplication::UnicodeUTF8));
        lbl_skill_count->setText(QApplication::translate("CustomProfessionEditor", "0", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("CustomProfessionEditor", "Skills Selected", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(CustomProfessionEditor);
    } // retranslateUi

};

namespace Ui {
    class CustomProfessionEditor: public Ui_CustomProfessionEditor {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CUSTOMPROFESSION_H
