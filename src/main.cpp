#include <QtGui/QApplication>
#include <QtDebug>
#include "mainwindow.h"
#include "dfinstance.h"
#include "utils.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();

    /*
    QByteArray skillpattern_miner = encode_skillpattern(0, 3340, 4);
    QByteArray skillpattern_metalsmith = encode_skillpattern(29, 0, 2);
    QByteArray skillpattern_swordsman = encode_skillpattern(40, 4620, 3);
    QByteArray skillpattern_pump_operator = encode_skillpattern(65, 462, 1);


    qDebug() << "MINER" << by_char(skillpattern_miner) << skillpattern_miner.size();
    qDebug() << "METAL" << by_char(skillpattern_metalsmith) << skillpattern_metalsmith.size();
    qDebug() << "SWORD" << by_char(skillpattern_swordsman) << skillpattern_swordsman.size();
    qDebug() << "PUMPO" << by_char(skillpattern_pump_operator) << skillpattern_pump_operator.size();
    */


    return 0;


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
