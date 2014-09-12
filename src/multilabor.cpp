#include <QDialog>
#include "multilabor.h"
#include "labor.h"
#include "dwarf.h"
#include "dwarftherapist.h"

MultiLabor::MultiLabor(QObject *parent)
    :QObject(parent)
    , m_dwarf(0)
    , m_role(0)
    , gdr(GameDataReader::ptr())
    , m_name("")
    , m_role_name("")
    , m_dialog(0)
    , m_selected_count(0)
    , m_internal_change_flag(false)
{
    connect(DT,SIGNAL(roles_changed()),this,SLOT(refresh()),Qt::UniqueConnection); //refresh after any role changes
    connect(DT,SIGNAL(units_refreshed()),this,SLOT(refresh()),Qt::UniqueConnection); //refresh information after a read
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
    read_settings();
}
MultiLabor::~MultiLabor(){
    gdr = 0;
    m_dwarf = 0;
    m_role = 0;
}

/*!
Get a vector of all enabled labors in this template by labor_id
*/
QVector<int> MultiLabor::get_enabled_labors() {
    QVector<int> labors;
    foreach(int labor, m_active_labors.uniqueKeys()) {
        if (m_active_labors.value(labor)) {
            labors << labor;
        }
    }
    return labors;
}

QVector<int> MultiLabor::get_enabled_skilled_labors(){
    QVector<int> labors;
    foreach(int labor, m_active_labors.uniqueKeys()) {
        if (m_active_labors.value(labor) && gdr->get_labor(labor)->skill_id >= 0) {
            labors << labor;
        }
    }
    return labors;
}

/*!
Check if the template has a labor enabled

\param[in] labor_id The id of the labor to check
\returns true if this labor is enabled
*/
bool MultiLabor::is_active(int labor_id) {
    return m_active_labors.value(labor_id, false);
}

void MultiLabor::labor_item_check_changed(QListWidgetItem *item) {
    m_internal_change_flag = true;
    if (item->checkState() == Qt::Checked) {
        item->setBackgroundColor(m_active_labor_col);
        add_labor(item->data(Qt::UserRole).toInt());
        if(!m_internal_change_flag)
            m_selected_count++;
    } else {
        item->setBackground(QBrush());
        remove_labor(item->data(Qt::UserRole).toInt());
        if(!m_internal_change_flag)
            m_selected_count--;
    }
    emit selected_count_changed(m_selected_count);
    m_internal_change_flag = false;
}

void MultiLabor::load_labors(QListWidget *labor_list){
    m_selected_count = 0;
    read_settings();
    QList<Labor*> labors = gdr->get_ordered_labors();
    qSort(labors.begin(),labors.end(),Labor::skilled_compare);
    foreach(Labor *l, labors) {
        QListWidgetItem *item = new QListWidgetItem(l->name, labor_list);
        item->setData(Qt::UserRole, l->labor_id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        if (is_active(l->labor_id)) {
            item->setCheckState(Qt::Checked);
            item->setBackgroundColor(m_active_labor_col);
            m_selected_count++;
        } else {
            item->setCheckState(Qt::Unchecked);
            item->setBackground(QBrush());
        }
        labor_list->addItem(item);
    }
    connect(labor_list,SIGNAL(itemChanged(QListWidgetItem*)),this,SLOT(labor_item_check_changed(QListWidgetItem*)),Qt::UniqueConnection);
    emit selected_count_changed(m_selected_count);
}

void MultiLabor::build_role_combo(QComboBox *cb_roles){
    //add roles
    cb_roles->addItem("None (Role Average)", "");
    QPair<QString,Role*> role_pair;
    foreach(role_pair, gdr->get_ordered_roles()){
        cb_roles->addItem(role_pair.first, role_pair.first);
    }
    int index = cb_roles->findText(m_role_name);
    if(index != -1)
        cb_roles->setCurrentIndex(index);
    else
        cb_roles->setCurrentIndex(0);
}


void MultiLabor::set_labor(int labor_id, bool active) {
    if (m_active_labors.contains(labor_id) && !active)
        m_active_labors.remove(labor_id);
    if (active)
        m_active_labors.insert(labor_id, true);
}

void MultiLabor::set_labors(Dwarf *d){
    if(!d){
        d = m_dwarf;
    }
    m_active_labors.clear();
    QList<Labor*> labors = gdr->get_ordered_labors();
    foreach(Labor *l, labors) {
        if(d->labor_enabled(l->labor_id))
            add_labor(l->labor_id);
    }
}

void MultiLabor::refresh(){
    m_qvariant_labors.clear();
    m_ratings.clear();
    m_labor_desc.clear();

    //check and update the role if necessary
    if(!m_role || m_role != gdr->get_role(m_role_name)){
        m_role = gdr->get_role(m_role_name); //default overridden or new role
    }else{
        //check for a name change and update
        QString curr_id = m_role->name;
        if(curr_id != m_role_name)
            m_role_name = curr_id;
    }

    //build descriptions


    //build ratings
    QList<Dwarf*> dwarves = DT->get_dwarves();
    //at the moment it's not possible to have roles associated to non-skill labors (hauling)
    //so we can exclude all non-skilled labors from the ratings
    QVector<int> labors = get_enabled_labors();
    int skilled_count = 0;
    int skill_id = -1;
    if(labors.count() > 0){
        foreach(int labor_id, labors){
            Labor *l = GameDataReader::ptr()->get_labor(labor_id);
            QString name = l->name;
            skill_id = l->skill_id;
            if(skill_id >= 0)
                skilled_count++;
            m_labor_desc.insert(l->labor_id,name);
            m_qvariant_labors.append(labor_id);
            foreach(Dwarf *d, dwarves){
                QList<float> ratings = m_ratings.take(d->id());
                if(ratings.size() <= 0)
                    ratings << 0 << 0 << 0 << 0;

                if(d->labor_enabled(l->labor_id) && ratings[ML_ACTIVE] <= 0)
                    ratings[ML_ACTIVE] = 1000;

                if(skill_id >= 0){
                    ratings[ML_SKILL] += d->get_skill_level(l->skill_id,false,true); //capped rating, ignore non-skilled labors
                    ratings[ML_SKILL_RATE] += d->get_skill(l->skill_id).skill_rate();
                }

                if(m_role_name.isEmpty()){
                    //find related roles based on the labor's skill
                    if(skill_id >= 0){
                        QVector<Role*> roles = gdr->get_skill_roles().value(l->skill_id);
                        if(roles.size() > 0){
                            ratings[ML_ROLE] += d->get_role_rating(roles.first()->name);
                        }
                    }
                }else if(!m_ratings.contains(d->id())){
                    ratings[ML_ROLE] = d->get_role_rating(m_role_name);
                }
                m_ratings.insert(d->id(),ratings);
            }
        }
        foreach(Dwarf *d, dwarves){
            if(skilled_count > 0){
                m_ratings[d->id()][ML_SKILL] /= skilled_count;
                m_ratings[d->id()][ML_SKILL_RATE] /= skilled_count;
                if(m_role_name == "")
                    m_ratings[d->id()][ML_ROLE] /= skilled_count;
            }else{
                m_ratings[d->id()][ML_SKILL] = -1;
                m_ratings[d->id()][ML_SKILL_RATE] = 0;
            }
//            if(m_active_labors.count() > 0)
//                m_ratings[d->id()][ML_ACTIVE] = (1000.0f * ((float)m_ratings[d->id()][ML_ACTIVE]/(float)m_active_labors.count()));
        }
    }
    read_settings();
}

float MultiLabor::get_rating(int id, ML_RATING_TYPE type){
    if(m_ratings.size() <= 0 && get_enabled_labors().size() > 0)
        refresh();
    if(m_ratings.contains(id)){
        QList<float> ratings = m_ratings.value(id);
        if(int(type) < 0 || int(type) > ratings.count()-1){
            LOGW << "out of" << ratings.count() << "ratings, rating type" << (int)type << "could not be found!";
            if(ratings.count() > 0)
                return ratings.at(0);
            else
                return 0.0;
        }else{
            return ratings.at(type);
        }
    }else{
        return 0.0;
    }
}

void MultiLabor::set_name(QString name){
    m_name = name;
}

/*!
Called when the show_builder_dialog widget's OK button is pressed, or the
dialog is otherwise accepted by the user

We intercept this call to verify the form is valid before saving it.
\sa is_valid()
*/
void MultiLabor::accept() {
    if (!is_valid()) {
        return;
    }
    if(m_dwarf){
        update_dwarf();
        m_dwarf = 0;
    }
    refresh();
    m_dialog->accept();
}

void MultiLabor::cancel(){
    m_dwarf = 0;
    m_dialog->reject();
}

void MultiLabor::read_settings(){
    QSettings *s = DT->user_settings();
    if(s)
        m_active_labor_col = s->value("options/colors/active_labor",QColor(Qt::white)).value<QColor>();
}
