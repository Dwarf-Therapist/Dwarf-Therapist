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

#include "notifierwidget.h"
#include "dwarftherapist.h"
#include "mainwindow.h"
#include "utils.h"
#include "notificationwidget.h"

#include <QStatusBar>

NotifierWidget::NotifierWidget(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::NotifierWidget)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
#if QT_VERSION < QT_VERSION_CHECK(5,0,0)
    setAttribute(Qt::WA_X11DoNotAcceptFocus);
#else
    setWindowFlags(Qt::Window | //necessary to show window?
                   Qt::WindowDoesNotAcceptFocus | //no focus
                   Qt::FramelessWindowHint); //no border
#endif
    this->setVisible(false);
}

NotifierWidget::~NotifierWidget(){
    delete ui;
}

void NotifierWidget::add_notification(NotifierWidget::notify_info ni){
    NotificationWidget *nw = new NotificationWidget(ni,this);
    ui->layout_main->addWidget(nw);

    connect(nw,SIGNAL(closed()),this,SLOT(notification_closed()));

    if(this->isHidden()){
        show();
        if(testAttribute(Qt::WA_X11DoNotAcceptFocus)){
            activateWindow();
        }
    }
}

void NotifierWidget::resizeEvent(QResizeEvent *evt){
    QDialog::resizeEvent(evt);
    reposition();
}

void NotifierWidget::notification_closed(){
    adjustSize();
}

void NotifierWidget::reposition(){
    if(this->findChildren<NotificationWidget*>().count() > 0){
        //reposition the notification to the bottom right corner of the application
        QPoint mainwindow_br = this->parentWidget()->frameGeometry().bottomRight();
        //padding to set it above the status bar
        QSize pad(12,32);
        if(DT->get_main_window()->statusBar()){
            pad.setHeight(DT->get_main_window()->statusBar()->height()+10);
        }
        mainwindow_br.setY(mainwindow_br.y()-pad.height());
        mainwindow_br.setX(mainwindow_br.x()-pad.width());
        QRect this_geo = this->frameGeometry();
        this_geo.moveBottomRight(mainwindow_br);
        this->move(this_geo.topLeft());
        //ensure it's on top
        raise();
        if(testAttribute(Qt::WA_X11DoNotAcceptFocus)){
            activateWindow();
        }
    }else{
        this->setHidden(true);
    }
}
