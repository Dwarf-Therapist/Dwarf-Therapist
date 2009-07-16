#ifndef ROTATED_HEADER_H
#define ROTATED_HEADER_H

#include <QtGui>

class DwarfModel;

class RotatedHeader : public QHeaderView {
	Q_OBJECT
public:
	RotatedHeader(Qt::Orientation orientation, QWidget *parent = 0);
	void set_model(DwarfModel *dm) {m_model = dm;}
	void paintSection(QPainter *p, const QRect &rect, int idx) const;
	QSize sizeHint() const;
protected:
	void leaveEvent(QEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
private:
	DwarfModel *m_model;
	QPoint m_p;
};

#endif