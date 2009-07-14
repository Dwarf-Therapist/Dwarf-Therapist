#ifndef STATETABLEVIEW_H
#define STATETABLEVIEW_H

#include <QtGui>

class UberDelegate;

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

private:
	UberDelegate *m_delegate;
};

#endif // STATETABLEVIEW_H
