/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef VIEW_COLUMN_H
#define VIEW_COLUMN_H

#include "columntypes.h"
#include "dwarfmodel.h"

class DTStandardItem;
class Dwarf;
class QSettings;
class QStandardItem;
class ViewColumnColors;
class ViewColumnSet;

/*!
ViewColumn
I can think of a need for:
* Labor cols
* status cols (happiness, wounds, etc...)
* radio cols (choose which weapon skill, or armor type)
* simple counters (num kills, num kids, total gold, total items)
*/
class ViewColumn : public QObject {
    Q_OBJECT
public:
    ViewColumn(QString title, COLUMN_TYPE type, ViewColumnSet *set = 0, QObject *parent = 0, int col_idx = -1);
    ViewColumn(QSettings &s, ViewColumnSet *set = 0, QObject *parent = 0);
    ViewColumn(const ViewColumn &to_copy); // copy ctor
    virtual ViewColumn* clone() = 0;
    virtual ~ViewColumn();

    typedef enum {
        CST_DEFAULT,
        CST_SKILL_RATE,
        CST_ROLE_RATING,
        CST_LEVEL,
        CST_MAXIMUM_VALUE,
        CST_FOCUS,
    } COLUMN_SORT_TYPE;

    typedef enum {
        STATE_TOGGLE=-1,
        STATE_ACTIVE,
        STATE_PENDING,
        STATE_DISABLED
    } CELL_STATE;

    static inline QString get_sort_type(const COLUMN_SORT_TYPE &type) {
        switch (type) {
        case CST_SKILL_RATE: return "SKILL_RATE";
        case CST_ROLE_RATING: return "ROLE_RATING";
        case CST_LEVEL: return "LEVEL";
        case CST_MAXIMUM_VALUE: return "MAXIMUM_VALUE";
        case CST_FOCUS: return "FOCUS";
        default:
            return "DEFAULT";
        }
    }

    static inline COLUMN_SORT_TYPE get_sort_type(const QString &name) {
        if (name.toUpper() == "SKILL_RATE") {
            return CST_SKILL_RATE;
        } else if (name.toUpper() == "ROLE_RATING") {
            return CST_ROLE_RATING;
        } else if (name.toUpper() == "LEVEL") {
            return CST_LEVEL;
        } else if (name.toUpper() == "MAXIMUM_VALUE") {
            return CST_MAXIMUM_VALUE;
        } else if (name.toUpper() == "FOCUS") {
            return CST_FOCUS;
        }
        return CST_DEFAULT;
    }


    QString title() {return m_title;}
    void set_title(QString title) {m_title = title;}

    bool override_color() {return m_override_bg_color;}
    void set_override_color(bool yesno) {m_override_bg_color = yesno;}

    QColor bg_color() {return m_bg_color;}
    void set_bg_color(QColor c) {m_bg_color = c;}

    ViewColumnColors *get_colors() {return m_cell_colors;}

    ViewColumnSet *set() {return m_set;}
    void set_viewcolumnset(ViewColumnSet *set) {m_set = set;}
    virtual COLUMN_TYPE type() {return m_type;}
    int count() {return m_count;}
    QHash<Dwarf*,DTStandardItem*> cells() {return m_cells;}
    QStandardItem *init_cell(Dwarf *d);

    //TODO: decouple tooltip creation from the item creation. that way tooltips could be instantly updated
    virtual QStandardItem *build_cell(Dwarf *d) = 0; // create a suitable item based on a dwarf

    QStandardItem *init_aggregate(QString group_name);
    virtual QStandardItem *build_aggregate(const QString &group_name, const QVector<Dwarf*> &dwarves) = 0; // create an aggregate cell based on several dwarves

    QString get_cell_value(Dwarf *d);
    virtual void write_to_ini(QSettings &s);

    QList<COLUMN_SORT_TYPE> get_sortable_types(){return m_sortable_types;}
    COLUMN_SORT_TYPE get_current_sort() {return m_current_sort;}

    void set_export_role(DwarfModel::DATA_ROLES new_role){m_export_data_role = new_role;}

    virtual QColor get_state_color(int state) const;

public slots:
    virtual void read_settings();
    virtual void refresh_color_map();
    void clear_cells();
    virtual void redraw_cells() {}
    virtual void refresh_sort(COLUMN_SORT_TYPE) {}

protected:
    QString m_title;
    QColor m_bg_color;
    QColor m_active_color;
    QColor m_disabled_color;
    bool m_override_bg_color;
    ViewColumnSet *m_set;
    COLUMN_TYPE m_type;
    QHash<Dwarf*, DTStandardItem*> m_cells;
    int m_count;
    DwarfModel::DATA_ROLES m_export_data_role;
    QList<COLUMN_SORT_TYPE> m_sortable_types;
    COLUMN_SORT_TYPE m_current_sort;
    ViewColumnColors *m_cell_colors;
    QList<int> m_available_states;
    QHash<int,QColor> m_cell_color_map;

    virtual void init_states();
    QString tooltip_name_footer(Dwarf *d);
};

#endif
