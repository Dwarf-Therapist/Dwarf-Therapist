/********************************************************************************
** Form generated from reading ui file 'about.ui'
**
** Created: Tue Jul 14 13:30:50 2009
**      by: Qt User Interface Compiler version 4.5.0
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_ABOUT_H
#define UI_ABOUT_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_form_about
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame_2;
    QVBoxLayout *verticalLayout_3;
    QLabel *label;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_3;
    QHBoxLayout *horizontalLayout;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QWidget *form_about)
    {
        if (form_about->objectName().isEmpty())
            form_about->setObjectName(QString::fromUtf8("form_about"));
        form_about->resize(483, 256);
        verticalLayout = new QVBoxLayout(form_about);
        verticalLayout->setSpacing(0);
        verticalLayout->setMargin(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame_2 = new QFrame(form_about);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        verticalLayout_3 = new QVBoxLayout(frame_2);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        label = new QLabel(frame_2);
        label->setObjectName(QString::fromUtf8("label"));
        QFont font;
        font.setFamily(QString::fromUtf8("Segoe UI"));
        font.setPointSize(22);
        label->setFont(font);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        verticalLayout_3->addWidget(label);


        verticalLayout->addWidget(frame_2);

        frame = new QFrame(form_about);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label_3 = new QLabel(frame);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setTextFormat(Qt::RichText);
        label_3->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        label_3->setOpenExternalLinks(true);

        verticalLayout_2->addWidget(label_3);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        label_2 = new QLabel(frame);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout->addWidget(label_2);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(frame);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(frame);


        retranslateUi(form_about);
        QObject::connect(pushButton, SIGNAL(clicked()), form_about, SLOT(close()));

        QMetaObject::connectSlotsByName(form_about);
    } // setupUi

    void retranslateUi(QWidget *form_about)
    {
        form_about->setWindowTitle(QApplication::translate("form_about", "About Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        frame_2->setStyleSheet(QApplication::translate("form_about", "background-color: white;", 0, QApplication::UnicodeUTF8));
        label->setStyleSheet(QString());
        label->setText(QApplication::translate("form_about", "Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        frame->setStyleSheet(QApplication::translate("form_about", "background-color: #dddddd;", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("form_about", "Copyright &copy; 2009 Trey Stout.\n"
"Made Possible from <a href=\"blah.org\">blah</a>", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("form_about", "Up to date/Update Available etc...", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("form_about", "OK", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(form_about);
    } // retranslateUi

};

namespace Ui {
    class form_about: public Ui_form_about {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
