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
#ifndef HEALTHLEGENDDOCK_H
#define HEALTHLEGENDDOCK_H

#include "basedock.h"
#include "healthinfo.h"
#include "unithealth.h"

class HealthLegendDock : public BaseDock {
    Q_OBJECT
public:
    HealthLegendDock(QWidget *parent = 0, Qt::WindowFlags flags = 0);
private:
    QTreeWidget *legend;
    QLineEdit* le_search;
    QPushButton *btn_toggle_tree;

    QIcon arr_in;
    QIcon arr_out;

    void search_legend(QString val);
    bool m_collapsed;

    void closeEvent(QCloseEvent *event);

protected slots:
    void search_changed(QString val);
    void clear_search();
    void toggle_tree();
    void selection_changed();
    void clear_filter();

signals:
    void item_selected(QList<QPair<int, int> >);
};

#endif // HEALTHLEGENDDOCK_H
