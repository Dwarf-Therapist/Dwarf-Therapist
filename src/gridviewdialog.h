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

#ifndef GRID_VIEW_DIALOG_H
#define GRID_VIEW_DIALOG_H

#include <QDialog>

class ContextMenuHelper;
class GridView;
class QItemSelection;
class QStandardItemModel;
class ViewColumnSet;
class ViewManager;
class RolePreferenceModel;

namespace Ui {
class GridViewDialog;
}

class GridViewDialog : public QDialog {
    Q_OBJECT
public:
    GridViewDialog(ViewManager *mgr, GridView *view, RolePreferenceModel *pref_model, QWidget *parent = 0);
    virtual ~GridViewDialog();

    QString name();
    GridView *view() {return m_view;}
    GridView *pending_view() {return m_pending_view;}
    ViewManager *manager() {return m_manager;}

public slots:
    void accept();
    void set_selection_changed(const QItemSelection&, const QItemSelection&);
    void draw_columns_for_set(ViewColumnSet *set, bool set_changed = false);

private:
    Ui::GridViewDialog *ui;
    GridView *m_view;
    GridView *m_pending_view;
    ViewManager *m_manager;
    bool m_is_editing;
    QString m_original_name;
    QStandardItemModel *m_set_model;
    QStandardItemModel *m_col_model;
    int m_temp_set;
    int m_temp_col;
    ViewColumnSet *m_active_set;
    ContextMenuHelper *m_cmh;
    RolePreferenceModel *m_pref_model;

private slots:
    //! for redrawing sets in the edit dialog
    void draw_sets();
    //! called when the order of sets changes
    void set_order_changed();
    //! called when the order of columns changes
    void column_order_changed();
    //! makes sure the name for this view is ok
    void check_name(const QString &);
    //! add the currently selected set in the combobox to this view's set list
    void add_set();
    //! when a row from the column list is moved, handling the view's row removed allows us to re-order
    void column_removed(QModelIndex, int, int);
    void set_removed(QModelIndex, int, int);

    //! overridden context menu for the set list
    void draw_set_context_menu(const QPoint &);
    //! edit the details of a set (from a double-click)
    void edit_set(const QModelIndex &);
    //! edit the details of a set (from an action)
    void edit_set();
    //! called from the context menu
    void remove_set();

    //! overridden context menu for the set list
    void draw_column_context_menu(const QPoint &);
    //! edit the details of a column (from a double-click)
    void edit_column(const QModelIndex &);
    //! edit the details of a column (from an action)
    void edit_column();
    //! called from the context menu
    void remove_column();

    //! column adders
    void all_clicked();

    void add_spacer_column();
    void add_happiness_column();
    void add_labor_column();
    void add_skill_column();
    void add_idle_column();
    void add_trait_column();
    void add_attribute_column();
    void add_role_column();
    void add_weapon_column();
    void add_health_column();
    void add_profession_column();
    void add_highest_moodable_column();
    void add_trained_column();
    void add_equipment_column();
    void add_super_labor_column();
    void add_custom_prof_column();
    void add_belief_column();
    void add_kills_column();
    void add_preference_column();
    void add_focus_column();
    void add_need_column();
};

#endif
