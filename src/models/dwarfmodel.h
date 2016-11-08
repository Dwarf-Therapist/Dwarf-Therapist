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
#ifndef DWARF_MODEL_H
#define DWARF_MODEL_H

#include <QStandardItemModel>
#include "columntypes.h"
#include "dfinstance.h"

class Dwarf;
class DwarfModel;
class DwarfModelProxy;
class GridView;
class Squad;
class ViewColumn;

/*
class CreatureGroup : public QStandardItem {
    Q_OBJECT
public:
    CreatureGroup();
    CreatureGroup(const QString &text);
    CreatureGroup(const QIcon &icon, const QString &text);
    virtual ~CreatureGroup();

    QList<QStandardItem*> build_row();
    void add_member(Dwarf *d);
    int type() const {return QStandardItem::UserType + 1;}
private:
    QList<int> m_member_ids;
};
*/

class DwarfModel : public QStandardItemModel {
    Q_OBJECT
public:
    typedef enum {
        GB_NOTHING = 0,
        GB_AGE,
        GB_CASTE,
        GB_CASTE_TAG,
        GB_CURRENT_JOB,
        GB_HAPPINESS,
        GB_HAS_NICKNAME,
        GB_HIGHEST_MOODABLE,
        GB_HIGHEST_SKILL,
        GB_LEGENDARY,
        GB_MIGRATION_WAVE,
        GB_MILITARY_STATUS,
        GB_PROFESSION,
        GB_RACE,
        GB_SEX,
        GB_SQUAD,
        GB_ASSIGNED_LABORS,
        GB_TOTAL_SKILL_LEVELS,
        GB_HEALTH,
        GB_GOALS,
        GB_SKILL_RUST,
        GB_ASSIGNED_SKILLED_LABORS,
        GB_JOB_TYPE,
        GB_OCCUPATION,
        GB_TOTAL
    } GROUP_BY;
    typedef enum {
        DR_RATING = Qt::UserRole + 1,
        DR_SORT_VALUE,
        DR_IS_AGGREGATE,
        DR_LABOR_ID,
        DR_OTHER_ID,
        DR_GROUP_NAME,
        DR_ID,
        DR_DEFAULT_BG_COLOR,
        DR_DEFAULT_FG_COLOR,
        DR_COL_TYPE,
        DR_SET_NAME,
        DR_BASE_SORT,
        DR_SPECIAL_FLAG, //used by some columns to indicate different things, usually boolean
        DR_DISPLAY_RATING, //this is the rating to use when determining drawing shapes, alternative to DR_RATING
        DR_AGE, //right click sort on first column
        DR_NAME, //right click sort on first column
        DR_SIZE, //right click sort on first column
        DR_GLOBAL, //global sort column
        DR_CUSTOM_PROF, //custom profession name
        DR_LABORS, //qvariant list of labor id
        DR_TOOLTIP, //used to redirect the tooltip instead of the default Qt::TooltipRole
        DR_EXPORT, //custom data when exporting to CSV
        DR_STATE
    } DATA_ROLES;

    DwarfModel(QObject *parent = 0);
    virtual ~DwarfModel();
    void set_instance(DFInstance *df) {m_df = QPointer<DFInstance>(df);}
    void set_grid_view(GridView *v) {m_gridview = v;}
    GridView * current_grid_view() {return m_gridview;}
    void clear_all(bool clr_pend); // reset everything to normal

    QHash<int,QPair<QString,int> > get_global_sort_info() {return m_global_sort_info;}
    QHash<int,QPair<int,Qt::SortOrder> > get_global_group_sort_info(){return m_global_group_sort_info;} //stores the last role and order for a group by key

    void set_global_group_sort_info(int role, Qt::SortOrder order);
    void set_global_sort_col(QString grid_view_name, int col_idx);
    void update_global_sort_col(int group_id);

    GROUP_BY current_grouping() const {return m_group_by;}
    const QMap<QString, QVector<Dwarf*> > *get_dwarf_groups() const {return &m_grouped_dwarves;}
    Dwarf *get_dwarf_by_id(int id) const {return m_dwarves.value(id, 0);}

    QVector<Dwarf*> get_dirty_dwarves();
    QList<Dwarf*> get_dwarves() {return m_dwarves.values();}
    void calculate_pending();
    int selected_col() const {return m_selected_col;}
    void filter_changed(const QString &);

    QModelIndex findOne(const QVariant &needle, int role = Qt::DisplayRole, int column = 0, const QModelIndex &start_index = QModelIndex());
    QList<QPersistentModelIndex> findAll(const QVariant &needle, int role = Qt::DisplayRole, int column = 0, QModelIndex start_index = QModelIndex());

    QList<Squad *> active_squads();
    Squad* get_squad(int id);

    int total_row_count(){return m_total_row_count;}
    bool clearing_data(){return m_clearing_data;}

public slots:
    void draw_headers();
    void update_header_info(int id, COLUMN_TYPE type);

    void build_row(const QString &key);
    void build_rows();
    void set_group_by(int group_by);
    void load_dwarves();
    void cell_activated(const QModelIndex &idx, DwarfModelProxy *proxy = 0); // a grid cell was clicked/doubleclicked or enter was pressed on it
    void clear_pending();
    void commit_pending();
    void section_right_clicked(int idx);

    void labor_group_toggled(const QString &group_name, const int idx_left, const int idx_right, DwarfModelProxy *proxy);
    void labor_group_toggled(Dwarf *d, const int idx_left, const int idx_right, DwarfModelProxy *proxy);

    void read_settings();

private:
    QPointer<DFInstance> m_df;
    QHash<int, Dwarf*> m_dwarves;
    QMap<QString, QVector<Dwarf*> > m_grouped_dwarves;
    GROUP_BY m_group_by;
    int m_selected_col;
    GridView *m_gridview;
    int m_total_row_count;
    bool m_clearing_data;

    //options
    QFont m_font;
    QChar m_symbol;
    QColor m_curse_col;
    QBrush m_cursed_bg;
    QBrush m_cursed_bg_light;
    bool m_animal_health;
    bool m_show_gender;
    bool m_decorate_nobles;
    bool m_highlight_nobles;
    bool m_highlight_cursed;
    bool m_show_labor_counts;
    int m_cell_width;
    int m_cell_padding;
    bool m_show_tooltips;

    QHash<int,QPair<QString,int> > m_global_sort_info; //keeps a pair of gridview name, column idx for each group by type used
    QHash<int,QPair<int,Qt::SortOrder> > m_global_group_sort_info;

    QBrush build_gradient_brush(QColor base_col, int alpha_start, int alpha_finish, QPoint start, QPoint end);
    QString build_col_tooltip(ViewColumn *col);
    QFont get_font(bool bold = false, bool italic = false, bool underline = false);

signals:
    void new_pending_changes(int);
    void new_creatures_count(int,int,int,QString);
    void preferred_header_size(int section, int width);
    void preferred_header_height(QString);
    void set_index_as_spacer(int);
    void clear_spacers();
    void need_redraw();
    void units_refreshed();
};
#endif
