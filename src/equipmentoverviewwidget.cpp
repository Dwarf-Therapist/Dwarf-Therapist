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

#include "dfinstance.h"
#include "dwarftherapist.h"
#include "equipmentoverviewwidget.h"
#include "equipwarn.h"
#include "item.h"
#include "adaptivecolorfactory.h"

#include <QCheckBox>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QSettings>

QString EquipmentOverviewWidget::m_option_name = "options/docks/equipoverview_include_mats";

EquipmentOverviewWidget::EquipmentOverviewWidget(QWidget *parent)
    : BaseTreeWidget(true, parent)
{
    m_tree_view->setColumnCount(3);
    m_tree_view->setHeaderLabels(QStringList() << tr("Item") << tr("Status") << tr("Count"));
    m_tree_view->setItemDelegate(new EquipWarnItemDelegate(m_tree_view));

    QCheckBox *chk_mats = new QCheckBox(tr("Include item materials"),this);
    chk_mats->setChecked(DT->user_settings()->value(m_option_name,false).toBool());
    chk_mats->setToolTip(tr("When checked, prefixes the item name with the general material type."));
    lbl_read = new QLabel(tr("<font color='red'>Requires read to apply changes.</font>"));
    lbl_read->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
    lbl_read->hide();

    layout()->addWidget(chk_mats);
    layout()->addWidget(lbl_read);

    connect(chk_mats, SIGNAL(clicked(bool)),this,SLOT(check_changed(bool)));

    m_option_state = chk_mats->isChecked();

    m_wear_level_desc.insert(Item::IS_MISSING,Item::missing_group_name());
    m_wear_level_desc.insert(Item::IS_UNCOVERED,Item::uncovered_group_name());
    m_wear_level_desc.insert(Item::IS_CLOTHED,tr("No Worn/Missing Equipment"));
    m_wear_level_desc.insert(Item::IS_WORN,tr("Some Wear"));
    m_wear_level_desc.insert(Item::IS_THREADBARE,tr("Threadbare"));
    m_wear_level_desc.insert(Item::IS_TATTERED,tr("Tattered"));
}

void EquipmentOverviewWidget::build_tree(){
    lbl_read->hide();

    if(DT && DT->get_DFInstance()){
        QString tooltip;

        m_option_state = DT->user_settings()->value(m_option_name,false).toBool();
        QHash<ITEM_TYPE,EquipWarn*> eq_warnings = DT->get_DFInstance()->get_equip_warnings();

        foreach(ITEM_TYPE i_type, eq_warnings.uniqueKeys()){
            EquipWarn *ew = eq_warnings.value(i_type);
            QString item_name = Item::get_item_generic_name(i_type);
            QStringList warn_desc;
            QVariantMap warn_counts;
            foreach(Item::ITEM_STATE i_status, ew->get_wear_counts().uniqueKeys()){
                int count = ew->get_wear_counts().value(i_status);
                warn_desc.append(tr("<font color=%1>%2 %3</font>")
                                 .arg(Item::get_color(i_status).name())
                                 .arg(count)
                                 .arg(m_wear_level_desc.value(i_status)));
                warn_counts.insert(QString::number((int)i_status),QVariant(count));
            }
            tooltip = QString("<center><h4>%1 %2</h4></center>%3")
                    .arg(QString::number(ew->get_total_count()))
                    .arg(capitalize(item_name))
                    .arg(warn_desc.join("<br/><br/>"));

            SortableTreeItem* item_type_node = new SortableTreeItem();
            item_type_node->setData(0, Qt::UserRole, i_type);
            item_type_node->setText(0, capitalize(item_name));
            item_type_node->setToolTip(0,tooltip);
            item_type_node->setData(0,Qt::UserRole+1, warn_counts);

            //custom sorting
            item_type_node->setData(0,SortableTreeItem::TREE_SORT_COL,item_name.toLower());
            item_type_node->setData(2,SortableTreeItem::TREE_SORT_COL+2,ew->get_total_count());

            AdaptiveColorFactory adaptive;
            QPair<QString,Item::ITEM_STATE> warn_key;
            foreach(warn_key, ew->get_details().uniqueKeys()){
                EquipWarn::warn_count wc = ew->get_details().value(warn_key);

                QString detail_name = warn_key.first;
                Item::ITEM_STATE i_status = warn_key.second;
                QString wear_desc = m_wear_level_desc.value(i_status);

                QColor col = Item::get_color(i_status);
                SortableTreeItem *item_node = new SortableTreeItem(item_type_node);
                QStringList unit_names = wc.unit_ids.keys();
                tooltip = QString("<center><h4><font color=%1>%2 %3 (%4)</font></h4></center>%5%6")
                        .arg(col.name())
                        .arg(wc.count)
                        .arg(detail_name)
                        .arg(wear_desc)
                        .arg(wc.count != wc.unit_ids.count() ? tr("%1 items (<font color=%2>%3</font>) among %4 citizens.<br/><br/>")
                                                               .arg(wc.count)
                                                               .arg(col.name())
                                                               .arg(wear_desc)
                                                               .arg(wc.unit_ids.count()) : "")
                        .arg(unit_names.join(unit_names.size() < 20 ? "<br/>" : ", "));

                item_node->setData(0, Qt::UserRole, detail_name);
                item_node->setToolTip(0, tooltip);
                item_node->setText(0, detail_name);

                item_node->setToolTip(1, tooltip);
                item_node->setData(1,Qt::TextColorRole, adaptive.color(col));
                item_node->setText(1,wear_desc);

                item_node->setData(2, Qt::UserRole, wc.unit_ids.values());
                item_node->setToolTip(2, tooltip);
                item_node->setText(2,QString("%1").arg(wc.count,2,10,QChar('0')));
                item_node->setTextAlignment(2,Qt::AlignRight);

                //custom sorting
                item_node->setData(0,SortableTreeItem::TREE_SORT_COL,detail_name.toLower());
                item_node->setData(1,SortableTreeItem::TREE_SORT_COL+1,wear_desc);
                item_node->setData(2,SortableTreeItem::TREE_SORT_COL+2,wc.count);

            }
            m_tree_view->addTopLevelItem(item_type_node);
            item_type_node->setFirstColumnSpanned(true);
        }
    }
        m_tree_view->sortByColumn(2,Qt::DescendingOrder); //count
}

void EquipmentOverviewWidget::search_tree(QString val){
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

void EquipmentOverviewWidget::selection_changed(){
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

void EquipmentOverviewWidget::check_changed(bool val){
    DT->user_settings()->setValue(m_option_name,val);
    if(val != m_option_state){
        lbl_read->show();
    }else{
        lbl_read->hide();
    }
}

void EquipWarnItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
    QStyledItemDelegate::paint(painter, option, index);

    //only draw summary totals for top level items
    if(!index.parent().isValid())
    {
        painter->save();

        QVariantMap counts = index.data(Qt::UserRole+1).toMap();
        QColor default_pen = painter->pen().color();
        QString curr_text = "";

        AdaptiveColorFactory adaptive;
        foreach(QString wear_level, counts.uniqueKeys()){
            int count = counts.value(wear_level).toInt();
            QColor col = adaptive.color(Item::get_color(static_cast<Item::ITEM_STATE>(wear_level.toInt())));
            if(curr_text != ""){
                curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
            }
            curr_text = prependText(painter,option,col,curr_text,QString("%1 ").arg(count==0 ? "-- " : QString("%1").arg(count,2,10,QChar('0'))));
        }
        painter->restore();
    }
}

QString EquipWarnItemDelegate::appendText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
    int text_width = painter->fontMetrics().width(curr_text);
    QRect r_display = option.rect.adjusted(text_width,0,0,0);
    painter->setPen(text_color);
    painter->drawText(r_display,Qt::AlignLeft | Qt::AlignVCenter,text);
    return (curr_text + text);
}

QString EquipWarnItemDelegate::prependText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
    int text_width = -painter->fontMetrics().width(curr_text);
    QRect r_display = option.rect.adjusted(0,0,text_width,0);
    painter->setPen(text_color);
    painter->drawText(r_display,Qt::AlignRight | Qt::AlignVCenter,text);
    return (curr_text + text);
}
