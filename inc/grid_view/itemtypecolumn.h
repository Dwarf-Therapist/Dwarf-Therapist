#ifndef ITEMTYPECOLUMN_H
#define ITEMTYPECOLUMN_H

#include "viewcolumn.h"
#include "global_enums.h"

class Item;

class ItemTypeColumn : public ViewColumn {
public:
    ItemTypeColumn(const QString &title, const ITEM_TYPE &itype, ViewColumnSet *set = 0, QObject *parent = 0, COLUMN_TYPE cType = CT_ITEMTYPE);
    ItemTypeColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    ItemTypeColumn(const ItemTypeColumn &to_copy); // copy ctor
    ItemTypeColumn* clone() {return new ItemTypeColumn(*this);}
    QStandardItem *build_cell(Dwarf *d);
    QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves);
    ITEM_TYPE item_type() {return m_iType;}
    void set_item_type(ITEM_TYPE iType) {m_iType = iType;}

    //override
    void write_to_ini(QSettings &s);

protected:
    ITEM_TYPE m_iType;
    float m_sort_val;
    QString build_tooltip_desc(Dwarf *d);
    QString split_list(QList<Item*> list, QString title, QString list_header, QString list_footer, QColor title_color);
};


#endif // ITEMTYPECOLUMN_H
