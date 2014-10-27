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

#include "labor.h"
#include "preference.h"
#include "material.h"
#include "caste.h"
#include "roleaspect.h"
#include "thought.h"

#include "squad.h"
#include "uniform.h"
#include "itemweapon.h"
#include "itemarmor.h"
#include "itemammo.h"

#include <QVector>
#include <QAction>
#include <QDialog>
#include <QTextEdit>
#include <QMessageBox>
#include <QClipboard>
#include <QDateTime>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#ifdef QT_QML_LIB
# include <QJSEngine>
#else
# include <QScriptEngine>
# define QJSEngine QScriptEngine
# define QJSValue QScriptValue
#endif

quint32 Dwarf::ticks_per_day = 1200;
quint32 Dwarf::ticks_per_month = 28 * Dwarf::ticks_per_day;
quint32 Dwarf::ticks_per_season = 3 * Dwarf::ticks_per_month;
quint32 Dwarf::ticks_per_year = 12 * Dwarf::ticks_per_month;

Dwarf::Dwarf(DFInstance *df, const uint &addr, QObject *parent)
    : QObject(parent)
    , m_id(-1)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_address(addr)
    , m_first_soul(0)
    , m_race_id(-1)
    , m_happiness(DH_MISERABLE)
    , m_raw_happiness(0)
    , m_mood_id(-1)
    , m_had_mood(false)
    , m_artifact_name("")
    , m_curse_name("")
    , m_caste_id(-1)
    , m_show_full_name(false)
    , m_total_xp(0)
    , m_migration_wave(0)
    , m_body_size(60000)
    , m_animal_type(none)
    , m_raw_profession(-1)
    , m_can_set_labors(false)
    , m_current_job_id(-1)
    , m_hist_figure(0x0)
    , m_squad_id(-1)
    , m_squad_position(-1)
    , m_pending_squad_name(QString::null)
    , m_age(0)
    , m_noble_position("")
    , m_is_pet(false)
    , m_highest_moodable_skill(-1)
    , m_race(0)
    , m_caste(0)
    , m_pref_tooltip(QString::null)
    , m_thought_desc(QString::null)
    , m_is_child(false)
    , m_is_baby(false)
    , m_is_animal(false)
    , m_true_name("")
    , m_true_birth_year(0)
    , m_validated(false)
    , m_is_valid(false)
    , m_uniform(0x0)
    , m_goals_realized(0)
    , m_worst_rust_level(0)
    , m_curse_type(eCurse::NONE)
{
    read_settings();
    refresh_data();
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));

    // setup context actions
    m_actions_memory.clear();
    QAction *dump_mem = new QAction(tr("Dump Memory..."), this);
    connect(dump_mem, SIGNAL(triggered()), SLOT(dump_memory()));
    m_actions_memory << dump_mem;
    QAction *dump_mem_to_file = new QAction(tr("Dump Memory To File"), this);
    connect(dump_mem_to_file, SIGNAL(triggered()), SLOT(dump_memory_to_file()));
    m_actions_memory << dump_mem_to_file;
    QAction *copy_address_to_clipboard = new QAction(
                tr("Copy Address to Clipboard"), this);
    connect(copy_address_to_clipboard, SIGNAL(triggered()),
            SLOT(copy_address_to_clipboard()));
    m_actions_memory << copy_address_to_clipboard;
}


Dwarf::~Dwarf() {
    qDeleteAll(m_actions_memory);
    m_actions_memory.clear();

    m_traits.clear();
    m_beliefs.clear();
    m_conflicting_beliefs.clear();

    m_skills.clear();
    m_sorted_skills.clear();
    m_attributes.clear();

    m_labors.clear();
    m_pending_labors.clear();
    m_role_ratings.clear();
    m_raw_role_ratings.clear();
    m_sorted_role_ratings.clear();
    m_sorted_custom_role_ratings.clear();
    m_states.clear();

    qDeleteAll(m_preferences);
    m_preferences.clear();
    qDeleteAll(m_grouped_preferences);
    m_grouped_preferences.clear();

    m_thoughts.clear();
    m_syndromes.clear();
    m_inventory_grouped.clear();

    m_hist_figure = 0;
    m_uniform = 0;
    m_race = 0;
    m_caste = 0;
    m_mem = 0;
    m_df = 0;

    disconnect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

Dwarf *Dwarf::get_dwarf(DFInstance *df, const VIRTADDR &addr) {
    MemoryLayout *mem = df->memory_layout();
    TRACE << "attempting to load dwarf at" << addr << "using memory layout" << mem->game_version();

    //only for test
    if(DT->arena_mode){
        Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
        return unverified_dwarf;
    }

    quint32 flags1 = df->read_addr(addr + mem->dwarf_offset("flags1"));
    quint32 flags2 = df->read_addr(addr + mem->dwarf_offset("flags2"));
    quint32 flags3 = df->read_addr(addr + mem->dwarf_offset("flags3"));

    int civ_id = df->read_int(addr + mem->dwarf_offset("civ"));
    int race_id = df->read_int(addr + mem->dwarf_offset("race"));

    TRACE << "examining creature at" << hex << addr;
    TRACE << "FLAGS1 :" << hexify(flags1);
    TRACE << "FLAGS2 :" << hexify(flags2);
    TRACE << "FLAGS3 :" << hexify(flags3);
    TRACE << "RACE   :" << hexify(race_id);

    bool is_caged = flags1 & 0x2000000;
    bool is_tame = flags1 & 0x4000000;

    if(!is_caged){
        if(civ_id != df->dwarf_civ_id()){ //non-animal, but wrong civ
            return 0;
        }
    }else{
        if(!is_tame){
            bool trainable = false;
            //if it's a caged, trainable beast, keep it in our list, but only if it's alive
            Race *r = df->get_race(race_id);
            if(r){
                trainable = r->caste_flag(TRAINABLE);
                r = 0;
            }
            if(!trainable){
                return 0;
            }
        }
    }

    Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
    if(!unverified_dwarf->is_valid()){
        delete unverified_dwarf;
        return 0;
    }else{
        return unverified_dwarf;
    }
}

bool Dwarf::has_invalid_flags(const QString creature_name, QHash<uint, QString> invalid_flags, quint32 dwarf_flags){
    foreach(uint invalid_flag, invalid_flags.uniqueKeys()) {
        QString reason = invalid_flags[invalid_flag];
        if(dwarf_flags & invalid_flag) {
            LOGI << "Ignoring" << creature_name << "who appears to be" << reason;
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

void Dwarf::refresh_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "refresh of dwarf called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of dwarf data at" << hexify(m_address);

    //read only the base information we need to validate if we should continue loading this dwarf
    read_id();
    read_flags();
    //LOGD << "checking unit id:" << m_id;
    read_race();
    read_first_name();
    read_last_name(m_address + m_mem->dwarf_offset("first_name"));
    read_nick_name();
    build_names(); //creates nice name.. which is used for debug messages so we need to do it first..
    read_states();  //read states before job
    read_mood(); //read before skills (soul aspect)
    read_soul();

    // read everything we need, order is important
    if(!m_validated)
        this->is_valid();
    if(m_is_valid){
        read_hist_fig(); //read before noble positions, curse
        read_caste(); //read before age
        read_labors();
        read_happiness();
        read_squad_info(); //read squad before job
        read_uniform();
        read_gender_orientation(); //read before profession
        read_profession(); //read profession before building the names, and before job
        read_current_job();
        read_syndromes(); //read syndromes before attributes
        read_turn_count(); //load time/date stuff for births/migrations - read before age
        set_age_and_migration(m_address + m_mem->dwarf_offset("birth_year"), m_address + m_mem->dwarf_offset("birth_time")); //set age before profession
        read_body_size(); //body size after caste and age
        //curse check will change the name and age
        read_curse(); //read curse before attributes
        read_soul_aspects(); //assumes soul already read, and requires caste to be read first
        read_animal_type(); //need skills loaded to check for hostiles
        read_noble_position();
        read_preferences();

        if(!m_is_animal || DT->user_settings()->value("options/animal_health", false).toBool()){
            bool req_diagnosis = !DT->user_settings()->value("options/diagnosis_not_required", false).toBool();

            //        LOGD << "Loading health for " << m_nice_name;
            m_unit_health = UnitHealth(m_df,this,req_diagnosis);
            //        LOGD << "Finished!";
        }
        read_inventory();

        if(m_is_animal)
            build_names(); //calculate names again as we need to check tameness for animals

        //test reading owned buildings
        /*
        QVector<VIRTADDR> building_addrs = m_df->enumerate_vector(m_address + 0x2e8-0x4);
        foreach(VIRTADDR addr, building_addrs){
            bool is_room = m_df->read_byte(addr+0x7c);
            QString room_name = m_df->read_string(addr+0xc);
            short mat_type = m_df->read_short(addr+0x24);
            short mat_index = m_df->read_short(addr+0x28);
            VIRTADDR vtable = m_df->read_addr(addr);
            int building_type = m_df->read_int(m_df->read_addr(vtable+0x68) + 0x1);
            VIRTADDR val_addr = m_df->read_addr(vtable+0xa8);
            int room_value = m_df->read_int(val_addr + 0x1);
            //            LOGD << "owns building " << building->display_name();
            //        }
            QVector<VIRTADDR> stored_items_addrs = m_df->enumerate_vector(addr + 0xec -0x4);
            foreach(VIRTADDR item_addr, stored_items_addrs){
                Item *stored = new Item(m_df, m_df->read_addr(item_addr), this);
                LOGD << "    - " << stored->display_name();
            }
        }
         */

    }

    TRACE << "finished refresh of dwarf data for dwarf:" << m_nice_name << "(" << m_translated_name << ")";
}

void Dwarf::refresh_minimal_data(){
    if(m_validated){
        read_flags(); //butcher/caged

        read_nick_name();
        read_labors();

        read_profession();

        build_names();
    }
}

bool Dwarf::is_valid(){
    if(m_validated)
        return m_is_valid;

    if (m_mem->is_complete()) {
        if(!m_is_animal){ //fortress civilian

            //check for migrants (which aren't dead/killed/ghosts)
            if(this->state_value(7) > 0
                    && !(m_unit_flags.at(0) & 0x2)
                    && !(m_unit_flags.at(1) & 0x80)
                    && !(m_unit_flags.at(2) & 0x1000)){
                LOGI << "Found migrant " << this->nice_name();
                m_validated = true;
                m_is_valid = true;
                return true;
            }

            //if a dwarf has gone crazy (berserk=7,raving=6,melancholy=5)
            int m_mood = this->m_mood_id;
            if(m_mood==7 || m_mood==6 || m_mood==5){
                LOGI << "Ignoring " << this->nice_name() << "who appears to have lost their mind.";
                m_validated = true;
                m_is_valid = false;
                return false;
            }

            //check opposed to life
            if(m_curse_flags & eCurse::OPPOSED_TO_LIFE){
                LOGI << "Ignoring " << this->nice_name() << " who appears to be a hostile undead!";
                m_validated = true;
                m_is_valid = false;
                return false;
            }

        }else{ //tame or caged animals
            if (m_unit_flags.at(0) & 0x6000000) {
                //exclude cursed animals, this may be unnecessary with the civ check
                //the full curse information hasn't been loaded yet, so just read the curse name
                QString curse_name = m_df->read_string(m_address + m_mem->dwarf_offset("curse"));
                if(!curse_name.isEmpty()){
                    LOGI << "Ignoring animal " << this->nice_name() << "who appears to be cursed or undead";
                    m_validated = true;
                    m_is_valid = false;
                    return false;
                }
            }
        }

        //check other invalid flags (invaders, ghosts, dead, merchants, etc.)
        if(has_invalid_flags(this->nice_name(), m_mem->invalid_flags_1(),m_unit_flags.at(0)) ||
                has_invalid_flags(this->nice_name(), m_mem->invalid_flags_2(),m_unit_flags.at(1)) ||
                has_invalid_flags(this->nice_name(), m_mem->invalid_flags_3(),m_unit_flags.at(2))){
            m_validated = true;
            m_is_valid = false;
            return false;
        }

        if(!this->m_first_soul){
            LOGI << "Ignoring" << this->nice_name() <<
                    "who appears to be soulless.";
            m_validated = true;
            m_is_valid = false;
            return false;
        }
        m_validated = true;
        m_is_valid = true;
        return true;
    }else{
        m_validated = true;
        m_is_valid = false;
        return false;
    }
}

void Dwarf::set_age_and_migration(VIRTADDR birth_year_offset, VIRTADDR birth_time_offset){
    m_birth_year = m_df->read_int(birth_year_offset);
    m_age = m_df->current_year() - m_birth_year;
    m_birth_time = m_df->read_int(birth_time_offset);
    quint32 current_year_time = m_df->current_year_time();
    quint32 current_time = m_df->current_time();
    quint32 arrival_time = current_time - m_turn_count;
    quint32 arrival_year = arrival_time / ticks_per_year;
    quint32 arrival_season = (arrival_time % ticks_per_year) / ticks_per_season;
    quint32 arrival_month = (arrival_time % ticks_per_year) / ticks_per_month;
    quint32 arrival_day = ((arrival_time % ticks_per_year) % ticks_per_month) / ticks_per_day;
    m_ticks_since_birth = m_age * ticks_per_year + current_year_time - m_birth_time;
    //this way we have the right sort order and all the data needed for the group by migration wave
    m_migration_wave = 100000 * arrival_year + 10000 * arrival_season + 100 * arrival_month + arrival_day;
    m_born_in_fortress = (m_ticks_since_birth == m_turn_count);

    m_age_in_months = m_ticks_since_birth / ticks_per_month;

    if(m_caste){
        if(m_age == 0 || m_ticks_since_birth < m_caste->baby_age() * ticks_per_year)
            m_is_baby = true;
        else if(m_ticks_since_birth < m_caste->child_age() * ticks_per_year)
            m_is_child = true;
    }
}

QString Dwarf::get_migration_desc(){
    qint32 wave = m_migration_wave;
    quint32 season = 0;
    quint32 year = 0;
    quint32 month = 0;
    quint32 day = 0;
    QString suffix = "th";

    day = (wave % 100) + 1;
    month = (wave / 100) % 100;
    season = (wave / 10000) % 10;
    year = wave / 100000;

    if ((day == 1) || (day == 21))
        suffix = "st";
    else if ((day == 2) || (day == 22))
        suffix = "nd";
    else if ((day == 3) || (day == 23))
        suffix = "rd";

    if(m_born_in_fortress){
        return QString("Born on the %1%4 of %2 in the year %3").arg(day).arg(GameDataReader::ptr()->m_months.at(month)).arg(year).arg(suffix);
    }else{
        return QString("Arrived in the %2 of the year %1").arg(year).arg(GameDataReader::ptr()->m_seasons.at(season));
    }
}

/*******************************************************************************
  DATA POPULATION METHODS
*******************************************************************************/

void Dwarf::read_id() {
    m_id = m_df->read_int(m_address + m_mem->dwarf_offset("id"));
    TRACE << "UNIT ID:" << m_id;
}

void Dwarf::read_hist_fig(){
    m_hist_figure = new HistFigure(m_df->read_int(m_address + m_mem->dwarf_offset("hist_id")),m_df,this);
    TRACE << "HIST_FIG_ID:" << m_hist_figure->id();
}

void Dwarf::read_gender_orientation() {
    BYTE sex = m_df->read_byte(m_address + m_mem->dwarf_offset("sex"));
    TRACE << "GENDER:" << sex;
    m_gender_info.gender = static_cast<GENDER_TYPE>(sex);
    m_gender_info.orientation = ORIENT_HETERO; //default

    QStringList icon_name;

    if(m_gender_info.gender == SEX_UNK){
        icon_name.append("question-white");
    }else if(m_gender_info.gender == SEX_M){
        icon_name.append("male");
    }else{
        icon_name.append("female");
    }

    int orient_offset = m_mem->soul_detail("orientation");
    if(m_gender_info.gender != SEX_UNK && m_first_soul && orient_offset != -1){
        quint32 orientation = m_df->read_addr(m_first_soul + orient_offset);
        m_gender_info.male_interest = orientation & (1 << 1);
        m_gender_info.male_commit = orientation & (1 << 2);
        m_gender_info.female_interest = orientation & (1 << 3);
        m_gender_info.female_commit = orientation & (1 << 4);

        //check the commitment/breed bits first. this determines animal breeding, and will override the interest bit, seemingly
        //for example, a male dwarf, with interest in males and commitment to females will marry a female
        icon_name.append(get_gender_icon_suffix(m_gender_info.male_commit,m_gender_info.female_commit));
    }
    icon_name.removeAll("");
    m_icn_gender = QString(":img/%1.png").arg(icon_name.join("-"));

    QStringList gender_desc;
    gender_desc << get_gender_desc(m_gender_info.gender) << get_orientation_desc(m_gender_info.orientation);
    gender_desc.removeAll("");
    m_gender_info.full_desc = gender_desc.join(" - ");
}

QString Dwarf::get_gender_icon_suffix(bool male_flag, bool female_flag, bool checking_interest){
    QString suffix = "";
    if(male_flag && female_flag){
        suffix ="bi";
        m_gender_info.orientation = ORIENT_BISEXUAL;
    }else if(male_flag && m_gender_info.gender == SEX_M){
        suffix ="male";
        m_gender_info.orientation = ORIENT_HOMO;
    }else if(female_flag && m_gender_info.gender == SEX_F){
        suffix ="female";
        m_gender_info.orientation = ORIENT_HOMO;
    }else if(!male_flag && !female_flag){
        if(m_is_animal || checking_interest){
            suffix ="asexual";
            m_gender_info.orientation = ORIENT_ASEXUAL;
        }else{
            return get_gender_icon_suffix(m_gender_info.male_interest,m_gender_info.female_interest,true);
        }
    }
    return suffix;
}

void Dwarf::read_mood(){
    m_mood_id = m_df->read_short(m_address + m_mem->dwarf_offset("mood"));

    if(m_mood_id < 0){
        //also mark if they've had a mood, if they're not IN a mood
        if(m_unit_flags.at(0) & 0x8){
            m_had_mood = true;
            m_artifact_name = m_df->get_translated_word(m_address + m_mem->dwarf_offset("artifact_name"));
            m_highest_moodable_skill = m_df->read_short(m_address +m_mem->dwarf_offset("mood_skill"));
        }
    }
}

void Dwarf::read_body_size(){
    //actual size of the creature
    int offset = m_mem->dwarf_offset("size_info");
    if(offset){
        m_body_size = m_df->read_int(m_address + offset);
    }else{
        LOGW << "Missing size_info offset!";
        m_body_size = body_size(true);
    }
}

int Dwarf::body_size(bool use_default){
    //we include returning the default size because for the weapon columns, the size actually doesn't matter to DF (bug?)
    if(use_default){
        int def_size = 6000; //default adult size for a dwarf
        if(m_is_baby)
            def_size = 300;
        else if(m_is_child)
            def_size = 1500;
        return def_size;
    }else{
        return m_body_size;
    }
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
        QVector<uint> entries = m_df->enumerate_vector(states_addr);
        foreach(uint entry, entries) {
            m_states.insert(m_df->read_short(entry), m_df->read_short(entry+0x4));
        }
    }
}

void Dwarf::read_curse(){
    QString curse_name = capitalizeEach(m_df->read_string(m_address + m_mem->dwarf_offset("curse")));

    if(!curse_name.isEmpty()){
        m_curse_type = eCurse::OTHER;

        //keep track of the curse type; currently vampires and werebeasts are the only truly cursed creatures we
        //want to be aware of for the purpose of highlighting them

        //TODO: check for removed flags as well
        if(m_curse_flags & eCurse::BLOODSUCKER){ //if(!has_flag(eCurse::BLOODSUCKER, m_curse_rem_flags) && has_flag(eCurse::BLOODSUCKER,m_curse_flags)){
            //if it's a vampire then find the vampire's fake identity and use that name/age instead to match DF
            find_true_ident();
            m_curse_type = eCurse::VAMPIRE;
        }

        m_curse_name = curse_name;
    }
}

void Dwarf::find_true_ident(){
    if(m_hist_figure->has_fake_identity()){
        //save the true name for display
        m_true_name = m_nice_name;
        m_true_birth_year = m_birth_year;
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
    return m_hist_figure->id();
}

HistFigure* Dwarf::hist_figure(){
    return m_hist_figure;
}

void Dwarf::read_caste() {
    m_caste_id = m_df->read_short(m_address + m_mem->dwarf_offset("caste"));
    m_caste = m_race->get_caste_by_id(m_caste_id);
    TRACE << "CASTE:" << m_caste_id;
}

void Dwarf::read_flags(){
    m_unit_flags.clear();
    quint32 flags1 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags1"));
    quint32 flags2 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags2"));
    quint32 flags3 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags3"));
    m_unit_flags << flags1 << flags2 << flags3;

    //currently only flags used are for cage and butcher animal
    m_caged = flags1;
    m_butcher = flags2;

    m_curse_flags = m_df->read_addr(m_address + m_mem->dwarf_offset("curse_add_flags1"));
    //    m_curse_flags2 = m_df->read_addr(m_address + m_mem->dwarf_offset("curse_add_flags2"));
}

void Dwarf::read_race() {
    m_race_id = m_df->read_int(m_address + m_mem->dwarf_offset("race"));
    m_race = m_df->get_race(m_race_id);
    TRACE << "RACE ID:" << m_race_id;
    m_is_animal = m_df->dwarf_race_id()!=m_race_id;
}

void Dwarf::read_first_name() {
    m_first_name = m_df->read_string(m_address + m_mem->dwarf_offset("first_name"));
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
    m_nick_name = m_df->read_string(m_address + m_mem->dwarf_offset("nick_name"));
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

    if(m_is_animal){
        QString creature_name = "";
        if(is_adult()){
            creature_name = caste_name();
            //for adult animals, check their profession for training and adjust the name accordingly
            if (m_raw_profession == 99) //trained war
                creature_name = tr("War ") + m_race->name();
            else if (m_raw_profession == 98) //trained hunt
                creature_name = tr("Hunting ") + m_race->name();
        }else{
            creature_name = race_name();
        }

        //check for a pet name
        if(m_nice_name==""){
            m_nice_name = creature_name;
            m_translated_name = "";
        }else{
            m_nice_name = tr("%1 (%2)").arg(creature_name).arg(m_nice_name);
        }
    }
    // uncomment to put address at front of name
    //m_nice_name = QString("0x%1 %2").arg(m_address, 8, 16, QChar('0')).arg(m_nice_name);
    // uncomment to put internal ID at front of name
    //m_nice_name = QString("%1 %2").arg(m_id).arg(m_nice_name);

    /*
    bool debugging_labors = true;
    if (debugging_labors) {
        // find first labor that is on for a dwarf...
        uchar buf[150];
        memset(buf, 0, 150);
        m_df->read_raw(m_address + m_df->memory_layout()->dwarf_offset("labors"), 150, &buf);
        for(int i = 0; i < 150; ++i) {
            if (buf[i] > 0) {
                m_nice_name = QString("%1 - %2").arg(m_nice_name).arg(i);
                break;
            }
        }
    }
    */
}

void Dwarf::read_profession() {
    // first see if there is a custom prof set...
    VIRTADDR custom_addr = m_address + m_mem->dwarf_offset("custom_profession");
    m_custom_profession = m_df->read_string(custom_addr);
    TRACE << "\tCUSTOM PROF:" << m_custom_profession;

    // we set both to the same to know it hasn't been edited yet
    m_pending_custom_profession = m_custom_profession;

    // now read the actual profession by id
    VIRTADDR addr = m_address + m_mem->dwarf_offset("profession");
    m_raw_profession = m_df->read_byte(addr);
    Profession *p = GameDataReader::ptr()->get_profession(m_raw_profession);
    QString prof_name = tr("Unknown Profession %1").arg(m_raw_profession);
    if (p) {
        m_can_set_labors = p->can_assign_labors();
        prof_name = p->name(is_male());
    } else {
        LOGE << tr("Read unknown profession with id '%1' for dwarf '%2'")
                .arg(m_raw_profession).arg(m_nice_name);
        m_can_set_labors = false;
    }
    if (!m_custom_profession.isEmpty()) {
        m_profession =  m_custom_profession;
    } else {
        m_profession = prof_name;
    }

    if(is_animal()){
        if(m_raw_profession == 102 && is_adult())
            m_profession = tr("Adult"); //adult animals have a profession of peasant by default, just use adult
        else if(m_is_child){
            m_profession = tr("Child");
            m_raw_profession = 103;
        }else if(m_is_baby){
            m_profession = tr("Baby");
            m_raw_profession = 104;
        }
    }

    int img_idx = 102; //default to peasant
    if(m_raw_profession > -1 && m_raw_profession < GameDataReader::ptr()->get_professions().count())
        img_idx = m_raw_profession + 1; //images start at 1, professions at 0, offest to match image

    //set the default path for the profession icon
    m_icn_prof = QPixmap(":/profession/prof_" + QString::number(img_idx) + ".png");
    //see if we have a custom profession or icon override
    CustomProfession *cp;
    if(!m_custom_profession.isEmpty()){
        cp = DT->get_custom_profession(m_custom_profession);
    }else{
        cp = DT->get_custom_prof_icon(m_raw_profession);
    }
    if(cp && cp->has_icon())
        m_icn_prof = cp->get_pixmap();

    LOGD << "reading profession for" << nice_name() << m_raw_profession << prof_name;
    TRACE << "EFFECTIVE PROFESSION:" << m_profession;
}

void Dwarf::read_noble_position(){
    m_noble_position = m_df->fortress()->get_noble_positions(m_hist_figure->id(),is_male());
}
void Dwarf::read_preferences(){
    if(m_is_animal)
        return;
    QVector<VIRTADDR> preferences = m_df->enumerate_vector(m_first_soul + m_mem->soul_detail("preferences"));
    int pref_type;
    int pref_id;
    int item_sub_type;
    short mat_type;
    int mat_index;

    QString pref_name = "Unknown";
    ITEM_TYPE itype;
    PREF_TYPES ptype;
    Preference *p;
    foreach(VIRTADDR pref, preferences){
        pref_type = m_df->read_short(pref);
        pref_id = m_df->read_short(pref + 0x2);
        item_sub_type = m_df->read_short(pref + 0x4);
        mat_type = m_df->read_short(pref + 0x6);
        mat_index = m_df->read_int(pref + 0x8);
        //short pref_string = m_df->read_short(pref + 0x10);

        itype = static_cast<ITEM_TYPE>(pref_id);
        ptype = static_cast<PREF_TYPES>(pref_type);

        p = new Preference(ptype, pref_name, this);

        //for each preference type, we have some flags we need to check and add so we get matches to the role's preferences
        //materials are the exception as all flags are passed in, moving forward it may be better to pass in flagarrays instead

        switch(pref_type){
        case 0: //like material
        {
            pref_name = m_df->find_material_name(mat_index,mat_type,NONE);
            Material *m = m_df->find_material(mat_index,mat_type);
            if(m && m->id() >= 0){
                p->set_pref_flags(m->flags());
            }
        }
            break;
        case 1: //like creature
        {
            Race* r = m_df->get_race(pref_id);
            if(r){
                pref_name = r->plural_name();
                p->set_pref_flags(r);
            }
        }
            break;
        case 2: //like food/drink
        {
            if(mat_index < 0 || itype==MEAT){
                if(itype==FISH)
                    mat_index = mat_type;
                Race* r = m_df->get_race(mat_index);
                if(r){
                    pref_name = r->name().toLower();
                }
            }else{
                pref_name = m_df->find_material_name(mat_index,mat_type,itype);
            }
            p->set_item_type(itype);
        }
            break;
        case 3: //hate creature
        {
            Race* r = m_df->get_race(pref_id);
            if(r)
                pref_name = r->plural_name();
        }
            break;
        case 4: //like item
        {
            p->set_item_type(itype);
            pref_name = m_df->get_preference_item_name(pref_id,item_sub_type);
            if(item_sub_type >= 0 && Item::has_subtypes(itype)){
                ItemSubtype *s = m_df->get_item_subtype(itype,item_sub_type);
                if(s){
                    p->set_pref_flags(s->flags());
                }
            }else if(Item::is_trade_good(itype)){
                p->add_flag(IS_TRADE_GOOD);
            }
        }
            break;
        case 5: //like plant
        {
            Plant *plnt = m_df->get_plant(pref_id);
            if(plnt){
                pref_name = plnt->name_plural();
                p->set_pref_flags(plnt->flags());
            }
        }
            break;
        case 6: //like tree
        {
            Plant *plnt = m_df->get_plant(pref_id);
            if (plnt)
                pref_name = plnt->name_plural();
        }
            break;
        case 7: //like color
        {
            pref_name = m_df->get_color(pref_id);
        }
            break;
        case 8: //like shape
        {
            pref_name = m_df->get_shape(pref_id);
        }
            break;

        }
        p->set_name(capitalize(pref_name));
        if(!pref_name.isEmpty())
            m_preferences.insert(pref_type, p);
        //        if(itype < NUM_OF_TYPES && itype != NONE)
        //            LOGW << pref_name << " " << (int)itype << " " << Item::get_item_desc(itype);

        m_pref_names.append(pref_name);
    }

    //add a special preference (actually a misc trait) for like outdoors
    if(has_state(14)){
        int val = state_value(14);
        QString pref = tr("Doesn't mind being outdoors");
        if(val == 2)
            pref = tr("Likes working outdoors");

        p = new Preference(LIKE_OUTDOORS,pref,this);
        p->add_flag(999);
        m_preferences.insert(LIKE_OUTDOORS,p);
    }

    bool build_tooltip = (!m_is_animal && !m_preferences.isEmpty() && DT->user_settings()->value("options/tooltip_show_preferences",true).toBool());
    //group preferences into pref desc - values (string list)
    QString desc_key;
    pref_name = "";
    //lists for the tooltip
    QStringList likes;
    QStringList consume;
    QStringList hates;
    QStringList other;
    foreach(int key, m_preferences.uniqueKeys()){
        QMultiMap<int, Preference *>::iterator i = m_preferences.find(key);
        while(i != m_preferences.end() && i.key() == key){
            PREF_TYPES pType = static_cast<PREF_TYPES>(key);
            desc_key = Preference::get_pref_desc(pType);
            pref_name = static_cast<Preference*>(i.value())->get_name();
            if(!pref_name.isEmpty()){
                //create/append to groups based on the categories description
                if(!m_grouped_preferences.contains(desc_key))
                    m_grouped_preferences.insert(desc_key, new QStringList);
                m_grouped_preferences.value(desc_key)->append(pref_name);
                //build the tooltip at the same time, organizing by likes, dislikes
                if(build_tooltip){
                    pref_name = pref_name.toLower();
                    if(pType == LIKE_ITEM || pType == LIKE_MATERIAL || pType == LIKE_PLANT || pType == LIKE_TREE || pType == LIKE_CREATURE){
                        likes.append(pref_name);
                    }else if(pType == LIKE_FOOD){
                        consume.append(pref_name);
                    }else if(pType == LIKE_COLOR){
                        likes.append(tr("the color ").append(pref_name));
                    }else if(pType == LIKE_SHAPE){
                        likes.append(tr("the shape of ").append(pref_name));
                    }else if(pType == HATE_CREATURE){
                        hates.append(pref_name);
                    }else{
                        other.append(capitalize(pref_name));
                    }
                }
            }
            i++;
        }
    }
    if(build_tooltip){
        m_pref_tooltip = tr("<b>Preferences: </b>");
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
    QVector<VIRTADDR> active_unit_syns = m_df->enumerate_vector(m_address + m_mem->dwarf_offset("active_syndrome_vector"));
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
                Caste *c_trans = r_trans->get_caste_by_id(0);
                if(r_trans && r_trans->flags().has_flag(NIGHT_CREATURE) && c_trans && c_trans->flags().has_flag(CRAZED)){
                    is_curse = true;
                    m_curse_type = eCurse::WEREBEAST;
                    m_curse_name = r_trans->name();
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
    //    qSort(m_syndromes.begin(),m_syndromes.end(),&Syndrome::sort_date_time);
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
    VIRTADDR addr = m_address + m_mem->dwarf_offset("labors");
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

void Dwarf::read_happiness() {
    int offset = m_mem->dwarf_offset("happiness");

    if(offset){
        VIRTADDR addr = m_address + offset;
        m_raw_happiness = m_df->read_int(addr);
    }else{
        m_raw_happiness = 0;
    }

    m_happiness = happiness_from_score(m_raw_happiness);
    TRACE << "\tRAW HAPPINESS:" << m_raw_happiness;
    TRACE << "\tHAPPINESS:" << happiness_name(m_happiness);

    if(!is_animal()){
        offset = m_mem->dwarf_offset("thoughts");
        if(offset){
            QVector<VIRTADDR> thoughts = m_df->enumerate_vector(m_address + offset);
            //time, id
            QMap<int,short> t;
            foreach(VIRTADDR addr, thoughts){
                short id = m_df->read_int(addr);
                int time = m_df->read_int(addr + 0x4);
                //the age of a thought increases by 1 per frame
                //to find how many days ago a thought was use: 10 frames per tick, 1200 ticks per day
                t.insertMulti(time,id);
            }

            int t_count = 0;
            foreach(int key, t.uniqueKeys()){
                QList<short> vals = t.values(key);
                for(int i = 0; i < vals.count(); i++) {
                    if(!m_thoughts.contains(vals.at(i)))
                        t_count = 1;
                    else
                        t_count = m_thoughts.take(vals.at(i)) + 1;

                    m_thoughts.insert(vals.at(i),t_count);
                }
            }

            QStringList display_desc;
            foreach(int id, m_thoughts.uniqueKeys()){
                Thought *t = GameDataReader::ptr()->get_thought(id);
                if(t){
                    t_count = m_thoughts.value(id);
                    display_desc.append(QString("<font color=%1>%2%3</font>")
                                        .arg(t->color().name())
                                        .arg(t->desc().toLower())
                                        .arg(t_count > 1 ? QString(" (x%1)").arg(t_count) : ""));
                }
            }

            m_thought_desc = display_desc.join(", ");
            if(m_thoughts.count() > 0){
                int index = m_thought_desc.indexOf(">") + 1;
                m_thought_desc[index] = m_thought_desc[index].toUpper();
            }
        }
    }
}

void Dwarf::read_current_job() {
    // TODO: jobs contain info about materials being used, if we ever get the
    // material list we could show that in here
    VIRTADDR addr = m_address + m_mem->dwarf_offset("current_job");
    VIRTADDR current_job_addr = m_df->read_addr(addr);

    m_current_sub_job_id.clear();

    TRACE << "Current job addr: " << hex << current_job_addr;

    if (current_job_addr != 0) {
        m_current_job_id = m_df->read_short(current_job_addr + m_mem->job_detail("id"));

        //if drinking blood and we're not showing vamps, change job to drink
        if (m_current_job_id == 223 && DT->user_settings()->value("options/highlight_cursed", false).toBool()==false)
            m_current_job_id = 17;

        DwarfJob *job = GameDataReader::ptr()->get_job(m_current_job_id);
        if (job) {
            m_current_job = job->description;

            int sub_job_offset = m_mem->job_detail("sub_job_id");
            if(sub_job_offset != -1) {
                m_current_sub_job_id = m_df->read_string(current_job_addr + sub_job_offset); //reaction name
                if(!job->reactionClass.isEmpty() && !m_current_sub_job_id.isEmpty()) {
                    Reaction* reaction = m_df->get_reaction(m_current_sub_job_id);
                    if(reaction!=0) {
                        m_current_job = capitalize(reaction->name());
                        TRACE << "Sub job: " << m_current_sub_job_id << m_current_job;
                    }
                }
            }

            if(m_current_sub_job_id.isEmpty()){
                QString material_name;
                int mat_index = m_df->read_int(current_job_addr + m_mem->job_detail("mat_index"));
                short mat_type = m_df->read_short(current_job_addr + m_mem->job_detail("mat_type"));
                if(mat_index >= 0 || mat_type >= 0){
                    material_name = m_df->find_material_name(mat_index ,mat_type, NONE);
                }
                if(material_name.isEmpty()){
                    quint32 mat_category = m_df->read_addr(current_job_addr + m_mem->job_detail("mat_category"));
                    material_name = DwarfJob::get_job_mat_category_name(mat_category);
                }
                if(!material_name.isEmpty())
                    m_current_job.replace("??",capitalize(material_name));
            }

        } else {
            m_current_job = tr("Unknown job");
        }

    } else {
        if(active_military()){
            m_current_job = tr("Soldier");
            m_current_job_id = -1;
        }else{
            short on_break_value = m_mem->job_detail("on_break_flag");
            m_is_on_break = has_state(on_break_value);
            m_current_job = m_is_on_break ? tr("On Break") : tr("No Job");
            if(m_is_on_break)
                m_current_job_id = -2;
            else
                m_current_job_id = -3;
        }
    }
    m_current_job = capitalizeEach(m_current_job);
    TRACE << "CURRENT JOB:" << m_current_job_id << m_current_sub_job_id << m_current_job;
}

void Dwarf::read_soul(){
    VIRTADDR soul_vector = m_address + m_mem->dwarf_offset("souls");
    QVector<VIRTADDR> souls = m_df->enumerate_vector(soul_vector);
    if (souls.size() != 1) {
        LOGI << nice_name() << "has" << souls.size() << "souls!";
        return;
    }
    m_first_soul = souls.at(0);
}

void Dwarf::read_soul_aspects() {
    if(!m_first_soul)
        read_soul();

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
    if (!m_custom_profession.isEmpty())
        return m_custom_profession;
    return m_profession;
}

bool Dwarf::active_military() {
    Profession *p = GameDataReader::ptr()->get_profession(m_raw_profession);
    return p && p->is_military();
}

DWARF_HAPPINESS Dwarf::happiness_from_score(int score) {
    if (score < 1)
        return DH_MISERABLE;
    else if (score <= 25)
        return DH_VERY_UNHAPPY;
    else if (score <= 50)
        return DH_UNHAPPY;
    else if (score <= 75)
        return DH_FINE;
    else if (score <= 125)
        return DH_CONTENT;
    else if (score <= 150)
        return DH_HAPPY;
    else
        return DH_ECSTATIC;
}

QString Dwarf::caste_name(bool plural_name) {
    QString tmp_name = "Unknown";
    if(m_caste){
        if(plural_name)
            tmp_name = m_caste->name_plural();
        else
            tmp_name = m_caste->name();
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

    if(m_is_baby){
        if(!plural_name)
            r_name = m_race->baby_name();
        else
            r_name = m_race->baby_name_plural();
    }else if (m_is_child){
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



bool Dwarf::get_flag_value(int bit)
{
    //m_caged is the flags1
    quint32 flg = m_caged;
    if(bit>31)
    {
        //the slaughter flag is in flags2
        bit-=32;
        flg=m_butcher;
    }
    quint32 mask = 1;
    for (int i=0;i<bit;i++)
        mask<<=1;
    return ((flg & mask)==mask);
}

void Dwarf::read_squad_info() {
    m_squad_id = m_df->read_int(m_address + m_mem->dwarf_offset("squad_id"));
    m_pending_squad_id = m_squad_id;
    m_squad_position = m_df->read_int(m_address + m_mem->dwarf_offset("squad_position"));
    m_pending_squad_position = m_squad_position;
    if(m_pending_squad_id >= 0 && !m_is_animal && is_adult()){
        Squad *s = m_df->get_squad(m_pending_squad_id);
        if(s)
            m_pending_squad_name = s->name();
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
            short sub_type = -1;
            short job_skill = -1;
            m_uniform = new Uniform(m_df,this);
            if(labor_enabled(0)){//mining
                job_skill = 0; //mining skill
                m_uniform->add_uniform_item(WEAPON,sub_type,job_skill);
            }else if(labor_enabled(10)){//woodcutting
                job_skill = 38; //axe skill
                m_uniform->add_uniform_item(WEAPON,sub_type,job_skill);
            }else if(labor_enabled(44)){//hunter
                //add a weapon
                job_skill = 44; //crossbow skill
                m_uniform->add_uniform_item(WEAPON,sub_type,job_skill);
                //add quiver
                m_uniform->add_uniform_item(QUIVER,sub_type,-1);
                //add ammo
                m_uniform->add_uniform_item(AMMO,sub_type,-1);
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

    QVector<VIRTADDR> used_items = m_df->enumerate_vector(m_address + m_mem->dwarf_offset("used_items_vector"));
    QHash<int,int> item_affection;
    foreach(VIRTADDR item_used, used_items){
        item_affection.insert(m_df->read_int(item_used),m_df->read_int(item_used+m_mem->dwarf_offset("affection_level")));
    }

    short inv_type = -1;
    short bp_id = -1;
    QString category_name = "";
    int inv_count = 0;
    bool include_mat_name = DT->user_settings()->value("options/docks/equipoverview_include_mats",false).toBool();
    foreach(VIRTADDR inventory_item_addr, m_df->enumerate_vector(m_address + m_mem->dwarf_offset("inventory"))){
        inv_type = m_df->read_short(inventory_item_addr + m_mem->dwarf_offset("inventory_item_mode"));
        bp_id = m_df->read_short(inventory_item_addr + m_mem->dwarf_offset("inventory_item_bodypart"));

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
                LOGD << "  + found weapon:" << iw->display_name(false);
            }else if(Item::is_armor_type(i_type)){
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
                if(wear_level > 0 && Item::is_armor_type(i_type,false)){
                    QString item_name = QString("%1 %2").arg((include_mat_name ? ir->get_material_name_base() : "")).arg(ir->get_details()->name_plural()).trimmed();
                    QPair<QString,int> key = qMakePair(item_name,wear_level);
                    if(m_equip_warnings.contains(key)){
                        m_equip_warnings[key] += ir->get_stack_size();
                    }else{
                        m_equip_warnings.insert(key,ir->get_stack_size());
                    }
                }

                LOGD << "  + found armor/clothing:" << ir->display_name(false);
            }else if(Item::is_supplies(i_type) || Item::is_ranged_equipment(i_type)){
                process_inv_item(category_name,i);
                LOGD << "  + found supplies/ammo/quiver:" << i->display_name(false);
            }else{
                LOGD << "  + found other item:" << i->display_name(false);
            }

            //process any items inside this item (ammo, food?, drink?)
            if(i->contained_items().count() > 0){
                foreach(Item *contained_item, i->contained_items()){
                    process_inv_item(category_name,contained_item,true);
                    LOGD << "    + contained item(s):" << contained_item->display_name(false);
                }
            }
            inv_count++;
        }else{
            LOGD << "  - skipping inventory item due to invalid type (" + QString::number(inv_type) + ")";
        }
    }
    LOGD << "  total inventory items found:" << inv_count;

    //missing uniform items
    if(m_uniform && m_uniform->get_missing_items().count() > 0){
        QString cat_name = Item::missing_group_name();
        foreach(ITEM_TYPE itype, m_uniform->get_missing_items().uniqueKeys()){
            foreach(ItemDefUniform *u, m_uniform->get_missing_items().value(itype)){
                Item *i = new Item(m_df,u,this);
                if(itype == WEAPON){
                    ItemWeapon *iw = new ItemWeapon(*i);
                    process_inv_item(cat_name,iw);
                }else if(itype == AMMO){ //check before ranged equipment to cast it properly
                    ItemAmmo *ia = new ItemAmmo(*i);
                    process_inv_item(cat_name,ia);
                }else if(Item::is_armor_type(itype)){
                    ItemArmor *ir = new ItemArmor(*i);
                    process_inv_item(cat_name,ir);
                }else if(Item::is_supplies(itype) || Item::is_ranged_equipment(itype)){
                    process_inv_item(cat_name,i);
                }
            }
        }
    }

    //ensure babies and animals have 100 coverage rating as they don't wear anything
    if(m_is_baby || m_is_animal){
        has_pants = true;
        has_shirt = true;
        shoes_count = 2;
    }

    //set our coverage ratings. currently this is only important for the 3 clothing types that, if missing, give bad thoughts
    QString title = "";
    if(has_pants){
        m_coverage_ratings.insert(PANTS,100.0f);
    }else{
        m_coverage_ratings.insert(PANTS,0.0f);
        m_missing_counts.insert(PANTS,1);
        title = tr("Legs Uncovered!");
        process_inv_item(Item::uncovered_group_name(),new Item(PANTS,title,this));
        m_equip_warnings.insert(qMakePair(title,-1),1);
    }
    if(has_shirt){
        m_coverage_ratings.insert(ARMOR,100.0f);
    }else{
        m_coverage_ratings.insert(ARMOR,0);
        m_missing_counts.insert(ARMOR,1);
        title = tr("Torso Uncovered!");
        process_inv_item(Item::uncovered_group_name(),new Item(ARMOR,title,this));
        m_equip_warnings.insert(qMakePair(title,-1),1);
    }
    //for shoes, compare how many they're wearing compared to how many legs they've still got
    float limb_count =  m_unit_health.limb_count();
    float foot_coverage = 100.0f;
    if(limb_count > 0)
        foot_coverage = shoes_count / limb_count * 100.0f;
    if(foot_coverage < 100.0f){
        title = tr("Feet Uncovered!");
        process_inv_item(Item::uncovered_group_name(),new Item(SHOES,title,this));
        int count = limb_count - shoes_count;
        m_missing_counts.insert(SHOES,count);
        m_equip_warnings.insert(qMakePair(title,-1),count);
    }
    m_coverage_ratings.insert(SHOES,foot_coverage);
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
    m_turn_count = m_df->read_int(m_address + m_mem->dwarf_offset("turn_count"));
    TRACE << "Turn Count:" << m_turn_count;
}

void Dwarf::read_skills() {
    VIRTADDR addr = m_first_soul + m_mem->soul_detail("skills");
    m_total_xp = 0;
    m_skills.clear();
    m_sorted_skills.clear();
    QVector<VIRTADDR> entries = m_df->enumerate_vector(addr);
    TRACE << "Reading skills for" << nice_name() << "found:" << entries.size();
    short type = 0;
    short rating = 0;
    int xp = 0;
    int rust = 0;

    int skill_rate = 100;
    if(m_caste)
        m_caste->load_skill_rates();

    foreach(VIRTADDR entry, entries) {
        type = m_df->read_short(entry);
        rating = m_df->read_short(entry + 0x04);
        xp = m_df->read_int(entry + 0x08);
        rust = m_df->read_int(entry + 0x10);

        //find the caste's skill rate
        if(m_caste){
            skill_rate = m_caste->get_skill_rate(type);
        }

        Skill s = Skill(type, xp, rating, rust, skill_rate);
        //calculate the values we'll need for roles immediately
        if(!m_is_animal && !m_is_baby){
            s.calculate_balanced_level();
        }

        m_total_xp += s.actual_exp();
        m_skills.insert(s.id(),s);
        m_sorted_skills.insertMulti(s.capped_level_precise(), s.id());

        if(!m_had_mood){
            if(GameDataReader::ptr()->moodable_skills().contains(type) &&
                    (m_highest_moodable_skill == -1 || s.actual_exp() > get_skill(m_highest_moodable_skill).actual_exp()))
                m_highest_moodable_skill = type;
        }

        if(s.rust_level() > m_worst_rust_level)
            m_worst_rust_level = s.rust_level();
    }
}

void Dwarf::read_personality() {
    if(!m_is_animal){
        VIRTADDR personality_addr = m_first_soul + m_mem->soul_detail("personality");

        //read personal beliefs before traits, as a dwarf will have a conflict with either personal beliefs or cultural beliefs
        m_beliefs.clear();
        QVector<VIRTADDR> m_beliefs_addrs = m_df->enumerate_vector(personality_addr + m_mem->soul_detail("beliefs"));
        foreach(VIRTADDR addr, m_beliefs_addrs){
            int belief_id = m_df->read_int(addr);
            if(belief_id >= 0){
                short val = m_df->read_short(addr + 0x0004);
                UnitBelief ub(belief_id,val,true);
                m_beliefs.insert(belief_id, ub);
            }
        }

        VIRTADDR traits_addr = personality_addr + m_mem->soul_detail("traits");
        m_traits.clear();
        m_conflicting_beliefs.clear();
        int trait_count = GameDataReader::ptr()->get_traits().count();
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

        QVector<VIRTADDR> m_goals_addrs = m_df->enumerate_vector(personality_addr + m_mem->soul_detail("goals"));
        m_goals.clear();
        foreach(VIRTADDR addr, m_goals_addrs){
            int goal_type = m_df->read_int(addr + 0x0004);
            if(goal_type >= 0){
                short val = m_df->read_short(addr + m_mem->soul_detail("goal_realized")); //goal realized
                //if we're not showing vampires, and this dwarf is a vampire, keep the goal hidden so they can't be identified from that
                if(goal_type == 11 && m_curse_type == eCurse::VAMPIRE &&  DT->user_settings()->value("options/highlight_cursed", false).toBool()==false)
                    continue;

                if(val > 0)
                    m_goals_realized++;
                m_goals.insert(goal_type,val);
            }
        }

        //TODO: see if this is still affecting things in DF2014
        //    //check the misc. traits for the level of detachment and add a special trait for it
        //    if(!m_is_animal && has_state(15)){
        //        int val = state_value(15);
        //        m_traits.insert(41,val);
        //    }
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
    VIRTADDR addr = m_address + m_mem->dwarf_offset("physical_attrs");
    for(int i=0; i<6; i++){
        load_attribute(addr, static_cast<ATTRIBUTES_TYPE>(i));
    }
    //read the mental attributes, but append to our array (augment the key by the number of physical attribs)
    int phys_size = m_attributes.size();
    addr = m_first_soul + m_mem->soul_detail("mental_attrs");
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
    if(!m_is_baby && !m_is_animal)
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
    if(m_is_baby){
        return QString::number(m_age_in_months).append(tr(" Month").append(m_age_in_months == 1 ? "" : "s").append(tr(" Old")));
    }else{
        return QString::number(m_age).append(tr(" Year").append(m_age == 1 ? "" : "s").append(tr(" Old")));
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
    return ((name == m_pending_custom_profession || name == m_custom_profession) &&
            m_pending_custom_profession != m_custom_profession);
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

    if (!m_can_set_labors && !DT->labor_cheats_allowed()) {
        LOGD << "IGNORING SET LABOR OF ID:" << labor_id << "TO:" << enabled << "FOR:" << m_nice_name << "PROF_ID" << m_raw_profession
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

bool Dwarf::toggle_flag_bit(int bit) {
    if (bit!=49) //only flag currently available to be toggled is butcher
        return false;
    if(m_animal_type==hostile || m_animal_type==wild_untamed || m_animal_type==unknown_trained)
        return false;
    int n=bit;
    if(bit>31)
    {
        n=bit-32;
    }
    quint32 mask = 1;
    for (int i=0;i<n;i++)
        mask<<=1;
    if (bit>31){
        //don't butcher if it's a pet, user will be notified via tooltip on column, same for non-butcherable
        if(!m_is_pet && m_caste->flags().has_flag(BUTCHERABLE))
            m_butcher^=mask;
    }

    return true;
}

bool Dwarf::is_flag_dirty(int bit_pos){
    if (bit_pos!=49){
        return false;
    }else{ //compare butcher flag
        return m_butcher != m_unit_flags.at(1);
    }
}

int Dwarf::pending_changes() {
    int cnt = get_dirty_labors().size();
    if (m_nick_name != m_pending_nick_name)
        cnt++;
    if(m_squad_id != m_pending_squad_id)
        cnt++;
    if (m_custom_profession != m_pending_custom_profession)
        cnt++;
    if (m_unit_flags.at(0) != m_caged)
        cnt++;
    if (m_unit_flags.at(1) != m_butcher)
        cnt++;
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

void Dwarf::commit_pending(bool single) {
    int addr = m_address + m_mem->dwarf_offset("labors");

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
        m_df->write_string(m_address + m_mem->dwarf_offset("nick_name"), m_pending_nick_name);
        m_df->write_string(m_first_soul + m_mem->soul_detail("name") + m_mem->dwarf_offset("nick_name"), m_pending_nick_name);
        if(m_hist_figure){
            m_hist_figure->write_nick_name(m_pending_nick_name);
        }
    }
    if (m_pending_custom_profession != m_custom_profession)
        m_df->write_string(m_address + m_mem->dwarf_offset("custom_profession"), m_pending_custom_profession);
    if (m_caged != m_unit_flags.at(0))
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags1"), 4, &m_caged);
    if (m_butcher != m_unit_flags.at(1))
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags2"), 4, &m_butcher);

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
        recheck_equipment();
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

void Dwarf::recheck_equipment(){
    // set the "recheck_equipment" flag if there was a labor change, or squad change
    BYTE recheck_equipment = m_df->read_byte(m_address + m_mem->dwarf_offset("recheck_equipment"));
    recheck_equipment |= 1;
    m_df->write_raw(m_address + m_mem->dwarf_offset("recheck_equipment"), 1, &recheck_equipment);
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
    if (m_caged != m_unit_flags.at(0)) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Caged changed to %1").arg(hexify(m_caged)));
        i->setIcon(0, QIcon(":img/book--pencil.png"));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_butcher != m_unit_flags.at(1)) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Butcher changed to %1").arg(hexify(m_butcher)));
        i->setIcon(0, QIcon(":img/book--pencil.png"));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_nick_name != m_nick_name) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString nick = m_pending_nick_name;
        i->setText(0, nick.isEmpty() ? tr("Nickname reset to default")
                                     : tr("Nickname changed to %1").arg(nick));
        i->setIcon(0, QIcon(":img/book--pencil.png"));
        i->setToolTip(0, i->text(0));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_custom_profession != m_custom_profession) {
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

QString Dwarf::tooltip_text() {
    QSettings *s = DT->user_settings();
    GameDataReader *gdr = GameDataReader::ptr();
    QString skill_summary, personality_summary, roles_summary;
    int max_roles = s->value("options/role_count_tooltip",3).toInt();
    if(max_roles > sorted_role_ratings().count())
        max_roles = sorted_role_ratings().count();

    //in some mods animals may have skills
    if(!m_skills.isEmpty() && s->value("options/tooltip_show_skills",true).toBool()){
        int max_level = s->value("options/min_tooltip_skill_level", true).toInt();
        bool check_social = !s->value("options/tooltip_show_social_skills",true).toBool();
        QMapIterator<float,int> i(m_sorted_skills);
        i.toBack();
        while(i.hasPrevious()){
            i.previous();
            if(m_skills.value(i.value()).capped_level() < max_level || (check_social && gdr->social_skills().contains(i.value()))) {
                continue;
            }
            skill_summary.append(QString("<li>%1</li>").arg(m_skills.value(i.value()).to_string()));
        }
    }

    if(!m_is_animal){
        if(s->value("options/tooltip_show_traits",true).toBool()){
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
        if(!sorted_roles.isEmpty() && max_roles > 0 && s->value("options/tooltip_show_roles",true).toBool()){
            roles_summary.append("<ol style=\"margin-top:0px; margin-bottom:0px;\">");
            for(int i = 0; i < max_roles; i++){
                roles_summary += tr("<li>%1  (%2%)</li>").arg(sorted_roles.at(i).name)
                        .arg(QString::number(sorted_roles.at(i).rating,'f',2));
            }
            roles_summary.append("</ol>");
        }
    }


    QStringList tt;
    QString title;
    if(s->value("options/tooltip_show_icons",true).toBool()){
        title += tr("<center><b><h3 style=\"margin:0;\"><img src='%1'> %2 %3</h3><h4 style=\"margin:0;\">%4</h4></b></center>")
                .arg(m_icn_gender).arg(m_nice_name).arg(embedPixmap(m_icn_prof))
                .arg(m_translated_name.isEmpty() ? "" : "(" + m_translated_name + ")");
    }else{
        title += tr("<center><b><h3 style=\"margin:0;\">%1</h3><h4 style=\"margin:0;\">%2</h4></b></center>")
                .arg(m_nice_name).arg(m_translated_name.isEmpty() ? "" : "(" + m_translated_name + ")");
    }

    if(!m_is_animal && s->value("options/tooltip_show_artifact",true).toBool() && !m_artifact_name.isEmpty())
        title.append(tr("<center><i><h5 style=\"margin:0;\">Creator of '%2'</h5></i></center>").arg(m_artifact_name));

    tt.append(title);

    if(s->value("options/tooltip_show_caste",true).toBool())
        tt.append(tr("<b>Caste:</b> %1").arg(caste_name()));

    if(m_is_animal || s->value("options/tooltip_show_age",true).toBool())
        tt.append(tr("<b>Age:</b> %1").arg(get_age_formatted()));

    if(m_is_animal || s->value("options/tooltip_show_size",true).toBool())
        tt.append(tr("<b>Size:</b> %1cm<sup>3</sup>").arg(QLocale(QLocale::system()).toString(m_body_size * 10)));

    if(!m_is_animal && s->value("options/tooltip_show_noble",true).toBool())
        tt.append(tr("<b>Profession:</b> %1").arg(profession()));

    if(!m_is_animal && m_pending_squad_id > -1 && s->value("options/tooltip_show_squad",true).toBool())
        tt.append(tr("<b>Squad:</b> %1").arg(m_pending_squad_name));

    if(!m_is_animal && m_noble_position != "" && s->value("options/tooltip_show_noble",true).toBool())
        tt.append(tr("<b>Noble Position%1:</b> %2").arg(m_noble_position.indexOf(",") > 0 ? "s" : "").arg(m_noble_position));

    if(!m_is_animal && s->value("options/tooltip_show_happiness",true).toBool())
        tt.append(tr("<b>Happiness:</b> %1 (%2)").arg(happiness_name(m_happiness)).arg(m_raw_happiness));

    if(!m_is_animal && !m_thought_desc.isEmpty() && s->value("options/tooltip_show_thoughts",true).toBool())
        tt.append(tr("<p style=\"margin:0px;\"><b>Thoughts: </b>%1</p>").arg(m_thought_desc));

    if(!skill_summary.isEmpty())
        tt.append(tr("<h4 style=\"margin:0px;\"><b>Skills:</b></h4><ul style=\"margin:0px;\">%1</ul>").arg(skill_summary));

    if(!m_is_animal && s->value("options/tooltip_show_mood",false).toBool())
        tt.append(tr("<b>Highest Moodable Skill:</b> %1")
                  .arg(gdr->get_skill_name(m_highest_moodable_skill, true)));

    if(!personality_summary.isEmpty())
        tt.append(tr("<p style=\"margin:0px;\"><b>Personality:</b> %1</p>").arg(personality_summary));

    if(!m_pref_tooltip.isEmpty())
        tt.append(tr("<p style=\"margin:0px;\">%1</p>").arg(m_pref_tooltip));

    if(!roles_summary.isEmpty())
        tt.append(tr("<h4 style=\"margin:0px;\"><b>Top %1 Roles:</b></h4>%2").arg(max_roles).arg(roles_summary));

    if(m_is_animal)
        tt.append(tr("<p style=\"margin:0px;\"><b>Trained Level:</b> %1</p>").arg(get_animal_trained_descriptor(m_animal_type)));

    if(s->value("options/tooltip_show_health",false).toBool() && (!m_is_animal || (m_is_animal && s->value("options/animal_health",false).toBool()))){

        bool symbols = s->value("options/tooltip_health_symbols",false).toBool();
        bool colors = s->value("options/tooltip_health_colors",true).toBool();

        //health info is in 3 sections: treatment, statuses and wounds
        QString health_info = "";

        QStringList treatments = m_unit_health.get_treatment_summary(colors,symbols);
        if(treatments.size() > 0)
            health_info.append(tr("<h4 style=\"margin:0px;\"><b>%1:</b></h4><ul style=\"margin:0px;\">%2</ul>").arg(tr("Treatment")).arg(treatments.join(", ")));

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

            qSort(status_wound_summary);
            health_info.append(tr("<h4 style=\"margin:0px;\"><b>%1:</b></h4><ul style=\"margin:0px;\">%2</ul>").arg(tr("Health Issues")).arg(status_wound_summary.join(", ")));
        }

        if(!health_info.isEmpty())
            tt.append(health_info);
    }

    if(m_syndromes.count() > 0 && s->value("options/tooltip_show_buffs",false).toBool()){
        QString buffs = get_syndrome_names(true,false);
        QString ailments = get_syndrome_names(false,true);

        if(!buffs.isEmpty())
            tt.append(tr("<b>Buffs:</b> %1<br/>").arg(buffs));
        if(!ailments.isEmpty())
            tt.append(tr("<b>Ailments:</b> %1<br/>").arg(ailments));
    }


    if(s->value("options/tooltip_show_caste_desc",true).toBool() && caste_desc() != "")
        tt.append(tr("%1").arg(caste_desc()));

    if(s->value("options/highlight_cursed", false).toBool() && m_curse_name != ""){
        QString curse_text = "";
        curse_text = tr("<br/><b>Curse: </b>A <b><i>%1</i></b>")
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
        tt.append(curse_text);
    }

    if(s->value("options/tooltip_show_kills",false).toBool() && m_hist_figure && m_hist_figure->total_kills() > 0){
        tt.append(m_hist_figure->formatted_summary());
    }

    return tt.join("<br/>");
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

Skill Dwarf::highest_moodable(){
    Skill hm = get_skill(m_highest_moodable_skill);
    return hm;
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
        double val = DwarfStats::get_attribute_rating(m_attributes[i].get_value(), true);
        m_attributes[i].set_rating(val);
    }
}

QList<double> Dwarf::calc_role_ratings(){
    calc_attribute_ratings();

    LOGD << ":::::::::::::::::::::::::::::::::::::::::::::::::::";
    LOGD << m_nice_name;

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

double Dwarf::calc_role_rating(Role *m_role){
    //if there's a script, use this in place of any aspects
    if(!m_role->script().trimmed().isEmpty()){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(this);
        m_engine.globalObject().setProperty("d", d_obj);
        return  m_engine.evaluate(m_role->script()).toNumber(); //just show the raw value the script generates
    }

    LOGD << "  +" << m_role->name() << "-" << m_nice_name;

    //no script, calculate rating based on specified aspects
    double rating_att = 0.0;
    double rating_trait = 0.0;
    double rating_skill = 0.0;
    double rating_prefs = 0.0;
    double rating_total = 0.0;

    //use the role's aspect section weights (these are defaulted to the global weights)
    float global_att_weight = m_role->attributes_weight.weight;
    float global_skill_weight = m_role->skills_weight.weight;
    float global_trait_weight = m_role->traits_weight.weight;
    float global_pref_weight = m_role->prefs_weight.weight;

    //without weights, there's nothing to calculate
    if((global_att_weight + global_skill_weight + global_trait_weight + global_pref_weight) == 0)
        return 50.0f;

    RoleAspect *a;
    double aspect_value = 0.0;
    float total_weight = 0.0;
    float weight = 1.0;

    //ATTRIBUTES
    if(m_role->attributes.count()>0){

        foreach(QString name, m_role->attributes.uniqueKeys()){
            a = m_role->attributes.value(name);
            weight = a->weight;

            ATTRIBUTES_TYPE attrib_id = GameDataReader::ptr()->get_attribute_type(name.toUpper());
            aspect_value = get_attribute(attrib_id).rating(true);

            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_att += (aspect_value*weight);

            total_weight += weight;

        }
        if(total_weight > 0){
            rating_att = (rating_att / total_weight) * 100.0f; //weighted average percentile
        }else{
            return 50.0f;
        }
    }else{
        rating_att = 50.0f;
    }

    //TRAITS
    if(m_role->traits.count()>0){
        total_weight = 0;
        foreach(QString trait_id, m_role->traits.uniqueKeys()){
            a = m_role->traits.value(trait_id);
            weight = a->weight;

            aspect_value = DwarfStats::get_trait_rating(trait(trait_id.toInt()));

            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_trait += (aspect_value * weight);

            total_weight += weight;
        }
        if(total_weight > 0){
            rating_trait = (rating_trait / total_weight) * 100.0f;//weighted average percentile
        }else{
            return 50.0f;
        }
    }else{
        rating_trait = 50.0f;
    }

    //SKILLS
    float total_skill_rates = 0.0;
    if(m_role->skills.count()>0){
        total_weight = 0;
        Skill s;
        foreach(QString skill_id, m_role->skills.uniqueKeys()){
            a = m_role->skills.value(skill_id);
            weight = a->weight;

            s = this->get_skill(skill_id.toInt());
            total_skill_rates += s.skill_rate();
            aspect_value = s.get_rating();

            LOGD << "      * skill:" << s.name() << "lvl:" << s.capped_level_precise() << "sim. lvl:" << s.get_simulated_level() << "balanced lvl:" << s.get_balanced_level()
                 << "rating:" << s.get_rating();

            if(aspect_value > 1.0)
                aspect_value = 1.0;

            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_skill += (aspect_value*weight);

            total_weight += weight;
        }
        if(total_skill_rates <= 0){
            //this unit cannot improve the skills associated with this role so cancel any rating
            return 0.0001;
        }else{
            if(total_weight > 0){
                rating_skill = (rating_skill / total_weight) * 100.0f;//weighted average percentile
            }else{
                rating_skill = 50.0f;
            }
        }
    }else{
        rating_skill = 50.0f;
    }

    //PREFERENCES
    if(m_role->prefs.count()>0){
        aspect_value = get_role_pref_match_counts(m_role);
        rating_prefs = DwarfStats::get_preference_rating(aspect_value) * 100.0f;
    }else{
        rating_prefs = 50.0f;
    }

    //weighted average percentile total
    rating_total = ((rating_att * global_att_weight)+(rating_skill * global_skill_weight)
                    +(rating_trait * global_trait_weight)+(rating_prefs * global_pref_weight))
            / (global_att_weight + global_skill_weight + global_trait_weight + global_pref_weight);

    if(rating_total == 0)
        rating_total = 0.0001;

    LOGD << "      -attributes:" << rating_att;
    LOGD << "      -skills:" << rating_skill;
    LOGD << "      -traits:" << rating_trait;
    LOGD << "      -preferences:" << rating_prefs;
    LOGD << "      -total:" << rating_total;
    LOGD << "  ------------------------------------------------------";

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
        float display_rating = DwarfStats::get_role_rating(m_raw_role_ratings.value(name)) * 100.0f;
        m_role_ratings.insert(name,display_rating);
        sr.rating = display_rating;
        sr.name = name;
        m_sorted_role_ratings.append(sr);
    }
    if(DT->user_settings()->value("options/show_custom_roles",false).toBool()){
        qSort(m_sorted_role_ratings.begin(),m_sorted_role_ratings.end(),&Dwarf::sort_ratings_custom);
    }else{
        qSort(m_sorted_role_ratings.begin(),m_sorted_role_ratings.end(),&Dwarf::sort_ratings);
    }
}

double Dwarf::get_role_pref_match_counts(Role *r, bool load_map){
    double total_rating = 0.0;
    foreach(Preference *role_pref,r->prefs){
        double matches = get_role_pref_match_counts(role_pref,(load_map ? r : 0));
        if(matches > 0){
            double rating = matches * role_pref->pref_aspect->weight;
            if(role_pref->pref_aspect->is_neg)
                rating = 1.0f-rating;
            total_rating += rating;
        }
    }
    return total_rating;
}

double Dwarf::get_role_pref_match_counts(Preference *role_pref, Role *r){
    double matches = 0;
    int key = role_pref->get_pref_category();
    QMultiMap<int, Preference *>::iterator i = m_preferences.find(key);
    Preference *p;
    while(i != m_preferences.end() && i.key() == key){
        p = static_cast<Preference*>(i.value());
        double match = (double)p->matches(role_pref,this);
        if(match > 0){
            if(r){
                m_role_pref_map[r->name()].append(qMakePair(role_pref->get_name(), p->get_name()));
            }
        }
        matches += match;
        i++;
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
