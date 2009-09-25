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

#ifndef GRID_VIEW_DIALOG_H
#define GRID_VIEW_DIALOG_H

#include <QtGui>
#include "defines.h"

class ViewManager;
class GridView;

class ViewManagerTreeView: public QTreeView{
	Q_OBJECT
public:
	ViewManagerTreeView(QWidget *parent = 0)
		: QTreeView(parent)
	{
		setDragDropMode(QAbstractItemView::InternalMove);
		setDropIndicatorShown(true);
		setDragEnabled(true);
		setAcceptDrops(true);
	}
	virtual ~ViewManagerTreeView() {}
	/*
	void mousePressEvent(QMouseEvent *event) {
		if (event->button() == Qt::LeftButton)
			m_drag_start = event->pos();
		return QTreeView::mousePressEvent(event);
	}

	void mouseMoveEvent(QMouseEvent *event) {
		if (event->buttons() & Qt::LeftButton) { // looks like dragging
			if ((event->pos() - m_drag_start).manhattanLength() < QApplication::startDragDistance()) // too short
				return;

			QModelIndex idx = indexAt(m_drag_start);
			if (!idx.isValid())
				return; // not an object

			QDrag *drag = new QDrag(this);
			QMimeData *mime = new QMimeData;
			LOGD << "starting a drag on index:" << idx;
			mime->setData("item/row", QVariant(idx.row()).toByteArray());
			mime->setData("item/col", QVariant(idx.column()).toByteArray());
			mime->setData("item/has_parent", QVariant(idx.parent().isValid()).toByteArray());
			drag->setMimeData(mime);
			Qt::DropAction action = drag->exec();
			LOGD << "action was" << action;
		}
	}
	
	void dragEnterEvent(QDragEnterEvent *event) {
		if (event->source() == this) // accept drags that we started
			event->acceptProposedAction();
	}

	void dragMoveEvent(QDragMoveEvent *event) {
		event->acceptProposedAction();
	}

	void dropEvent(QDropEvent *event) {
		LOGD << "DROPPED with MIME" << event->mimeData()->formats();
		if (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist")) {
			event->acceptProposedAction();
		} else {
			event->ignore();
		}
	}*/
private:
	QPoint m_drag_start;

};

class ViewManagerItemModel : public QStandardItemModel {
	Q_OBJECT
public:
	ViewManagerItemModel(QObject *parent)
		: QStandardItemModel(parent)
	{
	}

	Qt::DropActions supportedDragActions() const {
		return Qt::MoveAction;
	}

	Qt::DropActions supportedDropActions() const {
		return Qt::MoveAction;
	}

	Qt::ItemFlags flags(const QModelIndex &index) const {
		Qt::ItemFlags default_flags = QStandardItemModel::flags(index);
		default_flags ^= Qt::ItemIsDragEnabled;
		default_flags ^= Qt::ItemIsDropEnabled;

		if (index.isValid())
			return Qt::ItemIsDragEnabled | default_flags;
		else
			return Qt::ItemIsDropEnabled | default_flags;
	}
};

/************************************************************************************/


namespace Ui {
	class GridViewDialog;
}

class GridViewDialog : public QDialog {
	Q_OBJECT
public:
	GridViewDialog(ViewManager *mgr, GridView *view, QWidget *parent = 0);

	//! used to hack into the list of sets, since they don't seem to send a proper re-order signal
	//bool eventFilter(QObject *, QEvent *);
	QString name();
	QStringList sets();
	
	public slots:
		void accept();


private:
	Ui::GridViewDialog *ui;
	GridView *m_view;
	ViewManager *m_manager;
	bool m_is_editing;
	QString m_original_name;
	ViewManagerItemModel *m_model;

	private slots:

		//! for redrawing sets in the edit dialog
		void draw_sets();

		//! called whenever the user drags the sets into a different order
		void order_changed();

		//! makes sure the name for this view is ok
		void check_name(const QString &);

		//! add the currently selected set in the combobox to this view's set list
		void add_set();

		//! overridden context menu for the set list
		void draw_set_context_menu(const QPoint &);

		//! called from the context menu
		void remove_set_from_action();
};

#endif
