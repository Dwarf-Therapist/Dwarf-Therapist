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

#include "thoughtswidget.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "dfinstance.h"
#include "thought.h"
#include "emotion.h"
#include "emotiongroup.h"
#include "adaptivecolorfactory.h"

#include <QPainter>

ThoughtsWidget::ThoughtsWidget(QWidget *parent)
    : BaseTreeWidget(true, parent)
{
    m_tree_view->setColumnCount(3);
    m_tree_view->setHeaderLabels(QStringList() << tr("Thought/Emotion") << tr("Strength") << tr("Count"));
    m_tree_view->setItemDelegate(new ThoughtsItemDelegate(m_tree_view));
}

void ThoughtsWidget::build_tree(){
    QHash<int, EmotionGroup*> emotions = DT->get_DFInstance()->get_emotion_stats();
    QString tooltip;
    foreach(int id, emotions.uniqueKeys()){
        Thought *t = GameDataReader::ptr()->get_thought(id);
        EmotionGroup *eg = emotions.value(id);

        int stress_count = eg->get_stress_unit_count();
        int unaffected_count = eg->get_unaffected_unit_count();
        int eustress_count = eg->get_eustress_unit_count();
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

        SortableTreeItem* thought_node = new SortableTreeItem();
        thought_node->setData(0, Qt::UserRole, id);
        thought_node->setText(0, capitalize(t->title()));
        thought_node->setToolTip(0,tooltip);

        QVariantList total_counts;
        total_counts << eg->get_stress_count() << eg->get_unaffected_count() << eg->get_eustress_count();
        thought_node->setData(0,Qt::UserRole+1, total_counts);

        //custom sorting
        thought_node->setData(0,SortableTreeItem::TREE_SORT_COL,t->title().toLower());
        thought_node->setData(2,SortableTreeItem::TREE_SORT_COL+2,eg->get_total_occurrances());

        //add parent node
        AdaptiveColorFactory adaptive;
        foreach(EMOTION_TYPE e_type, eg->get_details().uniqueKeys()){
            Emotion *e = GameDataReader::ptr()->get_emotion(e_type);
            if(e){
                EmotionGroup::emotion_count ec = eg->get_details().value(e_type);

                QString emotion_desc;

                emotion_desc = e->get_name();

                SortableTreeItem *emotion_node = new SortableTreeItem(thought_node);
                QStringList unit_names = ec.unit_ids.keys();
                tooltip = QString("<center><h4><font color=%1>%2</font></h4></center>%3%4")
                        .arg(e->get_color().name())
                        .arg(e->get_name())
                        .arg(ec.count != ec.unit_ids.count() ? tr("This circumstance occurred %1 times among %2 citizens.<br/><br/>").arg(ec.count).arg(ec.unit_ids.count()) : "")
                        .arg(unit_names.join(unit_names.size() < 20 ? "<br/>" : ", "));

                emotion_node->setData(0, Qt::UserRole, e_type);
                emotion_node->setData(0,Qt::TextColorRole, adaptive.color(e->get_color()));
                emotion_node->setToolTip(0, tooltip);
                emotion_node->setText(0, emotion_desc);

                //use the inverted stress divisor as the strength of the emotion
                int strength = e->get_divider();
                if(strength != 0)
                    strength = -8/strength;

                emotion_node->setToolTip(1, tooltip);
                emotion_node->setText(1,QString("%1").arg(strength));
                emotion_node->setTextAlignment(1,Qt::AlignCenter);

                emotion_node->setData(2, Qt::UserRole, ec.unit_ids.values());
                emotion_node->setToolTip(2, tooltip);
                emotion_node->setText(2,QString("%1").arg(ec.count,2,10,QChar('0')));
                emotion_node->setTextAlignment(2,Qt::AlignRight);

                //custom sorting
                emotion_node->setData(0,SortableTreeItem::TREE_SORT_COL,e->get_name().toLower());
                emotion_node->setData(1,SortableTreeItem::TREE_SORT_COL+1,strength);
                emotion_node->setData(2,SortableTreeItem::TREE_SORT_COL+2,ec.count);

            }
            m_tree_view->addTopLevelItem(thought_node);
            thought_node->setFirstColumnSpanned(true);
        }
    }
    m_tree_view->sortByColumn(2,Qt::DescendingOrder); //count
}

void ThoughtsWidget::search_tree(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    QRegExp filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    int hidden;
    bool parent_matches;
    for(int i = 0; i < m_tree_view->topLevelItemCount(); i++){
        hidden = 0;
        parent_matches = m_tree_view->topLevelItem(i)->text(0).contains(filter);
        int count;
        for(count = 0; count < m_tree_view->topLevelItem(i)->childCount(); count++){
            if(!parent_matches && !m_tree_view->topLevelItem(i)->child(count)->text(0).contains(filter)){
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
        }
    }
}

void ThoughtsWidget::selection_changed(){
    QVariantList ids; //dwarf ids
    foreach(QTreeWidgetItem *item, m_tree_view->selectedItems()){
        if(item->childCount() <= 0){
            ids.append(item->data(2,Qt::UserRole).toList());
        }else{
            for(int idx=0;idx < item->childCount();idx++){
                ids.append(item->child(idx)->data(2,Qt::UserRole).toList());
            }
        }
    }
    emit item_selected(ids);
}

void ThoughtsItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    //only draw summary totals for top level items
    if(!index.parent().isValid())
    {
        AdaptiveColorFactory adaptive;

        painter->save();

        QVariantList counts = index.data(Qt::UserRole+1).toList();
        QColor default_pen = painter->pen().color();
        QString curr_text = "";
        //draw the eustress count
        int count = counts.at(2).toInt();
        curr_text = prependText(painter,option,adaptive.color(Qt::darkGreen),curr_text,QString("%1 ").arg(count==0 ? "-- " : QString("%1").arg(count,2,10,QChar('0'))));
        //spacer
        curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
        //draw the neutral count
        count = counts.at(1).toInt();
        curr_text = prependText(painter,option,adaptive.gray(0.5),curr_text,QString("%1").arg(count==0 ? " --" : QString("%1").arg(count,2,10,QChar('0'))));
        curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
        //draw the stress count
        count = counts.at(0).toInt();
        curr_text = prependText(painter,option,adaptive.color(Qt::darkRed),curr_text,QString(" %1").arg(count==0 ? " --" : QString("%1").arg(count,2,10,QChar('0'))));

        painter->restore();
    }
}

QString ThoughtsItemDelegate::appendText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
    int text_width = painter->fontMetrics().width(curr_text);
    QRect r_display = option.rect.adjusted(text_width,0,0,0);
    painter->setPen(text_color);
    painter->drawText(r_display,Qt::AlignLeft | Qt::AlignVCenter,text);
    return (curr_text + text);
}

QString ThoughtsItemDelegate::prependText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
    int text_width = -painter->fontMetrics().width(curr_text);
    QRect r_display = option.rect.adjusted(0,0,text_width,0);
    painter->setPen(text_color);
    painter->drawText(r_display,Qt::AlignRight | Qt::AlignVCenter,text);
    return (curr_text + text);
}
