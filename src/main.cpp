#include <QtGui/QApplication>
#include <QtDebug>
#include "mainwindow.h"
#include "dfinstance.h"

int main(int argc, char *argv[]) {
	QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

	/*
	int x = 242343435;
	int y = 242343435;

	char xs[4];
	xs[0] = (char)x;
	xs[1] = (char)(x >> 8);
	xs[2] = (char)(x >> 16);
	xs[3] = (char)(x >> 24);

	QByteArray foo(xs, 4);
	QByteArray foo2((char*)&y, 4);
	int idx = foo.indexOf(foo2);

	qDebug() << x << y << hex << &xs << &y;
	qDebug() << "MEMCMP" << memcmp((void*)&xs, (void*)&y, sizeof(int));
	//delete[] xs;
	return 0;
	*/
}
