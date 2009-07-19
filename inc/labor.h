#ifndef LABOR_H
#define LABOR_H

#include <QtGui>

class Labor : public QObject {
	Q_OBJECT
public:
	Labor(QString name, int id, int skill, int list_order, QColor color, QObject *parent = 0) 
		: QObject(parent)
		, name(name)
		, labor_id(id)
		, skill_id(skill)
		, list_order(list_order)
		, color(color)
	{}

	int operator<(const Labor &other) {
		return other.list_order < list_order;
	}

	QString name;
	int labor_id;
	int skill_id;
	int list_order;
	QColor color;
};

#endif;