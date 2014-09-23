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
#include <QLineEdit>
#include <QHeaderView>
#include <QPushButton>
#include <QCheckBox>
#include <QCloseEvent>
#include "equipmentoverviewdock.h"
#include "dwarftherapist.h"
#include "sortabletableitems.h"
#include "item.h"

#if QT_VERSION < 0x050000
# define setSectionResizeMode setResizeMode
#endif

QString EquipmentOverviewDock::m_option_name = "options/docks/equipoverview_include_mats";

EquipmentOverviewDock::EquipmentOverviewDock(QWidget *parent, Qt::WindowFlags flags)
    : BaseDock(parent, flags)
{
    setWindowTitle(tr("Equipment Overview"));
    setObjectName("dock_equipmentoverview");
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    w->setLayout(l);

    tw_wear = new QTableWidget(this);
    tw_wear->setColumnCount(3);
    tw_wear->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_wear->setWordWrap(true);
    tw_wear->setShowGrid(false);
    tw_wear->setGridStyle(Qt::NoPen);
    tw_wear->setAlternatingRowColors(true);
    tw_wear->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tw_wear->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw_wear->setHorizontalHeaderLabels(QStringList() << tr("Item") << tr("Count") << tr("Status"));
    tw_wear->verticalHeader()->hide();
    tw_wear->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    tw_wear->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    tw_wear->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    tw_wear->setColumnWidth(0,100);
    tw_wear->setColumnWidth(1,50);
    tw_wear->horizontalHeader()->setStretchLastSection(true);

    QHBoxLayout *s = new QHBoxLayout();
    QLabel *lbl_search = new QLabel("Search",this);
    s->addWidget(lbl_search);
    QLineEdit *le_search = new QLineEdit(this);
    le_search->setObjectName("le_search");
    s->addWidget(le_search);
    QPushButton *btn_clear_search = new QPushButton(this);
    QIcon icn(":img/cross.png");
    btn_clear_search->setIcon(icn);
    s->addWidget(btn_clear_search);
    l->addLayout(s);

    QPushButton *btn = new QPushButton("Clear Filter",this);
    QCheckBox *chk_mats = new QCheckBox(tr("Include item materials"),this);
    chk_mats->setChecked(DT->user_settings()->value(m_option_name,false).toBool());
    chk_mats->setToolTip(tr("When checked, prefixes the item name with the general material type."));
    lbl_read = new QLabel(tr("<font color='red'>Requires read to apply changes.</font>"));
    lbl_read->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Minimum);
    lbl_read->hide();

    w->layout()->addWidget(tw_wear);
    w->layout()->addWidget(chk_mats);
    w->layout()->addWidget(lbl_read);
    w->layout()->addWidget(btn);

    setWidget(w);

    connect(tw_wear,SIGNAL(itemSelectionChanged()),this,SLOT(selection_changed()));
    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
    connect(le_search, SIGNAL(textChanged(QString)),this, SLOT(search_changed(QString)));
    connect(btn_clear_search, SIGNAL(clicked()),this,SLOT(clear_search()));
    connect(chk_mats, SIGNAL(clicked(bool)),this,SLOT(check_changed(bool)));

    connect(DT,SIGNAL(units_refreshed()),this,SLOT(refresh()));

    m_option_state = chk_mats->isChecked();
}

void EquipmentOverviewDock::clear(){
    for(int r = tw_wear->rowCount(); r >=0; r--){
        tw_wear->removeRow(r);
    }
    tw_wear->clearContents();
}

void EquipmentOverviewDock::refresh(){
    clear();
    lbl_read->hide();

    if(DT && DT->get_DFInstance()){
        QString tooltip;
        QPair<QString,int> key;

        m_option_state = DT->user_settings()->value(m_option_name,false).toBool();
        QHash<QPair<QString,int>,int> worn_items = DT->get_DFInstance()->get_equip_warnings();
        if(worn_items.count()<=0){ //add a placeholder item
            key = qMakePair(tr("N/A"),0);
            worn_items.insert(key,0);
        }

        QStringList wear_level_desc;
        wear_level_desc << tr("No Worn/Missing Equipment") << tr("Some Wear") << tr("Heavily Worn") << tr("Tattered");

        tw_wear->setSortingEnabled(false);
        foreach(key, worn_items.uniqueKeys()){
            QString item_desc = key.first;
            int wear_level = key.second;
            int total_count = worn_items.value(key);

            tw_wear->insertRow(0);
            tw_wear->setRowHeight(0, 18);

            QString wear_desc;
            QColor col;
            if(wear_level >= 0){
                wear_desc = wear_level_desc.at(wear_level);
                col = Item::color_wear(wear_level);
            }else{
                wear_desc = Item::uncovered_group_name();
                col = Item::color_uncovered();
            }

            tooltip = QString("<center><h4>%1</h4></center>%2 %3")
                    .arg(capitalizeEach(item_desc))
                    .arg(total_count).arg(wear_desc);

            QTableWidgetItem *item_name = new QTableWidgetItem();
            item_name->setData(Qt::UserRole, item_desc);
            item_name->setText(item_desc);
            item_name->setToolTip(tooltip);

            QTableWidgetItem *item_wear_count = new QTableWidgetItem();
            item_wear_count->setData(Qt::DisplayRole, total_count);
            item_wear_count->setTextAlignment(Qt::AlignCenter);
            item_wear_count->setToolTip(tooltip);

            sortableNumericTableWidgetItem *item_wear_desc = new sortableNumericTableWidgetItem;
            item_wear_desc->setData(Qt::UserRole, wear_level);
            item_wear_desc->setText(wear_desc);
            item_wear_desc->setBackgroundColor(col);
            item_wear_desc->setToolTip(tooltip);


            tw_wear->setItem(0, 0, item_name);
            tw_wear->setItem(0, 1, item_wear_count);
            tw_wear->setItem(0, 2, item_wear_desc);
        }
        tw_wear->setSortingEnabled(true);
        tw_wear->sortItems(1, Qt::DescendingOrder);
        filter();
    }
}

void EquipmentOverviewDock::selection_changed(){
    QModelIndexList indexList = tw_wear->selectionModel()->selectedIndexes();
    QList<QPair<QString,int> > selected;
    if(indexList.count() > 0){
        int row = 0;
        int prev_row=-1;
        foreach (QModelIndex index, indexList) {
            row = index.row();
            if(row != prev_row)
                selected.append(qMakePair(tw_wear->item(row,0)->data(Qt::UserRole).toString(),tw_wear->item(row,2)->data(Qt::UserRole).toInt()));
            prev_row = row;
        }
    }
    emit item_selected(selected);
}

void EquipmentOverviewDock::search_changed(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    m_filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    filter();
}

void EquipmentOverviewDock::filter(){
    for(int i = 0; i < tw_wear->rowCount(); i++){
        if(m_filter.isEmpty() || tw_wear->item(i,0)->text().contains(m_filter) || tw_wear->item(i,2)->text().contains(m_filter)){
            tw_wear->setRowHidden(i,false);
        }else{
            tw_wear->setRowHidden(i,true);
        }
    }
}

void EquipmentOverviewDock::clear_filter(){
    tw_wear->clearSelection();
}

void EquipmentOverviewDock::check_changed(bool val){
    DT->user_settings()->setValue(m_option_name,val);
    if(val != m_option_state){
        lbl_read->show();
    }else{
        lbl_read->hide();
    }
}

void EquipmentOverviewDock::clear_search(){
    QLineEdit *s = qobject_cast<QLineEdit*>(QObject::findChild<QLineEdit*>("le_search"));
    if(s)
        s->setText("");
}

void EquipmentOverviewDock::closeEvent(QCloseEvent *event){
    clear_search();
    clear_filter();
    event->accept();
}

