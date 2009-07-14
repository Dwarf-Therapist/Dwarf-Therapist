#ifndef STATETABLEVIEW_H
#define STATETABLEVIEW_H

#include <QtGui>

class StateTableView : public QTreeView
{
	Q_OBJECT

public:
	StateTableView(QWidget *parent = 0);
	~StateTableView();

	void setModel(QAbstractItemModel *model);

	public slots:
		void filter_dwarves(QString text);
		void set_grid_size(int new_size);

private:
	//Ui::StateTableViewClass ui;
};

#endif // STATETABLEVIEW_H
