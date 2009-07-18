#ifndef STATETABLEVIEW_H
#define STATETABLEVIEW_H

#include <QtGui>

class UberDelegate;
class RotatedHeader;
class Dwarf;

class StateTableView : public QTreeView
{
	Q_OBJECT

public:
	StateTableView(QWidget *parent = 0);
	~StateTableView();

	void setModel(QAbstractItemModel *model);
	UberDelegate *get_delegate() {return m_delegate;}

	public slots:
		void filter_dwarves(QString text);
		void set_grid_size(int new_size);
		void jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem* previous);

protected:
	QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
	void contextMenuEvent(QContextMenuEvent *event);

private:
	UberDelegate *m_delegate;
	RotatedHeader *m_header;

	private slots:
		void new_custom_profession();

signals:
	void new_custom_profession(Dwarf *d);

};
#endif // STATETABLEVIEW_H
