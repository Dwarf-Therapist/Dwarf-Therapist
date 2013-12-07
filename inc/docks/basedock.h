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
#ifndef BASEDOCK_H
#define BASEDOCK_H

#include <QtWidgets>

class BaseDock : public QDockWidget {
    Q_OBJECT
public:
    BaseDock(QWidget *parent = 0, Qt::WindowFlags flags = 0)
        :QDockWidget(parent,flags)
    {
        connect(this,SIGNAL(topLevelChanged(bool)),this,SLOT(floating_changed(bool)));
    }

private slots:
    void floating_changed(bool floating){
        if(this->isVisible() && floating){
            this->setWindowFlags(Qt::Window);
            QPoint pos = this->pos();
            if(pos.x() < 0)
                pos.setX(0);
            if(pos.y() < 0)
                pos.setY(0);
            this->move(pos);
            this->show();
        }
    }

};

#endif // BASEDOCK_H
