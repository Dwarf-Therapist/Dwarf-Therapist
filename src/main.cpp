#include <QtGui>
#include <QtDebug>
#include "mainwindow.h"
#include "dfinstance.h"
#include "utils.h"

/*
 class SkillItem : public QGraphicsItem {
	QString m_text;
 public:
	SkillItem(QString text, QGraphicsItem *parent = 0) 
		: QGraphicsItem(parent)
		, m_text(text)
	{
		setAcceptHoverEvents(true);
		setFlags(QGraphicsItem::ItemIsSelectable);
		//setFlags(QGraphicsItem::ItemIsFocusable);
	}

     QRectF boundingRect() const {
         return QRectF(0, 0, 20, 20);
     }

     void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget) {
		 //qDebug() << opt->state;
		 p->save();
		 //p->setRenderHint(QPainter::Antialiasing);
		 if (opt->state & QStyle::State_MouseOver) {
			p->setBrush(QBrush(QColor::fromRgbF(0.2, 0.9, 0.2, 0.3)));
		 } else {
			p->setBrush(QBrush(QColor::fromRgbF(0.8, 0.8, 0.8, 0.3)));
		 }
         p->drawRect(0, 0, 20, 20);
		 p->drawText(4, 16, m_text);
		 p->restore();
     }
 };

class DwarfItem : public QGraphicsItem {
	QString m_name;
	int w;
public:
	DwarfItem(QString name, QGraphicsItem *parent = 0)
		: QGraphicsItem(parent)
		, m_name(name)
	{
		setAcceptHoverEvents(true);
		setFlags(QGraphicsItem::ItemIsSelectable);
		for (int i = 0; i < 72; i++) {
			SkillItem *si = new SkillItem(QString::number(i), this);
			si->translate(200 + (i*20), 0);
		};
		w = 200 + (72 * 20);
	}

	QRectF boundingRect() const {
		return QRectF(0, 0, w, 20);
	}

	void paint(QPainter *p, const QStyleOptionGraphicsItem *opt, QWidget *widget) {
		p->save();
		QPen pen;
		pen = QPen(QColor(0xd9d9d9));
		if (opt->state & QStyle::State_MouseOver) {
			//pen = QPen(QColor(0x666666));
			p->setBrush(QBrush(QColor(0xe0e0FF)));
		} else {
			
			p->setBrush(QBrush(QColor(0xF0F0F0)));
		}
		pen.setWidth(1);
		p->setPen(pen);
		p->drawRect(0, 0, w, 20);
		p->setPen(Qt::black);
		p->setFont(QFont("Arial", 10));
		p->drawText(4, 14, m_name);
		p->restore();
	}
};
*/
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
	
	/*
	QGraphicsScene s;
	s.setSceneRect(0, 0, 1000, 600);
	QGraphicsSimpleTextItem *t = s.addSimpleText("Masonry", QFont("Verdana", 10));
	t->rotate(65);
	QGraphicsSimpleTextItem *t2 = s.addSimpleText("Bowyer", QFont("Verdana", 10));
	t2->translate(18, 0);
	t2->rotate(65);

	for (int i = 0; i < 10; ++i) {
		DwarfItem *di = new DwarfItem("DUDE" + QString::number(i));
		di->translate(0, 10 + (i * (di->boundingRect().height() + 1)));
		s.addItem(di);
	}
	//SkillItem *si = new SkillItem("14");
	//si->translate(0, 150);
	//s.addItem(si);
	

	QGraphicsView *g = new QGraphicsView(&s, 0);
	//g->setRenderHint(QPainter::Antialiasing);
	g->centerOn(500, 500);
	g->show();
	*/
    return a.exec();

}
