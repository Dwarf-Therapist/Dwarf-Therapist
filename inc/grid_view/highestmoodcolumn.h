#ifndef HIGHESTMOODCOLUMN_H
#define HIGHESTMOODCOLUMN_H

#include "viewcolumn.h"
class Dwarf;

class HighestMoodColumn : public ViewColumn {
    Q_OBJECT
public:
    HighestMoodColumn(const QString &title, ViewColumnSet *set = 0, QObject *parent = 0);
    HighestMoodColumn(const HighestMoodColumn &to_copy); // copy ctor
    HighestMoodColumn* clone() {return new HighestMoodColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);

public slots:
    //void read_settings();

};

#endif // HIGHESTMOODCOLUMN_H
