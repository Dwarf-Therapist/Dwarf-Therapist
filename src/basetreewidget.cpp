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
#include "basetreewidget.h"
#include "dwarftherapist.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QCloseEvent>
#include <QLabel>

BaseTreeWidget::BaseTreeWidget(bool requires_refresh, QWidget *parent)
    : QWidget(parent)
    , m_collapsed(true)
{
    m_arr_in = QIcon(":img/arrow-in.png");
    m_arr_out = QIcon(":img/arrow-out.png");

    QVBoxLayout *l = new QVBoxLayout();
    setLayout(l);

    m_tree_view = new QTreeWidget(this);
    m_tree_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tree_view->setDropIndicatorShown(false);
    m_tree_view->setProperty("showSortIndicator",QVariant(true));
    m_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QHBoxLayout *s = new QHBoxLayout();
    QLabel *lbl_search = new QLabel(tr("Search"),this);
    s->addWidget(lbl_search);
    m_le_search = new QLineEdit(this);
    m_le_search->setObjectName("le_search");
    s->addWidget(m_le_search);

    m_btn_toggle_tree = new QPushButton(this);
    m_btn_toggle_tree->setIcon(m_arr_out);
    s->addWidget(m_btn_toggle_tree);

    QPushButton *btn_clear_search = new QPushButton(this);
    QIcon icn_cross(":img/cross.png");
    btn_clear_search->setIcon(icn_cross);
    s->addWidget(btn_clear_search);

    l->addLayout(s);

    QPushButton *btn = new QPushButton(tr("Clear Filter"),this);
    l->addWidget(m_tree_view);
    l->addWidget(btn);

    connect(btn, SIGNAL(clicked()),this,SLOT(clear_filter()));
    connect(m_le_search, SIGNAL(textChanged(QString)), this, SLOT(search_changed(QString)));
    connect(btn_clear_search, SIGNAL(clicked()),this,SLOT(clear_search()));
    connect(m_btn_toggle_tree, SIGNAL(clicked()), this, SLOT(toggle_tree()));
    connect(m_tree_view, SIGNAL(itemSelectionChanged()), this, SLOT(selection_changed()));

    m_requires_refresh = requires_refresh;

    if(DT){
        connect(DT,SIGNAL(units_refreshed()),this,SLOT(refresh()));
    }
}

BaseTreeWidget::~BaseTreeWidget(){
    delete m_le_search;
    delete m_btn_toggle_tree;
    delete m_tree_view;
}

void BaseTreeWidget::refresh(){
    m_tree_view->clear();

    if(DT && DT->get_DFInstance()){
        m_tree_view->setSortingEnabled(false);
        build_tree();
        m_tree_view->setSortingEnabled(true);
        //sortbycol
        m_tree_view->expandAll(); //expand before resize to contents; hidden items aren't resized
        for(int i=0;i < m_tree_view->columnCount();i++){
            m_tree_view->resizeColumnToContents(i);
        }
        m_tree_view->collapseAll();
        m_collapsed = true;
        if(!m_requires_refresh){
            disconnect(DT,SIGNAL(units_refreshed()),this,SLOT(refresh()));
        }
    }
}

void BaseTreeWidget::toggle_tree(){
    if(m_collapsed){
        m_tree_view->expandAll();
        m_collapsed = false;
        m_btn_toggle_tree->setIcon(m_arr_in);
    }else{
        m_tree_view->collapseAll();
        m_collapsed = true;
        m_btn_toggle_tree->setIcon(m_arr_out);
    }
}

void BaseTreeWidget::clear(){
    m_tree_view->clear();
}

void BaseTreeWidget::search_changed(QString val){
    search_tree(val);
}

void BaseTreeWidget::clear_search(){
    m_le_search->setText("");
    search_tree("");
}

void BaseTreeWidget::clear_filter(){
    m_tree_view->clearSelection();
}

void BaseTreeWidget::closeEvent(QCloseEvent *event){
    clear_search();
    clear_filter();
    event->accept();
}
