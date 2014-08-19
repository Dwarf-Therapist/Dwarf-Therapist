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
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include "viewmanager.h"
#include "statetableview.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "gridview.h"
#include "defines.h"
#include "truncatingfilelogger.h"
#include "dwarftherapist.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "utils.h"
#include "gamedatareader.h"
#include "weaponcolumn.h"
#include "spacercolumn.h"
#include "dfinstance.h"
#include "itemweaponsubtype.h"
#include "dwarf.h"

#if QT_VERSION < 0x050000
# define setSectionResizeMode setResizeMode
#endif

QMap<COLUMN_TYPE, ViewColumn::COLUMN_SORT_TYPE> ViewManager::m_default_column_sort;

ViewManager::ViewManager(DwarfModel *dm, DwarfModelProxy *proxy,
                         QWidget *parent)
    : QTabWidget(parent)
    , m_model(dm)
    , m_proxy(proxy)
    , m_add_tab_button(new QToolButton(this))
    , m_reset_sorting(false)
{
    m_proxy->setSourceModel(m_model);
    setTabsClosable(true);
    setMovable(true);

    //reload_views();

    m_add_tab_button->setText(tr("Add "));
    m_add_tab_button->setIcon(QIcon(":img/ui-tab--plus.png"));
    m_add_tab_button->setPopupMode(QToolButton::InstantPopup);
    m_add_tab_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_add_tab_button->setToolTip(tr("Add an existing view. New views can be copied or created from the [Windows->Docks->Grid Views] dock."));
    draw_add_tab_button();
    setCornerWidget(m_add_tab_button, Qt::TopLeftCorner);    

    connect(tabBar(), SIGNAL(tabMoved(int, int)), SLOT(write_views()));
    connect(tabBar(), SIGNAL(currentChanged(int)), SLOT(setCurrentIndex(int)), Qt::UniqueConnection);
    connect(this, SIGNAL(tabCloseRequested(int)), SLOT(remove_tab_for_gridview(int)));
    connect(m_model, SIGNAL(need_redraw()), SLOT(redraw_current_tab()));
    //draw_views();

    m_squad_warning = new QErrorMessage(this);    
}

ViewManager::~ViewManager(){
    qDeleteAll(m_views);
    m_views.clear();

    m_model = 0;
    m_proxy = 0;
    delete m_add_tab_button;
    m_selected_dwarfs.clear();

    delete m_squad_warning;
}

void ViewManager::draw_add_tab_button() {
    std::sort(m_views.begin(), m_views.end(), GridView::name_custom_sort());

    QIcon icn(":img/ui-tab--plus.png");
    QMenu *m = new QMenu(this);
    foreach(GridView *v, m_views) {
        QAction *a = m->addAction(icn, v->name(), this, SLOT(add_tab_from_action()));
        a->setData((qulonglong)v);
    }
    m_add_tab_button->setMenu(m);
}

void ViewManager::reload_views() {
    m_views.clear();
    m_last_index = -1;

    QSettings *u = DT->user_settings();
    //read in the default column sorts
    m_default_column_sort.clear();
    for(int i = 0; i < CT_TOTAL_TYPES; i++){
        QString val = u->value("options/grid/" + get_column_type(static_cast<COLUMN_TYPE>(i)),"").toString();
        if(val != ""){
            m_default_column_sort.insert(static_cast<COLUMN_TYPE>(i),ViewColumn::get_sort_type(val));
        }
    }

    int total_views = 0;
    QSettings *s = 0x0;

    QStringList view_names;
    QDir working_dir = QDir::current();
    QStringList search_paths;
    search_paths << QString("%1/default_gridviews.dtg").arg(working_dir.path());
    search_paths << QString("%1/../share/default_gridviews.dtg").arg(QCoreApplication::applicationDirPath());

    foreach(QString path, search_paths) {
        if (QFile::exists(path)) {
            LOGI << "Found default_gridviews.dtg:" << path;
            s = new QSettings(path, QSettings::IniFormat);
            break;
        }
    }

    if(s){
        total_views = s->beginReadArray("gridviews");
        QList<GridView*> built_in_views;
        for (int i = 0; i < total_views; ++i) {
            s->setArrayIndex(i);
            GridView *gv = GridView::read_from_ini(*s, this);
            gv->set_is_custom(false); // this is a default view
            m_views << gv;
            built_in_views << gv;
            view_names.append(gv->name());
        }
        s->endArray();
    }

    //packaged default views, if we've already loaded an override for a view, don't include these views
    s = new QSettings(":config/default_gridviews", QSettings::IniFormat);
    total_views = s->beginReadArray("gridviews");
    QList<GridView*> built_in_views;
    for (int i = 0; i < total_views; ++i) {
        s->setArrayIndex(i);
        GridView *gv = GridView::read_from_ini(*s, this);
        gv->set_is_custom(false); // this is a default view
        if(!view_names.contains(gv->name())){
            m_views << gv;
            built_in_views << gv;
        }
    }
    s->endArray();
    delete(s);
    s = 0;

    //special default weapon view
    add_weapons_view(built_in_views);

    // now read any gridviews out of the user's settings    
    total_views = u->beginReadArray("gridviews");
    for (int i = 0; i < total_views; ++i) {
        u->setArrayIndex(i);
        GridView *gv = GridView::read_from_ini(*u, this);
        bool name_taken = true;
        do {
            name_taken = false;
            foreach(GridView *built_in, built_in_views) {
                if (gv->name() == built_in->name()) {
                    name_taken = true;
                    break;
                }
            }
            if (name_taken) {
                QMessageBox::information(this, tr("Name in Use!"),
                    tr("A custom view was found in your settings called '%1.' "
                    "However, this name is already taken by a built-in view, "
                    "you must rename the custom view.").arg(gv->name()));
                QString new_name;
                while (new_name.isEmpty() || new_name == gv->name()) {
                    new_name = QInputDialog::getText(this, tr("Rename View"),
                                    tr("New name for '%1'").arg(gv->name()));
                }
                gv->set_name(new_name);
            }
        } while(name_taken);

        gv->set_is_custom(true); // this came from a user's settings
        m_views << gv;
    }
    u->endArray();
    u=0;

    LOGI << "Loaded" << m_views.size() << "views from disk";
    draw_add_tab_button();
}

void ViewManager::add_weapons_view(QList<GridView*> &built_in_views){
    if(!DT->get_main_window())
        return;

    DFInstance *m_df = DT->get_DFInstance();
    if(m_df->get_ordered_weapon_defs().length() > 0){
        //add a special weapons view, this will dynamically change depending on the raws read

        //    //group by skill type for display
        //    QHash<QString, QVector<GameDataReader::weapon>* > grouped_weapons;
        //    QPair<QString, GameDataReader::weapon> weapon_pair;
        //    foreach(weapon_pair, GameDataReader::ptr()->get_ordered_weapons()){
        //        if(!grouped_weapons.contains(weapon_pair.second.skill)){
        //            grouped_weapons.insert(weapon_pair.second.skill,new QVector<GameDataReader::weapon>);
        //        }
        //            grouped_weapons.value(weapon_pair.second.skill)->append(weapon_pair.second);
        //    }
        //    GridView *gv = new GridView("Weapons",this);
        //    foreach(QString key, grouped_weapons.uniqueKeys()){
        //        ViewColumnSet *vcs = new ViewColumnSet(key, this);

        //        QVector<GameDataReader::weapon> *glist = grouped_weapons.value(key);
        //        for(int i=0; i < glist->count(); i++){
        //            new WeaponColumn(glist->at(i).name,glist->at(i), vcs, this);
        //        }
        //        new SpacerColumn("",vcs,this);
        //        gv->add_set(vcs);
        //        gv->set_is_custom(false);
        //    }
        //    m_views << gv;
        //    built_in_views << gv;

//        //group by weapon size.. this is still impossible to find anything
//        QHash<QPair<long,long>, Weapon*> grouped_by_size;
//        QPair<QString, Weapon*> wp;
//        foreach(wp, m_df->get_ordered_weapons()){
//            QPair<long,long> key;
//            key.first = wp.second->single_grasp();
//            key.second = wp.second->multi_grasp();
//            if(!grouped_by_size.contains(key)){
//                grouped_by_size.insert(key,wp.second);
//            }else{
//                Weapon *w = grouped_by_size.value(key);
//                w->group_name = w->group_name.append(", ").append(wp.second->name_plural());
//            }
//        }

        GridView *gv = this->get_view("Weapons");
        if(!gv)
            gv = new GridView("Weapons",this);
        else
            gv->clear();

//        ViewColumnSet *vcs = new ViewColumnSet("All Weapons", this);
//        int count = grouped_by_size.count();
//        QPair<long,long> wkeys;
//        foreach(wkeys, grouped_by_size.uniqueKeys()){
//            new WeaponColumn(grouped_by_size.value(wkeys)->group_name, grouped_by_size.value(wkeys),vcs,this);
//        }

        ViewColumnSet *vcs = new ViewColumnSet("All Weapons", this);
        int count = m_df->get_ordered_weapon_defs().length();
        QPair<QString,ItemWeaponSubtype*> wp;
        foreach(wp, m_df->get_ordered_weapon_defs()){
            new WeaponColumn(wp.first,wp.second,vcs,this);
        }

        gv->add_set(vcs);
        gv->set_is_custom(false);
        //if the weapon columns count <= the vanilla df weapon count, append the columns onto the military tab as well
        if(count<=24){
            GridView *mv = this->get_view("Military");
            if(mv){
                mv->remove_set("All Weapons");
                mv->add_set(vcs);
                mv->set_is_custom(false);
            }
            mv = this->get_view("Military-Alt.");
            if(mv){
                mv->remove_set("All Weapons");
                mv->add_set(vcs);
                mv->set_is_custom(false);
            }
        }
        m_views << gv;
        built_in_views << gv;        
        //        if (m_add_weapons_tab)
        //        {
        //            add_tab_for_gridview(gv);
        //            m_add_weapons_tab = false;
        //        }
    }
}

void ViewManager::draw_views() {
    // see if we have a saved tab order...
    QTime start = QTime::currentTime();    
    disconnect(tabBar(), SIGNAL(currentChanged(int)), this, SLOT(setCurrentIndex(int)));
    while (count()) {
        QWidget *w = widget(0);
        w->deleteLater();
        removeTab(0);
    }
    connect(tabBar(), SIGNAL(currentChanged(int)), SLOT(setCurrentIndex(int)), Qt::UniqueConnection);

    QStringList tab_order = DT->user_settings()->value("gui_options/tab_order").toStringList();
    QString default_view_name = tr("Labors Full"); //default view to use if none are found/loaded
    if (tab_order.size() == 0) {
        tab_order << default_view_name << "Military" << "Social" << "Attributes" << "Roles" << "Animals";
    }
    GridView *v_default = 0; //keep a reference to the default view
    if (tab_order.size() > 0) {
        foreach(QString name, tab_order) {
            foreach(GridView *v, m_views) {
                if (v->name() == name){
                    add_tab_for_gridview(v);
                }
                if(v->name() == default_view_name)
                    v_default = v;
            }
        }
    }
    if(count() <= 0){ //there must always be at least one view
        if(v_default == 0)
            v_default = m_views.first();
        add_tab_for_gridview(v_default);
    }

    setCurrentWidget(widget(0));
    QTime stop = QTime::currentTime();
    LOGI << QString("redrew views in %L1ms").arg(start.msecsTo(stop));
}

void ViewManager::write_tab_settings() {
    QStringList tab_order;
    QString view_name;
    for (int i = 0; i < count(); ++i) {
        view_name = tabText(i);
        tab_order << view_name;
        DT->user_settings()->setValue(QString("gui_options/%1_group_by").arg(view_name),get_stv(i)->get_last_group_by());
    }
    if(!tab_order.isEmpty())
        DT->user_settings()->setValue("gui_options/tab_order", tab_order);
}

void ViewManager::write_views() {
    QSettings *s = DT->user_settings();
    s->remove("gridviews"); // look at us, taking chances like this!

    // find all custom gridviews...
    QList<GridView*> custom_views;
    foreach(GridView *gv, m_views) {
        if (gv->is_custom())
            custom_views << gv;
    }

    s->beginWriteArray("gridviews", custom_views.size());
    int i = 0;
    foreach(GridView *gv, custom_views) {
        s->setArrayIndex(i++);
        gv->write_to_ini(*s);
    }
    s->endArray();
    write_tab_settings();

    //write the default sorting
    foreach(COLUMN_TYPE cType, m_default_column_sort.uniqueKeys()){
        s->setValue("options/grid/" + get_column_type(cType), ViewColumn::get_sort_type(m_default_column_sort.value(cType)));
    }
}

void ViewManager::add_view(GridView *view) {
    view->re_parent(this);
    m_views << view;
    write_views();
    draw_add_tab_button();
    add_tab_for_gridview(view);
}

GridView *ViewManager::get_view(const QString &name) {
    GridView *retval = 0;
    foreach(GridView *view, m_views) {
        if (name == view->name()) {
            retval = view;
            break;
        }
    }
    return retval;
}

GridView *ViewManager::get_active_view() {
    GridView *retval = 0;
    foreach(GridView *view, m_views) {
        if (view->name() == tabText(currentIndex())) {
            retval = view;
            break;
        }
    }
    return retval;
}

void ViewManager::remove_view(GridView *view) {
    m_views.removeAll(view);
    for (int i = 0; i < count(); ++i) {
        if (tabText(i) == view->name())
            removeTab(i);
    }
    write_views();
    draw_add_tab_button();
    view->deleteLater();
}

void ViewManager::replace_view(GridView *old_view, GridView *new_view) {
    // if the current tab was updated, we need to redraw it
    bool update_current_index = false;
    for (int i = 0; i < count(); ++i) {
        if (tabText(i) == old_view->name()) {
            // update tab titles that were showing the old view
            setTabText(i, new_view->name());
            // going to have to redraw the active tab as its view was just
            // updated
            if (i == currentIndex())
                update_current_index = true;
        }
    }
    m_views.removeAll(old_view);
    m_views.append(new_view);
    write_views();
    draw_add_tab_button();

    if (update_current_index) {
        setCurrentIndex(currentIndex());
    }
}



StateTableView *ViewManager::get_stv(int idx) {
    if (idx == -1)
        idx = currentIndex();
    QWidget *w = widget(idx);
    if (w) {
        return qobject_cast<StateTableView*>(w);
    }
    return 0;
}

void ViewManager::setCurrentIndex(int idx) {
    if (idx < 0 || idx > count()-1) {
        LOGW << "tab switch to index" << idx << "requested but there are " <<
            "only" << count() << "tabs";
        return;
    }    

    StateTableView *stv = get_stv(idx);
    StateTableView *prev_view = get_stv(m_last_index);

    QSettings *s = DT->user_settings();
    bool group_all = s->value("options/grid/group_all_views",true).toBool();
    bool scroll_all = s->value("options/grid/scroll_all_views",false).toBool();
    int default_group = s->value("gui_options/group_by",0).toInt(); //used when groups are synchronized
    if(default_group < 0)
        default_group = 0;

    int sel_group = 0;    
    foreach(GridView *v, m_views) {
        if (v->name() == tabText(idx)) {
            stv->is_loading_rows = true;
            stv->setSortingEnabled(false);

            m_model->set_grid_view(v);
            if(group_all){
                if(prev_view && prev_view->get_last_group_by() > -1){ //use the previous view's group, keep in sync
                    sel_group = prev_view->get_last_group_by();
                    stv->set_last_group_by(prev_view->get_last_group_by());
                }else{ //no prev view use the default
                    sel_group = default_group;
                    stv->set_last_group_by(default_group);
                }
            }else{
                int default_view_group_by = stv->m_default_group_by; //use the default for the specific view
                if(default_view_group_by < 0)
                    default_view_group_by = 0;

                if(stv->get_last_group_by() < 0){
                    sel_group = default_view_group_by;
                    stv->set_last_group_by(default_view_group_by);
                }else{
                    sel_group = stv->get_last_group_by();
                }
            }

            //rows are built here
            m_model->set_group_by(sel_group);

            if(scroll_all){
                if(prev_view)
                    stv->set_scroll_positions(prev_view->vertical_scroll_position(), prev_view->horizontal_scroll_position());
            }

            if(stv->header()->count() > 0){
#if QT_VERSION >= 0x050000
                stv->header()->setSectionResizeMode(QHeaderView::Fixed);
                stv->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
#else
                stv->header()->setResizeMode(QHeaderView::Fixed);
                stv->header()->setResizeMode(0, QHeaderView::ResizeToContents);
#endif
            }

            //setup our sort types by column in case we've since changed them
            foreach(ViewColumnSet *set, m_model->current_grid_view()->sets()){
                foreach(ViewColumn *c, set->columns()){
                    ViewColumn::COLUMN_SORT_TYPE cst = ViewManager::get_default_col_sort(c->type());
                    if(c->get_sortable_types().count() > 0 && c->get_current_sort() != cst)
                        c->refresh_sort(cst);
                }
            }

            //original sorting per view sorting
//            m_proxy->sort(0,m_proxy->m_last_sort_order); //sort by the last sort order
//            stv->sortByColumn(stv->m_last_sorted_col,stv->m_last_sort_order); //individual column sort

            stv->setSortingEnabled(true);

            if(m_reset_sorting){ //on group by changed, sort by the group aggregate rows
                m_proxy->sort(0,DwarfModelProxy::DSR_DEFAULT,Qt::AscendingOrder);
                stv->m_last_sorted_col = 0;
                stv->m_last_sort_order = Qt::AscendingOrder;
                m_model->set_global_group_sort_info(DwarfModelProxy::DSR_DEFAULT,Qt::AscendingOrder); //sort by the sort value, asc
            }else{
                if(prev_view){
                    m_proxy->sort(0, static_cast<DwarfModelProxy::DWARF_SORT_ROLE>(m_model->get_global_group_sort_info().value(stv->get_last_group_by()).first),
                                  m_model->get_global_group_sort_info().value(stv->get_last_group_by()).second); //sort the groups/name column
                    //if there's a second sort on a specific column, apply that sort as well
                    QPair<QString,int> key_pair = m_model->get_global_sort_info().value(stv->get_last_group_by());
                    if(key_pair.second != 0){
                        LOGD << "sorting view" << stv->get_view_name() << "with the global sort for the group";
                        stv->sortByColumn(1, prev_view->m_last_sort_order); //global sort
                    }else{
                        LOGD << "not sorting view" << stv->get_view_name();
                    }
                    stv->m_last_sort_order = prev_view->m_last_sort_order;
                }else{
                    m_proxy->sort(0,DwarfModelProxy::DSR_DEFAULT,Qt::AscendingOrder);
                }
            }

            stv->m_selected_rows.clear(); //will be reloaded below when re-selecting, however after committing, selection is cleared..
            stv->m_selected.clear();
            foreach(Dwarf *d, m_selected_dwarfs) {
                stv->select_dwarf(d);
            }
            stv->is_loading_rows = false;
            stv->is_active = true;
            break;
        }
    }

    if(prev_view && prev_view != stv){
        prev_view->is_active = false;
    }

    //tabBar()->setCurrentIndex(idx);
    stv->restore_expanded_items();
    write_tab_settings();

    m_last_index = idx;
    stv->restore_scroll_positions();

    emit group_changed(stv->get_last_group_by());

    s = 0;
}

void ViewManager::dwarf_selection_changed(const QItemSelection &selected,
                                          const QItemSelection &deselected) {
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
    QItemSelectionModel *selection = qobject_cast<QItemSelectionModel*>
                                     (QObject::sender());    
    m_selected_dwarfs.clear();

    foreach(QModelIndex idx, selection->selectedRows(0)) {
        int dwarf_id = idx.data(DwarfModel::DR_ID).toInt();
        Dwarf *d = DT->get_dwarf_by_id(dwarf_id);
        if (d)
            m_selected_dwarfs << d;
    }

    if (m_selected_dwarfs.count() > 0)
        DT->get_main_window()->set_progress_message(QString::number(m_selected_dwarfs.count()).append(" selected."));
    else
        DT->get_main_window()->set_progress_message("");

    emit selection_changed();
}

int ViewManager::add_tab_from_action() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    if (!a)
        return -1;

    GridView *v = (GridView*)(a->data().toULongLong());
    int idx = add_tab_for_gridview(v);
    return idx;
}

int ViewManager::add_tab_for_gridview(GridView *v) {
    v->set_active(true);
    StateTableView *stv = new StateTableView(this);
    stv->setSortingEnabled(false);
    stv->sortByColumn(0,Qt::AscendingOrder);
    stv->set_model(m_model, m_proxy);
    stv->setSortingEnabled(true);
    stv->set_default_group(v->name());    
    connect(stv, SIGNAL(dwarf_focus_changed(Dwarf*)),
            SIGNAL(dwarf_focus_changed(Dwarf*))); // pass-thru
    connect(stv->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &,
                                    const QItemSelection &)),
            SLOT(dwarf_selection_changed(const QItemSelection &,
                                         const QItemSelection &)));
    connect(stv,SIGNAL(squad_leader_changed()),this,SLOT(show_squad_warning()),Qt::UniqueConnection);
    stv->show();
    m_reset_sorting = true;
    int new_idx = addTab(stv, v->name()); //this calls setCurrentIndex, redrawing and sorting the view
    m_reset_sorting = false;
    setCurrentWidget(stv);
    write_tab_settings();
    return new_idx;
}

void ViewManager::remove_tab_for_gridview(int idx) {
    if (count() < 2) {
        QMessageBox::warning(this, tr("Can't Remove Tab"),
            tr("Cannot remove the last tab!"));
        return;
    }
    foreach(GridView *v, m_views) {
        if (v->name() == tabText(idx)) {
            // find out if there are other dupes of this view still active...
            int active = 0;
            for(int i = 0; i < count(); ++i) {
                if (tabText(i) == v->name()) {
                    active++;
                }
            }
            if (active < 2)
                v->set_active(false);
        }
    }
    widget(idx)->deleteLater();
    removeTab(idx);
    write_tab_settings();
}

void ViewManager::expand_all() {
    StateTableView *stv = get_stv();
    if (stv)
        stv->expandAll();
}

void ViewManager::collapse_all() {
    StateTableView *stv = get_stv();
    if (stv)
        stv->collapseAll();
}

void ViewManager::jump_to_dwarf(QTreeWidgetItem *current,
                                QTreeWidgetItem *previous) {
    StateTableView *stv = get_stv();
    if (stv)
        stv->jump_to_dwarf(current, previous);
}

void ViewManager::jump_to_profession(QTreeWidgetItem *current, QTreeWidgetItem *previous){
    StateTableView *stv = get_stv();
    if (stv)
        stv->jump_to_profession(current, previous);
}

void ViewManager::set_group_by(int group_by) {
    if (m_model){
        get_stv(currentIndex())->set_last_group_by(group_by);
    }
    m_reset_sorting = true;
    setCurrentIndex(currentIndex());
    m_reset_sorting = false;
}

void ViewManager::redraw_current_tab() {
    setCurrentIndex(currentIndex());
}

void ViewManager::redraw_current_tab_headers(){    
    get_stv(currentIndex())->is_loading_rows = true;
    m_model->draw_headers();
    get_stv(currentIndex())->is_loading_rows = false;
}

void ViewManager::redraw_specific_header(int id, COLUMN_TYPE type){
    get_stv(currentIndex())->is_loading_rows = true;
    m_model->update_header_info(id,type);
    get_stv(currentIndex())->is_loading_rows = false;
}


void ViewManager::show_squad_warning(){
    if(m_squad_warning)
        m_squad_warning->showMessage(tr("It's advisable to open the Military screen in Dwarf Fortress after making any changes to squad leaders."));
}

void ViewManager::save_column_sort(COLUMN_TYPE cType, ViewColumn::COLUMN_SORT_TYPE sType){
    m_default_column_sort.insert(cType,sType);
}

void ViewManager::select_all(){
    m_selected_dwarfs.clear();
    StateTableView *s = get_stv(currentIndex());
    s->select_all();
}

void ViewManager::clear_selected(){
    m_selected_dwarfs.clear();
    DT->get_main_window()->set_progress_message("");
}

void ViewManager::reselect(QVector<int> ids){
    foreach(int id, ids){
        get_stv(currentIndex())->select_dwarf(id);
    }
}

void ViewManager::refresh_custom_professions(){
    StateTableView *s = get_stv(currentIndex());
    if(s)
        s->build_custom_profession_menu();
}

void ViewManager::rebuild_global_sort_keys(){
    //on a read of the data, find all the columns that were sorted on, and rebuild the sort keys for each dwarf, for each sort group
    if(m_model->get_global_sort_info().count() > 0){
        QPair<QString,int> key_pair;
        foreach(int group_id, m_model->get_global_sort_info().uniqueKeys()){
            key_pair = m_model->get_global_sort_info().value(group_id);
            //sorting on the name column is handled by the model proxy and persists through reads
            if(key_pair.second <= 0)
                continue;
            //find the view containing the column that was used to sort for this group
            GridView *gv = get_view(key_pair.first);
            if(gv){
                //find the specific column that was sorted on
                ViewColumn *vc = gv->get_column(key_pair.second);
                if(vc){
                    LOGD << "refreshing global sort for group" << group_id << "with keys from gridview" << gv->name() << "column" << vc->title();
                    //update each dwarf's sort key for the group, based on the cell's sort role
                    foreach(Dwarf *d, m_model->get_dwarves()){
                        QStandardItem *item = vc->build_cell(d); //sort role is calculated/built when the cell is built
                        d->set_global_sort_key(group_id, item->data(DwarfModel::DR_SORT_VALUE));
                    }
                }
            }
        }
    }
}
