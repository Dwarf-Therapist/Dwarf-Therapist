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

#include <QVBoxLayout>
#include <QHeaderView>
#include <QCloseEvent>
#include "healthlegenddock.h"
#include "healthcategory.h"

HealthLegendDock::HealthLegendDock(QWidget *parent, Qt::WindowFlags flags)
    : BaseDock(parent, flags)
{
    setWindowTitle(tr("Health Legend"));
    setObjectName("dock_health_legend");
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    arr_in = QIcon(":img/arrow-in.png");
    arr_out = QIcon(":img/arrow-out.png");

    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    w->setLayout(l);

    legend = new QTreeWidget(this);
    legend->setEditTriggers(QAbstractItemView::NoEditTriggers);
    legend->setDropIndicatorShown(false);
    legend->header()->setVisible(false);
    legend->setProperty("showSortIndicator",QVariant(true));
    legend->setColumnCount(2);
    legend->setColumnWidth(0,100);
    legend->setSelectionMode(QAbstractItemView::ExtendedSelection);


    legend->setSortingEnabled(false);
    foreach(HealthCategory *hc, UnitHealth::get_display_categories()){
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
        legend->addTopLevelItem(parent_node);
        parent_node->setFirstColumnSpanned(true);
    }

    legend->setSortingEnabled(true);
    legend->sortItems(0,Qt::AscendingOrder);
    legend->expandAll();
    m_collapsed = false;

    QHBoxLayout *s = new QHBoxLayout();
    QLabel *lbl_search = new QLabel("Search",this);
    s->addWidget(lbl_search);
    le_search = new QLineEdit(this);
    le_search->setObjectName("le_search");
    s->addWidget(le_search);

    btn_toggle_tree = new QPushButton(this);
    btn_toggle_tree->setIcon(arr_in);
    s->addWidget(btn_toggle_tree);

    QPushButton *btn_clear_search = new QPushButton(this);
    QIcon icn_cross(":img/cross.png");
    btn_clear_search->setIcon(icn_cross);
    s->addWidget(btn_clear_search);

    l->addLayout(s);

    QPushButton *btn = new QPushButton("Clear Filter",this);
    w->layout()->addWidget(legend);
    w->layout()->addWidget(btn);

    setWidget(w);

    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
    connect(le_search, SIGNAL(textChanged(QString)), this, SLOT(search_changed(QString)));
    connect(btn_clear_search, SIGNAL(clicked()),this,SLOT(clear_search()));
    connect(btn_toggle_tree, SIGNAL(clicked()), this, SLOT(toggle_tree()));
    connect(legend, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));    
}

void HealthLegendDock::toggle_tree(){
    if(m_collapsed){
        legend->expandAll();
        m_collapsed = false;
        btn_toggle_tree->setIcon(arr_in);
    }else{
        legend->collapseAll();
        m_collapsed = true;
        btn_toggle_tree->setIcon(arr_out);
    }
}

void HealthLegendDock::search_changed(QString val){
    search_legend(val);
}

void HealthLegendDock::clear_search(){
    le_search->setText("");
    search_legend("");
}

void HealthLegendDock::search_legend(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    QRegExp filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    int hidden;
    for(int i = 0; i < legend->topLevelItemCount(); i++){
        hidden = 0;
        int count;
        for(count = 0; count < legend->topLevelItem(i)->childCount(); count++){
            if(!legend->topLevelItem(i)->child(count)->text(0).contains(filter) && !legend->topLevelItem(i)->child(count)->text(1).contains(filter)){
                legend->topLevelItem(i)->child(count)->setHidden(true);
                hidden++;
            }else{
                legend->topLevelItem(i)->child(count)->setHidden(false);
            }
        }
        if(hidden == count){
            legend->topLevelItem(i)->setHidden(true);
        }else{
            legend->topLevelItem(i)->setHidden(false);
            QString title = legend->topLevelItem(i)->data(0,Qt::UserRole+1).toString() + QString(" (%1)").arg(QString::number(count-hidden));
            legend->topLevelItem(i)->setText(0,title);
        }
    }
}

void HealthLegendDock::selection_changed(){
    QList<QPair<int,int> > values; //pairs of id and display index
    foreach(QTreeWidgetItem *item, legend->selectedItems()){
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

void HealthLegendDock::clear_filter(){
    legend->clearSelection();
}

void HealthLegendDock::closeEvent(QCloseEvent *event){
    clear_search();
    clear_filter();
    event->accept();
}
