#ifndef STATETABLEVIEW_H
#define STATETABLEVIEW_H

#include <QtGui>

class HeaderDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	HeaderDelegate (QObject *parent = 0);
	QSize sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
	void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
};

class SkillDelegate : public QStyledItemDelegate {
	Q_OBJECT
public:
	SkillDelegate(QObject *parent = 0);
	void paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &idx) const;
};

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
