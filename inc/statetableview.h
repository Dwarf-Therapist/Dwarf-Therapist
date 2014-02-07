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

#include <QtWidgets>

class UberDelegate;
class RotatedHeader;
class Dwarf;
class DwarfModel;
class DwarfModelProxy;

class StateTableView : public QTreeView
{
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
    int m_last_group_by;
    int m_default_group_by;
    DwarfModel *get_model() {return m_model;}

    void set_default_group(QString name);

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
    int m_vscroll;
    int m_hscroll;
    QModelIndex m_last_cell;
    bool m_dragging;
    bool m_toggling_multiple;
    QString m_view_name;
    void keyPressEvent(QKeyEvent *event);    

    QMenu *m;
    QMenu *custom_prof_menu;
    QMenu *squads_menu;
    QMenu *debug_menu;

    QAction *m_prof_name;
    QAction *m_professions;

    QAction *m_assign_labors;
    QAction *m_assign_skilled_labors;
    QAction *m_remove_labors;

    QAction *m_unassign_squad;

    QAction *m_clear;
    QAction *m_commit;

    private slots:
        void set_nickname();
        void new_custom_profession();
        void apply_custom_profession();
        void custom_profession_from_dwarf();
        void reset_custom_profession();
        void set_custom_profession_text();
        void set_squad_name();
        void assign_to_squad();
        void remove_squad();
        void vscroll_value_changed(int value);
        void hscroll_value_changed(int value);
        void toggle_all_row_labors();
        void toggle_skilled_row_labors();
        void toggle_column_labors();
        void column_right_clicked(int);
        void sort_column();
        void edit_prof_icon();
        void remove_prof_icon();        
        void commit_pending();
        void clear_pending();
        //a squad object will emit a signal that we'll slot here,
        //and then emit another signal for the parent object to handle
        void emit_squad_leader_changed(){
            emit squad_leader_changed();
        }

signals:
    void new_custom_profession(Dwarf *d);
    void dwarf_focus_changed(Dwarf *d);
    void squad_leader_changed();


};
#endif // STATETABLEVIEW_H
