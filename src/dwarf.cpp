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
#include "dwarf.h"
#include "dfinstance.h"
#include "trait.h"
#include "belief.h"
#include "dwarfjob.h"
#include "gamedatareader.h"
#include "customprofession.h"
#include "memorylayout.h"
#include "dwarftherapist.h"
#include "mainwindow.h"
#include "profession.h"
#include "dwarfstats.h"
#include "races.h"
#include "reaction.h"
#include "histfigure.h"
#include "fortressentity.h"
#include "columntypes.h"
#include "plant.h"
#include "bodypart.h"
#include "unitemotion.h"
#include "unitneed.h"
#include "adaptivecolorfactory.h"

#include "labor.h"
#include "preference.h"
#include "rolepreference.h"
#include "material.h"
#include "caste.h"

#include "squad.h"
#include "uniform.h"
#include "itemuniform.h"
#include "itemweapon.h"
#include "itemarmor.h"
#include "itemammo.h"
#include "iteminstrument.h"

#include <QAction>
#include <QClipboard>
#include <QDateTime>
#include <QDialog>
#include <QMessageBox>
#include <QTextEdit>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QVector>
#include <QJSEngine>

Dwarf::Dwarf(DFInstance *df, VIRTADDR addr, QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_address(addr)
    , m_first_soul(0)
    , m_race_id(-1)
    , m_happiness(DH_FINE)
    , m_happiness_desc("")
    , m_stress_level(0)
    , m_mood_id(MT_NONE)
    , m_had_mood(false)
    , m_artifact_name("")
    , m_curse_name("")
    , m_caste_id(-1)
    , m_show_full_name(false)
    , m_total_xp(0)
    , m_body_size(60000)
    , m_animal_type(none)
    , m_raw_prof_id(-1)
    , m_raw_profession(0)
    , m_can_set_labors(false)
    , m_locked_mood(false)
    , m_stressed_mood(false)
    , m_current_job_id(-1)
    , m_hist_figure(0x0)
    , m_squad_id(-1)
    , m_squad_position(-1)
    , m_pending_squad_name()
    , m_age(0)
    , m_noble_position("")
    , m_is_pet(false)
    , m_race(0)
    , m_caste(0)
    , m_pref_tooltip()
    , m_emotions_desc()
    , m_is_animal(false)
    , m_true_name("")
    , m_true_birth_year(0)
    , m_is_valid(false)
    , m_uniform(0x0)
    , m_goals_realized(0)
    , m_worst_rust_level(0)
    , m_labor_reason("")
    , m_histfig_id(0)
    , m_occ_type(OCC_NONE)
    , m_can_assign_military(true)
    , m_active_military(false)
    , m_is_citizen(true)
    , m_current_focus(0)
    , m_undistracted_focus(0)
    , m_current_focus_degree(FOCUS_UNTROUBLED)
    , m_curse_type(eCurse::NONE)
{
    read_settings();
    read_data();
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));

    // setup context actions
#ifdef QT_DEBUG
    m_actions_memory.clear();
    QAction *dump_mem = new QAction(tr("Dump Memory..."), this);
    connect(dump_mem, SIGNAL(triggered()), SLOT(dump_memory()));
    m_actions_memory << dump_mem;
    QAction *dump_mem_to_file = new QAction(tr("Dump Memory To File"), this);
    connect(dump_mem_to_file, SIGNAL(triggered()), SLOT(dump_memory_to_file()));
    m_actions_memory << dump_mem_to_file;
    QAction *copy_address_to_clipboard = new QAction(tr("Copy Address to Clipboard"), this);
    connect(copy_address_to_clipboard, SIGNAL(triggered()),SLOT(copy_address_to_clipboard()));
    m_actions_memory << copy_address_to_clipboard;
#endif
}


Dwarf::~Dwarf() {
    qDeleteAll(m_actions_memory);
    m_actions_memory.clear();

    m_traits.clear();
    m_beliefs.clear();
    m_conflicting_beliefs.clear();

    m_skills.clear();
    m_sorted_skills.clear();
    m_moodable_skills.clear();
    m_attributes.clear();

    m_labors.clear();
    m_pending_labors.clear();
    m_role_ratings.clear();
    m_raw_role_ratings.clear();
    m_sorted_role_ratings.clear();
    m_sorted_custom_role_ratings.clear();
    m_states.clear();

    qDeleteAll(m_grouped_preferences);
    m_grouped_preferences.clear();

    qDeleteAll(m_emotions);
    m_emotions.clear();

    m_thoughts.clear();
    m_syndromes.clear();
    m_inventory_grouped.clear();

    m_hist_figure = 0;
    m_uniform = 0;
    m_race = 0;
    m_caste = 0;
    m_raw_profession = 0;
    m_mem = 0;
    m_df = 0;

    disconnect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

bool Dwarf::has_invalid_flags(QHash<uint, QString> invalid_flags, quint32 dwarf_flags){
    foreach(uint invalid_flag, invalid_flags.uniqueKeys()) {
        QString reason = invalid_flags[invalid_flag];
        if(dwarf_flags & invalid_flag) {
            set_validation(QString("%1 flags:%2").arg(reason).arg(dwarf_flags));
            return true;
        }
    }
    return false;
}

void Dwarf::read_settings() {
    /* This is pretty fucked up. There is no good way to let the options for
       full name, or show dabbling to just update existing cells. Will have to
       send signals when this stuff changes, or just bite the bullet and
       subclass the QStandardItem for the name items in the main model
       */
    QSettings *s = DT->user_settings();
    bool new_show_full_name = s->value("options/show_full_dwarf_names",false).toBool();
    if (new_show_full_name != m_show_full_name) {
        build_names();
        emit name_changed();
    }
    m_show_full_name = new_show_full_name;
}

void Dwarf::read_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "refresh unit called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of unit data at" << hexify(m_address);

    int civ_id = m_df->read_int(m_mem->dwarf_field(m_address, "civ"));
    TRACE << "  CIV:" << civ_id;

    //read the core information we need to validate if we should continue loading this unit
    read_id();
    read_flags();
    read_race(); //also sets m_is_animal
    read_first_name();
    read_last_name(m_mem->dwarf_field(m_address, "name"));
    read_nick_name();
    build_names(); //build names now for logging
    read_states();  //read states before job and validation
    read_caste(); //read before age
    read_turn_count(); //load time/date stuff for births/migrations - read before age
    set_age_and_migration(m_mem->dwarf_field(m_address, "birth_year"), m_mem->dwarf_field(m_address, "birth_time")); //set age before profession, after caste

    m_raw_prof_id = m_df->read_byte(m_mem->dwarf_field(m_address, "profession"));
    m_raw_profession = GameDataReader::ptr()->get_profession(m_raw_prof_id);
    m_histfig_id = m_df->read_int(m_mem->dwarf_field(m_address, "hist_id"));

    bool validated = true;
    //attempt to do some initial filtering on the civilization
    if(civ_id != m_df->dwarf_civ_id()){
        set_validation("doesn't belong to our civilization",&validated,false,LL_DEBUG);
    }else if(!m_is_animal){
        //filter out babies/children based on settings
        if(DT->hide_non_adults() && !is_adult()){
            set_validation("IGNORING child/baby",&validated,false,LL_DEBUG);
        }
        //filter out any non-mercenary visitors if necessary
        if (m_df->fortress()->address())
            m_is_citizen = m_df->fortress()->hist_figures().contains(m_histfig_id);
        TRACE << "HIST_FIG_ID:" << m_histfig_id;
        if(DT->hide_non_citizens() && !m_is_citizen && m_occ_type != OCC_MERC){
            set_validation("IGNORING visitor/guest",&validated,false,LL_DEBUG);
        }
    }

    //check caged untame but trainable beasts
    if(get_flag_value(FLAG_CAGED)){
        if(!get_flag_value(FLAG_TAME)){
            //if it's a caged, trainable beast, keep it in our list
            if(m_race){
                LOGI << QString("FOUND caged creature!");
                validated = m_race->caste_flag(TRAINABLE);
            }
        }
    }

    //if the initial validation has passed and the soul is valid, perform a more in depth check with the flags
    if(validated && read_soul()){
        this->validate();
    }

    if(m_is_valid){
        m_hist_figure = new HistFigure(m_histfig_id,m_df,this);
        // use fake identity to match DF
        find_fake_ident();
        read_squad_info(); //read squad before job
        read_gender_orientation(); //read before profession
        read_profession(); //read profession before building the names, and before job
        read_mood(); //read after profession and before job, emotions/skills (soul aspect)
        read_labors(); //read after profession and mood
        read_uniform(); //read after labors (hunters, miners, wood cutters)
        check_availability(); //after labors/profession/age
        read_current_job();
        read_syndromes(); //read syndromes before attributes
        read_body_size(); //body size after caste and age
        //curse check will change the name and age
        read_curse(); //read curse before attributes
        read_soul_aspects(); //assumes soul already read, and requires caste to be read first
        read_animal_type(); //need skills loaded to check for hostiles
        read_noble_position();
        read_preferences();

        m_unit_health = UnitHealth(m_df,this,!DT->user_settings()->value("options/diagnosis_not_required", false).toBool());
        read_inventory();

        if(m_is_animal || m_nice_name == "")
            build_names(); //calculate names again as we need to check tameness for animals
    }

    if(m_is_valid){
        LOGI << QString("FOUND %1 (%2) name:%3 id:%4 histfig_id:%5")
                .arg(race_name()).arg(hexify(m_address))
                .arg(m_nice_name).arg(m_id).arg(m_histfig_id);
    }
}

void Dwarf::refresh_minimal_data(){
    if(m_is_valid){
        read_flags(); //butcher/caged

        read_nick_name();
        read_labors();

        read_profession();

        build_names();
    }
}

bool Dwarf::validate(){
    if (m_mem->is_complete()) {

        //check for migrants (which aren't dead/killed/ghosts)
        if(this->state_value(FLAG_MIGRANT) > 0
                && (!get_flag_value(FLAG_DEAD) || get_flag_value(FLAG_INCOMING))
                && !get_flag_value(FLAG_KILLED)
                && !get_flag_value(FLAG_GHOST)){
            set_validation("migrant",&m_is_valid,true);
            return true;
        }

        //check opposed to life
        if(m_curse_flags & eCurse::OPPOSED_TO_LIFE){
            set_validation("appears to be a hostile undead!",&m_is_valid);
            return false;
        }

        if(m_is_animal && (get_flag_value(FLAG_TAME) || get_flag_value(FLAG_CAGED))){ //tame or caged animals
            //exclude cursed animals, this may be unnecessary with the civ check
            //the full curse information hasn't been loaded yet, so just read the curse name
            QString curse_name = m_df->read_string(m_mem->dwarf_field(m_address, "curse"));
            if(!curse_name.isEmpty()){
                set_validation("appears to be cursed or undead",&m_is_valid);
                return false;
            }
        }

        //check other invalid flags (invaders, ghosts, dead, merchants, etc.)
        if(has_invalid_flags(m_mem->invalid_flags_1(),m_unit_flags.at(0)) ||
                has_invalid_flags(m_mem->invalid_flags_2(),m_unit_flags.at(1)) ||
                has_invalid_flags(m_mem->invalid_flags_3(),m_unit_flags.at(2))){
            m_is_valid = false;
            return false;
        }

        if(!m_first_soul){
            set_validation("appears to be soulless",&m_is_valid);
            return false;
        }
        m_is_valid = true;
        return true;
    }else{
        m_is_valid = false;
        return false;
    }
}

void Dwarf::set_validation(QString reason, bool *valid_var, bool valid, LOG_LEVEL l){
    QString msg = QString("%1 %2 name:%3 id:%4 reason:%5")
            .arg(valid ? tr("FOUND") : tr("IGNORING"))
            .arg(race_name(false)).arg(m_nice_name).arg(m_id).arg(reason);

    if(l == LL_DEBUG){
        LOGD << msg;
    }else{
        LOGI << msg;
    }
    if(valid_var)
        *valid_var = valid;
}

void Dwarf::set_age_and_migration(VIRTADDR birth_year_offset, VIRTADDR birth_time_offset){
    m_birth_date = std::tuple<df_year, df_tick>{ // explicit constructor because of GCC 5 bug :(
        df_year(m_df->read_int(birth_year_offset)),
        df_tick(m_df->read_int(birth_time_offset))
    };
    m_age = m_df->current_time() - df_date_convert<df_time>(m_birth_date);
    m_arrival_time = m_df->current_time() - df_time(m_turn_count);
}

QString Dwarf::get_migration_desc(){
    if (born_in_fortress()) {
        auto date = df_date<df_year, df_month, df_day>::make_date(m_arrival_time);
        return QString("Born on the %1%4 of %2 in the year %3")
            .arg(std::get<df_day>(date).count())
            .arg(DFMonths[std::get<df_month>(date).count()])
            .arg(std::get<df_year>(date).count())
            .arg(day_suffix(std::get<df_day>(date).count()));
    }
    else {
        auto date = df_date<df_year, df_season>::make_date(m_arrival_time);
        return QString("Arrived in the %2 of the year %1")
            .arg(std::get<df_year>(date).count())
            .arg(DFSeasons[std::get<df_season>(date).count()]);
    }
}

/*******************************************************************************
  DATA POPULATION METHODS
*******************************************************************************/

void Dwarf::read_id() {
    m_id = m_df->read_int(m_mem->dwarf_field(m_address, "id"));
    TRACE << "UNIT ID:" << m_id;
}

static const char *sex_interest_icon_suffix (Dwarf::SEX_COMMITMENT interest)
{
    switch (interest) {
    case Dwarf::COMMIT_LOVER:
        return "i";
    case Dwarf::COMMIT_MARRIAGE:
        return "c";
    default:
        LOGE << "Invalid interest in suffix";
        return "";
    }
}

void Dwarf::read_gender_orientation() {
    QSettings *s = DT->user_settings();
    auto gender_info_option = s->value("options/gender_info", Option_ShowOrientation).toInt();
    bool show_orientation = gender_info_option >= Option_ShowOrientation;
    //bool show_commitment = !m_is_animal && gender_info_option >= Option_ShowCommitment;
    bool show_commitment = false; // hide commitment until it is better understood

    BYTE sex = m_df->read_byte(m_mem->dwarf_field(m_address, "sex"));
    TRACE << "GENDER:" << sex;
    m_gender_info.gender = static_cast<GENDER_TYPE>(sex);
    m_gender_info.orientation = ORIENT_HETERO; //default

    QStringList icon_name;

    if(m_gender_info.gender == SEX_UNK){
        icon_name.append("sex-unknown");
    }else if(m_gender_info.gender == SEX_M){
        icon_name.append("male");
    }else{
        icon_name.append("female");
    }

    int orient_offset = m_mem->soul_detail("orientation");
    if(m_gender_info.gender != SEX_UNK && m_first_soul && orient_offset != -1){
        quint32 orientation = m_df->read_addr(m_first_soul + orient_offset);
        m_gender_info.male = static_cast<SEX_COMMITMENT>((orientation & (3<<1))>>1);
        m_gender_info.female = static_cast<SEX_COMMITMENT>((orientation & (3<<3))>>3);

        SEX_COMMITMENT other, same;
        if (m_gender_info.gender == SEX_M) {
            other = m_gender_info.female;
            same = m_gender_info.male;
        }
        else {
            other = m_gender_info.male;
            same = m_gender_info.female;
        }


        if (other && same) {
            if (show_orientation)
                icon_name.append("bi");
            if (show_commitment) {
                icon_name.append(sex_interest_icon_suffix(m_gender_info.male));
                icon_name.append(sex_interest_icon_suffix(m_gender_info.female));
            }
            m_gender_info.orientation = ORIENT_BISEXUAL;
        }
        else if (other) {
            if (show_orientation)
                icon_name.append("hetero");
            if (show_commitment)
                icon_name.append(sex_interest_icon_suffix(other));
            m_gender_info.orientation = ORIENT_HETERO;
        }
        else if (same) {
            if (show_orientation)
                icon_name.append("homo");
            if (show_commitment)
                icon_name.append(sex_interest_icon_suffix(same));
            m_gender_info.orientation = ORIENT_HOMO;
        }
        else {
            if (show_orientation)
                icon_name.append("asexual");
            if (show_commitment)
                icon_name.append("bg");
            m_gender_info.orientation = ORIENT_ASEXUAL;
        }
    }
    icon_name.removeAll("");
    m_icn_gender = QString(":img/%1.png").arg(icon_name.join("-"));

    QStringList gender_desc;
    gender_desc << get_gender_desc(m_gender_info.gender);
    if (show_orientation)
        gender_desc << get_orientation_desc(m_gender_info.orientation);
    if (show_commitment) {
        static const QStringList preferences = { tr("Not interested in %1"), tr("Likes %1"), tr("Will marry %1") };
        for (const auto &t: {
                std::make_tuple(m_gender_info.male, tr("males")),
                std::make_tuple(m_gender_info.female, tr("females")) }) {
            auto pref = std::get<0>(t);
            if (pref) {
                if (pref < preferences.count())
                    gender_desc << preferences[std::get<0>(t)].arg(std::get<1>(t));
                else
                    gender_desc << "Invalid orientation flag";
            }
        }
    }
    gender_desc.removeAll("");
    m_gender_info.full_desc = gender_desc.join(" - ");
}

void Dwarf::read_mood(){
    m_mood_id = static_cast<MOOD_TYPE>(m_df->read_short(m_mem->dwarf_field(m_address, "mood")));
    int temp_offset = m_mem->dwarf_offset("temp_mood");
    if(m_mood_id == MT_NONE && temp_offset != -1){
        short temp_mood = m_df->read_short(m_address + temp_offset); //check temporary moods
        if(temp_mood > -1)
            m_mood_id = static_cast<MOOD_TYPE>(10 + temp_mood); //appended to craft/stress moods enum
    }

    //baby are currently ignored
    if(m_mood_id == MT_BABY){
        m_mood_id = MT_NONE;
    }

    //check if they've had a mood/artifact if they're not currently in a craft-type mood
    if(m_mood_id == MT_NONE || (int)m_mood_id > 4){
        if(get_flag_value(FLAG_HAD_MOOD)){
            m_had_mood = true;
            m_artifact_name = m_df->get_translated_word(m_mem->dwarf_field(m_address, "artifact_name"));
        }
        //filter out any other temporary combat moods, and set stressed mood flag
        if(m_mood_id != MT_NONE && m_mood_id != MT_MARTIAL && m_mood_id != MT_ENRAGED){
            m_stressed_mood = true;
        }
    }
    //additionally if it's a mood that disables labors (trying to make an artifact or insane), set the flag
    if(m_mood_id != MT_NONE &&
            (
                (m_mood_id == MT_BERSERK || m_mood_id == MT_INSANE || m_mood_id == MT_MELANCHOLY || m_mood_id == MT_TRAUMA) ||
                (int)m_mood_id <= 4)
            ){
        m_locked_mood = true;
    }
}

void Dwarf::read_body_size(){
    m_body_size = m_df->read_int(m_mem->dwarf_field(m_address, "size_info"));
    m_body_size_base = m_df->read_int(m_mem->dwarf_field(m_address, "size_base"));
}

void Dwarf::read_animal_type(){
    if(m_is_animal){
        qint32 animal_offset = m_mem->dwarf_offset("animal_type");
        if(animal_offset>=0)
            m_animal_type = static_cast<TRAINED_LEVEL>(m_df->read_int(m_address + animal_offset));

        //additionally if it's an animal set a flag if it's currently a pet
        //since butchering available pets simply by setting the flag breaks shit in game
        qint32 owner_offset = m_mem->dwarf_offset("pet_owner_id");
        if(owner_offset >=0){
            int pet_owner_id = m_df->read_int(m_address + owner_offset); //check for an owner
            m_is_pet = (pet_owner_id > 0);
        }else{
            m_is_pet = (!m_first_name.isEmpty() && !m_last_name.isEmpty()); //assume that a first and last name on an animal is a pet
        }
    }
}

void Dwarf::read_states(){
    //set of misc. traits and a value (cave adapt, migrant, likes outdoors, etc..)
    m_states.clear();
    uint states_offset = m_mem->dwarf_offset("states");
    if(states_offset) {
        VIRTADDR states_addr = m_address + states_offset;
        QVector<VIRTADDR> entries = m_df->enumerate_vector(states_addr);
        foreach(VIRTADDR entry, entries) {
            m_states.insert(m_df->read_short(entry), m_df->read_int(entry+0x4));
        }
    }
}

void Dwarf::read_curse(){
    QString curse_name = capitalizeEach(m_df->read_string(m_mem->dwarf_field(m_address, "curse")));

    if(!curse_name.isEmpty()){
        m_curse_type = eCurse::OTHER;

        //keep track of the curse type; currently vampires and werebeasts are the only truly cursed creatures we
        //want to be aware of for the purpose of highlighting them

        //TODO: check for removed flags as well
        if(m_curse_flags & eCurse::BLOODSUCKER){ //if(!has_flag(eCurse::BLOODSUCKER, m_curse_rem_flags) && has_flag(eCurse::BLOODSUCKER,m_curse_flags)){
            m_curse_type = eCurse::VAMPIRE;
        }

        m_curse_name = curse_name;
    }
}

void Dwarf::find_fake_ident(){
    if(m_hist_figure->has_fake_identity()){
        //save the true name for display
        m_true_name = m_nice_name;
        m_true_birth_year = get_birth_year();
        //overwrite the default name, birth year with the assumed identity
        m_first_name = m_hist_figure->fake_name();
        m_nick_name = m_hist_figure->fake_nick_name();
        read_last_name(m_hist_figure->fake_name_offset());
        build_names();
        //vamps also use a fake age
        set_age_and_migration(m_hist_figure->fake_birth_year_offset(),m_hist_figure->fake_birth_time_offset());
    }
}

int Dwarf::historical_id(){
    return m_histfig_id;
}

HistFigure* Dwarf::hist_figure(){
    return m_hist_figure;
}

void Dwarf::read_caste() {
    m_caste_id = m_df->read_short(m_mem->dwarf_field(m_address, "caste"));
    m_caste = m_race->get_caste_by_id(m_caste_id);
    TRACE << "CASTE:" << m_caste_id;
}

void Dwarf::read_flags(){
    m_unit_flags.clear();
    quint32 flags1 = m_df->read_addr(m_mem->dwarf_field(m_address, "flags1"));
    TRACE << "  FLAGS1:" << hexify(flags1);
    quint32 flags2 = m_df->read_addr(m_mem->dwarf_field(m_address, "flags2"));
    TRACE << "  FLAGS2:" << hexify(flags2);
    quint32 flags3 = m_df->read_addr(m_mem->dwarf_field(m_address, "flags3"));
    TRACE << "  FLAGS3:" << hexify(flags3);
    m_unit_flags << flags1 << flags2 << flags3;
    m_pending_flags = m_unit_flags;

    m_curse_flags = m_df->read_addr(m_mem->dwarf_field(m_address, "curse_add_flags1"));
    //    m_curse_flags2 = m_df->read_addr(m_mem->dwarf_field(m_address, "curse_add_flags2"));
}

void Dwarf::read_race() {
    m_race_id = m_df->read_int(m_mem->dwarf_field(m_address, "race"));
    m_race = m_df->get_race(m_race_id);
    TRACE << "RACE ID:" << m_race_id;
    if(m_race){
        m_is_animal = (!m_race->flags().has_flag(CAN_LEARN) && m_race->caste_flag(TRAINABLE));
    }
}

void Dwarf::read_first_name() {
    m_first_name = m_df->read_string(m_mem->word_field(m_mem->dwarf_field(m_address, "name"), "first_name"));
    if (m_first_name.size() > 1)
        m_first_name[0] = m_first_name[0].toUpper();
    TRACE << "FIRSTNAME:" << m_first_name;
}

void Dwarf::read_last_name(VIRTADDR name_offset) {
    //Generic
    bool use_generic = DT->user_settings()->value("options/use_generic_names", false).toBool();

    m_translated_last_name = m_df->get_translated_word(name_offset);
    if (use_generic)
        m_last_name = m_translated_last_name;
    else
        m_last_name = m_df->get_language_word(name_offset);
}


void Dwarf::read_nick_name() {
    m_nick_name = m_df->read_string(m_mem->word_field(m_mem->dwarf_field(m_address, "name"), "nickname"));
    TRACE << "\tNICKNAME:" << m_nick_name;
    m_pending_nick_name = m_nick_name;
}

void Dwarf::build_names() {
    if (m_pending_nick_name.isEmpty()) {
        m_nice_name = QString("%1 %2").arg(m_first_name, m_last_name);
        m_translated_name = QString("%1 %2").arg(m_first_name, m_translated_last_name);
    } else {
        if (m_show_full_name) {
            m_nice_name = QString("%1 '%2' %3").arg(m_first_name, m_pending_nick_name, m_last_name);
            m_translated_name = QString("%1 '%2' %3").arg(m_first_name, m_pending_nick_name, m_translated_last_name);
        } else {
            m_nice_name = QString("'%1' %2").arg(m_pending_nick_name, m_last_name);
            m_translated_name = QString("'%1' %2").arg(m_pending_nick_name, m_translated_last_name);
        }
    }
    m_nice_name = m_nice_name.trimmed();
    m_translated_name = m_translated_name.trimmed();

    QString creature_name = "";
    if(is_adult()){
        creature_name = caste_name();
    }else{
        creature_name = race_name(false);
    }
    if(m_is_animal){
        if(is_adult()){
            //for adult animals, check their profession for training and adjust the name accordingly
            if (m_raw_prof_id == 99) //trained war
                creature_name = tr("War ") + m_race->name();
            else if (m_raw_prof_id == 98) //trained hunt
                creature_name = tr("Hunting ") + m_race->name();
        }
        //append pet name if necessary
        if(!m_nice_name.isEmpty()){
            m_nice_name = tr("%1 (%2)").arg(creature_name).arg(m_nice_name);
        }
    }

    //if there's still no name, use the creature name
    if(m_nice_name==""){
        m_nice_name = capitalizeEach(creature_name);
        m_translated_name = "";
    }
}

void Dwarf::read_profession() {
    // first see if there is a custom prof set...
    VIRTADDR custom_addr = m_mem->dwarf_field(m_address, "custom_profession");
    m_custom_prof_name = m_df->read_string(custom_addr);
    TRACE << "\tCUSTOM PROF:" << m_custom_prof_name;

    // we set both to the same to know it hasn't been edited yet
    m_pending_custom_profession = m_custom_prof_name;

    QString prof_name = tr("Unknown Profession %1").arg(m_raw_prof_id);
    if(m_raw_profession){
        prof_name = m_raw_profession->name(is_male());
    }
    if (!m_custom_prof_name.isEmpty()) {
        m_prof_name =  m_custom_prof_name;
    } else {
        m_prof_name = prof_name;
    }

    if(is_animal()){
        switch (m_raw_prof_id) {
        case 103: // child
            m_prof_name = tr("Child");
            break;
        case 104: // baby
            m_prof_name = tr("Baby");
            break;
        default:
            m_prof_name = tr("Adult"); //adult animals have a profession of peasant by default, just use adult
        }
    }

    int img_idx = 102; //default to peasant
    if(m_raw_prof_id > -1 && m_raw_prof_id < GameDataReader::ptr()->get_professions().count())
        img_idx = m_raw_prof_id + 1; //images start at 1, professions at 0, offest to match image

    //set the default path for the profession icon
    m_icn_prof = QPixmap(":/profession/prof_" + QString::number(img_idx) + ".png");
    //see if we have a custom profession or icon override
    CustomProfession *cp;
    if(!m_custom_prof_name.isEmpty()){
        cp = DT->get_custom_profession(m_custom_prof_name);
    }else{
        cp = DT->get_custom_prof_icon(m_raw_prof_id);
    }
    if(cp && cp->has_icon())
        m_icn_prof = cp->get_pixmap();

    LOGD << "reading profession for" << nice_name() << m_raw_prof_id << prof_name;
    TRACE << "EFFECTIVE PROFESSION:" << m_prof_name;
}

void Dwarf::read_noble_position(){
    m_noble_position = m_df->fortress()->get_noble_positions(m_histfig_id,is_male());
}

void Dwarf::read_preferences(){
    if(m_is_animal)
        return;
    QVector<VIRTADDR> preferences = m_df->enumerate_vector(m_mem->soul_field(m_first_soul, "preferences"));

    foreach(VIRTADDR pref, preferences){
        auto pref_type = static_cast<PREF_TYPES>(m_df->read_short(pref));
        //0x2 unk
        auto pref_id = m_df->read_short(pref + 0x4);
        //0x6 unk
        auto item_sub_type = m_df->read_short(pref + 0x8);
        auto mat_type = m_df->read_int(pref + 0xc);
        auto mat_index = m_df->read_int(pref + 0x10);
        auto mat_state = static_cast<MATERIAL_STATES>(m_df->read_short(pref + 0x14));

        auto i_type = static_cast<ITEM_TYPE>(pref_id);

        std::unique_ptr<Preference> p;

        //for each preference type, we have some flags we need to check and add so we get matches to the role's preferences
        //materials are the exception as all flags are passed in, moving forward it may be better to pass in flagarrays instead
        switch(pref_type){
        case LIKE_MATERIAL:
            if (Material *m = m_df->find_material(mat_index,mat_type))
                p = std::make_unique<MaterialPreference>(m, mat_state);
            else {
                LOGE << "Material for preference not found" << mat_type << mat_index;
            }
            break;
        case LIKE_CREATURE:
            if (Race* r = m_df->get_race(pref_id))
                p = std::make_unique<CreaturePreference>(r);
            else {
                LOGE << "Creature for preference not found" << pref_id;
            }
            break;
        case LIKE_FOOD:
        {
            QString pref_name = tr("Unknown");
            if(mat_index < 0 || i_type==MEAT){
                int creature_id = mat_index;
                if(i_type==FISH)
                    creature_id = mat_type;

                if (Race* r = m_df->get_race(creature_id))
                    pref_name = r->name().toLower();
                else {
                    LOGE << "Creature for food preference not found" << creature_id;
                }
            }
            else
                pref_name = m_df->find_material_name(mat_index,mat_type,i_type,mat_state);
            p = std::make_unique<Preference>(LIKE_FOOD, pref_name);
            //TODO: add FoodPreference type
            //p->set_item_type(i_type);
            break;
        }
        case HATE_CREATURE:
            if (Race* r = m_df->get_race(pref_id))
                p = std::make_unique<CreatureDislike>(r);
            else {
                LOGE << "Creature for hate preference not found" << pref_id;
            }
            break;
        case LIKE_ITEM:
            if(item_sub_type >= 0 && Item::has_subtypes(i_type)) {
                if (ItemSubtype *s = m_df->get_item_subtype(i_type,item_sub_type))
                    p = std::make_unique<ItemPreference>(s);
                else {
                    LOGE << "Item subtype for preference not found" << i_type << item_sub_type;
                }
            }
            else
                p = std::make_unique<ItemPreference>(i_type, m_df->get_preference_item_name(pref_id,item_sub_type).toLower());
            break;
        case LIKE_PLANT:
            if (Plant *plnt = m_df->get_plant(pref_id))
                p = std::make_unique<PlantPreference>(plnt);
            else {
                LOGE << "Plant for preference not found" << pref_id;
            }
            break;
        case LIKE_TREE:
            if (Plant *plnt = m_df->get_plant(pref_id))
                p = std::make_unique<TreePreference>(plnt);
            else {
                LOGE << "Tree for preference not found" << pref_id;
            }
            break;
        case LIKE_COLOR:
        case LIKE_SHAPE:
        case LIKE_POETRY:
        case LIKE_MUSIC:
        case LIKE_DANCE:
            p = std::make_unique<Preference>(pref_type, m_df->get_preference_other_name(pref_id, pref_type));
            break;
        default:
            LOGE << "Unknown preference type" << pref_type;
        }
        if (!p)
            p = std::make_unique<Preference>(pref_type, tr("Unknown"));
        m_pref_names.append(p->get_name());
        m_preferences.emplace(pref_type, std::move(p));
    }

    bool build_tooltip = (!m_is_animal && !m_preferences.empty() && DT->user_settings()->value("options/tooltip_show_preferences",true).toBool());
    //group preferences into pref desc - values (string list)
    QString desc_key;
    //lists for the tooltip
    QStringList likes;
    QStringList consume;
    QStringList hates;
    QStringList other;
    for (const auto &p: m_preferences) {
        auto pType = static_cast<PREF_TYPES>(p.first);
        desc_key = Preference::get_pref_desc(pType);
        auto pref_name = p.second->get_name();
        auto pref_desc = p.second->get_description();
        if(!pref_name.isEmpty()){
            //create/append to groups based on the categories description
            if(!m_grouped_preferences.contains(desc_key))
                m_grouped_preferences.insert(desc_key, new QStringList);
            m_grouped_preferences.value(desc_key)->append(pref_name);
            //build the tooltip at the same time, organizing by likes, dislikes
            if(build_tooltip){
                switch (pType) {
                case LIKE_ITEM:
                case LIKE_MATERIAL:
                case LIKE_PLANT:
                case LIKE_TREE:
                case LIKE_CREATURE:
                case LIKE_COLOR:
                case LIKE_SHAPE:
                case LIKE_POETRY:
                case LIKE_MUSIC:
                case LIKE_DANCE:
                    likes.append(pref_desc);
                    break;
                case LIKE_FOOD:
                    consume.append(pref_desc);
                    break;
                case HATE_CREATURE:
                    hates.append(pref_desc);
                    break;
                default:
                    other.append(capitalize(pref_desc));
                }
            }
        }
    }
    if(build_tooltip){
        m_pref_tooltip.clear();
        if(likes.count()>0)
            m_pref_tooltip.append(tr("Likes ")).append(formatList(likes)).append(". ");
        if(consume.count()>0)
            m_pref_tooltip.append(tr("Prefers to consume ")).append(formatList(consume)).append(". ");
        if(hates.count()>0)
            m_pref_tooltip.append(tr("Hates ")).append(formatList(hates)).append(". ");
        if(other.count()>0)
            m_pref_tooltip.append(formatList(other)).append(". ");
        m_pref_tooltip = m_pref_tooltip.trimmed();
    }
}

void Dwarf::read_syndromes(){
    m_syndromes.clear();
    QVector<VIRTADDR> active_unit_syns = m_df->enumerate_vector(m_mem->dwarf_field(m_address, "active_syndrome_vector"));
    //when showing syndromes, be sure to exclude 'vampcurse' and 'werecurse' if we're hiding cursed dwarves
    bool show_cursed = DT->user_settings()->value("options/highlight_cursed",false).toBool();
    bool is_curse = false;
    foreach(VIRTADDR syn, active_unit_syns){
        Syndrome s = Syndrome(m_df,syn);
        if(s.display_name().contains("vampcurse",Qt::CaseInsensitive))
            is_curse = true;
        if(s.has_transformation()){
            int race_id = s.get_transform_race();
            if(race_id >= 0){
                Race *r_trans = m_df->get_race(race_id);
                if(r_trans){
                    Caste *c_trans = r_trans->get_caste_by_id(0);
                    if(r_trans && r_trans->flags().has_flag(NIGHT_CREATURE) && c_trans && c_trans->flags().has_flag(CRAZED)){
                        is_curse = true;
                        m_curse_type = eCurse::WEREBEAST;
                        m_curse_name = r_trans->name();
                    }
                }
            }else if(s.display_name().contains("werecurse",Qt::CaseInsensitive)){
                //older version without the necessary offset, use a generic name if it seems like a werecurse
                is_curse = true;
                m_curse_type = eCurse::WEREBEAST;
                m_curse_name = tr("Werebeast");
            }
        }

        if(!show_cursed && is_curse)
            continue;

        m_syndromes.append(s);
        QHash<ATTRIBUTES_TYPE,Syndrome::syn_att_change> syn_changes = s.get_attribute_changes();
        if(syn_changes.count() > 0){
            Syndrome::syn_att_change att_change_detail;
            foreach(ATTRIBUTES_TYPE a_type,syn_changes.keys()){
                att_change_detail = s.get_attribute_changes().value(a_type);
                QPair<float,int> totals;
                if(att_change_detail.is_permanent){
                    if(!m_attribute_mods_perm.contains(a_type))
                        totals = qMakePair(1.0f,0);
                    else
                        totals = m_attribute_mods_perm.value(a_type);
                }else{
                    if(!m_attribute_mods_temp.contains(a_type))
                        totals = qMakePair(1.0f,0);
                    else
                        totals = m_attribute_mods_temp.value(a_type);
                }

                //add valid percentage changes
                if(att_change_detail.percent != 100 && att_change_detail.percent != 0)
                    totals.first *= ((float)att_change_detail.percent/100.0f);
                //add valid flat increases
                if(att_change_detail.added != 0){
                    totals.second += att_change_detail.added;
                }
                if((totals.first != 1 && totals.first != 0) || totals.second != 0){
                    if(att_change_detail.is_permanent)
                        m_attribute_mods_perm.insert(a_type,totals);
                    else
                        m_attribute_mods_temp.insert(a_type,totals);

                    QStringList syns = m_attribute_syndromes.take(a_type);
                    if(!syns.contains(s.display_name(true,false)))
                        syns.append(s.display_name(true,false));
                    m_attribute_syndromes.insert(a_type,syns);
                }
            }
        }
    }
    //    std::sort(m_syndromes.begin(),m_syndromes.end(),&Syndrome::sort_date_time);
}

bool Dwarf::is_buffed(){
    foreach(Syndrome s, m_syndromes){
        if(!s.is_sickness())
            return true;
    }
    return false;
}

QString Dwarf::buffs(){
    return get_syndrome_names(true,false);
}

bool Dwarf::has_syndrome(QString name){
    QRegExp rx = QRegExp(".*" + name + ".*",Qt::CaseInsensitive);
    foreach(Syndrome s, m_syndromes){
        if(s.name().contains(rx) || s.class_names().join(" ").contains(rx))
            return true;
    }
    return false;
}

QString Dwarf::syndromes(){
    return get_syndrome_names(true,true);
}

QString Dwarf::get_syndrome_names(bool include_buffs, bool include_sick) {
    short d_type = DT->user_settings()->value("options/syndrome_display_type",0).toInt();
    bool show_name = (d_type == 0 || d_type ==2);
    bool show_class = (d_type >= 1);
    QStringList names;
    foreach(Syndrome s, m_syndromes){
        if((include_buffs && !s.is_sickness()) || (include_sick && s.is_sickness())){
            QString syn = s.display_name(show_name,show_class);
            //            if(!s.syn_effects().isEmpty())
            //                syn.append(tr("(%1)").arg(s.syn_effects()));
            names.append(syn);
        }
    }
    return names.join(", ");
}


void Dwarf::read_labors() {
    VIRTADDR addr = m_mem->dwarf_field(m_address, "labors");
    // read a big array of labors in one read, then pick and choose
    // the values we care about
    QByteArray buf(94, 0);
    m_df->read_raw(addr, 94, buf);

    // get the list of identified labors from game_data.ini
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        bool enabled = buf.at(l->labor_id) > 0;
        m_labors[l->labor_id] = enabled;
        m_pending_labors[l->labor_id] = enabled;
    }
}

void Dwarf::check_availability(){
    if(m_is_animal){
        m_can_set_labors = false;
        m_can_assign_military = false;
        m_labor_reason = tr("for livestock.");
        return;
    }

    //set occupation
    VIRTADDR occ_addr = m_df->find_occupation(m_histfig_id);
    if(occ_addr != 0){
        m_occ_type = static_cast<UNIT_OCCUPATION>(m_df->read_int(occ_addr + 0x4));
    }

    //check that labors can be toggled
    if(m_locked_mood){
        m_can_set_labors = false;
        m_labor_reason = tr("due to mood (%1)").arg(GameDataReader::ptr()->get_mood_name(m_mood_id,true));
    }else if(!m_is_citizen){
        m_can_set_labors = false;
        m_labor_reason = tr("for non-citizens.");
    }else{
        //TODO: rather than checking the profession, check the fortress entity's permitted jobs
        //however this would also disable other labours like alchemy not used in DF
        if(m_raw_profession){
            m_can_set_labors = m_raw_profession->can_assign_labors();
            if(!is_baby() && DT->labor_cheats_allowed()){
                m_can_set_labors = true;
            }
            if(!m_can_set_labors){
                if(!is_adult()){
                    m_labor_reason = tr("for children and babies.");
                }else{
                    m_labor_reason = tr("for this profession.");
                }
            }
        }else{
            LOGE << tr("Read unknown profession with id '%1' for dwarf '%2'").arg(m_raw_prof_id).arg(m_nice_name);
            m_can_set_labors = false;
            m_labor_reason = tr("due to unknown profession");
        }
    }

    //check squad assignment
    if (m_raw_profession && (m_is_citizen || m_occ_type == OCC_MERC) && !m_locked_mood)
        m_can_assign_military = (is_adult() && m_raw_profession->can_assign_military()) || DT->labor_cheats_allowed();
    else
        m_can_assign_military = false;
}

void Dwarf::read_current_job(){
    VIRTADDR addr = m_mem->dwarf_field(m_address, "current_job");
    VIRTADDR current_job_addr = m_df->read_addr(addr);
    m_current_sub_job_id.clear();

    TRACE << "Current job addr: " << hex << current_job_addr;
    if(current_job_addr != 0){
        m_current_job_id = m_df->read_short(m_mem->job_field(current_job_addr, "id"));

        //if drinking blood and we're not showing vamps, change job to drink
        if(m_current_job_id == (int)DwarfJob::JOB_DRINK_BLOOD && DT->user_settings()->value("options/highlight_cursed", false).toBool()==false){
            m_current_job_id = 17; //DRINK
        }

        DwarfJob *job = GameDataReader::ptr()->get_job(m_current_job_id);
        if(job){
            m_current_job = job->name();
            if(job->id() >= 55 && job->id() <= 67){ //strange moods
                m_current_job = job->name(GameDataReader::ptr()->get_mood_name(m_mood_id));
            }

            int sub_job_offset = m_mem->job_detail("sub_job_id");
            if(sub_job_offset != -1){
                m_current_sub_job_id = m_df->read_string(current_job_addr + sub_job_offset); //reaction name
                if(!job->reactionClass().isEmpty() && !m_current_sub_job_id.isEmpty()) {
                    Reaction* reaction = m_df->get_reaction(m_current_sub_job_id);
                    if(reaction!=0) {
                        m_current_job = capitalize(reaction->name());
                        TRACE << "Sub job: " << m_current_sub_job_id << m_current_job;
                    }
                }
            }

            if(m_current_sub_job_id.isEmpty() && job->has_placeholder()){
                QString material_name;
                short mat_type = m_df->read_short(m_mem->job_field(current_job_addr, "mat_type"));
                int mat_index = m_df->read_int(m_mem->job_field(current_job_addr, "mat_index"));
                if(mat_index >= 0 || mat_type >= 0){
                    material_name = m_df->find_material_name(mat_index ,mat_type, NONE);
                }
                if(material_name.isEmpty()){
                    quint32 mat_category = m_df->read_addr(m_mem->job_field(current_job_addr, "mat_category"));
                    material_name = DwarfJob::get_job_mat_category_name(mat_category);
                }
                if(!material_name.isEmpty()){
                    m_current_job = job->name(capitalize(material_name));
                }
            }

        }else{
            m_current_job = tr("Unknown job");
        }

    }else{
        //default to no job/break
        m_current_job_id = DwarfJob::JOB_IDLE;

        BYTE meeting = 0;
        int offset = m_mem->dwarf_offset("meeting");
        if(offset != -1){
            meeting = m_df->read_byte(m_address + offset);
        }
        if(meeting == 2){ //needs more work; !=2 for conduct meeting
            m_current_job_id = DwarfJob::JOB_MEETING;
        }else{
            //it appears that other activities have priority over squad orders, so process them first
            QPair<int,QString> activity_desc = m_df->find_activity(m_histfig_id);
            if(activity_desc.first != DwarfJob::JOB_UNKNOWN){
                m_current_job_id = activity_desc.first;
                m_current_job = capitalizeEach(activity_desc.second);
            }else{
                if(m_active_military && m_squad_id >= 0){
                    Squad *s = m_df->get_squad(m_squad_id);
                    if(s){
                        activity_desc = s->get_order(m_histfig_id);
                        if(activity_desc.first != DwarfJob::JOB_UNKNOWN){
                            m_current_job_id = activity_desc.first;
                            m_current_job = capitalizeEach(activity_desc.second);
                        }else{
                            //there's no order, but the unit has a squad and is on duty
                            m_current_job_id = DwarfJob::JOB_SOLDIER;
                        }
                    }
                }
            }
        }
        if(m_current_job.isEmpty()){
            m_current_job = capitalizeEach(GameDataReader::ptr()->get_job(m_current_job_id)->name());
        }
    }

    TRACE << "CURRENT JOB:" << m_current_job_id << m_current_sub_job_id << m_current_job;
}

bool Dwarf::read_soul(){
    VIRTADDR soul_vector = m_mem->dwarf_field(m_address, "souls");
    QVector<VIRTADDR> souls = m_df->enumerate_vector(soul_vector);
    if (souls.size() != 1) {
        LOGI << nice_name() << "has" << souls.size() << "souls!";
        return false;
    }
    m_first_soul = souls.at(0);
    return true;
}

void Dwarf::read_soul_aspects() {
    if(!m_first_soul){
        if(!read_soul()){
            return;
        }
    }

    read_skills();
    read_attributes();
    read_personality();

    TRACE << "SKILLS:" << m_skills.size();
    TRACE << "TRAITS:" << m_traits.size();
    TRACE << "ATTRIBUTES:" << m_attributes.size();
}


QString Dwarf::profession() {
    if (!m_pending_custom_profession.isEmpty())
        return m_pending_custom_profession;
    if (!m_custom_prof_name.isEmpty())
        return m_custom_prof_name;
    return m_prof_name;
}

QString Dwarf::caste_name(bool plural_name) {
    QString tmp_name = "Unknown";
    if(m_caste){
        if(plural_name)
            tmp_name = m_caste->name_plural();
        else
            tmp_name = m_caste->name();
    }else if(m_race){
        tmp_name = m_race->name();
    }
    return tmp_name;
}

QString Dwarf::caste_tag() {
    QString tmp_name = "Unknown";
    if(m_caste){
        tmp_name = m_caste->tag();
    }
    return tmp_name;
}

QString Dwarf::caste_desc() {
    if(m_caste){
        QString tmp_desc = m_caste->description();
        if(tmp_desc.size()>0)
            tmp_desc[0]=tmp_desc[0].toUpper();
        return tmp_desc;
    }else{
        return "";
    }
}

QString Dwarf::race_name(bool base, bool plural_name) {
    QString r_name = "";
    if(!m_race)
        return r_name;

    if(!plural_name)
        r_name = m_race->name();
    else
        r_name = m_race->plural_name();

    if(base)
        return r_name;

    if(is_baby()){
        if(!plural_name)
            r_name = m_race->baby_name();
        else
            r_name = m_race->baby_name_plural();
    }else if (is_child()){
        if(!plural_name)
            r_name = m_race->child_name();
        else
            r_name = m_race->child_name_plural();
    }
    return capitalizeEach(r_name);
}

QString Dwarf::happiness_name(DWARF_HAPPINESS happiness) {
    switch(happiness) {
    case DH_MISERABLE: return tr("Miserable");
    case DH_VERY_UNHAPPY: return tr("Very Unhappy");
    case DH_UNHAPPY: return tr("Unhappy");
    case DH_FINE: return tr("Fine");
    case DH_CONTENT: return tr("Quite Content");
    case DH_HAPPY: return tr("Happy");
    case DH_ECSTATIC: return tr("Ecstatic");
    default: return "UNKNOWN";
    }
}

QString Dwarf::get_goal_summary() const {
    GameDataReader *gdr = GameDataReader::ptr();
    if (m_goals.size() == 1) {
        auto goal = m_goals.begin();
        return gdr->get_goal_desc(goal.key(), goal.value() != 0);
    }
    else if (m_goals.size() > 1) {
        QString goal_summary;
        bool separator = false;
        if (m_goals_realized < m_goals.size()) {
            for (auto goal = m_goals.begin(); goal != m_goals.end(); ++goal) {
                if (goal.value() == 0) {
                    if (separator) {
                        goal_summary.append(tr(". "));
                    }
                    else {
                        separator = true;
                    }
                    goal_summary.append(gdr->get_goal_desc(goal.key(), false));
                }
            }
        }
        if (m_goals_realized > 0) {
            if (separator) {
                goal_summary.append(tr(". "));
            }
            else {
                separator = true;
            }
            goal_summary.append(tr("One or more dreams realized"));
        }
        return goal_summary;
    }
    else {
        return gdr->get_goal_desc(0, false);
    }
}

int Dwarf::get_need_type_focus(int need_id) const
{
    auto r = m_needs.equal_range(need_id);
    auto it = std::min_element(r.first, r.second, [] (const auto &a, const auto &b) {
        return a.second->focus_level() < b.second->focus_level();
    });
    return it == r.second ? 0 : it->second->focus_level();
}

int Dwarf::get_need_type_level(int need_id) const
{
    auto r = m_needs.equal_range(need_id);
    int sum = 0;
    for (auto it = r.first; it != r.second; ++it) {
        sum += it->second->need_level();
    }
    return sum;
}

int Dwarf::get_need_focus(int need_id, int deity_id) const
{
    auto r = m_needs.equal_range(need_id);
    for (auto it = r.first; it != r.second; ++it) {
        if (it->second->deity_id() == deity_id)
            return it->second->focus_level();
    }
    return 0;
}

int Dwarf::get_need_level(int need_id, int deity_id) const
{
    auto r = m_needs.equal_range(need_id);
    for (auto it = r.first; it != r.second; ++it) {
        if (it->second->deity_id() == deity_id)
            return it->second->need_level();
    }
    return 0;
}

int Dwarf::get_need_focus_degree(int need_id, int deity_id) const
{
    auto r = m_needs.equal_range(need_id);
    for (auto it = r.first; it != r.second; ++it) {
        if (it->second->deity_id() == deity_id)
            return it->second->focus_degree();
    }
    return UnitNeed::NOT_DISTRACTED;
}

QString Dwarf::get_focus_adjective(int degree) {
    static const char * const adjectives[FOCUS_DEGREE_COUNT] = {
        QT_TR_NOOP("badly distracted"),
        QT_TR_NOOP("distracted"),
        QT_TR_NOOP("unfocused"),
        QT_TR_NOOP("untroubled"),
        QT_TR_NOOP("somewhat focused"),
        QT_TR_NOOP("quite focused"),
        QT_TR_NOOP("very focused"),
    };
    if (degree < 0 || degree >= FOCUS_DEGREE_COUNT)
        return QString();
    return tr(adjectives[degree]);
}

QString Dwarf::get_focus_adjective() const {
    return get_focus_adjective(m_current_focus_degree);
}

QColor Dwarf::get_focus_color(int degree, bool tooltip, bool background)
{
    QPalette::ColorRole fg, bg;
    if (tooltip) {
        fg = QPalette::ToolTipText;
        bg = QPalette::ToolTipBase;
    }
    else {
        fg = QPalette::WindowText;
        bg = QPalette::Window;
    }
    if (background)
        std::swap(fg, bg);
    AdaptiveColorFactory color(fg, bg);
    switch (degree) {
    case FOCUS_BADLY_DISTRACTED:
        return color.color(Qt::red);
    case FOCUS_DISTRACTED:
        return color.color(QColor::fromHsv(30, 255, 255)); // orange instead of yellow
    case FOCUS_UNFOCUSED:
        return color.color(QColor::fromHsv(60, 255, 160)); // instead of dark yellow (brown)
    case FOCUS_UNTROUBLED:
        return color.gray(0.75);
    case FOCUS_SOMEWHAT_FOCUSED:
        return color.gray(1.00);
    case FOCUS_QUITE_FOCUSED:
        return color.color(QColor::fromHsv(90, 255, 255)); // yellowish green instead of dark green
    case FOCUS_VERY_FOCUSED:
        return color.color(QColor::fromHsv(150, 255, 255)); // blueish green instead of green
    default:
        return QColor();
    }
}

QString Dwarf::get_focus_desc(bool color) const
{
    if (m_is_animal)
        return QString();

    QString desc = tr("Overall, %1 is %2 %3.")
        .arg(m_first_name)
        .arg(get_focus_adjective())
        .arg(m_current_focus_degree < FOCUS_SOMEWHAT_FOCUSED
                ? tr("by unmet needs")
                : tr("with satisfied needs"));
    if (color) {
        desc.prepend(QString("<span style=\"color: %1\">")
                .arg(get_focus_color(m_current_focus_degree, true).name()));
        desc.append("</span>");
    }
    for (const auto &p: m_needs) {
        desc.append(" ");
        if (color)
            desc.append(QString("<span style=\"color: %1\">")
                    .arg(UnitNeed::degree_color(p.second->focus_degree(), true).name()));
        desc.append(p.second->description());
        if (color)
            desc.append("</span>");
    }
    return desc;
}

bool Dwarf::get_flag_value(int bit_pos)
{
    int idx = bit_pos / 32;
    quint32 flg = m_pending_flags[idx];
    quint32 mask = build_flag_mask(bit_pos);
    return ((flg & mask)==mask);
}

bool Dwarf::toggle_flag_bit(int bit_pos) {
    if (bit_pos==FLAG_CAGED) //ignore caged
        return false;

    //currently flags are only for animals
    if(m_animal_type >= unknown_trained){
        return false;
    }

    bool set_flag = true;
    if(bit_pos==FLAG_BUTCHER){
        //don't butcher if it's a pet, user will be notified via tooltip on column, same for non-butcherable
        set_flag = (!m_is_pet && m_caste->flags().has_flag(BUTCHERABLE));
    }else if(bit_pos==FLAG_GELD){
        set_flag = (m_gender_info.gender == SEX_M && m_caste->is_geldable() && !has_health_issue(eHealth::HI_GELDED,0));
    }

    if(set_flag){
        int idx = bit_pos / 32;
        quint32 mask = build_flag_mask(bit_pos);
        m_pending_flags[idx] ^= mask;
        return true;
    }else{
        return false;
    }
}

quint32 Dwarf::build_flag_mask(int bit){
    bit %= 32;
    return (1 << bit);
}

void Dwarf::read_squad_info() {
    m_squad_id = m_df->read_int(m_mem->dwarf_field(m_address, "squad_id"));
    m_pending_squad_id = m_squad_id;
    m_squad_position = m_df->read_int(m_mem->dwarf_field(m_address, "squad_position"));
    m_pending_squad_position = m_squad_position;
    if(m_pending_squad_id >= 0 && !m_is_animal && is_adult()){
        Squad *s = m_df->get_squad(m_pending_squad_id);
        if(s){
            m_pending_squad_name = s->name();
            m_active_military = s->on_duty(m_histfig_id);
        }
    }
}

void Dwarf::read_uniform(){
    if(!m_is_animal && is_adult()){
        if(m_pending_squad_id >= 0){
            Squad *s = m_df->get_squad(m_pending_squad_id);
            if(s){
                //military uniform
                m_uniform = s->get_uniform(m_pending_squad_position);
            }
        }else{
            //build a work uniform
            m_uniform = new Uniform(m_df,this);
            if(labor_enabled(0)){//mining
                m_uniform->add_uniform_item(WEAPON,-1,0); //mining skill
            }else if(labor_enabled(10)){//woodcutting
                m_uniform->add_uniform_item(WEAPON,-1,37); //axe skill
            }else if(labor_enabled(44)){//hunter
                //add a weapon of crossbow/bow/blowgun skills
                QList<int> skills = QList<int>() << 43 << 51 << 52;
                m_uniform->add_uniform_item(WEAPON,-1,skills);
                //add quiver
                m_uniform->add_uniform_item(QUIVER,-1,-1);
                //add ammo
                m_uniform->add_uniform_item(AMMO,-1,-1);
            }
            if(!m_uniform->has_items()){
                delete m_uniform;
                m_uniform = 0;
            }
        }
    }
}

void Dwarf::read_inventory(){
    LOGD << "reading inventory for" << m_nice_name;
    m_coverage_ratings.clear();
    m_max_inventory_wear.clear();
    m_equip_warnings.clear();
    m_inventory_grouped.clear();

    bool has_shirt = false;
    int shoes_count = 0;
    bool has_pants = false;

    QVector<VIRTADDR> used_items = m_df->enumerate_vector(m_mem->dwarf_field(m_address, "used_items_vector"));
    QHash<int,int> item_affection;
    foreach(VIRTADDR item_used, used_items){
        item_affection.insert(m_df->read_int(item_used),m_df->read_int(m_mem->dwarf_field(item_used, "affection_level")));
    }

    short inv_type = -1;
    short bp_id = -1;
    QString category_name = "";
    int inv_count = 0;
    bool include_mat_name = DT->user_settings()->value("options/docks/equipoverview_include_mats",false).toBool();
    foreach(VIRTADDR inventory_item_addr, m_df->enumerate_vector(m_mem->dwarf_field(m_address, "inventory"))){
        inv_type = m_df->read_short(m_mem->dwarf_field(inventory_item_addr, "inventory_item_mode"));
        bp_id = m_df->read_short(m_mem->dwarf_field(inventory_item_addr, "inventory_item_bodypart"));

        if(inv_type == 1 || inv_type == 2 || inv_type == 4 || inv_type == 8 || inv_type == 10){
            if(bp_id >= 0)
                category_name = m_caste->get_body_part(bp_id)->name();
            else
                category_name = Item::missing_group_name();

            VIRTADDR item_ptr = m_df->read_addr(inventory_item_addr);
            Item *i = new Item(m_df,item_ptr,this);
            ITEM_TYPE i_type = i->item_type();

            int affection_level = item_affection.value(i->id());
            if(affection_level > 0)
                i->set_affection(affection_level);

            if(i_type == WEAPON){
                ItemWeapon *iw = new ItemWeapon(*i);
                process_inv_item(category_name,iw);
                LOGV << "  + found weapon:" << iw->display_name(false);
            }else if(i_type == INSTRUMENT){
                ItemInstrument *ii = new ItemInstrument(*i);
                process_inv_item(category_name,ii);
                LOGV << "  + found instrument:" << ii->display_name(false);
            }else if(Item::is_armor_type(i_type,true)){
                ItemArmor *ir = new ItemArmor(*i);
                process_inv_item(category_name,ir);

                if(i_type == PANTS && !has_pants)
                    has_pants = true;
                else if(i_type == ARMOR && !has_shirt)
                    has_shirt = true;
                else if(i_type == SHOES)
                    shoes_count++;

                int wear_level = ir->wear();
                if(wear_level > m_max_inventory_wear.value(i_type)){
                    m_max_inventory_wear.insert(i_type,wear_level);
                }
                if(wear_level > 0 && Item::is_armor_type(i_type)){
                    add_inv_warn(ir,include_mat_name,static_cast<Item::ITEM_STATE>(wear_level));
                }

                LOGV << "  + found armor/clothing:" << ir->display_name(false);
            }else if(Item::is_supplies(i_type) || Item::is_ranged_equipment(i_type)){
                process_inv_item(category_name,i);
                LOGV << "  + found supplies/ammo/quiver:" << i->display_name(false);
            }else{
                LOGV << "  + found other item:" << i->display_name(false);
            }

            //process any items inside this item (ammo, food?, drink?)
            if(i->contained_items().count() > 0){
                foreach(Item *contained_item, i->contained_items()){
                    process_inv_item(category_name,contained_item,true);
                    LOGV << "    + contained item(s):" << contained_item->display_name(false);
                }
            }
            inv_count++;
        }else{
            LOGV << "  - skipping inventory item due to invalid type (" + QString::number(inv_type) + ")";
        }
    }
    LOGD << "  total inventory items found:" << inv_count;

    //missing uniform items
    if(m_uniform && m_uniform->get_missing_items().count() > 0){
        QString cat_name = Item::missing_group_name();
        foreach(ITEM_TYPE itype, m_uniform->get_missing_items().uniqueKeys()){
            foreach(ItemDefUniform *u, m_uniform->get_missing_items().value(itype)){
                Item *i = new ItemUniform(m_df,u,this);
                if(itype == WEAPON){
                    ItemWeapon *iw = new ItemWeapon(*i);
                    process_inv_item(cat_name,iw);
                }else if(itype == AMMO){ //check before ranged equipment to cast it properly
                    ItemAmmo *ia = new ItemAmmo(*i);
                    process_inv_item(cat_name,ia);
                }else if(Item::is_armor_type(itype,true)){
                    ItemArmor *ir = new ItemArmor(*i);
                    process_inv_item(cat_name,ir);
                }else if(Item::is_supplies(itype) || Item::is_ranged_equipment(itype)){
                    process_inv_item(cat_name,i);
                }
                add_inv_warn(i,include_mat_name,Item::IS_MISSING);
            }
        }
    }

    //ensure babies and animals have 100 coverage rating as they don't wear anything
    if(is_baby() || m_is_animal){
        has_pants = true;
        has_shirt = true;
        shoes_count = 2;
    }

    //set our coverage ratings. currently this is only important for the 3 clothing types that, if missing, give bad thoughts
    process_uncovered(PANTS,tr("Legs Uncovered!"),(has_pants ? 1 : 0),1);
    process_uncovered(ARMOR,tr("Torso Uncovered!"),(has_shirt ? 1 : 0),1);
    //for shoes, compare how many they're wearing compared to how many legs they've still got
    process_uncovered(SHOES,tr("Feet Uncovered!"),shoes_count,m_unit_health.limb_count());
}

void Dwarf::process_uncovered(ITEM_TYPE i_type, QString desc, int count, int req_count){
    if(req_count == 0 || count >= req_count){
        m_coverage_ratings.insert(i_type,100.0f);
    }else{
        m_coverage_ratings.insert(i_type,count / req_count * 100.0f);
        m_missing_counts.insert(i_type,1);
        Item *i = new Item(i_type,desc,this);
        i->add_to_stack(req_count - count);
        process_inv_item(Item::uncovered_group_name(),i);
        add_inv_warn(i,false,Item::IS_UNCOVERED);
    }
}

void Dwarf::add_inv_warn(Item *i, bool include_mat_name, Item::ITEM_STATE i_status){
    //show generic material names for specific items (backpacks, flasks, assigned) or missing uniform items
    QString item_name = i->item_name(true,include_mat_name,(i_status != Item::IS_MISSING || i->id() > 0 ? true : false));
    EquipWarn::warn_info wi;
    wi.key = qMakePair(item_name,i_status);
    wi.count = i->get_stack_size();
    wi.iType = i->item_type();
    m_equip_warnings.append(wi);
}

void Dwarf::process_inv_item(QString category, Item *item, bool is_contained_item){
    if(m_uniform)
        m_uniform->check_uniform(category,item);

    //don't process items contained in other items
    if(is_contained_item)
        return;

    //add the item to the display list by body part
    QList<Item*> items;
    if(m_inventory_grouped.contains(category))
        items = m_inventory_grouped.take(category);
    items.append(item);
    if(items.length() > 0){
        m_inventory_grouped.insert(category,items);
    }
}

//returns 1-3, higher being more worn out items
int Dwarf::get_max_wear_level(ITEM_TYPE itype){
    if(itype == NONE){
        int worst_wear=0;
        foreach(int rating, m_max_inventory_wear.values()){
            if(rating > worst_wear){
                worst_wear = rating;
                if(worst_wear >= 3)
                    break;
            }
        }
        return worst_wear;
    }else{
        return m_max_inventory_wear.value(itype);
    }
    return 0;
}

int Dwarf::get_missing_equip_count(ITEM_TYPE itype){
    int count = 0;
    if(m_uniform)
        count = m_uniform->get_missing_equip_count(itype);
    else
        count = m_missing_counts.value(itype);
    return count;
}

float Dwarf::get_uniform_rating(ITEM_TYPE itype){
    if(m_uniform)
        return m_uniform->get_uniform_rating(itype);
    else
        return 100;
}

float Dwarf::get_coverage_rating(ITEM_TYPE itype){
    float rating = 100.0f;
    if(itype == NONE){
        rating = 0;
        foreach(int val, m_coverage_ratings.values()){
            rating += val;
        }
        rating /= (float)m_coverage_ratings.count();
    }else{
        if(m_coverage_ratings.contains(itype))
            rating = m_coverage_ratings.value(itype);
    }
    if(rating > 100)
        rating = 100;
    return rating;
}


void Dwarf::read_turn_count() {
    m_turn_count = m_df->read_int(m_mem->dwarf_field(m_address, "turn_count"));
    TRACE << "Turn Count:" << m_turn_count;
}

void Dwarf::read_skills() {
    VIRTADDR addr = m_mem->soul_field(m_first_soul, "skills");
    m_total_xp = 0;
    m_skills.clear();
    m_sorted_skills.clear();
    m_moodable_skills.clear();

    QVector<VIRTADDR> entries = m_df->enumerate_vector(addr);
    TRACE << "Reading skills for" << nice_name() << "found:" << entries.size();
    short skill_id = 0;
    short rating = 0;
    int xp = 0;
    int rust = 0;

    int skill_rate = 100;
    if(m_caste)
        m_caste->load_skill_rates();

    QMultiMap<int,Skill> skills_by_level;

    foreach(VIRTADDR entry, entries) {
        skill_id = m_df->read_short(entry);
        rating = m_df->read_short(entry + 0x04);
        xp = m_df->read_int(entry + 0x08);
        rust = m_df->read_int(entry + 0x10);

        //find the caste's skill rate
        if(m_caste){
            skill_rate = m_caste->get_skill_rate(skill_id);
        }

        Skill s = Skill(skill_id, xp, rating, rust, skill_rate);
        //calculate the values we'll need for roles immediately
        if(!m_is_animal && !is_baby()){
            s.calculate_balanced_level();
        }

        m_total_xp += s.actual_exp();
        m_skills.insert(s.id(),s);
        m_sorted_skills.insertMulti(s.capped_level_precise(), s.id());
        if(!m_had_mood && GameDataReader::ptr()->moodable_skills().contains(skill_id)){
            skills_by_level.insertMulti(s.raw_level(),s);
        }

        if(s.rust_level() > m_worst_rust_level)
            m_worst_rust_level = s.rust_level();
    }

    if(!m_had_mood){
        if(skills_by_level.count() > 0){
            foreach(Skill s, skills_by_level.values(skills_by_level.keys().last())){
                m_moodable_skills.insert(s.id(),s);
            }
        }
    }else{
        int mood_skill = m_df->read_short(m_mem->dwarf_field(m_address, "mood_skill"));
        m_moodable_skills.insert(mood_skill, get_skill(mood_skill));
    }
}

void Dwarf::read_emotions(VIRTADDR personality_base){
    QString pronoun = (m_gender_info.gender == SEX_M ? tr("he") : tr("she"));
    //read list of circumstances and emotions, group and build desc
    int offset = m_mem->soul_detail("emotions");
    if(offset != -1){
        QVector<VIRTADDR> emotions_addrs = m_df->enumerate_vector(personality_base + offset);
        //load emotions and sort by descending date
        std::vector<std::unique_ptr<UnitEmotion>> all_emotions;
        all_emotions.reserve(emotions_addrs.count());
        foreach(VIRTADDR addr, emotions_addrs){
            auto ue = std::make_unique<UnitEmotion>(addr, m_df, this);
            if(ue->get_thought_id() >= 0)
                all_emotions.push_back(std::move(ue));
        }
        std::sort(all_emotions.begin(), all_emotions.end(),
                [](const auto &a, const auto &b) { return a->get_time().count() > b->get_time().count(); });
        //keep the most recent emotion, and discard duplicates, but maintain a count of occurrances
        int thought_id;
        bool duplicate;
        for (auto &ue: all_emotions) {
            thought_id = ue->get_thought_id();
            //keep a list of all thoughts as well for filtering
            if(!m_thoughts.contains(thought_id))
                m_thoughts.append(thought_id);

            //remove any duplicate emotional circumstances
            duplicate = false;
            foreach(UnitEmotion *valid, m_emotions){
                if(valid->equals(*ue)){
                    valid->increment_count();
                    duplicate = true;
                    break;
                }
            }
            if(!duplicate){
                m_emotions.append(ue.release());
            }
        }
        all_emotions.clear();

        QStringList weekly_emotions;
        QStringList seasonal_emotions;
        int stress_vuln = m_traits.value(8); //vulnerability to stress

        int max_weeks = DT->user_settings()->value("options/tooltip_thought_weeks",-1).toInt();

        df_time last_week_tick = m_df->current_time() - df_week(abs(max_weeks));

        foreach(UnitEmotion *ue, m_emotions){
            int stress_effect = ue->set_effect(stress_vuln);
            QChar sign;
            if(stress_effect < 0){
                sign = QLocale().positiveSign(); //stress down, happiness up
            }else if(stress_effect > 0){
                sign = QLocale().negativeSign();
            }
            QString desc = QString("%1%2%3")
                    .arg(ue->get_desc().toLower())
                    .arg(!sign.isNull() ? QString("(%1%2)").arg(sign).arg(abs(stress_effect)) : "")
                    .arg(ue->get_count() > 1 ? QString(" (x%1)").arg(ue->get_count()) : "");

            if(ue->get_time() >= last_week_tick){
                weekly_emotions.append(desc);
            }else if(max_weeks == -1){
                seasonal_emotions.append(desc); //only show last season message if showing all
            }

        }
        QStringList dated_emotions;
        if(weekly_emotions.size() > 0){
            QString duration_plural = (max_weeks == 4 ? tr("month") : tr("weeks"));
            dated_emotions.append(tr("Within the last%1%2 %3 felt ")
                                  .arg(max_weeks > 1 && max_weeks < 4 ? QString(" %1 ").arg(max_weeks)  : " ")
                                  .arg(max_weeks > 1 ? duration_plural : tr("week"))
                                  .arg(pronoun).append(formatList(weekly_emotions)));
        }
        if(seasonal_emotions.size() > 0)
            dated_emotions.append(tr("Within the last season %1 felt ").arg(pronoun).append(formatList(seasonal_emotions)));
        m_emotions_desc =  dated_emotions.join(".<br/><br/>");
    }

    //read stress and convert to happiness level
    offset = m_mem->soul_detail("stress_level");
    if(offset != -1){
        m_stress_level = m_df->read_int(personality_base+offset);
    }else{
        m_stress_level = 0;
    }

    auto gdr = GameDataReader::ptr();
    int i = 0;
    while (i < DH_TOTAL_LEVELS-1 &&
           m_stress_level < gdr->get_happiness_threshold(static_cast<DWARF_HAPPINESS>(i)))
         ++i;
    m_happiness = static_cast<DWARF_HAPPINESS>(i);
    QString stress_desc = gdr->get_happiness_desc(m_happiness);
    //check for catatonic, it changes the stress desc
    if(m_mood_id == MT_TRAUMA){
        stress_desc = tr("has been overthrown by the stresses of day-to-day living");
    }
    if(!stress_desc.trimmed().isEmpty()){
        m_emotions_desc.prepend(QString("<b>%1 %2.</b> ")
                .arg(capitalize(pronoun))
                .arg(stress_desc));
    }

    m_happiness_desc = QString("<b>%1</b> (Stress: %2)")
            .arg(happiness_name(m_happiness))
            .arg(formatNumber(m_stress_level,DT->format_SI()));

    TRACE << "\tRAW STRESS LEVEL:" << m_stress_level;
    TRACE << "\tHAPPINESS:" << happiness_name(m_happiness);
}

void Dwarf::read_personality() {
    if(!m_is_animal){
        VIRTADDR personality_addr = m_mem->soul_field(m_first_soul, "personality");

        //read personal beliefs before traits, as a dwarf will have a conflict with either personal beliefs or cultural beliefs
        m_beliefs.clear();
        QVector<VIRTADDR> beliefs_addrs = m_df->enumerate_vector(m_mem->soul_field(personality_addr, "beliefs"));
        foreach(VIRTADDR addr, beliefs_addrs){
            int belief_id = m_df->read_int(addr);
            if(belief_id >= 0){
                short val = m_df->read_short(addr + 0x0004);
                UnitBelief ub(belief_id,val,true);
                m_beliefs.insert(belief_id, ub);
            }
        }

        VIRTADDR traits_addr = m_mem->soul_field(personality_addr, "traits");
        m_traits.clear();
        m_conflicting_beliefs.clear();
        int trait_count = GameDataReader::ptr()->get_total_trait_count();
        for (int trait_id = 0; trait_id < trait_count; ++trait_id) {
            short val = m_df->read_short(traits_addr + trait_id * 2);
            if(val < 0)
                val = 0;
            if(val > 100)
                val = 100;
            m_traits.insert(trait_id, val);

            QList<int> possible_conflicts = GameDataReader::ptr()->get_trait(trait_id)->get_conflicting_beliefs();
            foreach(int belief_id, possible_conflicts){
                UnitBelief ub = get_unit_belief(belief_id);
                if((ub.belief_value() > 10 && val < 40)  || (ub.belief_value() < -10 && val > 60)){
                    m_beliefs[belief_id].add_trait_conflict(trait_id);
                    m_conflicting_beliefs.insertMulti(trait_id, ub);
                }
            }
        }

        //add special traits for cave adaptation and detachment, scale them to normal trait ranges

        int combat_hardened = m_df->read_int(m_mem->soul_field(personality_addr, "combat_hardened"));
        //scale from 40-90. this sets the values (33,75,100) at 56,78,90 respectively
        //since anything below 65 doesn't really have an effect
        combat_hardened = ((combat_hardened*(90-40)) / 100) + 40;
        m_traits.insert(-1,combat_hardened);

        int cave_adapt = has_state(STATE_CAVE_ADAPT) ? state_value(STATE_CAVE_ADAPT) : 0;
        //cave adapt is in ticks, 1 year and 1.5 years. only include it if there's a value > 0
        //scale from 40-90. this will set 1 year at 73, and 1.5 years at 90
        //anything below that we don't want to draw as it won't have an effect, and
        //the drawing will only start at > 60 (~7.5 months)
        cave_adapt = ((cave_adapt*(90-40)) / 604800) + 40;
        if(cave_adapt > 100)
            cave_adapt = 100;
        m_traits.insert(-2,cave_adapt);

        QVector<VIRTADDR> m_goals_addrs = m_df->enumerate_vector(m_mem->soul_field(personality_addr, "goals"));
        m_goals.clear();
        foreach(VIRTADDR addr, m_goals_addrs){
            int goal_type = m_df->read_int(addr + 0x0004);
            if(goal_type >= 0){
                short val = m_df->read_short(m_mem->soul_field(addr, "goal_realized")); //goal realized
                //if we're not showing vampires, and this dwarf is a vampire, keep the goal hidden so they can't be identified from that
                if(goal_type == 11 && m_curse_type == eCurse::VAMPIRE &&  DT->user_settings()->value("options/highlight_cursed", false).toBool()==false)
                    continue;

                if(val > 0)
                    m_goals_realized++;
                m_goals.insert(goal_type,val);
            }
        }

        //read after traits
        read_emotions(personality_addr);

        // Needs and focus
        QVector<VIRTADDR> m_need_addrs = m_df->enumerate_vector(personality_addr + m_mem->soul_detail("needs"));
        m_needs.clear();
        for (VIRTADDR addr: m_need_addrs) {
            auto need = std::make_unique<UnitNeed>(addr, m_df, this);
            m_needs.emplace(need->id(), std::move(need));
        }
        m_current_focus = m_df->read_int(personality_addr + m_mem->soul_detail("current_focus"));
        m_undistracted_focus = m_df->read_int(personality_addr + m_mem->soul_detail("undistracted_focus"));
        int ratio = m_undistracted_focus != 0 ? (m_current_focus*100)/m_undistracted_focus : 100;
        if (ratio <= 60)
            m_current_focus_degree = FOCUS_BADLY_DISTRACTED;
        else if (ratio <= 80)
            m_current_focus_degree = FOCUS_DISTRACTED;
        else if (ratio < 100)
            m_current_focus_degree = FOCUS_UNFOCUSED;
        else if (ratio == 100)
            m_current_focus_degree = FOCUS_UNTROUBLED;
        else if (ratio < 120)
            m_current_focus_degree = FOCUS_SOMEWHAT_FOCUSED;
        else if (ratio < 140)
            m_current_focus_degree = FOCUS_QUITE_FOCUSED;
        else
            m_current_focus_degree = FOCUS_VERY_FOCUSED;

        //add a special preference for like outdoors
        int likes_outdoors = m_df->read_int(m_mem->soul_field(personality_addr, "likes_outdoors"));
        if (likes_outdoors > 0)
            m_preferences.emplace(
                    LIKE_OUTDOORS,
                    std::make_unique<OutdoorPreference>(likes_outdoors)
            );
    }
}

bool Dwarf::trait_is_conflicted(const int &trait_id){
    return (m_conflicting_beliefs.values(trait_id).count() > 0);
}

bool Dwarf::trait_is_active(int trait_id){
    //if it's a special trait, always show it
    if(trait_id >= 50)
        return true;
    short val = trait(trait_id);
    int deviation = abs(val - 50); // how far from the norm is this trait?
    if (deviation <= 10) {
        return false;
    }
    return true;
}

bool Dwarf::belief_is_active(const int &belief_id){
    return (m_beliefs.value(belief_id).is_active());
}

//this returns either the personal belief (if it exists), or the cultural belief (default)
UnitBelief Dwarf::get_unit_belief(int belief_id){
    if(!m_beliefs.contains(belief_id)){
        UnitBelief ub(belief_id,m_df->fortress()->get_belief_value(belief_id),false);
        m_beliefs.insert(belief_id,ub);
    }
    return m_beliefs.value(belief_id);
}

void Dwarf::read_attributes() {
    m_attributes.clear();
    //read the physical attributes
    VIRTADDR addr = m_mem->dwarf_field(m_address, "physical_attrs");
    for(int i=0; i<6; i++){
        load_attribute(addr, static_cast<ATTRIBUTES_TYPE>(i));
    }
    //read the mental attributes, but append to our array (augment the key by the number of physical attribs)
    int phys_size = m_attributes.size();
    addr = m_mem->soul_field(m_first_soul, "mental_attrs");
    for(int i=0; i<13; i++){
        load_attribute(addr, static_cast<ATTRIBUTES_TYPE>(i+phys_size));
    }
}

void Dwarf::load_attribute(VIRTADDR &addr, ATTRIBUTES_TYPE id){
    int cti = 500;
    QPair<int,QString> desc; //index, description of descriptor

    int value = (int)m_df->read_int(addr);
    int display_value = value;
    int limit = (int)m_df->read_int(addr+0x4);

    //apply any permanent syndrome changes to the raw/base value
    int perm_add = 0;
    int perm_perc = 1;
    if(m_attribute_mods_perm.contains(id)){
        perm_perc = m_attribute_mods_perm.value(id).first;
        value *= perm_perc;
        perm_add = m_attribute_mods_perm.value(id).second;
        value += perm_add;
    }
    //apply temp syndrome changes to the display value
    if(m_attribute_mods_temp.contains(id)){
        display_value *= perm_perc;
        display_value *= m_attribute_mods_temp.value(id).first;

        display_value += perm_add;
        display_value += m_attribute_mods_temp.value(id).second;
    }else{
        display_value *= perm_perc;
        display_value += perm_add;
    }

    if(m_caste){
        cti = m_caste->get_attribute_cost_to_improve(id);
        desc = m_caste->get_attribute_descriptor_info(id, display_value);
    }
    Attribute a = Attribute(id, value, display_value, limit, cti, desc.first, desc.second);

    //immediately calculate balanced value for roles
    if(!is_baby() && !m_is_animal)
        a.calculate_balanced_value();

    if(m_attribute_syndromes.contains(id))
        a.set_syn_names(m_attribute_syndromes.value(id));

    m_attributes.append(a);
    addr+=0x1c;
}

Attribute Dwarf::get_attribute(ATTRIBUTES_TYPE id){
    if(id < m_attributes.count())
        return m_attributes.at(id);
    else
        return Attribute(id,0,0,0);
}

Skill Dwarf::get_skill(int skill_id) {
    if(!m_skills.contains(skill_id)){
        int skill_rate = 100;
        if(m_caste){
            skill_rate = m_caste->get_skill_rate(skill_id);
        }
        Skill s = Skill(skill_id, 0, -1, 0, skill_rate);
        s.get_balanced_level();
        m_skills.insert(skill_id,s);
        m_sorted_skills.insertMulti(s.capped_level_precise(), skill_id);
    }
    return m_skills.value(skill_id);
}

float Dwarf::skill_level(int skill_id){
    return get_skill_level(skill_id,false,false);
}
float Dwarf::skill_level_raw(int skill_id){
    return get_skill_level(skill_id,true,false);
}
float Dwarf::skill_level_precise(int skill_id){
    return get_skill_level(skill_id,false,true);
}
float Dwarf::skill_level_raw_precise(int skill_id){
    return get_skill_level(skill_id,true,true);
}

float Dwarf::get_skill_level(int skill_id, bool raw, bool precise) {
    float retval = -1;
    if(m_skills.contains(skill_id)){
        if(raw){
            if(precise)
                retval = m_skills.value(skill_id).raw_level_precise();
            else
                retval = m_skills.value(skill_id).raw_level();
        }else{
            if(precise)
                retval = m_skills.value(skill_id).capped_level_precise();
            else
                retval = m_skills.value(skill_id).capped_level();
        }
    }
    return retval;
}

short Dwarf::labor_rating(int labor_id) {
    GameDataReader *gdr = GameDataReader::ptr();
    Labor *l = gdr->get_labor(labor_id);
    if (l)
        return get_skill_level(l->skill_id);
    else
        return -1;
}

QString Dwarf::get_age_formatted(){
    using std::chrono::duration_cast;
    if (m_age < df_year(1)) {
        auto age_in_months = duration_cast<df_month>(m_age).count();
        return QString::number(age_in_months).append(tr(" Month").append(age_in_months == 1 ? "" : "s").append(tr(" Old")));
    }
    else {
        auto age_in_years = duration_cast<df_year>(m_age).count();
        return QString::number(age_in_years).append(tr(" Year").append(age_in_years == 1 ? "" : "s").append(tr(" Old")));
    }
}

short Dwarf::pref_value(const int &labor_id) {
    if (!m_pending_labors.contains(labor_id)) {
        LOGW << m_nice_name << "pref_value for labor_id" << labor_id << "was not found in pending labors hash!";
        return 0;
    }
    return m_pending_labors.value(labor_id);
}

bool Dwarf::labor_enabled(int labor_id) {
    return m_pending_labors.value(labor_id, false);
}

bool Dwarf::is_labor_state_dirty(int labor_id) {
    return m_labors[labor_id] != m_pending_labors[labor_id];
}

bool Dwarf::is_custom_profession_dirty(QString name){
    return ((name == m_pending_custom_profession || name == m_custom_prof_name) &&
            m_pending_custom_profession != m_custom_prof_name);
}

QVector<int> Dwarf::get_dirty_labors() {
    QVector<int> labors;
    Q_ASSERT(m_labors.size() == m_pending_labors.size());
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (is_labor_state_dirty(labor_id))
            labors << labor_id;
    }
    return labors;
}

bool Dwarf::toggle_labor(int labor_id) {
    set_labor(labor_id, !m_pending_labors[labor_id], false);
    return true;
}

void Dwarf::clear_labors(){
    foreach(int key, m_pending_labors.uniqueKeys()){
        if(labor_enabled(key))
            set_labor(key,false,false);
    }
}

void Dwarf::assign_all_labors(){
    foreach(int key, m_pending_labors.uniqueKeys()){
        if(!labor_enabled(key))
            set_labor(key,true,false);
    }
}

void Dwarf::toggle_skilled_labors(){
    foreach(int key, m_pending_labors.uniqueKeys()){
        toggle_skilled_labor(key);
    }
}

void Dwarf::toggle_skilled_labor(int labor_id){
    bool enable = labor_rating(labor_id) >= 1; //ignore dabbling
    set_labor(labor_id,enable,false);
}

void Dwarf::set_labor(int labor_id, bool enabled, bool update_cols_realtime) {
    Labor *l = GameDataReader::ptr()->get_labor(labor_id);
    if (!l) {
        LOGD << "UNKNOWN LABOR: Bailing on set labor of id" << labor_id << "enabled?" << enabled << "for" << m_nice_name;
        return;
    }

    if(!m_can_set_labors) {
        LOGD << "IGNORING SET LABOR OF ID:" << labor_id << "TO:" << enabled << "FOR:" << m_nice_name << "PROF_ID" << m_raw_prof_id
             << "PROF_NAME:" << profession() << "CUSTOM:" << m_pending_custom_profession;
        return;
    }

    //user is turning a labor on, so we must turn off exclusives
    if (enabled && DT->user_settings()->value("options/labor_exclusions",true).toBool()) {
        foreach(int excluded, l->get_excluded_labors()) {
            if(labor_enabled(excluded)){
                m_df->update_labor_count(excluded, -1);
                if(update_cols_realtime)
                    DT->update_specific_header(excluded,CT_LABOR);
            }
            //TRACE << "LABOR" << labor_id << "excludes" << excluded;
            m_pending_labors[excluded] = false;
        }
    }

    int change = 0;
    if(labor_enabled(labor_id) && !enabled){
        change = -1;
    }else if(!labor_enabled(labor_id) && enabled){
        change = 1;
    }
    if(change != 0){
        m_df->update_labor_count(labor_id,enabled ? 1 : -1);
        if(update_cols_realtime)
            DT->update_specific_header(labor_id,CT_LABOR);
    }
    m_pending_labors[labor_id] = enabled;
}


bool Dwarf::is_flag_dirty(int bit_pos){
    int idx = bit_pos / 32;
    return m_unit_flags.at(idx) != m_pending_flags.at(idx);
}

int Dwarf::pending_changes() {
    int cnt = get_dirty_labors().size();
    if (m_nick_name != m_pending_nick_name)
        cnt++;
    if(m_squad_id != m_pending_squad_id)
        cnt++;
    if (m_custom_prof_name != m_pending_custom_profession)
        cnt++;
    for(int i = 0; i < m_unit_flags.count();i++){
        if (m_unit_flags.at(i) != m_pending_flags.at(i)){
            cnt++;
        }
    }
    return cnt;
}

void Dwarf::clear_pending() {
    if(pending_changes() <= 0)
        return;
    //update our header numbers before we refresh
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id >= 0 && is_labor_state_dirty(labor_id))
            m_df->update_labor_count(labor_id, labor_enabled(labor_id) ? -1 : 1);
    }

    //revert any squad changes
    if(m_pending_squad_id != m_squad_id){
        if(m_pending_squad_id >= 0){
            Squad *s = m_df->get_squad(m_pending_squad_id);
            if(s)
                s->remove_from_squad(this); //updates uniform/inventory/squad
        }
        if(m_squad_id >= 0){
            Squad *s = m_df->get_squad(m_squad_id);
            if(s)
                s->assign_to_squad(this); //updates uniform/inventory/squad
        }
    }

    //refresh any other data that may have been pending commit
    refresh_minimal_data();
}

void Dwarf::clear_pending_labors() {
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id >= 0 && is_labor_state_dirty(labor_id))
            set_labor(labor_id, m_labors[labor_id], false);
    }
}

void Dwarf::commit_pending(bool single) {
    VIRTADDR addr = m_mem->dwarf_field(m_address, "labors");

    QByteArray buf(94, 0);
    m_df->read_raw(addr, 94, buf); // set the buffer as it is in-game
    bool needs_equip_recheck = false;
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id < 0)
            continue;
        // change values to what's pending
        buf[labor_id] = m_pending_labors.value(labor_id);
        //only recheck equipment if a labor which requires equipment has been changed
        //mining, woodcutting or hunting
        if(!needs_equip_recheck && (is_labor_state_dirty(0) || is_labor_state_dirty(10) || is_labor_state_dirty(44)))
            needs_equip_recheck = true;
    }

    m_df->write_raw(addr, 94, buf.data());

    if (m_pending_nick_name != m_nick_name){
        m_df->write_string(m_mem->word_field(m_mem->dwarf_field(m_address, "name"), "nickname"), m_pending_nick_name);
        m_df->write_string(m_mem->word_field(m_mem->soul_field(m_first_soul, "name"), "nickname"), m_pending_nick_name);
        if(m_hist_figure){
            m_hist_figure->write_nick_name(m_pending_nick_name);
        }
    }
    if (m_pending_custom_profession != m_custom_prof_name)
        m_df->write_string(m_mem->dwarf_field(m_address, "custom_profession"), m_pending_custom_profession);

    for(int i=0; i < m_unit_flags.count(); i++){
        if (m_pending_flags.at(i) != m_unit_flags.at(i)){
            m_df->write_raw(m_mem->dwarf_field(m_address, QString("flags%1").arg(i+1)), 4, &m_pending_flags[i]);
        }
    }

    int pen_sq_id = -1;
    int pen_sq_pos = -1;
    QString pen_sq_name = "";
    if(m_pending_squad_id != m_squad_id){
        if(!single){    //currently we can't apply squad changes individually
            Squad *s;
            if(m_pending_squad_id > -1){
                s = m_df->get_squad(m_pending_squad_id);
                if(s){
                    s->assign_to_squad(this,true);
                    read_squad_info();
                }
            }else{
                s = m_df->get_squad(m_squad_id);
                if(s){
                    s->remove_from_squad(this,true);
                    read_squad_info();
                }
            }
            needs_equip_recheck = true;
        }else{
            //save current pending squad changes to restore later
            pen_sq_id = m_pending_squad_id;
            pen_sq_pos = m_pending_squad_position;
            pen_sq_name = m_pending_squad_name;
        }
    }

    //set flag to get equipment and recheck our uniform and inventory for missing items
    if(needs_equip_recheck){
        recheck_equipment(m_pending_squad_id);
        read_uniform();
        read_inventory();
    }

    //refresh any other data we may have just committed
    refresh_minimal_data();

    if(single){
        //restore the saved pending squad changes
        m_pending_squad_id = pen_sq_id;
        m_pending_squad_position = pen_sq_pos;
        m_pending_squad_name = pen_sq_name;
    }
}

void Dwarf::recheck_equipment(int squad_id){
    // set the equipment update flags for squad change
    if (squad_id != -1) {
        if (auto squad = m_df->get_squad(squad_id)) {
            auto flags = m_df->read_int(m_mem->squad_field(squad->address(), "equipment_update"));
            flags |= 0x7ff;
            m_df->write_int(m_mem->squad_field(squad->address(), "equipment_update"), flags);
        }
    }
    auto flags = m_df->read_int(m_mem->global_address(m_df, "global_equipment_update"));
    flags |= 0x7ff;
    m_df->write_int(m_mem->global_address(m_df, "global_equipment_update"), flags);
    // set the "recheck_equipment" flag if there was a labor change
    BYTE recheck_equipment = m_df->read_byte(m_mem->dwarf_field(m_address, "recheck_equipment"));
    recheck_equipment |= 1;
    m_df->write_raw(m_mem->dwarf_field(m_address, "recheck_equipment"), 1, &recheck_equipment);
}


void Dwarf::set_nickname(const QString &nick) {
    m_pending_nick_name = nick;
    build_names();
}

void Dwarf::set_custom_profession_text(const QString &prof_text) {
    m_pending_custom_profession = prof_text;
}

void Dwarf::apply_custom_profession(CustomProfession *cp) {
    //clear all labors if the custom profession is not being applied as a mask
    if(!cp->is_mask()){
        foreach(int labor_id, m_pending_labors.uniqueKeys()) {
            set_labor(labor_id, false,false);
        }
    }
    //enable the custom profession's labors
    foreach(int labor_id, cp->get_enabled_labors()) {
        set_labor(labor_id, true,false);
    }
    m_pending_custom_profession = cp->get_name();
}

void Dwarf::reset_custom_profession(bool reset_labors){
    CustomProfession *cp = DT->get_custom_profession(m_pending_custom_profession);
    if(cp){
        if(reset_labors && !cp->is_mask()){
            m_pending_labors = m_labors;
        }
        foreach(int labor_id, cp->get_enabled_labors()){
            set_labor(labor_id,false,false);
        }
    }
    m_pending_custom_profession = "";
}

QTreeWidgetItem *Dwarf::get_pending_changes_tree() {
    QVector<int> labors = get_dirty_labors();
    QTreeWidgetItem *d_item = new QTreeWidgetItem;
    d_item->setText(0, QString("%1 (%2)").arg(nice_name()).arg(pending_changes()));
    d_item->setData(0, Qt::UserRole, id());

    build_pending_flag_node(0,tr("Cage"),FLAG_CAGED,d_item);
    build_pending_flag_node(1,tr("Butcher"),FLAG_BUTCHER,d_item);
    build_pending_flag_node(2,tr("Geld"),FLAG_GELD,d_item);

    if (m_pending_nick_name != m_nick_name) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString nick = m_pending_nick_name;
        i->setText(0, nick.isEmpty() ? tr("Nickname reset to default")
                                     : tr("Nickname changed to %1").arg(nick));
        i->setIcon(0, QIcon(":img/book--pencil.png"));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_custom_profession != m_custom_prof_name) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString prof = m_pending_custom_profession;
        i->setText(0, prof.isEmpty() ? tr("Profession reset to default")
                                     : tr("Profession changed to %1").arg(prof));
        i->setIcon(0, QIcon(":img/book--pencil.png"));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_squad_id != m_squad_id){
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString title = "";
        QString icn = "plus-circle.png";
        if(m_pending_squad_id < 0 && m_df->get_squad(m_squad_id) != 0){
            title = tr("Remove from squad %1").arg(m_df->get_squad(m_squad_id)->name());
            icn = "minus-circle.png";
        }else{
            title = tr("Assign to squad %1").arg(m_pending_squad_name);
        }
        i->setText(0,title);
        i->setIcon(0, QIcon(QString(":img/%1").arg(icn)));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    foreach(int labor_id, labors) {
        Labor *l = GameDataReader::ptr()->get_labor(labor_id);

        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        if (l) {
            i->setText(0, l->name);
            if (labor_enabled(labor_id)) {
                i->setIcon(0, QIcon(":img/plus-circle.png"));
            } else {
                i->setIcon(0, QIcon(":img/minus-circle.png"));
            }
            i->setToolTip(0, i->text(0));
        }
        i->setData(0, Qt::UserRole, id());
    }
    return d_item;
}

void Dwarf::build_pending_flag_node(int index, QString title, UNIT_FLAGS flag, QTreeWidgetItem *parent){
    if (m_pending_flags.at(index) != m_unit_flags.at(index)) {
        QTreeWidgetItem *i = new QTreeWidgetItem(parent);
        QString icon_path = ":img/plus-circle.png";
        QString msg = title;
        if(!get_flag_value(flag)){
            icon_path = ":img/minus-circle.png";
            msg = tr("Cancel %1").arg(title);
        }
        i->setIcon(0, QIcon(icon_path));
        i->setText(0, msg);
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
}

QString Dwarf::tooltip_text() {
    QSettings *s = DT->user_settings();
    s->beginGroup("options");
    GameDataReader *gdr = GameDataReader::ptr();
    QString skill_summary, personality_summary, roles_summary;
    int max_roles = s->value("role_count_tooltip",3).toInt();
    if(max_roles > sorted_role_ratings().count())
        max_roles = sorted_role_ratings().count();

    //in some mods animals may have skills
    if(!m_skills.isEmpty() && s->value("tooltip_show_skills",true).toBool()){
        int max_level = s->value("min_tooltip_skill_level", true).toInt();
        bool check_social = !s->value("tooltip_show_social_skills",true).toBool();
        bool include_level = s->value("tooltip_show_skills_level",true).toBool();
        bool include_exp_summary = s->value("tooltip_show_skills_exp_summary",true).toBool();
        bool use_color = s->value("tooltip_show_skills_use_color",true).toBool();
        bool top_skills_only = s->value("tooltip_only_top_skills",true).toBool();
        int top_skill_count = s->value("tooltip_top_skill_count",12).toInt();
        QMapIterator<float,int> i(m_sorted_skills);
        i.toBack();
        while(i.hasPrevious()){
            i.previous();
            if(m_skills.value(i.value()).capped_level() < max_level || (check_social && gdr->social_skills().contains(i.value()))) {
                continue;
            }
            skill_summary.append(QString("<li>%1</li>").arg(m_skills.value(i.value()).to_string(include_level, include_exp_summary, use_color)));
            if (top_skills_only && --top_skill_count == 0)
                break;
        }
    }

    if(!m_is_animal){
        if(s->value("tooltip_show_traits",true).toBool()){
            QString conflict_color = QColor(176,23,31).name();
            if(!m_traits.isEmpty()){
                QStringList notes;
                for (int trait_id = 0; trait_id < m_traits.size(); ++trait_id) {
                    notes.clear();
                    if (trait_is_active(trait_id)){
                        Trait *t = gdr->get_trait(trait_id);
                        if (!t)
                            continue;
                        int val = m_traits.value(trait_id);
                        QString msg = capitalize(t->level_message(val));
                        if(trait_is_conflicted(trait_id))
                            msg = QString("<font color=%1>%2</font>").arg(conflict_color).arg(msg);
                        personality_summary.append(msg);
                        QString temp = t->skill_conflicts_msgs(val);
                        if(!temp.isEmpty())
                            notes.append("<u>" + temp + "</u>");
                        temp = t->special_messages(val);
                        if(!temp.isEmpty())
                            notes.append(temp);

                        if(!notes.isEmpty())
                            personality_summary.append(" (" + notes.join(" and ") + ")");

                        personality_summary.append(". ");
                    }
                }
            }

            //beliefs
            QStringList beliefs_list;
            foreach(int belief_id, m_beliefs.uniqueKeys()) {
                if(!belief_is_active(belief_id))
                    continue;
                Belief *b = gdr->get_belief(belief_id);
                if (!b)
                    continue;
                beliefs_list.append(capitalize(b->level_message(m_beliefs.value(belief_id).belief_value())));
            }
            if(beliefs_list.size() > 0)
                personality_summary.append(QString("<font color=%1>%2. </font>").arg(Trait::belief_color.name()).arg(beliefs_list.join(". ")));

            //goals
            QStringList goal_list;
            for(int i=0;i<m_goals.size();i++){
                QString desc = capitalize(gdr->get_goal_desc(m_goals.keys().at(i),(bool)m_goals.values().at(i)));
                goal_list.append(desc);
            }
            if(goal_list.size() > 0)
                personality_summary.append(QString("<font color=%1>%2.</font>").arg(Trait::goal_color.name()).arg(goal_list.join(". ")));
        }

        QList<Role::simple_rating> sorted_roles = sorted_role_ratings();
        if(!sorted_roles.isEmpty() && max_roles > 0 && s->value("tooltip_show_roles",true).toBool()){
            roles_summary.append("<ol style=\"margin-top:0; margin-bottom:0;\">");
            for(int i = 0; i < max_roles; i++){
                roles_summary += tr("<li>%1  (%2%)</li>").arg(sorted_roles.at(i).name)
                        .arg(QString::number(sorted_roles.at(i).rating,'f',2));
            }
            roles_summary.append("</ol>");
        }
    }

    QString html;

    // styled templates used for generating tool-tip content
    const QString item_with_title ("<div><b>%1</b> %2</div>");
    const QString paragraph ("<div style=\"margin-top:0.5em;margin-bottom:0.5em\">%1</div>");
    const QString paragraph_with_title ("<div style=\"margin-top:0.5em;margin-bottom:0.5em\"><b>%1</b> %2</div>");
    const QString paragraph_with_header ("<div style=\"margin-top:0.5em;margin-bottom:0.5em\"><h4 style=\"margin:0\"><b>%1</b></h4>%2</div>");
    const QString list_with_header ("<div style=\"margin-top:0.5em;margin-bottom:0.5em\">"
                                    "<h4 style=\"margin:0\"><b>%1</b></h4>"
                                    "<ul style=\"margin:0\">%2</ul>"
                                    "</div>");

    QString title, first_column, second_column;
    if(s->value("tooltip_show_icons",true).toBool()){
        title += tr("<b><h3 style=\"margin:0\"><img src='%1'> %2 %3</h3><h4 style=\"margin:0\">%4</h4></b>")
                .arg(m_icn_gender).arg(m_nice_name).arg(embedPixmap(m_icn_prof))
                .arg(m_translated_name.isEmpty() ? "" : "(" + m_translated_name + ")");
    }else{
        title += tr("<b><h3 style=\"margin:0\">%1</h3><h4 style=\"margin:0\">%2</h4></b>")
                .arg(m_nice_name).arg(m_translated_name.isEmpty() ? "" : "(" + m_translated_name + ")");
    }

    if(!m_is_animal && s->value("tooltip_show_artifact",true).toBool() && !m_artifact_name.isEmpty())
        title.append(tr("<i><h5 style=\"margin:0;\">Creator of '%2'</h5></i>").arg(m_artifact_name));

    html.append(QString("<div align=\"center\" style=\"margin:1em\">%1</div>").arg(title));

#ifdef QT_DEBUG
    html.append(QString("<div align=\"center\"><h4>ID: %1 HIST_ID: %2</h4></div>").arg(m_id).arg(m_histfig_id));
#endif

    if(s->value("tooltip_show_caste",true).toBool())
        first_column.append(item_with_title.arg(tr("Caste:")).arg(caste_name()));

    if(m_is_animal || s->value("tooltip_show_age",true).toBool())
        first_column.append(item_with_title.arg(tr("Age:")).arg(get_age_formatted()));

    if(m_is_animal || s->value("tooltip_show_size",true).toBool())
        first_column.append(item_with_title.arg(tr("Size:")).arg(QLocale(QLocale::system()).toString(m_body_size * 10) + " cm<sup>3</sup>"));

    if(!m_is_animal && s->value("tooltip_show_noble",true).toBool())
        first_column.append(item_with_title.arg(tr("Profession:")).arg(profession()));

    if(!m_is_animal && m_pending_squad_id > -1 && s->value("tooltip_show_squad",true).toBool())
        first_column.append(item_with_title.arg(tr("Squad:")).arg(m_pending_squad_name));

    if(!m_is_animal && m_noble_position != "" && s->value("tooltip_show_noble",true).toBool())
        first_column.append(item_with_title.arg(m_noble_position.indexOf(",") > 0 ? tr("Noble Positions") : tr("Noble Position")).arg(m_noble_position));

    if(!m_is_animal && s->value("tooltip_show_happiness",true).toBool()){
        first_column.append(item_with_title.arg(tr("Happiness:")).arg(m_happiness_desc));
        if(m_stressed_mood)
            first_column.append(item_with_title.arg(tr("Mood:")).arg(gdr->get_mood_desc(m_mood_id,true)));
    }

    if(s->value("tooltip_show_orientation",false).toBool())
        first_column.append(item_with_title.arg(tr("Gender/Orientation")).arg(m_gender_info.full_desc));

    if(!m_is_animal && !m_emotions_desc.isEmpty() && s->value("tooltip_show_thoughts",true).toBool())
        second_column.append(paragraph.arg(m_emotions_desc));

    QString focus_desc = get_focus_desc(true);
    if(!m_is_animal && !focus_desc.isEmpty() && s->value("tooltip_show_needs",false).toBool())
        second_column.append(paragraph.arg(focus_desc));

    if(!skill_summary.isEmpty())
        first_column.append(list_with_header.arg(tr("Skills:")).arg(skill_summary));

    if(!m_is_animal && s->value("tooltip_show_mood",false).toBool() && !had_mood()){
        QStringList skill_names;
        if (!m_moodable_skills.isEmpty()) {
            foreach(int skill_id, m_moodable_skills.keys()){
                skill_names << gdr->get_skill_name(skill_id, true);
            }
            skill_names.removeDuplicates();
        }
        else {
            skill_names << tr("Craftsdwarf (Bone/Stone/Wood)");
        }
        first_column.append(paragraph_with_title.arg(tr("Highest Moodable Skill:")).arg(skill_names.join(",")));
    }

    if(!personality_summary.isEmpty())
        second_column.append(paragraph_with_title.arg(tr("Personality:")).arg(personality_summary));

    if(!m_pref_tooltip.isEmpty())
        second_column.append(paragraph_with_title.arg(tr("Preferences:")).arg(m_pref_tooltip));

    if(!roles_summary.isEmpty())
        first_column.append(paragraph_with_header.arg(tr("Top %n Roles:", "", max_roles)).arg(roles_summary));

    if(m_is_animal)
        first_column.append(paragraph_with_title.arg(tr("Trained Level:")).arg(get_animal_trained_descriptor(m_animal_type)));

    if(s->value("tooltip_show_health",false).toBool()){

        bool symbols = s->value("tooltip_health_symbols",false).toBool();
        bool colors = s->value("tooltip_health_colors",true).toBool();

        //health info is in 3 sections: treatment, statuses and wounds
        QString health_info = "";

        QStringList treatments = m_unit_health.get_treatment_summary(colors,symbols);
        if(treatments.size() > 0)
            health_info.append(tr("<h4 style=\"margin:0;\"><b>%1:</b></h4><ul style=\"margin:0;\">%2</ul>").arg(tr("Treatment")).arg(treatments.join(", ")));

        //combine statuses and wounds so it's more compact for the toolip
        QStringList status_wound_summary = m_unit_health.get_status_summary(colors, symbols);
        QMap<QString,QStringList> wound_summary = m_unit_health.get_wound_summary(colors, symbols);
        if(wound_summary.size() > 0){
            foreach(QString key, wound_summary.uniqueKeys()){
                QStringList info = wound_summary.value(key);
                foreach(QString i, info){
                    if(!status_wound_summary.contains(i))
                        status_wound_summary.append(i);
                }
            }

            std::sort(status_wound_summary.begin(), status_wound_summary.end());
            health_info.append(tr("<h4 style=\"margin:0;\"><b>%1:</b></h4><ul style=\"margin:0;\">%2</ul>").arg(tr("Health Issues")).arg(status_wound_summary.join(", ")));
        }

        if(!health_info.isEmpty())
            first_column.append(paragraph.arg(health_info));
    }

    if(m_syndromes.count() > 0 && s->value("tooltip_show_buffs",false).toBool()){
        QString buffs = get_syndrome_names(true,false);
        QString ailments = get_syndrome_names(false,true);

        if(!buffs.isEmpty())
            first_column.append(paragraph_with_title.arg(tr("Buffs:")).arg(buffs));
        if(!ailments.isEmpty())
            first_column.append(paragraph_with_title.arg(tr("Ailments:")).arg(ailments));
    }


    if(s->value("tooltip_show_caste_desc",true).toBool() && caste_desc() != "")
        first_column.append(paragraph.arg(caste_desc()));

    if(s->value("highlight_cursed", false).toBool() && m_curse_name != ""){
        QString curse_text = tr("<b>Curse: </b>A <b><i>%1</i></b>")
                .arg(capitalizeEach(m_curse_name));
        //if we have an assumed identity, show it
        if(m_nice_name != m_true_name && !m_true_name.isEmpty()){
            curse_text.append(tr(" by the name of %1, ").arg(m_true_name));
            if(m_true_birth_year > 0){
                curse_text.append(tr("born in the year %1.").arg(m_true_birth_year));
            }else{
                curse_text.append(tr("born %1 years before the Age of Myth.").arg(abs(m_true_birth_year)));
            }
        }
        first_column.append(paragraph.arg(curse_text));
    }

    if(s->value("tooltip_show_kills",false).toBool() && m_hist_figure && m_hist_figure->total_kills() > 0){
        first_column.append(paragraph.arg(m_hist_figure->formatted_summary()));
    }

    s->endGroup();

    if (!second_column.isEmpty())
        html.append(QString("<table><tr><td width=\"300\">%1</td><td width=\"600\">%2</td></tr></table>")
                    .arg(first_column)
                    .arg(second_column));
    else
        html.append(first_column);
    return html;
}


void Dwarf::dump_memory() {
    QDialog *d = new QDialog(DT->get_main_window());
    d->setAttribute(Qt::WA_DeleteOnClose, true);
    d->setWindowTitle(QString("%1, %2 [addr: 0x%3] [id:%4]")
                      .arg(m_nice_name).arg(profession())
                      .arg(m_address, 8, 16, QChar('0'))
                      .arg(m_id));
    d->resize(800, 600);
    QVBoxLayout *v = new QVBoxLayout(d);
    QTextEdit *te = new QTextEdit(d);
    te->setReadOnly(true);
    te->setFontFamily("Courier");
    te->setFontPointSize(8);
    QByteArray data;
    m_df->read_raw(m_address, 0xb90, data);
    te->setText(m_df->pprint(data));
    v->addWidget(te);
    d->setLayout(v);
    d->show();
}

void Dwarf::dump_memory_to_file() {
    QString filename = QString("%1-%2.txt").arg(nice_name())
            .arg(QDateTime::currentDateTime()
                 .toString("MMM-dd hh-mm-ss"));
    QDir d = QDir::current();
    d.cd("log");
    QFile *f = new QFile(d.filePath(filename), this);
    if (f->open(QFile::ReadWrite)) {
        f->write(QString("NAME: %1\n").arg(nice_name()).toLatin1());
        f->write(QString("ADDRESS: %1\n").arg(hexify(m_address)).toLatin1());
        QByteArray data;
        m_df->read_raw(m_address, 0xb90, data);
        f->write(m_df->pprint(data).toLatin1());
        f->close();
        QMessageBox::information(DT->get_main_window(), tr("Dumped"),
                                 tr("%1 has been dumped to %2")
                                 .arg(nice_name())
                                 .arg(d.absoluteFilePath(filename)));
    } else {
        QMessageBox::warning(DT->get_main_window(), tr("Unable to Dump Dwarf"),
                             tr("Could not write new file in log directory! "
                                "(%1)").arg(f->errorString()));
    }
    f->deleteLater();
}

void Dwarf::show_details() {
    DT->get_main_window()->show_dwarf_details_dock(this);
}

void Dwarf::copy_address_to_clipboard() {
    qApp->clipboard()->setText(hexify(m_address));
}

Skill Dwarf::highest_skill() {
    Skill highest = Skill(-1, 0, -1, 0);
    foreach(Skill s, m_skills.values()) {
        if (s.actual_exp() > highest.actual_exp()) {
            highest = s;
        }
    }
    return highest;
}

int Dwarf::total_skill_levels() {
    int ret_val = 0;
    foreach(Skill s, m_skills.values()) {
        if(s.raw_level() > 0)
            ret_val += s.raw_level();
    }
    return ret_val;
}

int Dwarf::total_assigned_labors(bool include_skill_less) {
    int ret_val = 0;
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        if(!include_skill_less && l->skill_id < 0)
            continue;
        if (m_labors[l->labor_id] > 0)
            ret_val++;
    }
    return ret_val;
}

//load all the attribute display ratings
void Dwarf::calc_attribute_ratings(){
    for(int i = 0; i < m_attributes.count(); i++){
        double val = DwarfStats::attributes_raw.rating(m_attributes[i].get_value());
        m_attributes[i].set_rating(val);
    }
}

QList<double> Dwarf::calc_role_ratings(){
    calc_attribute_ratings();

    LOGV << ":::::::::::::::::::::::::::::::::::::::::::::::::::";
    LOGV << m_nice_name;

    m_role_ratings.clear();
    m_raw_role_ratings.clear();
    m_sorted_role_ratings.clear();
    m_sorted_custom_role_ratings.clear();
    double rating = 0.0;
    foreach(Role *m_role, GameDataReader::ptr()->get_roles()){
        if(m_role){
            rating = calc_role_rating(m_role);
            m_raw_role_ratings.insert(m_role->name(), rating);
        }
    }
    return m_raw_role_ratings.values();
}

template<typename T, typename F>
static double calc_rating(const std::vector<std::pair<T, Role::aspect_weight>> &aspects, F get_rating)
{
    if (aspects.empty())
        return 50.0;

    double total = 0.0;
    double total_weight = 0.0;
    for (const auto &p: aspects){
        auto &w = p.second;
        double value = get_rating(p.first);

        if(w.is_neg)
            value = 1.0-value;
        total += value * w.weight;
        total_weight += w.weight;
    }

    if (total_weight > 0)
        return (total / total_weight) * 100.0; //weighted average percentile
    else
        return 50.0f;
}

double Dwarf::calc_role_rating(Role *m_role){
    //if there's a script, use this in place of any aspects
    if(!m_role->script().trimmed().isEmpty()){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(this);
        m_engine.globalObject().setProperty("d", d_obj);
        return  m_engine.evaluate(m_role->script()).toNumber(); //just show the raw value the script generates
    }

    LOGV << "  +" << m_role->name() << "-" << m_nice_name;

    //no script, calculate rating based on specified aspects
    enum Aspects {
        Attributes = 0,
        Facets,
        Beliefs,
        Goals,
        Needs,
        Skills,
        Preferences,
        AspectCount
    };
    double rating_aspect[AspectCount] = { 0.0 };

    //use the role's aspect section weights (these are defaulted to the global weights)
    float global_aspect_weight[AspectCount] = {
        m_role->attributes_weight.weight(),
        m_role->facets_weight.weight(),
        m_role->beliefs_weight.weight(),
        m_role->goals_weight.weight(),
        m_role->needs_weight.weight(),
        m_role->skills_weight.weight(),
        m_role->prefs_weight.weight(),
    };
    float global_aspect_weight_total = 0.0f;
    for (int i = 0; i < AspectCount; ++i)
        global_aspect_weight_total += global_aspect_weight[i];

    //without weights, there's nothing to calculate
    if(global_aspect_weight_total == 0.0f)
        return 50.0f;

    //ATTRIBUTES
    rating_aspect[Attributes] = calc_rating(m_role->attributes, [this] (const QString &name) {
        ATTRIBUTES_TYPE attrib_id = GameDataReader::ptr()->get_attribute_type(name.toUpper());
        return get_attribute(attrib_id).rating(true);
    });

    //FACETS
    rating_aspect[Facets] = calc_rating(m_role->facets, [this] (int id) {
        return DwarfStats::facets.rating(trait(id));
    });

    //BELIEFS
    rating_aspect[Beliefs] = calc_rating(m_role->beliefs, [this] (int id) {
        return DwarfStats::beliefs.rating(belief_value(id));
    });

    //GOALS
    rating_aspect[Goals] = calc_rating(m_role->goals, [this] (int id) {
        return m_goals.value(id, 1) <= 0 ? 1.0 : 0.0; // has goal and not realized
    });

    //NEEDS
    rating_aspect[Needs] = calc_rating(m_role->needs, [this] (int id) {
        return DwarfStats::needs.rating(get_need_type_level(id));
    });

    //SKILLS
    float total_skill_rates = 0.0;
    rating_aspect[Skills] = calc_rating(m_role->skills, [this, &total_skill_rates] (int id) {
        auto s = get_skill(id);
        total_skill_rates += s.skill_rate();

        LOGV << "      * skill:" << s.name() << "lvl:" << s.capped_level_precise() << "sim. lvl:" << s.get_simulated_level() << "balanced lvl:" << s.get_balanced_level()
             << "rating:" << s.get_rating();

        return s.get_rating();
    });
    if (!m_role->skills.empty() && total_skill_rates <= 0){
        //this unit cannot improve the skills associated with this role so cancel any rating
        return 0.0001;
    }

    //PREFERENCES
    if(!m_role->prefs.empty()){
        auto value = get_role_pref_match_counts(m_role);
        rating_aspect[Preferences] = DwarfStats::preferences.rating(value) * 100.0f;
    }else{
        rating_aspect[Preferences] = 50.0f;
    }

    //weighted average percentile total
    double rating_total = 0.0;
    for (int i = 0; i < AspectCount; ++i)
        rating_total += rating_aspect[i] * global_aspect_weight[i];
    rating_total /= global_aspect_weight_total;

    if(rating_total == 0.0)
        rating_total = 0.0001;

    LOGV << "      -attributes:" << rating_aspect[Attributes];
    LOGV << "      -skills:" << rating_aspect[Skills];
    LOGV << "      -facets:" << rating_aspect[Facets];
    LOGV << "      -beliefs:" << rating_aspect[Beliefs];
    LOGV << "      -goals:" << rating_aspect[Goals];
    LOGV << "      -needs:" << rating_aspect[Needs];
    LOGV << "      -preferences:" << rating_aspect[Preferences];
    LOGV << "      -total:" << rating_total;
    LOGV << "  ------------------------------------------------------";

    return rating_total;
}

const QList<Role::simple_rating> &Dwarf::sorted_role_ratings(){
    return m_sorted_role_ratings;
}

float Dwarf::get_role_rating(QString role_name){
    return m_role_ratings.value(role_name);
}
float Dwarf::get_raw_role_rating(QString role_name){
    return m_raw_role_ratings.value(role_name);
}

void Dwarf::refresh_role_display_ratings(){
    GameDataReader *gdr = GameDataReader::ptr();
    //keep a sorted list of the display ratings for tooltips, detail pane, etc.
    foreach(QString name, m_raw_role_ratings.uniqueKeys()){
        Role::simple_rating sr;
        sr.is_custom = gdr->get_role(name)->is_custom();
        float display_rating = DwarfStats::roles.rating(m_raw_role_ratings.value(name)) * 100.0f;
        m_role_ratings.insert(name,display_rating);
        sr.rating = display_rating;
        sr.name = name;
        m_sorted_role_ratings.append(sr);
    }
    if(DT->user_settings()->value("options/show_custom_roles",false).toBool()){
        std::sort(m_sorted_role_ratings.begin(),m_sorted_role_ratings.end(),&Dwarf::sort_ratings_custom);
    }else{
        std::sort(m_sorted_role_ratings.begin(),m_sorted_role_ratings.end(),&Dwarf::sort_ratings);
    }
}

double Dwarf::get_role_pref_match_counts(const Role *r, bool load_map){
    double total_rating = 0.0;
    for (const auto &p: r->prefs) {
        const auto *role_pref = p.first.get();
        const auto &w = p.second;
        double matches = get_role_pref_match_counts(role_pref,(load_map ? r : 0));
        if(matches > 0){
            double rating = matches * w.weight;
            if(w.is_neg)
                rating = 1.0f-rating;
            total_rating += rating;
        }
    }
    return total_rating;
}

double Dwarf::get_role_pref_match_counts(const RolePreference *role_pref, const Role *r){
    double matches = 0;
    int key = role_pref->get_pref_category();
    auto range = m_preferences.equal_range(key);
    for (auto it = range.first; it != range.second; ++it) {
        auto p = it->second.get();
        double match = (double)role_pref->match(p,this);
        if(match > 0){
            if(r){
                m_role_pref_map[r->name()].append(qMakePair(role_pref->get_name(), p->get_name()));
            }
        }
        matches += match;
    }
    //give a 0.1 bonus for each match after the first, this only applies when getting matches for groups
    if(matches > 1.0)
        matches = 1.0f + ((matches-1.0f) / 10.0f);

    return matches;
}

Reaction *Dwarf::get_reaction()
{
    if(m_current_sub_job_id.isEmpty())
        return 0;
    else
        return m_df->get_reaction(m_current_sub_job_id);
}

bool Dwarf::find_preference(QString pref_name, QString category_name){
    return has_preference(pref_name,category_name);
}

bool Dwarf::has_preference(QString pref_name, QString category){
    if(category.isEmpty()){
        return m_pref_names.contains(pref_name,Qt::CaseInsensitive);
    }else{
        if(!m_grouped_preferences.keys().contains(category))
            return false;
        return m_grouped_preferences.value(category)->contains(pref_name,Qt::CaseInsensitive);
    }
}

bool Dwarf::has_health_issue(int id, int idx){
    if(idx < 0){
        if(m_unit_health.get_most_severe(static_cast<eHealth::H_INFO>(id)))
            return true;
        else
            return false;
    }else{
        return m_unit_health.has_info_detail(static_cast<eHealth::H_INFO>(id),idx);
    }
}

QString Dwarf::squad_name(){
    return m_pending_squad_name;
}

void Dwarf::update_squad_info(int squad_id, int position, QString name){
    m_pending_squad_id = squad_id;
    m_pending_squad_position = position;
    m_pending_squad_name = name;
    //try to update the uniform and inventory
    read_uniform();
    read_inventory();
}
