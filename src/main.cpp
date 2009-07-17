#include <QtGui>
#include <QtDebug>
#include "mainwindow.h"
#include "dfinstance.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
	
	/*
	{
		QSettings s(QSettings::IniFormat, QSettings::UserScope, "UDP Software", "Dwarf Therapist");
		QColor c(50, 200, 100);
		s.setValue("test/color", c);
		qDebug() << c;
	}
	{
		QSettings s(QSettings::IniFormat, QSettings::UserScope, "UDP Software", "Dwarf Therapist");
		QColor c = s.value("test/color").value<QColor>();
		qDebug() << c;
	}
	*/
    return a.exec();
}
