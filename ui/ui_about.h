/********************************************************************************
** Form generated from reading ui file 'about.ui'
**
** Created: Mon Jul 20 12:33:42 2009
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

class Ui_AboutDialog
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *frame_2;
    QLabel *label;
    QLabel *label_4;
    QLabel *lbl_our_version;
    QFrame *frame;
    QVBoxLayout *verticalLayout_2;
    QLabel *lbl_text;
    QHBoxLayout *horizontalLayout;
    QLabel *lbl_up_to_date;
    QSpacerItem *horizontalSpacer;
    QPushButton *pushButton;

    void setupUi(QWidget *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(400, 200);
        AboutDialog->setMaximumSize(QSize(400, 200));
        verticalLayout = new QVBoxLayout(AboutDialog);
        verticalLayout->setSpacing(0);
        verticalLayout->setMargin(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        frame_2 = new QFrame(AboutDialog);
        frame_2->setObjectName(QString::fromUtf8("frame_2"));
        frame_2->setFrameShape(QFrame::StyledPanel);
        frame_2->setFrameShadow(QFrame::Raised);
        label = new QLabel(frame_2);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 20, 201, 41));
        QFont font;
        font.setFamily(QString::fromUtf8("Segoe UI"));
        font.setPointSize(22);
        label->setFont(font);
        label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label->setOpenExternalLinks(false);
        label_4 = new QLabel(frame_2);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(270, -15, 131, 151));
        label_4->setPixmap(QPixmap(QString::fromUtf8(":/img/dwarf.jpg")));
        label_4->setScaledContents(true);
        lbl_our_version = new QLabel(frame_2);
        lbl_our_version->setObjectName(QString::fromUtf8("lbl_our_version"));
        lbl_our_version->setGeometry(QRect(20, 60, 190, 20));

        verticalLayout->addWidget(frame_2);

        frame = new QFrame(AboutDialog);
        frame->setObjectName(QString::fromUtf8("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout_2 = new QVBoxLayout(frame);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        lbl_text = new QLabel(frame);
        lbl_text->setObjectName(QString::fromUtf8("lbl_text"));
        lbl_text->setTextFormat(Qt::RichText);
        lbl_text->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_text->setOpenExternalLinks(true);

        verticalLayout_2->addWidget(lbl_text);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lbl_up_to_date = new QLabel(frame);
        lbl_up_to_date->setObjectName(QString::fromUtf8("lbl_up_to_date"));
        lbl_up_to_date->setOpenExternalLinks(true);

        horizontalLayout->addWidget(lbl_up_to_date);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(frame);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout_2->addLayout(horizontalLayout);


        verticalLayout->addWidget(frame);

        verticalLayout->setStretch(0, 5);
        verticalLayout->setStretch(1, 4);

        retranslateUi(AboutDialog);
        QObject::connect(pushButton, SIGNAL(clicked()), AboutDialog, SLOT(close()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QWidget *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        frame_2->setStyleSheet(QApplication::translate("AboutDialog", "background-color: white;", 0, QApplication::UnicodeUTF8));
        label->setStyleSheet(QString());
        label->setText(QApplication::translate("AboutDialog", "Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        label_4->setText(QString());
        lbl_our_version->setText(QApplication::translate("AboutDialog", "VERSION", 0, QApplication::UnicodeUTF8));
        frame->setStyleSheet(QApplication::translate("AboutDialog", "background-color: #dddddd;", 0, QApplication::UnicodeUTF8));
        lbl_text->setText(QApplication::translate("AboutDialog", "Copyright &copy; 2009 Trey Stout.\n"
"\n"
"<a href=\"http://udpviper.com/forums\">UDP Viper</a>", 0, QApplication::UnicodeUTF8));
        lbl_up_to_date->setText(QApplication::translate("AboutDialog", "Up to date/Update Available etc...", 0, QApplication::UnicodeUTF8));
        pushButton->setText(QApplication::translate("AboutDialog", "OK", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(AboutDialog);
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUT_H
