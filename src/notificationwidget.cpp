/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "notificationwidget.h"
#include "dwarftherapist.h"
#include "utils.h"

#include <QDesktopServices>
#include <QUrl>

class NotifierWidget;

NotificationWidget::NotificationWidget(NotifierWidget::notify_info ni,NotifierWidget *parent)
    : QFrame(parent)
    , ui(new Ui::NotificationWidget)
{
    ui->setupUi(this);

    setFrameShape(QFrame::StyledPanel);
    setStyleSheet(QString("QFrame{background: %1}").arg(QFrame::palette().color(QPalette::Window).name()));

    connect(ui->btn_close,SIGNAL(clicked(bool)),this,SLOT(close_notification()));
    connect(ui->lbl_msg,SIGNAL(linkActivated(QString)),this,SLOT(url_clicked(QString)));

    display(ni);
}

NotificationWidget::~NotificationWidget(){
    delete ui;
}

void NotificationWidget::display(NotifierWidget::notify_info ni){
    if(ni.is_warning){
        ui->lbl_icon->setPixmap(QPixmap(":img/exclamation--frame.png"));
    }
    if(!ni.title.trimmed().isEmpty()){
        ui->lbl_title->setText(QString("<b>%1</b>").arg(ni.title));
    }

    //format the message
    QString msg = "";
    if(!ni.details.trimmed().isEmpty()){
        msg.append(QString("<blockquote>%1</blockquote>").arg(ni.details));
    }
    if(!ni.url.trimmed().isEmpty()){
        QString url_msg = ni.url_msg;
        if(url_msg.isEmpty()){
            url_msg = tr("Click here for more details.");
        }
        msg.append(QString("<nobr><p><a href=\"%1\" style=\"color: %2\">%3</p></nobr>")
                   .arg(ni.url)
                   .arg(QFrame::palette().color(QPalette::Text).name())
                   .arg(url_msg));
    }

    ui->lbl_msg->setText(msg);
    ui->lbl_msg->setToolTip(msg);

}

void NotificationWidget::url_clicked(const QString &url){
    QDesktopServices::openUrl(QUrl(url));
    close_notification();
}

void NotificationWidget::close_notification(){
    close();
    emit closed();
}
