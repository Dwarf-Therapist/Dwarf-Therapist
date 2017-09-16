#ifndef HIGHESTMOODCOLUMN_H
#define HIGHESTMOODCOLUMN_H

#include "skillcolumn.h"

class Dwarf;

class HighestMoodColumn : public SkillColumn {
    Q_OBJECT
public:
    HighestMoodColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    HighestMoodColumn(const QString &title, ViewColumnSet *set = 0, QObject *parent = 0);
    HighestMoodColumn(const HighestMoodColumn &to_copy); // copy ctor
    HighestMoodColumn* clone() {return new HighestMoodColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);
    void write_to_ini(QSettings &s);
};

#endif // HIGHESTMOODCOLUMN_H
