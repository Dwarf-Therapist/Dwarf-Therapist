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
#ifndef CONTEXTMENUHELPER_H
#define CONTEXTMENUHELPER_H

#include <QtWidgets>

class ContextMenuHelper : public QObject
{
    Q_OBJECT
public :

    ContextMenuHelper(QObject *parent=0)
        :QObject(parent)
    {}

    QMenu *create_title_menu(QMenu *root, QString title, QString tooltip, bool add_all_action = true){
        QMenu *tmp = root->addMenu(title);
        tmp->setToolTip(tooltip);
        tmp->setTearOffEnabled(true);
        if(add_all_action)
            add_all_menu(tmp);
        return tmp;
    }

    void add_sub_menus(QMenu *m, int groups, bool add_all=true){
        int step = 26;
        if(groups > 0)
            step = 26 / groups;

        if(step >= 26)
            return;

        if(step <= 0)
            step = 1;
        QMenu *tmp;
        int idx_next;
        if(m){
            for(int idx=65; idx <= 90; idx=idx+step){
                if(idx == 90 && step > 1)
                    continue;
                if(idx+step+1 > 90){
                    idx_next = 90;
                }else{
                    idx_next = idx+step-1;
                }
                if(step == 1){
                    tmp = m->addMenu(QString("%1").arg(QChar(idx)));
                }else{
                    tmp = m->addMenu(QString("%1-%2").arg(QChar(idx)).arg(QChar(idx_next)));
                }
                tmp->setTearOffEnabled(true);
                if(add_all)
                    add_all_menu(tmp);
            }
        }
    }

    QMenu* find_menu(QMenu *root, QString name){
        QMenu *ret = 0;
        QList<QMenu*> menus = root->findChildren<QMenu*>();
        if(menus.count()>0){
            ret = menus.at(0);
            foreach(QMenu *m, menus){
                QChar first = name.toUpper().at(0);
                if((m->title().length() > 1 && first >= m->title().at(0) && first <= m->title().at(2)) || first == m->title().at(0)){
                    ret = m;
                    break;
                }
            }
            return ret;
        }else{
            return root;
        }
    }

    void add_all_menu(QMenu *m){
        m->addAction("Add All", this, SLOT(add_all()));
        m->addSeparator();
    }

public slots:
    void add_all(){
        QAction *a = qobject_cast<QAction*>(QObject::sender());
        if(a){
            QMenu *parent_menu = qobject_cast<QMenu*>(a->parent());
            if(parent_menu){
                foreach(QAction *root_action, parent_menu->actions()){
                    if(root_action->data().isValid())
                        root_action->trigger();
                }
                QList<QMenu*> menus = parent_menu->findChildren<QMenu*>();
                foreach(QMenu *m, menus){
                    foreach(QAction *sub_action, m->actions()){
                        if(sub_action->data().isValid())
                            sub_action->trigger();
                    }
                }
            }
        }
        all_clicked();
    }

signals:
    void all_clicked();

};
#endif // CONTEXTMENUHELPER_H
