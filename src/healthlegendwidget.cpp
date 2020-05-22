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

#include "healthlegendwidget.h"
#include "healthcategory.h"
#include "unithealth.h"

HealthLegendWidget::HealthLegendWidget(QWidget *parent)
    : BaseTreeWidget(false, parent)
{
    m_tree_view->setColumnCount(2);
    m_tree_view->setHeaderLabels(QStringList() << tr("Title/Abbrev") << tr("Description"));
}

void HealthLegendWidget::build_tree(){
    foreach(HealthCategory *hc, UnitHealth::get_display_categories().values()){
        if(hc->id() < 0)
            continue;

        QString name = hc->name();
        QTreeWidgetItem* parent_node = new QTreeWidgetItem();
        parent_node->setData(0, Qt::UserRole, hc->id());
        parent_node->setData(0, Qt::UserRole+1, name);
        parent_node->setText(0, name);

        //add parent node
        foreach(HealthInfo* hi, hc->descriptions()){
            QTreeWidgetItem *node = new QTreeWidgetItem(parent_node);

            node->setData(0, Qt::UserRole, hi->severity());
            node->setData(0,Qt::TextColorRole, hi->color());
            node->setText(0, hi->symbol(false));

            node->setData(1, Qt::UserRole, hi->description(false));
            node->setToolTip(1, hi->description(false));
            node->setText(1,hi->description(false));
        }
        m_tree_view->addTopLevelItem(parent_node);
        parent_node->setFirstColumnSpanned(true);
    }
    m_tree_view->sortItems(0,Qt::AscendingOrder);
}

void HealthLegendWidget::search_tree(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    QRegExp filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    int hidden;
    for(int i = 0; i < m_tree_view->topLevelItemCount(); i++){
        hidden = 0;
        int count;
        for(count = 0; count < m_tree_view->topLevelItem(i)->childCount(); count++){
            if(!m_tree_view->topLevelItem(i)->child(count)->text(0).contains(filter) && !m_tree_view->topLevelItem(i)->child(count)->text(1).contains(filter)){
                m_tree_view->topLevelItem(i)->child(count)->setHidden(true);
                hidden++;
            }else{
                m_tree_view->topLevelItem(i)->child(count)->setHidden(false);
            }
        }
        if(hidden == count){
            m_tree_view->topLevelItem(i)->setHidden(true);
        }else{
            m_tree_view->topLevelItem(i)->setHidden(false);
            QString title = m_tree_view->topLevelItem(i)->data(0,Qt::UserRole+1).toString() + QString(" (%1)").arg(QString::number(count-hidden));
            m_tree_view->topLevelItem(i)->setText(0,title);
        }
    }
}

void HealthLegendWidget::selection_changed(){
    QList<QPair<int,int> > values; //pairs of id and display index
    foreach(QTreeWidgetItem *item, m_tree_view->selectedItems()){
        int id = 0;
        int idx = -1;
        if(item->childCount() <= 0){
            id = item->parent()->data(0,Qt::UserRole).toInt();
            idx = item->data(0,Qt::UserRole).toInt();
        }else{
            id = item->data(0,Qt::UserRole).toInt();
        }
        values.append(qMakePair(id,idx));
    }
        emit item_selected(values);
}
