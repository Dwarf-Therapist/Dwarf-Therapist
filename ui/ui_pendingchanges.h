/********************************************************************************
** Form generated from reading ui file 'pendingchanges.ui'
**
** Created: Sun Aug 2 10:41:39 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_PENDINGCHANGES_H
#define UI_PENDINGCHANGES_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_PendingChanges
{
public:
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;
    QListWidget *list_pending;

    void setupUi(QWidget *PendingChanges)
    {
        if (PendingChanges->objectName().isEmpty())
            PendingChanges->setObjectName(QString::fromUtf8("PendingChanges"));
        PendingChanges->resize(240, 320);
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/hammer.png")), QIcon::Normal, QIcon::Off);
        PendingChanges->setWindowIcon(icon);
        verticalLayout = new QVBoxLayout(PendingChanges);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label = new QLabel(PendingChanges);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout->addWidget(label);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(PendingChanges);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);

        list_pending = new QListWidget(PendingChanges);
        list_pending->setObjectName(QString::fromUtf8("list_pending"));
        QFont font;
        font.setPointSize(8);
        list_pending->setFont(font);
        list_pending->setEditTriggers(QAbstractItemView::NoEditTriggers);
        list_pending->setAlternatingRowColors(true);
        list_pending->setSelectionMode(QAbstractItemView::NoSelection);
        list_pending->setSelectionBehavior(QAbstractItemView::SelectRows);
        list_pending->setUniformItemSizes(true);

        verticalLayout->addWidget(list_pending);


        retranslateUi(PendingChanges);

        QMetaObject::connectSlotsByName(PendingChanges);
    } // setupUi

    void retranslateUi(QWidget *PendingChanges)
    {
        PendingChanges->setWindowTitle(QApplication::translate("PendingChanges", "Pending Changes", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("PendingChanges", "Pending Changes", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("PendingChanges", "Refresh", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(PendingChanges);
    } // retranslateUi

};

namespace Ui {
    class PendingChanges: public Ui_PendingChanges {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PENDINGCHANGES_H
