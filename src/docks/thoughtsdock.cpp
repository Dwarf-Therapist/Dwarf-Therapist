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
#include "thoughtsdock.h"
#include "dwarftherapist.h"
#include "thought.h"
#include "gamedatareader.h"
#include "dfinstance.h"
#include "emotiongroup.h"
#include "emotion.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHeaderView>
#include <QPushButton>
#include <QCloseEvent>

ThoughtsDock::ThoughtsDock(QWidget *parent, Qt::WindowFlags flags)
    : BaseDock(parent, flags)
{
    setWindowTitle(tr("Thoughts"));
    setObjectName("dock_thoughts");
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    arr_in = QIcon(":img/arrow-in.png");
    arr_out = QIcon(":img/arrow-out.png");

    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    w->setLayout(l);

    tw_thoughts = new QTreeWidget(this);
    tw_thoughts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tw_thoughts->setDropIndicatorShown(false);
    tw_thoughts->header()->setVisible(true);
    tw_thoughts->setProperty("showSortIndicator",QVariant(true));
    tw_thoughts->setColumnCount(2);
    tw_thoughts->setColumnWidth(200,30);
    tw_thoughts->setHeaderLabels(QStringList() << "Description" << "Count");
    tw_thoughts->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tw_thoughts->setItemDelegate(new ThoughtsItemDelegate());

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
    w->layout()->addWidget(tw_thoughts);
    w->layout()->addWidget(btn);

    setWidget(w);

    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
    connect(le_search, SIGNAL(textChanged(QString)), this, SLOT(search_changed(QString)));
    connect(btn_clear_search, SIGNAL(clicked()),this,SLOT(clear_search()));
    connect(btn_toggle_tree, SIGNAL(clicked()), this, SLOT(toggle_tree()));
    connect(tw_thoughts, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));

    if(DT)
        connect(DT,SIGNAL(units_refreshed()),this,SLOT(refresh()));
}

void ThoughtsDock::refresh(){
    tw_thoughts->clear();

    if(DT && DT->get_DFInstance()){
        QHash<int, EmotionGroup*> emotions = DT->get_DFInstance()->get_emotion_stats();

        tw_thoughts->setSortingEnabled(false);
        QString tooltip;
        foreach(int id, emotions.uniqueKeys()){
            Thought *t = GameDataReader::ptr()->get_thought(id);
            EmotionGroup *eg = emotions.value(id);
            QString count_desc = eg->get_count_desc();

            int stress_count = eg->get_stress_count();
            int unaffected_count = eg->get_unaffected_count();
            int eustress_count = eg->get_eustress_count();
            QStringList stress_desc;
            if(stress_count > 0)
                stress_desc.append(tr("%1 felt negative emotions which added to their stress.").arg(stress_count));
            if(unaffected_count > 0)
                stress_desc.append(tr("%1 were unaffected.").arg(unaffected_count));
            if(eustress_count > 0)
                stress_desc.append(tr("%1 felt positive emotions which reduced stress.").arg(eustress_count));

            tooltip = QString("<center><h4>%1</h4></center>%2<br/><br/>%3")
                    .arg(capitalize(t->title()))
                    .arg(tr("Felt ... ") + t->desc())
                    .arg(stress_desc.join("<br/><br/>"));

            SortableTreeItem* parent_node = new SortableTreeItem();
            parent_node->setData(0, Qt::UserRole, id);
            parent_node->setText(0, capitalize(t->title()));
            parent_node->setToolTip(0,tooltip);

            QVariantList counts;
            counts << stress_count << unaffected_count << eustress_count;
            parent_node->setData(0,Qt::UserRole+1, counts);

            parent_node->setData(0,Qt::UserRole+50,t->title().toLower());//custom sorting
            parent_node->setData(1,Qt::UserRole+51,eg->get_total_count());//custom sorting

            //add parent node
            foreach(EMOTION_TYPE e_type, eg->get_details().uniqueKeys()){
                Emotion *e = GameDataReader::ptr()->get_emotion(e_type);
                EmotionGroup::emotion_count ec = eg->get_details().value(e_type);

                QString emotion_desc;
                if(e){
                    emotion_desc = e->get_name();
                }
                SortableTreeItem *node = new SortableTreeItem(parent_node);

                tooltip = QString("<center><h4><font color=%1>%2</font></h4></center>%3")
                        .arg(e->get_color().name())
                        .arg(e->get_name())
                        .arg(ec.unit_names.join("<br/>"));

                node->setData(0, Qt::UserRole, e_type);
                node->setData(0,Qt::TextColorRole, e->get_color());
                node->setToolTip(0, tooltip);
                node->setText(0, emotion_desc);

                node->setData(0,Qt::UserRole+50,e->get_name().toLower());//custom sorting
                node->setData(1,Qt::UserRole+51,ec.count);//custom sorting

                node->setData(1, Qt::UserRole, ec.unit_ids);
                node->setToolTip(1, tooltip);
                node->setText(1,QString("%1").arg(ec.count,2,10,QChar('0')));
                node->setTextAlignment(1,Qt::AlignRight);
            }
            tw_thoughts->addTopLevelItem(parent_node);
            parent_node->setFirstColumnSpanned(true);
        }
        tw_thoughts->setSortingEnabled(true);
        tw_thoughts->sortByColumn(1,Qt::DescendingOrder); //count
        tw_thoughts->expandAll(); //expand before resize to contents; hidden items aren't resized
        tw_thoughts->resizeColumnToContents(0);
        tw_thoughts->collapseAll();
        m_collapsed = true;
    }
}

void ThoughtsDock::toggle_tree(){
    if(m_collapsed){
        tw_thoughts->expandAll();
        m_collapsed = false;
        btn_toggle_tree->setIcon(arr_in);
    }else{
        tw_thoughts->collapseAll();
        m_collapsed = true;
        btn_toggle_tree->setIcon(arr_out);
    }
}

void ThoughtsDock::clear(){
    tw_thoughts->clear();
}

void ThoughtsDock::search_changed(QString val){
    search_tree(val);
}

void ThoughtsDock::clear_search(){
    le_search->setText("");
    search_tree("");
}

void ThoughtsDock::search_tree(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    QRegExp filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    int hidden;
    bool parent_matches;
    for(int i = 0; i < tw_thoughts->topLevelItemCount(); i++){
        hidden = 0;
        parent_matches = tw_thoughts->topLevelItem(i)->text(0).contains(filter);
        int count;
        for(count = 0; count < tw_thoughts->topLevelItem(i)->childCount(); count++){
            if(!parent_matches && !tw_thoughts->topLevelItem(i)->child(count)->text(0).contains(filter)){
                tw_thoughts->topLevelItem(i)->child(count)->setHidden(true);
                hidden++;
            }else{
                tw_thoughts->topLevelItem(i)->child(count)->setHidden(false);
            }
        }
        if(hidden == count){
            tw_thoughts->topLevelItem(i)->setHidden(true);
        }else{
            tw_thoughts->topLevelItem(i)->setHidden(false);
        }
    }
}

void ThoughtsDock::selection_changed(){
    QVariantList ids; //dwarf ids
    foreach(QTreeWidgetItem *item, tw_thoughts->selectedItems()){
        if(item->childCount() <= 0){
            ids.append(item->data(1,Qt::UserRole).toList());
        }else{
            for(int idx=0;idx < item->childCount();idx++){
                ids.append(item->child(idx)->data(1,Qt::UserRole).toList());
            }
        }
    }
        emit item_selected(ids);
}

void ThoughtsDock::clear_filter(){
    tw_thoughts->clearSelection();
}

void ThoughtsDock::closeEvent(QCloseEvent *event){
    clear_search();
    clear_filter();
    event->accept();
}
