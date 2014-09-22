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
#include <QVBoxLayout>
#include <QLineEdit>
#include <QHeaderView>
#include <QPushButton>
#include <QCloseEvent>

#if QT_VERSION < 0x050000
# define setSectionResizeMode setResizeMode
#endif

ThoughtsDock::ThoughtsDock(QWidget *parent, Qt::WindowFlags flags)
    : BaseDock(parent, flags)
{
    setWindowTitle(tr("Thoughts"));
    setObjectName("dock_thoughts");
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    QWidget *w = new QWidget();
    QVBoxLayout *l = new QVBoxLayout();
    w->setLayout(l);

    // THOUGHTS TABLE
    tw_thoughts = new QTableWidget(this);
    tw_thoughts->setColumnCount(3);
    tw_thoughts->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_thoughts->setWordWrap(true);
    tw_thoughts->setShowGrid(false);
    tw_thoughts->setGridStyle(Qt::NoPen);
    tw_thoughts->setAlternatingRowColors(true);
    tw_thoughts->setSelectionMode(QAbstractItemView::ExtendedSelection);
    tw_thoughts->setSelectionBehavior(QAbstractItemView::SelectRows);
    tw_thoughts->setHorizontalHeaderLabels(QStringList() << "Thought" << "Count" << "Description");
    tw_thoughts->verticalHeader()->hide();
    tw_thoughts->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Interactive);
    tw_thoughts->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    tw_thoughts->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Interactive);
    tw_thoughts->setColumnWidth(0,100);
    tw_thoughts->setColumnWidth(1,50);
    tw_thoughts->horizontalHeader()->setStretchLastSection(true);

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
    w->layout()->addWidget(tw_thoughts);
    w->layout()->addWidget(btn);

    setWidget(w);

    connect(tw_thoughts,SIGNAL(itemSelectionChanged()),this,SLOT(selection_changed()));
    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
    connect(le_search, SIGNAL(textChanged(QString)),this, SLOT(search_changed(QString)));
    connect(btn_clear_search, SIGNAL(clicked()),this,SLOT(clear_search()));
}

void ThoughtsDock::clear(){
    for(int r = tw_thoughts->rowCount(); r >=0; r--){
        tw_thoughts->removeRow(r);
    }
    tw_thoughts->clearContents();
}

void ThoughtsDock::refresh(){
    clear();

    if(DT && DT->get_DFInstance()){
        QHash<short, QPair<int,int> > thoughts = DT->get_DFInstance()->get_thought_stats();

        tw_thoughts->setSortingEnabled(false);
        QString tooltip;
        foreach(short id, thoughts.uniqueKeys()){
                tw_thoughts->insertRow(0);
                tw_thoughts->setRowHeight(0, 18);

                Thought *t = GameDataReader::ptr()->get_thought(id);
                int total_count = thoughts.value(id).first;
                int dwarf_count = thoughts.value(id).second;
                tooltip = QString("<center><h4>%1</h4></center>%2 (%3)")
                        .arg(capitalize(t->title()))
                        .arg(capitalize(t->desc()))
                        .arg(QString::number(t->effect()));

                if(t->effect()==0)
                    tooltip += tr("<p><b>Can be beneficial or detrimental depending on the individual</b></p>");

                QTableWidgetItem *item_title = new QTableWidgetItem();
                item_title->setData(Qt::UserRole, id);
                item_title->setText(capitalize(t->title()));
                item_title->setToolTip(tooltip);

                QTableWidgetItem *item_count = new QTableWidgetItem();
                item_count->setData(Qt::DisplayRole, dwarf_count);
                item_count->setTextAlignment(Qt::AlignCenter);
                item_count->setToolTip(tr("This thought has occurred a total of %1 times among %2 civilians.<br/><br/>Click to show these individuals.")
                                       .arg(total_count).arg(dwarf_count));
                item_count->setBackgroundColor(t->color());

                QTableWidgetItem *item_desc = new QTableWidgetItem();
                item_desc->setText(capitalize(t->desc()));
                item_desc->setToolTip(tooltip);

                tw_thoughts->setItem(0, 0, item_title);
                tw_thoughts->setItem(0, 1, item_count);
                tw_thoughts->setItem(0, 2, item_desc);
            }
        tw_thoughts->setSortingEnabled(true);
        tw_thoughts->sortItems(1, Qt::DescendingOrder);
        filter();
    }
}

void ThoughtsDock::selection_changed(){
    QModelIndexList indexList = tw_thoughts->selectionModel()->selectedIndexes();
    QList<short> selected;
    if(indexList.count() > 0){
        int row = 0;
        int prev_row=-1;
        foreach (QModelIndex index, indexList) {
            row = index.row();
            if(row != prev_row)
                selected.append(tw_thoughts->item(row,0)->data(Qt::UserRole).toInt());
            prev_row = row;
        }
    }
    emit item_selected(selected);
}

void ThoughtsDock::search_changed(QString val){
    val = "(" + val.replace(" ", "|") + ")";
    m_filter = QRegExp(val,Qt::CaseInsensitive, QRegExp::RegExp);
    filter();
}

void ThoughtsDock::filter(){
    for(int i = 0; i < tw_thoughts->rowCount(); i++){
        if(m_filter.isEmpty() || tw_thoughts->item(i,0)->text().contains(m_filter) || tw_thoughts->item(i,2)->text().contains(m_filter)){
            tw_thoughts->setRowHidden(i,false);
        }else{
            tw_thoughts->setRowHidden(i,true);
        }
    }
}

void ThoughtsDock::clear_filter(){
    tw_thoughts->clearSelection();
}

void ThoughtsDock::clear_search(){
    QLineEdit *s = qobject_cast<QLineEdit*>(QObject::findChild<QLineEdit*>("le_search"));
    if(s)
        s->setText("");
}

void ThoughtsDock::closeEvent(QCloseEvent *event){
    clear_search();
    clear_filter();
    event->accept();
}
