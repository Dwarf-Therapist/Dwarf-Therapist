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
#include "ui_notification.h"

#include <QDesktopServices>
#include <QUrl>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <QFrame>

NotificationWidget::NotificationWidget(NotifierWidget::notify_info ni,NotifierWidget *parent)
    : QFrame(parent)
    , ui(new Ui::NotificationWidget)
    , m_fader(0)
    , m_mouse_hover(false)
{
    ui->setupUi(this);

    setFrameShape(QFrame::StyledPanel);
    setAttribute(Qt::WA_Hover);
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
        msg.append(QString("<p><a href=\"%1\" style=\"color: %2\">%3</p>")
                   .arg(ni.url)
                   .arg(QFrame::palette().color(QPalette::Text).name())
                   .arg(url_msg));
    }

    ui->lbl_msg->setText(msg);
    ui->lbl_msg->setToolTip(msg);

    //start fading after 10s
    QTimer::singleShot(10000, this, SLOT(fade_out_start()));
}

void NotificationWidget::fade_out_start(){
    if(!m_fader){
        QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
        this->setGraphicsEffect(eff);
        m_fader = new QPropertyAnimation(eff,"opacity");
        m_fader->setStartValue(1);
        m_fader->setEndValue(0);
        m_fader->setDuration(20000);
        m_fader->setEasingCurve(QEasingCurve::OutBack);
        m_fader->start(QPropertyAnimation::DeleteWhenStopped);
        //close the notification as soon as opacity hits 0; this is often before the animation finishes
        connect(eff,SIGNAL(opacityChanged(qreal)),this,SLOT(opacity_changed(qreal)));
    }
}

void NotificationWidget::opacity_changed(qreal opacity){
    if(opacity <= 0){
        if(m_fader){
            m_fader->stop();
        }
        close_notification();
    }else{
        //if the mouse is on the notification, or there is a dialog open, reset the fade animation
        if((m_mouse_hover || QApplication::activeModalWidget()) && opacity < 1){
            m_fader->setCurrentTime(0);
        }
    }
}

void NotificationWidget::enterEvent(QEvent *evt){
    QFrame::enterEvent(evt);
    m_mouse_hover = true;
    if(m_fader){
        m_fader->pause();
        m_fader->setCurrentTime(0);
    }
}

void NotificationWidget::leaveEvent(QEvent *evt){
    QFrame::leaveEvent(evt);
    m_mouse_hover = false;
    if(m_fader && m_fader->state() == QAbstractAnimation::Paused){
        m_fader->resume();
    }
}

void NotificationWidget::url_clicked(const QString &url){
    QDesktopServices::openUrl(QUrl(url));
    close_notification();
}

void NotificationWidget::close_notification(){
    close();
    emit closed();
}
