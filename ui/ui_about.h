/********************************************************************************
** Form generated from reading ui file 'about.ui'
**
** Created: Fri Jul 24 12:06:35 2009
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
    QVBoxLayout *verticalLayout_2;
    QFrame *frame_upper;
    QLabel *lbl_our_version;
    QLabel *label_title;
    QLabel *lbl_image;
    QFrame *frame_lower;
    QVBoxLayout *verticalLayout;
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
        AboutDialog->setMinimumSize(QSize(400, 200));
        AboutDialog->setMaximumSize(QSize(400, 200));
        QIcon icon;
        icon.addPixmap(QPixmap(QString::fromUtf8(":/img/hammer.png")), QIcon::Normal, QIcon::Off);
        AboutDialog->setWindowIcon(icon);
        verticalLayout_2 = new QVBoxLayout(AboutDialog);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setMargin(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        frame_upper = new QFrame(AboutDialog);
        frame_upper->setObjectName(QString::fromUtf8("frame_upper"));
        frame_upper->setFrameShape(QFrame::StyledPanel);
        frame_upper->setFrameShadow(QFrame::Raised);
        lbl_our_version = new QLabel(frame_upper);
        lbl_our_version->setObjectName(QString::fromUtf8("lbl_our_version"));
        lbl_our_version->setGeometry(QRect(20, 60, 190, 20));
        label_title = new QLabel(frame_upper);
        label_title->setObjectName(QString::fromUtf8("label_title"));
        label_title->setGeometry(QRect(10, 20, 201, 41));
        QFont font;
        font.setFamily(QString::fromUtf8("Segoe UI"));
        font.setPointSize(22);
        label_title->setFont(font);
        label_title->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);
        label_title->setOpenExternalLinks(false);
        lbl_image = new QLabel(frame_upper);
        lbl_image->setObjectName(QString::fromUtf8("lbl_image"));
        lbl_image->setGeometry(QRect(270, -15, 131, 151));
        lbl_image->setPixmap(QPixmap(QString::fromUtf8(":/img/dwarf.jpg")));
        lbl_image->setScaledContents(true);

        verticalLayout_2->addWidget(frame_upper);

        frame_lower = new QFrame(AboutDialog);
        frame_lower->setObjectName(QString::fromUtf8("frame_lower"));
        frame_lower->setFrameShape(QFrame::StyledPanel);
        frame_lower->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame_lower);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        lbl_text = new QLabel(frame_lower);
        lbl_text->setObjectName(QString::fromUtf8("lbl_text"));
        lbl_text->setTextFormat(Qt::RichText);
        lbl_text->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        lbl_text->setOpenExternalLinks(true);

        verticalLayout->addWidget(lbl_text);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        lbl_up_to_date = new QLabel(frame_lower);
        lbl_up_to_date->setObjectName(QString::fromUtf8("lbl_up_to_date"));
        lbl_up_to_date->setOpenExternalLinks(true);

        horizontalLayout->addWidget(lbl_up_to_date);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        pushButton = new QPushButton(frame_lower);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout->addWidget(pushButton);


        verticalLayout->addLayout(horizontalLayout);


        verticalLayout_2->addWidget(frame_lower);

        verticalLayout_2->setStretch(0, 5);
        verticalLayout_2->setStretch(1, 4);

        retranslateUi(AboutDialog);
        QObject::connect(pushButton, SIGNAL(clicked()), AboutDialog, SLOT(close()));

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QWidget *AboutDialog)
    {
        AboutDialog->setWindowTitle(QApplication::translate("AboutDialog", "About Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        AboutDialog->setStyleSheet(QString());
        frame_upper->setStyleSheet(QApplication::translate("AboutDialog", "background: white;", 0, QApplication::UnicodeUTF8));
        lbl_our_version->setText(QApplication::translate("AboutDialog", "VERSION", 0, QApplication::UnicodeUTF8));
        label_title->setStyleSheet(QString());
        label_title->setText(QApplication::translate("AboutDialog", "Dwarf Therapist", 0, QApplication::UnicodeUTF8));
        lbl_image->setText(QString());
        frame_lower->setStyleSheet(QString());
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
