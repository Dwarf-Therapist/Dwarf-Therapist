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
#ifndef NOTIFIERWIDGET_H
#define NOTIFIERWIDGET_H

#include <QDialog>

class MainWindow;
namespace Ui { class NotifierWidget; }

class NotifierWidget : public QDialog {
    Q_OBJECT
public:
    NotifierWidget(MainWindow *parent = 0);
    ~NotifierWidget();

    struct notify_info{
        QString title;
        QString details;
        QString url;
        QString url_msg;
        bool is_warning;
    };

public slots:
     void reposition();
     void add_notification(NotifierWidget::notify_info ni);

private:
    Ui::NotifierWidget *ui;
    void resizeEvent(QResizeEvent*);

private slots:
    void notification_closed();

};

#endif // NOTIFIERWIDGET_H

