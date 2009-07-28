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
#include <QtGui>
#include "qmath.h"

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "uberdelegate.h"
#include "rotatedheader.h"
#include "dwarf.h"
#include "defines.h"

StateTableView::StateTableView(QWidget *parent)
	: QTreeView(parent)
	, m_model(0)
	, m_proxy(0)
	, m_delegate(new UberDelegate(this))
	, m_header(new RotatedHeader(Qt::Horizontal, this))
	, m_grid_focus(false)
	, m_single_click_labor_changes(false)
{
	setItemDelegate(m_delegate);
	setHeader(m_header);
}

StateTableView::~StateTableView()
{
}

void StateTableView::set_model(DwarfModel *model, DwarfModelProxy *proxy) {
	QTreeView::setModel(proxy);
	m_model = model;
	m_proxy = proxy;

	m_delegate->set_model(model);
	m_delegate->set_proxy(proxy);
	header()->setResizeMode(QHeaderView::Fixed);
	header()->setResizeMode(0, QHeaderView::ResizeToContents);

	connect(m_header, SIGNAL(section_right_clicked(int)), m_model, SLOT(section_right_clicked(int)));
	
	//header()->setDefaultSectionSize(24); //set grid size
	
	connect(this, SIGNAL(activated(const QModelIndex&)), proxy, SLOT(labor_clicked(const QModelIndex&)));
}

void StateTableView::new_custom_profession() {
	QModelIndex idx = currentIndex();
	if (idx.isValid()) {
		const DwarfModel *m = dynamic_cast<const DwarfModel*>(model());
		int id = idx.data(DwarfModel::DR_ID).toInt();
		Dwarf *d = m->get_dwarf_by_id(id);
		if (d)
			emit new_custom_profession(d);
	}
}

void StateTableView::set_grid_size(int new_size) {
	return;
	if (model()->rowCount() < 1) {
		return;
	}
	// TODO: apply this to the delegate's size hint?
}

void StateTableView::filter_dwarves(QString text) {
	m_proxy->setFilterFixedString(text);
	m_proxy->setFilterKeyColumn(0);
	m_proxy->setFilterRole(Qt::DisplayRole);
}

void StateTableView::jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
	if (!current)
		return;
	int dwarf_id = current->data(0, Qt::UserRole).toInt();
	
	Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
	if (d && d->m_name_idx.isValid()) {
		QModelIndex proxy_idx = m_proxy->mapFromSource(d->m_name_idx);
		if (proxy_idx.isValid()) {
			scrollTo(proxy_idx);
			selectionModel()->select(proxy_idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
		}
	}
}

void StateTableView::jump_to_profession(QListWidgetItem* current, QListWidgetItem* previous) {
	if (!current)
		return;
	QString prof_name = current->text();
	QModelIndexList matches = m_proxy->match(m_proxy->index(0,0), Qt::DisplayRole, prof_name);
	if (matches.size() > 0) {
		QModelIndex group_header = matches.at(0);
		scrollTo(group_header);
		expand(group_header);
		selectionModel()->select(group_header, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
	}
}

void StateTableView::set_single_click_labor_changes(bool enabled) {
	m_single_click_labor_changes = enabled;
	LOGD << "setting single click labor changes:" << enabled;
	disconnect(this, SIGNAL(clicked(const QModelIndex&)), m_proxy, SLOT(labor_clicked(const QModelIndex&)));
	if (enabled) {
		connect(this, SIGNAL(clicked(const QModelIndex&)), m_proxy, SLOT(labor_clicked(const QModelIndex&)));
	}
}