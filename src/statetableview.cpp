#include <QtGui>
#include "qmath.h"

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "uberdelegate.h"
#include "rotatedheader.h"
#include "dwarf.h"

StateTableView::StateTableView(QWidget *parent)
	: QTreeView(parent)
	, m_delegate(new UberDelegate(this))
	, m_header(new RotatedHeader(Qt::Horizontal, this))
{
	setItemDelegate(m_delegate);
	this->setHeader(m_header);
	m_header->setClickable(true);
	m_header->setSortIndicatorShown(true);
}

StateTableView::~StateTableView()
{
}

void StateTableView::setModel(QAbstractItemModel *model) {
	QTreeView::setModel(model);
	setColumnWidth(0, 200);
	m_header->set_model(qobject_cast<DwarfModel*>(model));
	disconnect(this, SIGNAL(activated(const QModelIndex&)));
	connect(this, SIGNAL(activated(const QModelIndex&)), 
			model, SLOT(labor_clicked(const QModelIndex&)));
	connect(this, SIGNAL(clicked(const QModelIndex&)), 
			model, SLOT(labor_clicked(const QModelIndex&)));
}

QModelIndex StateTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers) {
	const DwarfModel *m = dynamic_cast<const DwarfModel*>(model());
	QModelIndex cur = currentIndex();
	QModelIndex cur_parent = currentIndex().parent();
	QModelIndex new_parent = cur_parent;
	

	int row = cur.row();
	int col = cur.column();
	int mod = 1;
	if (Qt::ShiftModifier & modifiers)
		mod = 10;
	
	switch(cursorAction) {
		case MoveUp: // cursor moving up
			row -= mod;
			if (m->current_grouping() != DwarfModel::GB_NOTHING) {
				if (cur_parent.isValid()) { // this is a skill item
					if (row < 0) { // top of the subskills for a group
						row = cur_parent.row();
						new_parent = QModelIndex();
					}
				} else { // this is a group item
					if (row <= 0) // TODO: make this an option wrap-around on scrolling
						return cur;
					QModelIndex up_model = cur.sibling(row, 0); // see if there is a group above this one
					if (model()->hasChildren(up_model)) { // does it have kids?
						row = model()->rowCount(up_model) - 1; // set it to the last kid
						new_parent = up_model;
					}
				}
			}
			if (row < 0)
				row = 0;
			break;
		case MoveDown:
			row += mod;
			if (m->current_grouping() != DwarfModel::GB_NOTHING) {
				if (cur_parent.isValid()) { // this is a skill item
					if (row > model()->rowCount(cur_parent) - 1) { // bottom of the subskills for a group
						if (cur_parent.row() < model()->rowCount()) {
							row = cur_parent.row() + 1;
							new_parent = QModelIndex();
						} else {
							return cur;
						}
					}
				} else { // this is a group item
					if (row > model()->rowCount(cur_parent) - 1) // TODO: make this an option wrap-around on scrolling
						return cur;
					row = 0; // set it to the first kid
					new_parent = model()->index(cur.row(), 0);
				}
			}
			if (row > m->rowCount() - 1)
				row = m->rowCount() - 1;
			break;
		case MoveRight:
			col += mod;
			if (col > m->columnCount() - 1)
				col = m->columnCount() - 1;
			break;
		case MoveLeft:
			col -= mod;
			if (col < 1)
				col = 1;
			break;
	}

	return model()->index(row, col, new_parent);
}

void StateTableView::set_grid_size(int new_size) {
	return;
	if (model()->rowCount() < 1) {
		return;
	}
	// TODO: apply this to the delegate's size hint?
}

void StateTableView::filter_dwarves(QString text) {
	// TODO: apply filtering to model
}

void StateTableView::jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem* previous) {
	if (!current)
		return;
	int dwarf_id = current->data(0, Qt::UserRole).toInt();
	const DwarfModel *m = dynamic_cast<const DwarfModel*>(model());
	Dwarf *d = m->get_dwarf_by_id(dwarf_id);
	if (d && d->m_name_idx.isValid()) {
		this->scrollTo(d->m_name_idx);
		this->selectionModel()->select(QItemSelection(d->m_name_idx, d->m_name_idx), QItemSelectionModel::SelectCurrent);
	}
}