#include "gridview.h"

ViewColumn::ViewColumn(QString title, ViewColumnSet *set, QObject *parent)
	: QObject(parent)
	, m_title(title)
	, m_set(set)
	, m_override_set_colors(false)
{}

ViewColumnSet::ViewColumnSet(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

GridView::GridView(QString name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}