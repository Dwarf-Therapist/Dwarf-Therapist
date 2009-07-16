#include <QtGui>
#include "qmath.h"

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "uberdelegate.h"
#include "rotatedheader.h"

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