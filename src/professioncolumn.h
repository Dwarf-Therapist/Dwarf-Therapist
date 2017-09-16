#ifndef PROFESSION_COLUMN_H
#define PROFESSION_COLUMN_H

#include "viewcolumn.h"
class Dwarf;

class ProfessionColumn : public ViewColumn {
    Q_OBJECT
public:
    ProfessionColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    ProfessionColumn(const QString &title, ViewColumnSet *set = 0, QObject *parent = 0);
    ProfessionColumn(const ProfessionColumn &to_copy); // copy ctor
    ProfessionColumn* clone() {return new ProfessionColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

};

#endif // PROFESSION_COLUMN_H
