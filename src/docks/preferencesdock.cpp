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
#include "preferencesdock.h"
#include "dwarftherapist.h"

PreferencesDock::PreferencesDock(QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setWindowTitle(tr("Preferences"));
    setObjectName("dock_preferences");
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    w->setLayout(l);

    // PREFERENCES TABLE
    tw_prefs = new QTableWidget(this);
    tw_prefs->setColumnCount(4);
    tw_prefs->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_prefs->setWordWrap(true);
    tw_prefs->setShowGrid(false);
    tw_prefs->setGridStyle(Qt::NoPen);
    tw_prefs->setAlternatingRowColors(true);
    tw_prefs->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tw_prefs->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw_prefs->setHorizontalHeaderLabels(QStringList() << "Name" << "+" << "-" << "Type");
    tw_prefs->verticalHeader()->hide();
    tw_prefs->horizontalHeader()->setResizeMode(0, QHeaderView::Interactive);
    tw_prefs->horizontalHeader()->setResizeMode(1, QHeaderView::Interactive);
    tw_prefs->horizontalHeader()->setResizeMode(2, QHeaderView::Interactive);
    tw_prefs->horizontalHeader()->setResizeMode(3, QHeaderView::Interactive);
    tw_prefs->horizontalHeader()->setDefaultSectionSize(75);
    tw_prefs->horizontalHeader()->resizeSection(1,40);
    tw_prefs->horizontalHeader()->resizeSection(2,40);

    QPushButton *btn = new QPushButton("Clear Filter",this);
    w->layout()->addWidget(tw_prefs);
    w->layout()->addWidget(btn);

    setWidget(w);

    connect(tw_prefs,SIGNAL(itemSelectionChanged()),this,SLOT(selection_changed()));
    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
}

void PreferencesDock::refresh(){
    for(int i = tw_prefs->rowCount(); i >=0; i--){
        tw_prefs->removeRow(i);
    }

    if(DT && DT->get_DFInstance()){
        QHash<QString,DFInstance::pref_stat > prefs = DT->get_DFInstance()->get_preference_stats();

        tw_prefs->setSortingEnabled(false);
        foreach(QString name, prefs.uniqueKeys()){
            DFInstance::pref_stat pref = prefs.value(name);
            tw_prefs->insertRow(0);
            tw_prefs->setRowHeight(0, 18);

            QTableWidgetItem *pref_name = new QTableWidgetItem();
            pref_name->setText(capitalize(name));
            pref_name->setToolTip(pref_name->text());

            QTableWidgetItem *pref_likes = new QTableWidgetItem();
            pref_likes->setData(Qt::DisplayRole, (int)pref.likes);
            pref_likes->setTextAlignment(Qt::AlignCenter);
            pref.names_likes.sort();
            pref_likes->setToolTip(pref.names_likes.join("<br>"));

            QTableWidgetItem *pref_dislikes = new QTableWidgetItem();
            pref_dislikes->setData(Qt::DisplayRole, (int)pref.dislikes);
            pref_dislikes->setTextAlignment(Qt::AlignCenter);
            pref.names_dislikes.sort();
            pref_dislikes->setToolTip(pref.names_dislikes.join("<br>"));

            QTableWidgetItem *pref_type = new QTableWidgetItem();
            pref_type->setText(pref.pref_category);
            pref_type->setToolTip(pref.pref_category);

            tw_prefs->setItem(0, 0, pref_name);
            tw_prefs->setItem(0, 1, pref_likes);
            tw_prefs->setItem(0, 2, pref_dislikes);
            tw_prefs->setItem(0, 3, pref_type);
        }
        tw_prefs->setSortingEnabled(true);
        tw_prefs->sortItems(1, Qt::DescendingOrder);
    }
}

void PreferencesDock::selection_changed(){
    QStringList selected;
    QModelIndexList indexList = tw_prefs->selectionModel()->selectedIndexes();
    int row;
    int prev_row=-1;
    foreach (QModelIndex index, indexList) {
        row = index.row();
        if(row != prev_row)
            selected.append(tw_prefs->item(row,0)->text().toLower());
        prev_row = row;
    }

    emit item_selected(selected);
}

void PreferencesDock::clear_filter(){
    tw_prefs->clearSelection();
}
