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
#ifndef STATE_TABLE_VIEW_H
#define STATE_TABLE_VIEW_H

#include "dwarfmodelproxy.h"

#include <QItemSelection>
#include <QTreeView>

class Dwarf;
class DwarfModel;
class QMenu;
class QTreeWidgetItem;
class RotatedHeader;
class UberDelegate;

class StateTableView : public QTreeView {
    Q_OBJECT
public:
    StateTableView(QWidget *parent = 0);
    ~StateTableView();

    void set_model(DwarfModel *model, DwarfModelProxy *proxy);
    UberDelegate *get_delegate() {return m_delegate;}
    RotatedHeader *get_header() {return m_header;}

    QItemSelection m_selected;
    QModelIndexList m_selected_rows;
    int m_last_sorted_col;
    Qt::SortOrder m_last_sort_order;
    int m_default_group_by;
    DwarfModel *get_model() {return m_model;}
    DwarfModelProxy *get_proxy() {return m_proxy;}

    int get_last_group_by(){return m_last_group_by;}
    void set_last_group_by(int group_id);
    void set_default_group(QString name);

    QString get_view_name(){return m_view_name;}

    void restore_scroll_positions();
    void set_scroll_positions(int v_value, int h_value);

    //use these to read the scroll positions rather than the scrollbar itself, as sometimes we ignore the scrollbar changes
    int vertical_scroll_position() {return m_vscroll;}
    int horizontal_scroll_position() {return m_hscroll;}

    bool is_loading_rows;
    bool is_active;

    public slots:
        void read_settings();
        void filter_dwarves(QString text);
        void set_single_click_labor_changes(bool enabled);
        void jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void jump_to_profession(QTreeWidgetItem* current, QTreeWidgetItem* previous);
        void select_dwarf(Dwarf *d);
        void select_dwarf(int id);
        void select_all();
        void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

        // expand/collapse persistence
        void expandAll();
        void collapseAll();
        void index_expanded(const QModelIndex &idx);
        void index_collapsed(const QModelIndex &idx);
        void restore_expanded_items();
        void currentChanged(const QModelIndex &, const QModelIndex &);
        void activate_cells(const QModelIndex &index);
        void header_pressed(int index);
        void header_clicked(int index);

        void build_custom_profession_menu();

protected:
    virtual void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

private:
    DwarfModel *m_model;
    DwarfModelProxy *m_proxy;
    UberDelegate *m_delegate;
    RotatedHeader *m_header;
    int m_grid_size;
    QList<int> m_expanded_rows;
    bool m_auto_expand_groups;
    bool m_single_click_labor_changes;
    //! we have to store this ourselves since the click(), accept() etc... don't send which button caused them
    Qt::MouseButton m_last_button;
    bool m_column_already_sorted;
    int m_last_group_by;
    int m_vscroll;
    int m_hscroll;
    QModelIndex m_last_cell;
    bool m_dragging;
    bool m_toggling_multiple;
    QString m_view_name;
    void keyPressEvent(QKeyEvent *event);

    QMenu *m;
    QMenu *customization_menu;
    QMenu *squads_menu;
    QMenu *debug_menu;

    QAction *m_prof_name;
    QAction *m_professions;
    QAction *m_update_profession;
    QAction *m_super_labors;

    QAction *m_assign_labors;
    QAction *m_assign_skilled_labors;
    QAction *m_remove_labors;

    QAction *m_unassign_squad;

    QAction *m_clear;
    QAction *m_commit;

    static const int MAX_STR_LEN = 16;
    QString ask_name(const QString &msg, const QString &str_name, const QString &str_default);
    QModelIndexList selected_units();

    private slots:
        void set_nickname();

        void set_custom_profession_text(bool prompt = true);
        void clear_custom_profession_text();
        void apply_custom_profession();
        void reset_custom_profession();
        void custom_profession_from_dwarf();
        void update_custom_profession_from_dwarf();
        void refresh_update_c_prof_menu(Dwarf *d);

        void super_labor_from_dwarf();

        void vscroll_value_changed(int value);
        void hscroll_value_changed(int value);
        void toggle_all_row_labors();

        void toggle_skilled_row_labors();
        void toggle_labor_group();
        void toggle_column_labors();
        void column_right_clicked(int);

        void change_column_sort_method();
        void sort_named_column(int column, DwarfModelProxy::DWARF_SORT_ROLE role, Qt::SortOrder order);

        void commit_pending();
        void clear_pending();

        void update_sort_info(int index);
        void set_global_sort_keys(int);

        //a squad object will emit a signal that we'll slot here,
        //and then emit another signal for the parent object to handle
        void emit_squad_leader_changed(){
            emit squad_leader_changed();
        }
        void set_squad_name();
        void assign_to_squad();
        void remove_squad();

signals:
    void dwarf_focus_changed(Dwarf *d);
    void squad_leader_changed();


};
#endif // STATETABLEVIEW_H
