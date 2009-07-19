#ifndef LABOR_H
#define LABOR_H

#include <QtGui>

class Labor : public QObject {
	Q_OBJECT
public:
	Labor(QString name, int id, int skill, QColor color, QObject *parent = 0) 
		: QObject(parent)
		, name(name)
		, labor_id(id)
		, skill_id(skill)
		, color(color)
	{}

	QString name;
	int labor_id;
	int skill_id;
	QColor color;
};

#endif;