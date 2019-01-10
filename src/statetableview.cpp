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

#include "mainwindow.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "uberdelegate.h"
#include "rotatedheader.h"
#include "dwarf.h"
#include "defines.h"
#include "columntypes.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "laborcolumn.h"
#include "dwarftherapist.h"
#include "customprofession.h"
#include "viewmanager.h"
#include "squad.h"
#include "gamedatareader.h"
#include "profession.h"
#include "labor.h"
#include "defaultfonts.h"
#include "dtstandarditem.h"
#include "fortressentity.h"

#include <QContextMenuEvent>
#include <QInputDialog>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QScrollBar>
#include <QTreeWidgetItem>
#include <QWheelEvent>

StateTableView::~StateTableView()
{}

StateTableView::StateTableView(QWidget *parent)
    : QTreeView(parent)
    , m_last_sorted_col(0)
    , m_last_sort_order(Qt::AscendingOrder)
    , m_default_group_by(-1)
    , m_model(0)
    , m_proxy(0)
    , m_delegate(new UberDelegate(this))
    , m_header(new RotatedHeader(Qt::Horizontal, this))
    , m_expanded_rows(QList<int>())
    , m_last_button(Qt::NoButton)
    , m_last_group_by(-1)
    , m_vscroll(0)
    , m_hscroll(0)
    , m_dragging(false)
    , m_toggling_multiple(false)
    , m_view_name("")
{
    read_settings();

    setMouseTracking(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setUniformRowHeights(true);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setIndentation(8);
    //setFocusPolicy(Qt::NoFocus); // keep the dotted border off of things
    setFocusPolicy(Qt::ClickFocus);

    setItemDelegate(m_delegate);
    setHeader(m_header);

    verticalScrollBar()->setFocusPolicy(Qt::StrongFocus);
    horizontalScrollBar()->setFocusPolicy(Qt::StrongFocus);

    // Set StaticContents to enable minimal repaints on resizes.
    viewport()->setAttribute(Qt::WA_StaticContents);

    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));

    connect(this, SIGNAL(expanded(const QModelIndex &)),SLOT(index_expanded(const QModelIndex &)));
    connect(this, SIGNAL(collapsed(const QModelIndex &)),SLOT(index_collapsed(const QModelIndex &)));

    connect(m_header, SIGNAL(sectionPressed(int)), SLOT(header_pressed(int)));
    connect(m_header, SIGNAL(sectionClicked(int)), SLOT(header_clicked(int)));

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(hscroll_value_changed(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(vscroll_value_changed(int)));

    m = new QMenu(this);
    m->addAction(QIcon(":img/ui-text-field-select.png"),tr("Set Nickname..."), this, SLOT(set_nickname()));
    m->addSeparator();

    customization_menu = new QMenu(m);

    //CUSTOM PROFESSIONS/SUPER LABORS
    customization_menu->setTitle(tr("Customization"));
    customization_menu->setTearOffEnabled(true);
    customization_menu->setWindowTitle("Customization");

    build_custom_profession_menu();
    m->addMenu(customization_menu);

    //TOGGLE ALL LABORS
    m->addSeparator();
    m_assign_labors = m->addAction(QIcon(":img/plus-circle.png"), tr("Assign All Labors"), this, SLOT(toggle_all_row_labors()));
    m_assign_labors->setData(true);
    m_assign_skilled_labors = m->addAction(QIcon(":img/plus-white.png"), tr("Assign Skilled Labors"), this, SLOT(toggle_skilled_row_labors()));
    m_assign_skilled_labors->setStatusTip(tr("Assigns all labors above 'Dabbling'."));
    m_remove_labors = m->addAction(QIcon(":img/minus-circle.png"), tr("Clear All Labors"), this, SLOT(toggle_all_row_labors()));
    m_remove_labors->setData(false);

    m->addSeparator();
    m_clear = m->addAction(QIcon(":img/table--minus.png"),tr("Clear pending changes."),this,SLOT(clear_pending()));
    m_commit = m->addAction(QIcon(":img/table--arrow.png"),tr("Commit changes."),this,SLOT(commit_pending()));
    m->addSeparator();

    squads_menu = new QMenu(m);
    nobles_menu = new QMenu(m);
    m_unassign_squad = new QAction(QIcon(QString::fromUtf8(":/img/minus-circle.png")),"",m);
    m->addSeparator();
    debug_menu = new QMenu(m);
    debug_menu->setTitle(tr("Memory Tools"));
}

void StateTableView::build_custom_profession_menu(){
    customization_menu->clear();
    m_prof_name = customization_menu->addAction(QIcon(":img/ui-text-field-select.png"), tr("Set custom profession name..."), this, SLOT(set_custom_profession_text()));
    customization_menu->addAction(QIcon(":img/ui-text-field-clear-button.png"), tr("Clear custom profession name"), this, SLOT(clear_custom_profession_text()));
    customization_menu->addAction(tr("Reset to default profession"), this, SLOT(reset_custom_profession()));
    customization_menu->addSeparator();
    m_professions = customization_menu->addAction(QIcon(":img/new.png"), tr("New custom profession from this unit..."), this, SLOT(custom_profession_from_dwarf()));
    m_update_profession = customization_menu->addAction(QIcon(":img/refresh.png"), "", this, SLOT(update_custom_profession_from_dwarf()));
    m_super_labors = customization_menu->addAction(QIcon(":img/new.png"), tr("New super labor column from this unit..."), this, SLOT(super_labor_from_dwarf()));
    customization_menu->addSeparator();
    foreach(CustomProfession *cp, DT->get_custom_professions()) {
        customization_menu->addAction(QIcon(cp->get_pixmap()), cp->get_name(), this, SLOT(apply_custom_profession()));
    }
}


void StateTableView::read_settings() {
    QSettings *s = DT->user_settings();

    //font
    QFont fnt = s->value("options/grid/font", QFont(DefaultFonts::getRowFontName(), DefaultFonts::getRowFontSize())).value<QFont>();
    setFont(fnt);

    //cell size
    m_grid_size = s->value("options/grid/cell_size", DEFAULT_CELL_SIZE).toInt();
    int pad = s->value("options/grid/cell_padding", 0).toInt();
    setIconSize(QSize(m_grid_size - 2 - pad * 2, m_grid_size - 2 - pad * 2));

    set_single_click_labor_changes(s->value("options/single_click_labor_changes", true).toBool());
    m_auto_expand_groups = s->value("options/auto_expand_groups", false).toBool();
}

void StateTableView::set_single_click_labor_changes(bool enabled){
    m_single_click_labor_changes = enabled;

    //if using double click, then wait until the cell is activated to change cell states
    //if using single click, we can process the cell states immediately upon a click
    if(!m_single_click_labor_changes){
        connect(this,SIGNAL(doubleClicked(QModelIndex)),SLOT(activate_cells(QModelIndex)),Qt::UniqueConnection);
        disconnect(this,SIGNAL(clicked(QModelIndex)),this,SLOT(activate_cells(QModelIndex)));
    }else{
        connect(this, SIGNAL(clicked(const QModelIndex&)), SLOT(activate_cells(const QModelIndex &)), Qt::UniqueConnection);
        disconnect(this,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(activate_cells(QModelIndex)));
    }
}

void StateTableView::set_default_group(QString name){
    m_default_group_by = DT->user_settings()->value(QString("gui_options/%1_group_by").arg(name),-1).toInt();
    m_view_name = name;
}

void StateTableView::set_model(DwarfModel *model, DwarfModelProxy *proxy) {
    QTreeView::setModel(proxy);
    m_model = model;
    m_proxy = proxy;

    m_delegate->set_model(model);
    m_delegate->set_proxy(proxy);

    connect(m_header, SIGNAL(section_right_clicked(int)), m_model,SLOT(section_right_clicked(int)));
    connect(m_header, SIGNAL(section_right_clicked(int)), this,SLOT(column_right_clicked(int)));
    connect(m_header, SIGNAL(sort(int,DwarfModelProxy::DWARF_SORT_ROLE,Qt::SortOrder)),
            this, SLOT(sort_named_column(int,DwarfModelProxy::DWARF_SORT_ROLE,Qt::SortOrder)));
            //m_proxy, SLOT(sort(int,DwarfModelProxy::DWARF_SORT_ROLE, Qt::SortOrder)));
    connect(m_model, SIGNAL(preferred_header_size(int, int)), m_header, SLOT(resizeSection(int, int)));
    connect(m_model, SIGNAL(set_index_as_spacer(int)), m_header, SLOT(set_index_as_spacer(int)));
    connect(m_model, SIGNAL(clear_spacers()), m_header, SLOT(clear_spacers()));

    connect(m_model,SIGNAL(preferred_header_height(QString)),m_header,SLOT(set_header_height(QString)));

    set_single_click_labor_changes(DT->user_settings()->value("options/single_click_labor_changes", true).toBool());
}

void StateTableView::filter_dwarves(QString text) {
    m_proxy->setFilterFixedString(text);
    m_proxy->setFilterKeyColumn(0);
    m_proxy->setFilterRole(Qt::DisplayRole);
}

void StateTableView::jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem*) {
    if (!current)
        return;
    int dwarf_id = current->data(0, Qt::UserRole).toInt();

    Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
    if (d && d->m_name_idx.isValid()) {
        QModelIndex proxy_idx = m_proxy->mapFromSource(d->m_name_idx);
        if (proxy_idx.isValid()) {
            //scrollTo(proxy_idx);
            selectionModel()->select(proxy_idx, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
        }
    }
}

void StateTableView::jump_to_profession(QTreeWidgetItem *current, QTreeWidgetItem *){
    if (!current)
        return;
    QString prof_name = current->text(0);
    QModelIndexList matches = m_proxy->match(m_proxy->index(0,0), Qt::DisplayRole, prof_name);
    if (matches.size() > 0) {
        QModelIndex group_header = matches.at(0);
        expand(group_header);
        selectionModel()->select(group_header, QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows);
    }
}

void StateTableView::contextMenuEvent(QContextMenuEvent *event) {
    QModelIndex idx = indexAt(event->pos());
    if (!idx.isValid())
        return;

    if(idx.column() == 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
        // we're on top of a name
        int id = idx.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);

        //remove any dynamically changed menus
        m->removeAction(squads_menu->menuAction());
        m->removeAction(nobles_menu->menuAction());
        m->removeAction(debug_menu->menuAction());
        m->removeAction(m_unassign_squad);

        if(!d->is_animal()){

            customization_menu->setEnabled(true);
            m_assign_labors->setEnabled(d->can_set_labors());
            m_assign_skilled_labors->setEnabled(d->can_set_labors());
            m_remove_labors->setEnabled(d->can_set_labors());

            refresh_update_c_prof_menu(d);

            // Nobles menu
            nobles_menu->setTitle(tr("Assign noble position..."));
            nobles_menu->clear();
            nobles_menu->setEnabled(true);
            for (const auto &p: m_model->get_instance()->fortress()->get_noble_assignments()) {
                auto action = nobles_menu->addAction(p.second.name, this, SLOT(assign_noble()));
                action->setData(p.first);
            }
            m->addMenu(nobles_menu);

            // Squads menu
            if(d->can_assign_military()){
                if(m_model->active_squads().count() <= 0){
                    squads_menu->setTitle(tr("No squads found."));
                    squads_menu->setEnabled(false);
                    m->addMenu(squads_menu);
                }else{
                    squads_menu->setTitle(tr("Assign to squad..."));
                    squads_menu->clear();
                    squads_menu->setEnabled(true);
                    QIcon icon;
                    icon.addFile(QString::fromUtf8(":/img/plus-circle.png"), QSize(), QIcon::Normal, QIcon::Off);
                    foreach(Squad *s, m_model->active_squads()){
                        if(d->squad_id() != s->id()){
                            QAction *add;
                            if(s->assigned_count() < 10){
                                add = squads_menu->addAction(tr("%1 (%2 members)").arg(s->name()).arg(s->assigned_count()), this, SLOT(assign_to_squad()));
                            }else{
                                add = squads_menu->addAction(tr("%1 (Full)").arg(s->name()));
                                add->setEnabled(false);
                            }
                            add->setData(s->id());
                            add->setIcon(icon);
                        }
                    }
                    if(squads_menu->actions().count()>0)
                        m->addMenu(squads_menu);

                    //squad removal (can't remove captain if there are subordinates)
                    if(d->squad_id() != -1){
                        Squad *s = m_model->get_squad(d->squad_id());
                        if(s){
                            if((d->squad_position()==0 && s->assigned_count()==1) || d->squad_position() != 0){
                                m_unassign_squad->setText(tr("Remove from squad."));
                                connect(m_unassign_squad,SIGNAL(triggered()),this, SLOT(remove_squad()));
                                m->addAction(m_unassign_squad);
                            }else{
                                m_unassign_squad->setText(tr("Remove subordinates first!"));
                                disconnect(m_unassign_squad, SIGNAL(triggered()), this, SLOT(remove_squad()));
                                m->addAction(m_unassign_squad);
                            }
                        }
                    }
                }
            }else{
                squads_menu->setTitle(tr("Ineligible for military."));
                squads_menu->actions().clear();
                squads_menu->setEnabled(false);
                m->addMenu(squads_menu);
            }
        }else{
            //animal, hide stuffs
            customization_menu->hideTearOffMenu();
            customization_menu->setEnabled(false);
            m_assign_labors->setEnabled(false);
            m_assign_skilled_labors->setEnabled(false);
            m_remove_labors->setEnabled(false);
        }
        m->addSeparator();

        m_commit->setData(id);
        m_clear->setData(id);

        if(d->pending_changes() > 0){
            m_commit->setVisible(true);
            m_clear->setVisible(true);
        }else{
            m_commit->setVisible(false);
            m_clear->setVisible(false);
        }

        //dwarf actions (debug/memory stuff)
        m->addSeparator();
        debug_menu->clear();

#ifdef QT_DEBUG
        debug_menu->addActions(d->get_mem_actions());
        m->addMenu(debug_menu);
#endif

        m->exec(viewport()->mapToGlobal(event->pos()));
    } else if (idx.data(DwarfModel::DR_COL_TYPE).toInt() == CT_LABOR) {
        QMenu labor(this);
        QString set_name = idx.data(DwarfModel::DR_SET_NAME).toString();
        QString group_name = idx.data(DwarfModel::DR_GROUP_NAME).toString();
        QVariantList data;
        data << set_name << group_name << 0;

        if (idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) { //aggregate labor
            QAction *a = labor.addAction(QIcon(":/img/ui-button-toggle.png"),tr("Toggle %1 for all units in Group: %2").arg(set_name).arg(group_name));
            a->setData(data);
            connect(a, SIGNAL(triggered()), this, SLOT(toggle_labor_group()));
        } else { // single dwarf labor
            int count = selected_units().count();
            QString title = tr("%1 selected units.").arg(count);
            if(count == 1){
                // find the dwarf...
                int dwarf_id = idx.data(DwarfModel::DR_ID).toInt();
                Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
                if(d){
                    title = d->nice_name();
                }
            }
            QAction *a = labor.addAction(QIcon(":/img/ui-button-toggle.png"),tr("Toggle %1 for %2").arg(set_name).arg(title));
            data.replace(data.length()-1,1);
            a->setData(data);
            connect(a, SIGNAL(triggered()), this, SLOT(toggle_labor_group()));
        }
        labor.exec(viewport()->mapToGlobal(event->pos()));
    } else if (idx.data(DwarfModel::DR_IS_AGGREGATE).toBool() && m_model->current_grouping()==DwarfModel::GB_SQUAD){
        QMenu squad_name(this);
        QAction *a = squad_name.addAction(QIcon(":/img/ui-text-field-select.png"),tr("Change Squad Name"),this,SLOT(set_squad_name()));
        if(idx.data(DwarfModel::DR_ID).toInt() >= 0){
            a->setData(idx.data(DwarfModel::DR_ID));
            squad_name.exec(viewport()->mapToGlobal(event->pos()));
        }
    } else if (idx.data(DwarfModel::DR_COL_TYPE).toInt() == CT_PROFESSION) {
        QMenu prof_icon(this);
        int id = idx.data(DwarfModel::DR_SORT_VALUE).toInt(); //sort value is the profession id
        QAction *a = prof_icon.addAction(QIcon(":/img/image--pencil.png"),tr("Customize %1 Icon")
                                         .arg(GameDataReader::ptr()->get_profession(id)->name()),DT,SLOT(edit_customization()));
        //build the data list
        QVariantList data;
        data << id << CUSTOM_ICON;
        a->setData(data);

        if(DT->get_custom_prof_icon(id)){
            a = prof_icon.addAction(QIcon(":img/minus-circle.png"), tr("Reset to Default"), DT, SLOT(delete_customization()));
            a->setData(data);
        }

        prof_icon.exec(viewport()->mapToGlobal(event->pos()));
    }
}

void StateTableView::refresh_update_c_prof_menu(Dwarf *d){
    if(!d->custom_profession_name().isEmpty()){
        m_update_profession->setVisible(true);
        m_update_profession->setEnabled(true);
        m_update_profession->setText(tr("Update custom profession (%1) from this unit").arg(d->custom_profession_name()));
    }else{
        m_update_profession->setEnabled(false);
        m_update_profession->setVisible(false);
    }
}

void StateTableView::toggle_all_row_labors(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    bool enable = a->data().toBool();
    int id = 0;
    foreach(QModelIndex i, selected_units()) {
        id = i.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);
        if (d) {
            if(enable)
                d->assign_all_labors();
            else
                d->clear_labors();
        }
    }
    m_model->calculate_pending();
    DT->emit_labor_counts_updated();
}

void StateTableView::toggle_column_labors(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if(!a->data().canConvert<QVariantList>())
        return;
    QList<QVariant> data = a->data().toList();
    int labor_id = data.at(0).toInt();
    bool enable = data.at(1).toBool();
    bool skilled = data.at(2).toBool();
    foreach(Dwarf *d, m_proxy->get_filtered_dwarves()){
        if(skilled)
            d->toggle_skilled_labor(labor_id);
        else
            d->set_labor(labor_id,enable,false);
    }
    m_model->calculate_pending();
    DT->emit_labor_counts_updated();
}

void StateTableView::toggle_skilled_row_labors(){
    int id = 0;
    foreach(QModelIndex i, selected_units()) {
        id = i.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);
        if (d) {
            d->toggle_skilled_labors();
        }
    }
    m_model->calculate_pending();
    DT->emit_labor_counts_updated();
}

void StateTableView::toggle_labor_group(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if(!a->data().canConvert<QVariantList>())
        return;
    QList<QVariant> data = a->data().toList();
    QString set_name = data.at(0).toString();
    QString group_name = data.at(1).toString();
    int type = data.at(2).toInt();

    int first_col = 0;
    int last_col = 0;
    foreach(ViewColumnSet *set,DT->get_main_window()->get_view_manager()->get_active_view()->sets()){
        if(set->name() == set_name){
            last_col = first_col + set->columns().count();
            break;
        }
        first_col += set->columns().count();
    }
    if(type == 1){
        foreach(QModelIndex i, selected_units()) {
            int dwarf_id = i.data(DwarfModel::DR_ID).toInt();
            Dwarf *d = m_model->get_dwarf_by_id(dwarf_id);
            if(d){
                m_model->labor_group_toggled(d,first_col+1,last_col,m_proxy);
            }
        }
    }else{
        m_model->labor_group_toggled(group_name,first_col+1,last_col,m_proxy);
    }
    m_model->calculate_pending();
    DT->emit_labor_counts_updated();
}

void StateTableView::commit_pending(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    if(d){
        d->commit_pending(true);
        m_model->calculate_pending();
        DT->get_main_window()->get_view_manager()->redraw_current_tab();
    }
}

void StateTableView::clear_pending(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    if(d){
        d->clear_pending();
        m_model->calculate_pending();
        DT->get_main_window()->get_view_manager()->redraw_current_tab();
    }
}

void StateTableView::set_squad_name(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    Squad *s = m_model->get_squad(a->data().toInt());
    if(s){
        QString alias = ask_name(tr("Enter a new squad name for %1, or leave blank to reset to their default name."),
                                 tr("Squad Name"),s->name());
        if(!alias.isNull()) {
            QModelIndex idx = m_model->findOne(s->name(),DwarfModel::DR_GROUP_NAME);
            if(idx.isValid()){
                m_model->itemFromIndex(idx)->setText(QString("%1 (%2)").arg(alias).arg(s->assigned_count()));
            }
            s->rename_squad(alias);
            m_model->calculate_pending();
        }
    }
}

void StateTableView::assign_to_squad(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    Squad *new_squad = m_model->get_squad(a->data().toInt());
    if(new_squad == 0){
        return;
    }else{
        connect(new_squad,SIGNAL(squad_leader_changed()),this,SLOT(emit_squad_leader_changed()));
    }
    int id = -1;
    const QModelIndexList sel = selected_units();
    if(sel.count() <= 0)
        return;
    foreach(QModelIndex i, sel) {
        id = i.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);
        if (d->can_assign_military())
            new_squad->assign_to_squad(d);
    }
    disconnect(new_squad,SIGNAL(squad_leader_changed()),this,SLOT(emit_squad_leader_changed()));
    m_model->calculate_pending();
    DT->get_main_window()->get_view_manager()->redraw_current_tab();
}

void StateTableView::assign_noble(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int assignment_id = a->data().toInt();
    const QModelIndexList sel = selected_units();
    if(sel.count() <= 0)
        return;
    foreach(QModelIndex i, sel) {
        int id = i.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);
        d->add_noble_assignment(assignment_id);
    }
    m_model->calculate_pending();
    DT->get_main_window()->get_view_manager()->redraw_current_tab();
}

void StateTableView::remove_squad(){
    const QModelIndexList sel = selected_units();
    if(sel.count() <= 0)
        return;

    foreach(QModelIndex i, sel) {
        int id = i.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = m_model->get_dwarf_by_id(id);
        if (d) {
            if(d->squad_position()==0)
                emit squad_leader_changed();
            Squad *s = m_model->get_squad(d->squad_id());
            if(s)
                s->remove_from_squad(d);
        }
    }
    m_model->calculate_pending();
    DT->get_main_window()->get_view_manager()->redraw_current_tab();
}

void StateTableView::set_nickname() {
    const QModelIndexList sel = selected_units();
    if(sel.count() <= 0)
        return;

    int dwarfId = sel.at(0).data(DwarfModel::DR_ID).toInt();

    QString defaultText = "";

    if (sel.count() == 1) {
        Dwarf* d = m_model->get_dwarf_by_id(dwarfId);
        defaultText = d->nickname();

        if (d->nickname().isEmpty()) {
            defaultText = d->nice_name();
        }
    }

    QString new_nick = ask_name(tr((sel.count() > 1) ? "Enter a new nickname for the selected dwarves." : "Enter a new nickname for the selected dwarf."),
                                tr("Nickname"), defaultText);
    if(!new_nick.isNull()) {
        foreach(QModelIndex i, sel) {
            int id = i.data(DwarfModel::DR_ID).toInt();
            Dwarf *d = m_model->get_dwarf_by_id(id);
            if (d) {
                d->set_nickname(new_nick);
                m_model->setData(i, d->nice_name(), Qt::DisplayRole);
            }
        }
        m_model->calculate_pending();
    }
}

QString StateTableView::ask_name(const QString &msg, const QString &str_name, const QString &str_default){
    bool ok;
    QString new_name = QInputDialog::getText(this, tr("New %1").arg(str_name),
                                             msg + tr(" Leave blank to reset to the default."), QLineEdit::Normal,
                                             tr(qPrintable(str_default)), &ok);
    if(ok){
        if(new_name.length() > MAX_STR_LEN){
            QMessageBox::warning(this, tr("Max Length Exceeded"),
                                 tr("Due to technical limitations, %1 must be under %2 characters "
                                    "long. If you require a longer %1, you'll have to set it within Dwarf Fortress!").arg(str_name).arg(MAX_STR_LEN));
            new_name.resize(MAX_STR_LEN);
        }
        return new_name;
    }else{
        return QString();
    }
}

void StateTableView::custom_profession_from_dwarf() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    if(DT->add_custom_profession(d)){
        m_model->calculate_pending();
        DT->emit_labor_counts_updated();
    }
}

void StateTableView::update_custom_profession_from_dwarf() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    DT->update_multilabor(d,d->custom_profession_name(),CUSTOM_PROF);
}

void StateTableView::super_labor_from_dwarf(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int id = a->data().toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    DT->add_super_labor(d);
}

void StateTableView::apply_custom_profession() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    CustomProfession *cp = DT->get_custom_profession(a->text());
    if (!cp)
        return;

    foreach(const QModelIndex idx, selected_units()) {
        Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
        if (d)
            d->apply_custom_profession(cp);
    }
    m_model->calculate_pending();
    DT->emit_labor_counts_updated();
}

void StateTableView::reset_custom_profession() {
    foreach(const QModelIndex idx, selected_units()) {
        Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
        if (d)
            d->reset_custom_profession(true);
    }
    m_model->calculate_pending();
}

void StateTableView::set_custom_profession_text(bool prompt) {
    QString prof_name = "";
    if(prompt){
        bool warn = false;
        do {
            bool ok;
            if (warn)
                QMessageBox::warning(this, tr("Name too long!"), tr("Profession names must be 15 characters or shorter!"));
            prof_name = QInputDialog::getText(this, tr("New Custom Profession Name"),
                                              tr("Custom Profession"), QLineEdit::Normal, QString(), &ok);
            if (!ok)
                return;
            warn = prof_name.length() > 15;
        } while(warn);
    }
    foreach(const QModelIndex idx, selected_units()) {
        Dwarf *d = m_model->get_dwarf_by_id(idx.data(DwarfModel::DR_ID).toInt());
        if (d)
            d->set_custom_profession_text(prof_name);
    }
    m_model->calculate_pending();
}

void StateTableView::clear_custom_profession_text(){
    set_custom_profession_text(false);
}

void StateTableView::currentChanged(const QModelIndex &cur, const QModelIndex &idx) {
    // current item changed, so find out what dwarf the current item is for...
    if (!m_proxy) // special case tables (like skill legend, don't have a proxy)
        return;
    int id = m_proxy->data(cur, DwarfModel::DR_ID).toInt();
    Dwarf *d = m_model->get_dwarf_by_id(id);
    if (d) {
        if(idx.column() <= 0 && !idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()){
            m_prof_name->setData(id);
            m_professions->setData(id);
            m_update_profession->setData(id);
            refresh_update_c_prof_menu(d);
            m_super_labors->setData(id);
        }
        //LOGD << "focus changed to" << d->nice_name();
        emit dwarf_focus_changed(d);
    }
}

void StateTableView::select_all(){
    expandAll();
    selectAll();
    m_selected_rows = selectionModel()->selectedRows(0);
    m_selected = selectionModel()->selection();
    m_toggling_multiple = (m_selected_rows.count() > 1);
}

void StateTableView::select_dwarf(Dwarf *d) {
    select_dwarf(d->id());
}

void StateTableView::select_dwarf(int id) {
    for(int top = 0; top < m_proxy->rowCount(); ++top) {
        QModelIndex idx = m_proxy->index(top, 0);
        if (idx.data(DwarfModel::DR_ID).toInt() == id) {
            this->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
            break;
        } else if (m_proxy->rowCount(idx)) { // has children
            for (int sub = 0; sub < m_proxy->rowCount(idx); ++sub) {
                QModelIndex sub_idx = m_proxy->index(sub, 0, idx);
                if (sub_idx.data(DwarfModel::DR_ID).toInt() == id) {
                    this->selectionModel()->select(sub_idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
                }
            }
        }
    }
    m_selected_rows = selectionModel()->selectedRows(0);
    m_selected = selectionModel()->selection();
}

/************************************************************************/
/* Hey look, our own mouse handling (/rolleyes)                         */
/************************************************************************/
void StateTableView::mousePressEvent(QMouseEvent *event) {
    m_last_button = event->button();
    m_last_cell = indexAt(QPoint(-1,-1));
    //normally, after this event, rows are selected or deselected, before the clicked event is handled
    //however if we have multiple selections, we don't want it to deselect rows when labor cells are toggled
    //so we set a flag here to reselect the deselected rows in the selectionchanged event, and then let the clicked event be handled as normal

    //if multiple rows are selected, and we're clicking on a cell within one of the selected rows, which isn't the name column
    //then dont't deselect, otherwise behave as normal, unless it's the name column clicked, then we want to reselect again
    QModelIndex idx = indexAt(event->pos());
    int col = idx.column();
    int row = idx.row();
    bool intersects = false;
    for(int i = 0; i < m_selected_rows.count(); i++){
        if(m_selected_rows.at(i).row() == row){
            intersects = true;
            break;
        }
    }
    if(m_selected_rows.count() > 1 && col != 0 && intersects){
        m_toggling_multiple = true;
    }else{
        m_toggling_multiple = false;
        m_selected_rows.clear();
        m_selected.clear();
    }

    QTreeView::mousePressEvent(event);
}

void StateTableView::mouseMoveEvent(QMouseEvent *event) {
    QModelIndex idx = indexAt(event->pos());
    if(idx.isValid()){
        m_header->column_hover(idx.column());
        //if we're dragging while holding down the left button, and not over the names
        //then start painting labor cells
        //if we're dragging over the names, allow it to select the rows
        if(m_last_button == Qt::LeftButton && idx.column() != 0){
            m_dragging = true;
            activate_cells(idx);
        }else{
            if(idx.column() == 0){
                m_selected_rows = selectionModel()->selectedRows(0);
                m_selected = selectionModel()->selection();
            }
            else
                m_dragging = false;
        }
        if(m_proxy){
          m_proxy->redirect_tooltip(idx);
        }
    }
    QTreeView::mouseMoveEvent(event);
}

void StateTableView::mouseReleaseEvent(QMouseEvent *event) {
    m_last_button = Qt::NoButton;//event->button();
    if(!m_dragging)
        m_last_cell = indexAt(QPoint(-1,-1));
    QTreeView::mouseReleaseEvent(event);
}

void StateTableView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected){
    if(m_toggling_multiple){
        selectionModel()->select(deselected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }else if(m_dragging){
        selectionModel()->select(selected, QItemSelectionModel::Deselect);
    }else{
        QTreeView::selectionChanged(selected,deselected);
    }
}

void StateTableView::activate_cells(const QModelIndex &idx){
    if(m_dragging && m_last_cell == idx){
        return;
    }

    if (//m_last_button == Qt::LeftButton && // only activate on left-clicks
            m_proxy && // we only do this for views that have proxies (dwarf views)
            idx.column() != 0) // don't single-click activate the name column
    {
        if(m_toggling_multiple){
            //ignore aggregate rows when toggling via multiple selections
            bool is_agg = false;
            foreach(QModelIndex i, m_selected_rows){
                is_agg = m_proxy->index(i.row(),i.column(),i.parent()).data(DwarfModel::DR_IS_AGGREGATE).toBool();
                if(!is_agg){
                    const QModelIndex idx_sel = m_proxy->index(i.row(),idx.column(),i.parent());
                    m_proxy->cell_activated(idx_sel);
                }
            }
        }else{
            m_proxy->cell_activated(idx);
        }
        //update the column counts and any related exclusive labors
        ViewColumn *c = m_model->current_grid_view()->get_column(idx.column());
        if(c && c->type()==CT_LABOR){
            int id = static_cast<LaborColumn*>(c)->labor_id();
            DT->update_specific_header(id,c->type());
            QList<int> related = GameDataReader::ptr()->get_labor(id)->get_excluded_labors();
            foreach(int id, related)
                DT->update_specific_header(id,c->type());
        }
        m_model->calculate_pending();
    }else{
        if(m_proxy && idx.column() == 0){
            m_selected_rows = selectionModel()->selectedRows(0);
            m_selected = selectionModel()->selection();
        }
    }
    m_last_cell = idx;
}

void StateTableView::header_clicked(int index) {
    if (!m_column_already_sorted && index > 0) {
        m_header->setSortIndicator(index, Qt::DescendingOrder);
    }
    update_sort_info(index);
}

//this handles the right click context menu sorting on the first (name) column of any/all gridviews
void StateTableView::sort_named_column(int column, DwarfModelProxy::DWARF_SORT_ROLE role, Qt::SortOrder order) {
    m_proxy->sort(column,role,order);
    m_model->set_global_group_sort_info(m_proxy->m_last_sort_role,m_proxy->m_last_sort_order);
}

void StateTableView::column_right_clicked(int idx){
    QMenu *m = new QMenu(this);

    ViewColumn *col = m_model->current_grid_view()->get_column(idx);
    if(col && col->get_sortable_types().count() > 0){
        QString title = "Sort " + get_column_type(col->type()).toLower().replace("_"," ") + " columns by..";
        m->setWindowTitle(title);

        QAction *a = m->addAction(title);
        //initialize our little data struct we'll pass when a sort type is chosen
        QList<QVariant> data;
        data << idx << col->type() << ViewColumn::CST_LEVEL;
        QIcon current(":img/ui-button-navigation.png");
        foreach(ViewColumn::COLUMN_SORT_TYPE sType, col->get_sortable_types()){
            data.replace(2, sType);
            a = m->addAction(capitalizeEach(ViewColumn::get_sort_type(sType).toLower().replace("_"," ")), this, SLOT(change_column_sort_method()));
            a->setData(QVariant(data));
            if(sType == col->get_current_sort())
                a->setIcon(current);
        }

        //args: labor_id, enable, skilled_only
        if(col->type() == CT_LABOR){
            int labor_id = static_cast<LaborColumn*>(col)->labor_id();
            m->addSeparator();
            a = m->addAction(QIcon(":img/plus-circle.png"),tr("Assign %1 to everyone.").arg(col->title()),this, SLOT(toggle_column_labors()));
            data.clear();
            data << labor_id << true << false;
            a->setData(QVariant(data));

            a = m->addAction(QIcon(":img/plus-white.png"),tr("Assign %1 to skilled workers only.").arg(col->title()),this, SLOT(toggle_column_labors()));
            data.replace(2,true); //skilled only
            a->setData(QVariant(data));

            a = m->addAction(QIcon(":img/minus-circle.png"),tr("Unassign %1 from everyone.").arg(col->title()),this, SLOT(toggle_column_labors()));
            data.replace(1,false); //disable
            data.replace(2, false); //ignore skill
            a->setData(QVariant(data));
        }

        m->exec(this->mapFrom(this,QCursor::pos()));
    }
}

void StateTableView::change_column_sort_method(){
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if(!a->data().canConvert<QVariantList>())
        return;
    QList<QVariant> data = a->data().toList();
    int idx = data.at(0).toInt();
    COLUMN_TYPE cType = static_cast<COLUMN_TYPE>(data.at(1).toInt());
    ViewColumn::COLUMN_SORT_TYPE sType = static_cast<ViewColumn::COLUMN_SORT_TYPE>(data.at(2).toInt());

    foreach(ViewColumnSet *set, m_model->current_grid_view()->sets()){
        foreach(ViewColumn *c, set->columns()){
            if(c->type() == cType){
                c->refresh_sort(sType);
            }
        }
    }

    update_sort_info(idx);

    //save what we're sorting by
    ViewManager::save_column_sort(cType,sType);

    //clear the guides
    m_model->section_right_clicked(-1);
}

void StateTableView::update_sort_info(int index){
    m_header->set_last_sorted_idx(index);
    m_last_sorted_col = index;
    m_last_sort_order = m_header->sortIndicatorOrder();

    if(index > 0){
        set_global_sort_keys(index);
        if(index > GLOBAL_SORT_COL_IDX)
            m_model->set_global_sort_col(m_view_name,index);
    }else{
        m_model->set_global_group_sort_info(m_proxy->m_last_sort_role,m_proxy->m_last_sort_order);
        //if the user explicitly sorts by the first column, remove the other sort
        m_model->set_global_sort_col(m_view_name,-1);
    }
}

void StateTableView::set_global_sort_keys(int index){
    //update each unit with their row index from the sorted view
    ViewColumn *col = m_model->current_grid_view()->get_column(index);
    if(col){
        int rank = 0;
        QModelIndex idx;
        foreach(Dwarf *d, col->cells().uniqueKeys()){
            idx = col->cells().value(d)->index(); //get the index in the model
            idx = m_proxy->mapFromSource(idx); //get the index in the proxy
            rank = idx.row(); //row in the sorted view
            if(m_last_sort_order == Qt::DescendingOrder)
                rank = col->cells().count() - rank;
            d->set_global_sort_key(m_last_group_by,rank);
        }
    }
}

void StateTableView::set_last_group_by(int group_id){
    m_model->update_global_sort_col(group_id);
    m_last_group_by = group_id;
}


void StateTableView::header_pressed(int index) {
    m_column_already_sorted = (m_header->sortIndicatorSection() == index);
}

/************************************************************************/
/* Handlers for expand/collapse persistence                             */
/************************************************************************/
void StateTableView::expandAll() {
    m_expanded_rows.clear();
    for(int i = 0; i < m_proxy->rowCount(); ++i) {
        m_expanded_rows << i;
    }
    QTreeView::expandAll();
}

void StateTableView::collapseAll() {
    m_expanded_rows.clear();
    QTreeView::collapseAll();
}

void StateTableView::index_expanded(const QModelIndex &idx) {
    m_expanded_rows << idx.row();
}

void StateTableView::index_collapsed(const QModelIndex &idx) {
    int i = m_expanded_rows.indexOf(idx.row());
    if (i != -1)
        m_expanded_rows.removeAt(i);
}

void StateTableView::restore_expanded_items() {
    if (m_auto_expand_groups) {
        expandAll();
        return;
    }
    disconnect(this, SIGNAL(expanded(const QModelIndex &)), 0, 0);
    foreach(int row, m_expanded_rows) {
        expand(m_proxy->index(row, 0));
    }
    connect(this, SIGNAL(expanded(const QModelIndex &)), SLOT(index_expanded(const QModelIndex &)));
}

void StateTableView::keyPressEvent(QKeyEvent *event ){
    switch(event->key()){
    case Qt::Key_Escape:
        selectionModel()->clear();
        m_selected_rows.clear();
        m_selected.clear();
        break;
    case Qt::Key_PageDown:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->pageStep());
        break;
    case Qt::Key_PageUp:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->pageStep());
        break;
    case Qt::Key_Home:
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->pageStep());
        break;
    case Qt::Key_End:
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->pageStep());
        break;
    case Qt::Key_Up:
        verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep());
        break;
    case Qt::Key_Down:
        verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep());
        break;
    case Qt::Key_Right:
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep());
        break;
    case Qt::Key_Left:
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep());
        break;
    default:
        break;
    }
}

void StateTableView::restore_scroll_positions(){
    m_vscroll = qBound(verticalScrollBar()->minimum(), m_vscroll, verticalScrollBar()->maximum());
    verticalScrollBar()->setValue(m_vscroll);

    m_hscroll = qBound(horizontalScrollBar()->minimum(), m_hscroll, horizontalScrollBar()->maximum());
    horizontalScrollBar()->setValue(m_hscroll);
}

//when loading rows, the slider will move back to the top
//on linux, changing tabs also resets the slider of the previous tab,
//so don't record the position of the scroll if it moves on an inactive view

//additionally when the model clears data after a read, it will reset the scroll positions

void StateTableView::vscroll_value_changed(int value){
    if(!is_loading_rows && is_active && m_model != 0 && !m_model->clearing_data())
        m_vscroll = value;
}
void StateTableView::hscroll_value_changed(int value){
    if(!is_loading_rows && is_active && m_model != 0 && !m_model->clearing_data())
        m_hscroll = value;
}

void StateTableView::set_scroll_positions(int v_value, int h_value){
    m_vscroll = v_value;
    m_hscroll = h_value;
}

void StateTableView::wheelEvent(QWheelEvent *event){
    if(event->modifiers() & Qt::ControlModifier || event->modifiers() & Qt::AltModifier){
        QWheelEvent *evt_h = new QWheelEvent(
                event->posF(), event->globalPosF(), event->pixelDelta(), event->angleDelta(),
                event->delta(), Qt::Horizontal, event->buttons(), event->modifiers());
        QTreeView::wheelEvent(evt_h);
    }else{
        QTreeView::wheelEvent(event);
    }
}

QModelIndexList StateTableView::selected_units(){
    QModelIndexList selected = selectionModel()->selection().indexes();
    for(int i=selected.count()-1;i >= 0;i--) {
        QModelIndex idx = selected.at(i);
        if(idx.column() != 0 || idx.data(DwarfModel::DR_IS_AGGREGATE).toBool()) {
            selected.removeAt(i);
        }
    }
    return selected;
}
