#include <QtGui>
#include "qmath.h"

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "uberdelegate.h"

StateTableView::StateTableView(QWidget *parent)
	: QTreeView(parent)
	, m_delegate(new UberDelegate(this))
{
	setItemDelegate(m_delegate);
}

StateTableView::~StateTableView()
{
}

void StateTableView::setModel(QAbstractItemModel *model) {
	QTreeView::setModel(model);
	//resizeColumnToContents(0);
	this->setColumnWidth(0, 200);
	//setRowHeight(0, 100);
}

void StateTableView::set_grid_size(int new_size) {
	if (model()->rowCount() < 1) {
		return;
	}
	for (int i=1; i < model()->rowCount(); ++i) {
		//setRowHeight(i, new_size);
	}
	for (int i=1; i < model()->columnCount(); ++i) {
		setColumnWidth(i, new_size);
	}
}

void StateTableView::filter_dwarves(QString text) {
	//model()-

}