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
#include <QVector>
#include <QtScript>
#include <QDebug>
#include "dwarf.h"
#include "dfinstance.h"
#include "skill.h"
#include "trait.h"
#include "dwarfjob.h"
#include "defines.h"
#include "gamedatareader.h"
#include "customprofession.h"
#include "memorylayout.h"
#include "dwarftherapist.h"
#include "dwarfdetailswidget.h"
#include "mainwindow.h"
#include "profession.h"
#include "militarypreference.h"
#include "dwarfstats.h"
#include "races.h"
#include "reaction.h"
#include "fortressentity.h"
#include "columntypes.h"
#include "plant.h"

#include "utils.h"
#include "labor.h"
#include "preference.h"
#include "material.h"
#include "caste.h"
#include "attribute.h"
#include "weapon.h"
#include "roleaspect.h"
#include "thought.h"

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
    , m_is_male(true)
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
    , m_squad_id(-1)
    , m_squad_position(-1)
    , m_hist_id(-1)
    , m_squad_name(QString::null)
    , m_flag1(0)
    , m_flag2(0)    
    , m_age(0)    
    , m_hist_nickname(0)
    , m_fake_nickname(0)
    , m_noble_position("")
    , m_is_pet(false)    
    , m_highest_moodable_skill(-1)
    , m_race(0)
    , m_caste(0)
    , m_is_child(false)
    , m_is_baby(false)    
    , m_true_name("")
    , m_true_birth_year(0)
{
    read_settings();
    refresh_data();
    connect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));

    // setup context actions
    m_actions.clear();
//    QAction *show_details = new QAction(tr("Show Details..."), this);
//    connect(show_details, SIGNAL(triggered()), SLOT(show_details()));
//    m_actions << show_details;
    QAction *dump_mem = new QAction(tr("Dump Memory..."), this);
    connect(dump_mem, SIGNAL(triggered()), SLOT(dump_memory()));
    m_actions << dump_mem;
    QAction *dump_mem_to_file = new QAction(tr("Dump Memory To File"), this);
    connect(dump_mem_to_file, SIGNAL(triggered()), SLOT(dump_memory_to_file()));
    m_actions << dump_mem_to_file;
    QAction *copy_address_to_clipboard = new QAction(
            tr("Copy Address to Clipboard"), this);
    connect(copy_address_to_clipboard, SIGNAL(triggered()),
            SLOT(copy_address_to_clipboard()));
    m_actions << copy_address_to_clipboard;
}


Dwarf::~Dwarf() {
    qDeleteAll(m_actions);
    m_actions.clear();

    m_traits.clear();    
    m_skills.clear();    
    m_attributes.clear();

    m_labors.clear();
    m_pending_labors.clear();
    m_role_ratings.clear();
    m_raw_role_ratings.clear();
    m_sorted_role_ratings.clear();
    m_states.clear();

    qDeleteAll(m_preferences);
    m_preferences.clear();
    qDeleteAll(m_grouped_preferences);
    m_grouped_preferences.clear();

    m_thoughts.clear();

    m_race = 0;
    m_caste = 0;
    m_mem = 0;
    m_df = 0;

    disconnect(DT, SIGNAL(settings_changed()), this, SLOT(read_settings()));
}

void Dwarf::read_settings() {
    /* This is pretty fucked up. There is no good way to let the options for
       full name, or show dabbling to just update existing cells. Will have to
       send signals when this stuff changes, or just bite the bullet and
       subclass the QStandardItem for the name items in the main model
       */
    QSettings *s = DT->user_settings();
    bool new_show_full_name = s->value("options/show_full_dwarf_names",
                                       false).toBool();
    if (new_show_full_name != m_show_full_name) {
        calc_names();
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

    m_flag1 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags1"));
    m_flag2 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags2"));

    // read everything we need, order is important
    read_id();
    read_race();
    read_caste();    
    read_first_name();
    read_last_name(m_address + m_mem->dwarf_offset("first_name"));
    read_nick_name();
    calc_names(); //creates nice name.. which is used for debug messages so we need to do it first..
    read_profession();
    read_body_size();
    read_labors();
    read_happiness();
    read_states();  //read states before job
    read_current_job();
    read_souls();
    read_squad_info();
    read_turn_count();
    //load time/date stuff for births/migrations
    set_age(m_address + m_mem->dwarf_offset("birth_year"), m_address + m_mem->dwarf_offset("birth_time"));
    //curse check will change the name and age
    read_curse();
    read_sex();
    read_mood();    
    read_animal_type(); //need skills loaded to check for hostiles
    read_noble_position();
    read_preferences();

    if(m_is_animal)
        calc_names(); //calculate names again as we need to check tameness for animals

    //currently only flags used are for cage and butcher animal
    m_caged = m_flag1;
    m_butcher = m_flag2;

    TRACE << "finished refresh of dwarf data for dwarf:" << m_nice_name << "(" << m_translated_name << ")";

}

void Dwarf::set_age(VIRTADDR birth_year_offset, VIRTADDR birth_time_offset){
    m_birth_year = m_df->read_int(birth_year_offset);
    m_age = m_df->current_year() - m_birth_year;
    m_birth_time = m_df->read_int(birth_time_offset);
    quint32 current_year_time = m_df->current_year_time();
    quint32 current_time = m_df->current_year() * 0x62700 + current_year_time;
    quint32 arrival_time = current_time - m_turn_count;
    quint32 ticks_per_day = 1200;
    quint32 ticks_per_month = 28 * ticks_per_day;
    quint32 ticks_per_season = 3 * ticks_per_month;
    quint32 ticks_per_year = 12 * ticks_per_month;
    quint32 arrival_year = arrival_time / ticks_per_year;
    quint32 arrival_season = (arrival_time % ticks_per_year) / ticks_per_season;
    quint32 arrival_month = (arrival_time % ticks_per_year) / ticks_per_month;
    quint32 arrival_day = ((arrival_time % ticks_per_year) % ticks_per_month) / ticks_per_day;
    quint32 time_since_birth = m_age * ticks_per_year + current_year_time - m_birth_time;
    //this way we have the right sort order and all the data needed for the group by migration wave
    m_migration_wave = 100000 * arrival_year + 10000 * arrival_season + 100 * arrival_month + arrival_day;
    m_born_in_fortress = (time_since_birth == m_turn_count);
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
    else
        suffix = "th";
    if(m_born_in_fortress)
    {
        return QString("Born on the %1%4 of %2 in the year %3").arg(day).arg(GameDataReader::ptr()->m_months.at(month)).arg(year).arg(suffix);
    }
    else
    {
        return QString("Arrived in the %2 of the year %1").arg(year).arg(GameDataReader::ptr()->m_seasons.at(season));
    }
    return "Unknown Arrival";
}

/*******************************************************************************
  DATA POPULATION METHODS
*******************************************************************************/

void Dwarf::read_id() {
    m_id = m_df->read_int(m_address + m_mem->dwarf_offset("id"));
    //m_id = m_address; // HACK: this will allow dwarfs in the list even when
    // the id offset isn't know for this version
    TRACE << "ID:" << m_id;
    m_hist_id = m_df->read_int(m_address + m_mem->dwarf_offset("hist_id"));
}

void Dwarf::read_sex() {
    // TODO: actually break down this caste
    BYTE sex = m_df->read_byte(m_address + m_mem->dwarf_offset("sex"));
    m_is_male = (int)sex == 1;
    TRACE << "MALE:" << m_is_male;

    if(m_is_male)
        m_icn_gender = tr(":img/male.png");
    else
        m_icn_gender = tr(":img/female.png");
}

void Dwarf::read_mood(){
    m_mood_id = m_df->read_short(m_address + m_mem->dwarf_offset("mood"));

    if(m_mood_id < 0){
        //also mark if they've had a mood, if they're not IN a mood
        if((m_flag1 & 0x00000008)==0x00000008){
            m_had_mood = true;
            m_artifact_name = m_df->get_translated_word(m_address + m_mem->dwarf_offset("artifact_name"));
        }
    }
}

void Dwarf::read_body_size(){
    //default dwarf sizes
    if(m_is_child)
        m_body_size = 1500;
    else if(m_is_baby)
        m_body_size = 300;
    else
        m_body_size = 6000;

    if(m_race){
        if(m_caste){
            if(m_is_child)
                m_body_size = m_caste->child_size();
            else if(m_is_baby)
                m_body_size = m_caste->baby_size();
            else
                m_body_size = m_caste->adult_size();
        }
    }
}

void Dwarf::read_animal_type(){
    if(m_is_animal){
        qint32 animal_offset = m_mem->dwarf_offset("animal_type");
        if(animal_offset>=0)
            m_animal_type = static_cast<TRAINED_LEVEL>(m_df->read_int(m_address + animal_offset));

        if(m_animal_type == semi_wild){
            int z = 0;
        }

        //additional if it's an animal set a flag if it's currently a pet, as butchering available pets breaks shit in game
        //due to not being able to remove the pet entry from the special_refs for the unit stuff
        if(m_mem->dwarf_offset("specific_refs") != -1){
            QVector<VIRTADDR> refs = m_df->enumerate_vector(m_address + m_mem->dwarf_offset("specific_refs"));
            foreach(VIRTADDR ref, refs){
                if(m_df->read_int(ref)==7){ //pet type
                    m_is_pet = true;
                    break;
                }
            }
        }
    }
}

void Dwarf::read_states(){
    //vector holding a set of enum shorts, not pants
    m_states.clear();
    uint states_offset = m_mem->dwarf_offset("states");
    if(states_offset) {
        VIRTADDR states_addr = m_address + states_offset;
        QVector<uint> entries = m_df->enumerate_vector(states_addr);
        foreach(uint entry, entries) {
            m_states.append(m_df->read_short(entry));
        }
    }
}

void Dwarf::read_curse(){
    QString curse_name = m_df->read_string(m_address + m_mem->dwarf_offset("curse"));
    if(curse_name != ""){
        m_curse_name = capitalizeEach(curse_name);

        //if it's a vampire then find the vampire's fake identity and use that name/age instead to match DF
        quint32 curse_flags = m_df->read_addr(m_address + m_mem->dwarf_offset("curse_add_flags1"));
        if((curse_flags & 0x10000000)==0x10000000) //check bloodsucker flag in curse flags
            find_true_ident();
    }
}

void Dwarf::find_true_ident(){
    VIRTADDR fake_id = m_df->find_fake_identity(m_hist_id);
    if(fake_id){
        //save the true name for display
        m_true_name = m_nice_name;
        m_true_birth_year = m_birth_year;
        //overwrite the default name, birth year with the assumed identity
        m_first_name = capitalize(m_df->read_string(fake_id + m_mem->hist_figure_offset("fake_name") + m_mem->dwarf_offset("first_name")));
        m_fake_nickname = fake_id + m_mem->hist_figure_offset("fake_name") + m_mem->dwarf_offset("nick_name");
        m_nick_name = m_df->read_string(m_fake_nickname);
        read_last_name(fake_id + m_mem->hist_figure_offset("fake_name"));
        calc_names();
        //vamps also use a the fake age
        set_age(fake_id + m_mem->hist_figure_offset("fake_birth_year"),fake_id + m_mem->hist_figure_offset("fake_birth_time"));
    }
}

void Dwarf::read_caste() {
    m_caste_id = m_df->read_short(m_address + m_mem->dwarf_offset("caste"));
    m_caste = m_race->get_caste_by_id(m_caste_id);
    TRACE << "CASTE:" << m_caste_id;    
}

void Dwarf::read_race() {
    m_race_id = m_df->read_int(m_address + m_mem->dwarf_offset("race"));
    m_race = m_df->get_race(m_race_id);
    TRACE << "RACE ID:" << m_race_id;
    m_is_animal = (m_df->dwarf_race_id()!=m_race_id);
}

void Dwarf::read_first_name() {
    m_first_name = m_df->read_string(m_address + m_mem->dwarf_offset("first_name"));
    if (m_first_name.size() > 1)
        m_first_name[0] = m_first_name[0].toUpper();
    TRACE << "FIRSTNAME:" << m_first_name;
}

//! used by read_last_name to find word chunks
QString Dwarf::word_chunk(uint word, bool use_generic) {
    QString out = "";
    if (word != 0xFFFFFFFF) {
        if (use_generic) {
            out = DT->get_generic_word(word);
        } else {
            out = DT->get_dwarf_word(word);
        }
    }
    return out;
}

QString Dwarf::read_chunked_name(const VIRTADDR &addr, bool use_generic) {
    // last name reading taken from patch by Zhentar (issue 189)
    QString first, second, third;

    first.append(word_chunk(m_df->read_addr(addr), use_generic));
    first.append(word_chunk(m_df->read_addr(addr + 0x4), use_generic));
    second.append(word_chunk(m_df->read_addr(addr + 0x8), use_generic));
    second.append(word_chunk(m_df->read_addr(addr + 0x14), use_generic));
    third.append(word_chunk(m_df->read_addr(addr + 0x18), use_generic));

    QString out = first;
    out = out.toLower();
    if (!out.isEmpty()) {
        out[0] = out[0].toUpper();
    }
    if (!second.isEmpty()) {
        second = second.toLower();
        second[0] = second[0].toUpper();
        out.append(" " + second);
    }
    if (!third.isEmpty()) {
        third = third.toLower();
        third[0] = third[0].toUpper();
        out.append(" " + third);
    }
    return out;
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

    if(!m_is_animal){
        //save the nickname address as we need to update it when nicknames are changed
        m_hist_nickname = m_df->find_historical_figure(m_hist_id);
        if(m_hist_nickname)
            m_hist_nickname += m_mem->hist_figure_offset("hist_name") + m_mem->dwarf_offset("nick_name");
    }
}

void Dwarf::calc_names() {
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

    if(m_is_animal)
    {
//        QString tametype = "";
//        switch (m_animal_type) {
//        case semi_wild:
//            tametype = tr("Semi-wild");
//            break;
//        case trained:
//            tametype = tr("Trained");
//            break;
//        case well_trained:
//            tametype = tr("Well trained");
//            break;
//        case skillfully_trained:
//            tametype = tr("Skillfully trained");
//            break;
//        case expertly_trained:
//            tametype = tr("Expertly trained");
//            break;
//        case exceptionally_trained:
//            tametype = tr("Exceptionally trained");
//            break;
//        case masterfully_trained:
//            tametype = tr("Masterfully trained");
//            break;
//        case domesticated:
//            tametype = tr("Domesticated");
//            break;
//        case unknown_trained:
//            tametype = tr("Unknown");
//            break;
//        case wild_untamed:
//            tametype = tr("Wild");
//            break;
//        case hostile:
//            tametype = tr("Hostile");
//            break;
//        default:
//            tametype = "";
//            break;
//        }

//        tametype = (tametype != "" ? " (" + tametype + ") " : " ");

        if(m_nice_name=="")
        {
            m_nice_name = race_name();
            m_translated_name = "";
            //m_nice_name = tr("Stray %1 %2").arg(race_name()).arg(tametype);
            //m_translated_name = tr("Stray %1").arg(race_name());
        }
        else
        {
            m_nice_name = tr("%1 (%2)").arg(race_name()).arg(m_nice_name);
            //m_nice_name += QString(", %1 %2").arg(race_name()).arg(tametype);
            //m_translated_name += QString(", %1").arg(race_name());
        }        

        //m_nice_name = race_name(true) + prof + tametype + m_nice_name;


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
        prof_name = p->name(m_is_male);
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

    if(is_animal() && m_profession==tr("Peasant"))
        m_profession = tr("Adult"); //adult animals have a profession of peasant by default, just use adult

    int prof_id = 90; //default to none
    if(m_raw_profession > -1 && m_raw_profession < GameDataReader::ptr()->get_professions().count())
        prof_id = m_raw_profession + 1; //images start at 1, professions at 0, offest to match image

    //set the default path for the profession icon
    m_icn_prof = QPixmap(":/profession/img/profession icons/prof_" + QString::number(prof_id) + ".png");
    //see if we have a custom profession or icon override
    CustomProfession *cp;
    if(!m_custom_profession.isEmpty()){
        cp = DT->get_custom_profession(m_custom_profession);
    }else{
        cp = DT->get_custom_prof_icon(m_raw_profession);
    }
    if(cp)
        m_icn_prof = cp->get_pixmap();

    TRACE << "reading profession for" << nice_name() << m_raw_profession <<
            prof_name;
    TRACE << "EFFECTIVE PROFESSION:" << m_profession;

    if(m_profession == tr("Child"))
        m_is_child = true;
    else if (m_profession == tr("Baby"))
        m_is_baby = true;
}

void Dwarf::read_noble_position(){
    m_noble_position = m_df->fortress()->get_noble_positions(m_hist_id,m_is_male);
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
            if(m->id() >= 0){
                p->set_material_flags(m->flags());
            }
            m = 0;
        }
            break;
        case 1: //like creature
        {
            Race* r = m_df->get_race(pref_id);
            if(r){
                pref_name = r->plural_name().toLower();
                p = new Preference(ptype,pref_name,this);
                //set trainable flags as well for like creatures
                if(r->is_trainable()){ //really only need to add one flag for a match..
                    p->add_flag(TRAINABLE_HUNTING);
                    p->add_flag(TRAINABLE_WAR); //seems this may only exist in mods?
                }
                //for fish dissection
                if(r->flags().has_flag(VERMIN_FISH))
                    p->add_flag(VERMIN_FISH);
                //for milker
                if(r->is_milkable()){
                    p->add_flag(MILKABLE);
                }
                //animal dissection / beekeeper
                if(r->is_vermin_extractable()){
                    p->add_flag(HAS_EXTRACTS);
                }
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
                pref_name = r->plural_name().toLower();
        }
            break;
        case 4: //like item
        {
            pref_name = m_df->get_item(pref_id,item_sub_type).toLower();
            p->set_item_type(itype);

            //special case for weapon items. find the weapon and set ranged/melee flag for comparison
            if(itype == WEAPON){
                Weapon *w = m_df->get_weapon(capitalizeEach(pref_name));
                if(w && w->is_ranged())
                    p->add_flag(ITEMS_WEAPON_RANGED);
                else
                    p->add_flag(ITEMS_WEAPON);
            }
        }
            break;
        case 5: //like plant
        {
            Plant *plnt = m_df->get_plant(pref_id);
            if(plnt){
                pref_name = plnt->name_plural().toLower();
                if(plnt->flags().has_flag(7)){
                    p->add_flag(7);
                }
            }
        }
            break;
        case 6: //like tree
        {
            pref_name = m_df->get_plant(pref_id)->name_plural().toLower();
        }
            break;
        case 7: //like color
        {
            pref_name = m_df->get_color(pref_id).toLower();
        }
            break;
        case 8: //like shape
        {
            pref_name = m_df->get_shape(pref_id).toLower();
        }
            break;

        }
        p->set_name(capitalize(pref_name));
        m_preferences.insert(pref_type, p);
        //        if(itype < NUM_OF_TYPES && itype != NONE)
        //            LOGW << pref_name << " " << (int)itype << " " << Item::get_item_desc(itype);
    }

    //add a special preference (actually a misc trait) for like outdoors
    if(has_state(14)){
        p = new Preference(LIKE_OUTDOORS,tr("Does not mind being outdoors"),this);
        p->add_flag(999);
        m_preferences.insert(LIKE_OUTDOORS,p);
    }

    //group preferences into pref desc - values (string list)
    QString desc_key;
    foreach(int key, m_preferences.uniqueKeys()){
        QMultiMap<int, Preference *>::iterator i = m_preferences.find(key);
        while(i != m_preferences.end() && i.key() == key){
            desc_key = Preference::get_pref_desc(static_cast<PREF_TYPES>(key));
            if(!m_grouped_preferences.contains(desc_key))
                m_grouped_preferences.insert(desc_key, new QStringList);

            p = (Preference*)i.value();
            m_grouped_preferences.value(desc_key)->append(p->get_name());
            i++;
        }
    }
}

void Dwarf::read_labors() {
    VIRTADDR addr = m_address + m_mem->dwarf_offset("labors");
    // read a big array of labors in one read, then pick and choose
    // the values we care about
    QByteArray buf(102, 0);
    m_df->read_raw(addr, 102, buf);

    // get the list of identified labors from game_data.ini
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        bool enabled = buf.at(l->labor_id) > 0;
        m_labors[l->labor_id] = enabled;
        m_pending_labors[l->labor_id] = enabled;
    }
    // also store prefs in this structure
    foreach(MilitaryPreference *mp, gdr->get_military_preferences()) {
        m_labors[mp->labor_id] = static_cast<ushort>(buf[mp->labor_id]);
        m_pending_labors[mp->labor_id] = static_cast<ushort>(buf[mp->labor_id]);
    }
}

void Dwarf::read_happiness() {
    VIRTADDR addr = m_address + m_mem->dwarf_offset("happiness");
    m_raw_happiness = m_df->read_int(addr);
    m_happiness = happiness_from_score(m_raw_happiness);
    TRACE << "\tRAW HAPPINESS:" << m_raw_happiness;
    TRACE << "\tHAPPINESS:" << happiness_name(m_happiness);

    if(!is_animal()){

        QVector<VIRTADDR> thoughts = m_df->enumerate_vector(m_address + m_mem->dwarf_offset("thoughts"));
        //time, id
        QMap<int,short> t;
        foreach(VIRTADDR addr, thoughts){
            short id = m_df->read_int(addr);
            int time = m_df->read_int(addr + 0x4);
            t.insertMulti(time,id);
        }        

        foreach(int key, t.uniqueKeys()){
            QList<short> vals = t.values(key);
            for(int i = 0; i < vals.count(); i++) {
                if(!m_thoughts.contains(vals.at(i)))
                    m_thoughts.append(vals.at(i));
            }
        }

        QStringList display_desc;
        foreach(int id, m_thoughts){
            Thought *t = GameDataReader::ptr()->get_thought(id);
            if(t)
                display_desc.append(QString("<font color=%1>%2</font>").arg(t->color().name()).arg(t->desc().toLower()));
        }

        m_thought_desc = display_desc.join(", ");
        if(m_thoughts.count() > 0){
            int index = m_thought_desc.indexOf(">") + 1;
            m_thought_desc[index] = m_thought_desc[index].toUpper();
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
        short on_break_value = m_mem->job_detail("on_break_flag");
        m_is_on_break = has_state(on_break_value);
        m_current_job = m_is_on_break ? tr("On Break") : tr("No Job");
        if(m_is_on_break)
            m_current_job_id = -2;
    }
    TRACE << "CURRENT JOB:" << m_current_job_id << m_current_sub_job_id << m_current_job;
}

void Dwarf::read_souls() {
    VIRTADDR soul_vector = m_address + m_mem->dwarf_offset("souls");
    QVector<VIRTADDR> souls = m_df->enumerate_vector(soul_vector);
    if (souls.size() != 1) {
        LOGD << nice_name() << "has" << souls.size() << "souls!";
        return;
    }
    m_first_soul = souls.at(0);
    read_skills();
    read_attributes();
    read_traits();
    TRACE << "SKILLS:" << m_skills.size();
    TRACE << "TRAITS:" << m_traits.size();
}


/******* OTHER CRAP*/

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



Dwarf::DWARF_HAPPINESS Dwarf::happiness_from_score(int score) {
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
    if (base){
        if(!plural_name)
            r_name = m_race->name();
        else
            r_name = m_race->plural_name();
    }else if (m_is_baby){
        r_name = m_race->baby_name();
        if (r_name=="")
        {
            if (m_is_animal)
                r_name = caste_name() + " Baby";
            else
                r_name = m_race->name() + " Baby";
        }
    }
    else if (m_is_child)
        r_name = m_race->child_name();
    else if (profession()==tr("Trained War"))
        r_name = tr("War ") + m_race->name();
    else if (profession()==tr("Trained Hunter"))
        r_name = tr("Hunting ") + m_race->name();
    else
    {
        if (m_is_animal)
            r_name = caste_name();
        else
            r_name = m_race->name();
    }
    return capitalizeEach(r_name);
}

QString Dwarf::happiness_name(DWARF_HAPPINESS happiness) {
    switch(happiness) {
        case DH_MISERABLE: return tr("Miserable");
        case DH_VERY_UNHAPPY: return tr("Very Unhappy");
        case DH_UNHAPPY: return tr("Unhappy");
        case DH_FINE: return tr("Fine");
        case DH_CONTENT: return tr("Content");
        case DH_HAPPY: return tr("Happy");
        case DH_ECSTATIC: return tr("Ecstatic");
        default: return "UNKNOWN";
    }
}

Dwarf *Dwarf::get_dwarf(DFInstance *df, const VIRTADDR &addr) {
    MemoryLayout *mem = df->memory_layout();
    TRACE << "attempting to load dwarf at" << addr << "using memory layout" << mem->game_version();

    //only for test
//    bool check_all = false;
//    if (check_all)
//    {
//        Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
//        return unverified_dwarf;
//    }

    quint32 flags1 = df->read_addr(addr + mem->dwarf_offset("flags1"));
    quint32 flags2 = df->read_addr(addr + mem->dwarf_offset("flags2"));
    quint32 flags3 = df->read_addr(addr + mem->dwarf_offset("flags3"));

    int civ_id = df->read_int(addr + mem->dwarf_offset("civ"));
    int race_id = df->read_int(addr + mem->dwarf_offset("race"));

    bool is_caged = ((flags1 & 0x2000000) == 0x2000000);
    bool is_tame = ((flags1 & 0x4000000) == 0x4000000);

    if(!is_caged){
        if(civ_id != df->dwarf_civ_id()) //non-animal, but wrong civ
            return 0;
    }else if(!is_tame){
        bool trainable = false;
        //if it's a caged, trainable beast, keep it in our list
        Race *r = df->get_race(race_id);
        if(r){
            trainable = r->is_trainable();
            r = 0;
        }
        if(!trainable)
            return 0;
    }

    Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
    TRACE << "examining creature at" << hex << addr;
    TRACE << "FLAGS1 :" << hexify(flags1);
    TRACE << "FLAGS2 :" << hexify(flags2);
    TRACE << "FLAGS3 :" << hexify(flags3);
    TRACE << "RACE   :" << hexify(race_id);

    if (mem->is_complete()) {
        if(!is_tame && !is_caged){ //fortress civilian
            //need to do a special check for migrants, they have both the incoming (0x0400 flag) and the dead flag (0x0002)
            //also need to do a check for the migrant state, as some merchants can arrive with dead and incoming flags as well
            //however they won't have the migrant state
            if((flags1 & 0x00000402)==0x00000402 && unverified_dwarf->has_state(7)){ //7=migrant
                LOGD << "Found migrant " << unverified_dwarf->nice_name();
                return unverified_dwarf;
            }

            //if a dwarf has gone crazy (berserk=7,raving=6)
            int m_mood = unverified_dwarf->m_mood_id;
            if(m_mood==7 || m_mood==6){
                LOGD << "Ignoring" << unverified_dwarf->nice_name() << "who appears to have lost their mind.";
                delete unverified_dwarf;
                return 0;
            }

        }else{ //tame or caged animals
            //exclude cursed animals, this may be unnecessary with the civ check
            if(!unverified_dwarf->curse_name().isEmpty()){
                LOGD << "Ignoring animal " << unverified_dwarf->nice_name() << "who appears to be cursed or undead";
                delete unverified_dwarf;
                return 0;
            }
        }

        if(has_invalid_flags(unverified_dwarf->nice_name(), mem->invalid_flags_1(),flags1) ||
                has_invalid_flags(unverified_dwarf->nice_name(), mem->invalid_flags_2(),flags2) ||
                has_invalid_flags(unverified_dwarf->nice_name(), mem->invalid_flags_3(),flags3)){
            delete unverified_dwarf;
            return 0;
        }

        //finally, if we've got no attributes at all, it's probably a corpse part (most likely from a mod)
        if(unverified_dwarf->m_attributes.count() <=0 ){
            LOGD << "Ignoring" << unverified_dwarf->nice_name() <<
                    "who appears to be a corpse.";
            delete unverified_dwarf;
            return 0;
        }
    }

    return unverified_dwarf;
}

bool Dwarf::has_invalid_flags(const QString creature_name, QHash<uint, QString> invalid_flags, quint32 dwarf_flags){
    foreach(uint invalid_flag, invalid_flags.uniqueKeys()) {
        QString reason = invalid_flags[invalid_flag];
        if ((dwarf_flags & invalid_flag) == invalid_flag) {
            LOGD << "Ignoring" << creature_name << "who appears to be" << reason;
            return true;
        }
    }
    return false;
}

bool Dwarf::get_flag_value(int bit)
{
    //m_caged is the flags1
    quint32 flg=m_caged;
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
    m_squad_position = m_df->read_int(m_address + m_mem->dwarf_offset("squad_position"));
}

void Dwarf::read_turn_count() {
    m_turn_count = m_df->read_int(m_address + m_mem->dwarf_offset("turn_count"));
    TRACE << "Turn Count:" << m_turn_count;
}

void Dwarf::read_skills() {
    VIRTADDR addr = m_first_soul + m_mem->soul_detail("skills");
    m_total_xp = 0;
    m_skills.clear();
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
        m_total_xp += s.actual_exp();
        m_skills.append(s);

        if(GameDataReader::ptr()->moodable_skills().contains(type) &&
                (m_highest_moodable_skill == -1 || s.actual_exp() > get_skill(m_highest_moodable_skill).actual_exp()))
            m_highest_moodable_skill = type;

//        if(rust > 0)
//            LOGD << nice_name() << m_profession << " reading:" << entry << " skill:" << s->name()
//                 << " rating:" << rating << " last_used:"
//                 << last_used << " r.count:" << rust_counter << " r:" << rust
//                 << " d.count:" << demotion_counter <<;
    }        
}

void Dwarf::read_traits() {
    VIRTADDR addr = m_first_soul + m_mem->soul_detail("traits");
    m_traits.clear();
    for (int i = 0; i < 30; ++i) {
        short val = m_df->read_short(addr + i * 2);
        if(val < 0)
            val = 0;
        if(val > 100)
            val = 100;        
        m_traits.insert(i, val);
    }
}

bool Dwarf::trait_is_active(int trait_id){
    short val = trait(trait_id);
    int deviation = abs(val - 50); // how far from the norm is this trait?
    if (deviation <= 10) {
        return false;
    }
    return true;
}

void Dwarf::read_attributes() {    
    m_attributes.clear();
    //read the physical attributes
    VIRTADDR addr = m_address + m_mem->dwarf_offset("physical_attrs");        
    for(int i=0; i<6; i++){
        load_attribute(addr,i);        
    }
    //read the mental attributes, but append to our array (augment the key by the number of physical attribs)
    int phys_size = m_attributes.size();
    addr = m_first_soul + m_mem->soul_detail("mental_attrs");
    for(int i=0; i<13; i++){        
        load_attribute(addr,i+phys_size);        
    }
}

void Dwarf::load_attribute(VIRTADDR &addr, int id){
    int cti = 500;
    QPair<int,QString> desc; //index, description of descriptor

    int value = (int)m_df->read_int(addr);
    int limit = (int)m_df->read_int(addr+0x4);

    if(limit > 5000)
        limit = 5000;
    if(m_caste){
        cti = m_caste->get_attribute_cost_to_improve(id);
        desc = m_caste->get_attribute_descriptor_info(static_cast<ATTRIBUTES_TYPE>(id),value);
    }
    Attribute a = Attribute(id, value, limit, cti, desc.first, desc.second);
    m_attributes.append(a);
    addr+=0x1c;
}

Attribute Dwarf::get_attribute(int id){
    return m_attributes.at(id);
}

Skill Dwarf::get_skill(int skill_id) {
    foreach(Skill s, m_skills) {
        if (s.id() == skill_id) {
            return s;
        }
    }
    //skill doesn't exist
    Skill s = Skill(skill_id, 0, -1, 0, m_caste->get_skill_rate(skill_id));
    m_skills.append(s);
    return m_skills.last();
}

float Dwarf::skill_level(int skill_id, bool raw, bool precise) {
    float retval = -1;
    foreach(Skill s, m_skills) {
        if (s.id() == skill_id) {
            if(raw){
                if(precise)
                    retval = s.raw_level_precise();
                else
                    retval = s.raw_level();
            }else{
                if(precise)
                    retval = s.capped_level_precise();
                else
                    retval = s.capped_level();
            }
            break;
        }
    }
    return retval;
}

short Dwarf::labour_rating(int labor_id) {
    GameDataReader *gdr = GameDataReader::ptr();
    Labor *l = gdr->get_labor(labor_id);
    if (l)
        return skill_level(l->skill_id);
    else
        return -1;
}



short Dwarf::pref_value(const int &labor_id) {
    if (!m_pending_labors.contains(labor_id)) {
        LOGW << m_nice_name << "pref_value for labor_id" << labor_id << "was not found in pending labors hash!";
        return 0;
    }
    return m_pending_labors.value(labor_id);
}

void Dwarf::toggle_pref_value(const int &labor_id) {
    short next_val = GameDataReader::ptr()->get_military_preference(labor_id)->next_val(pref_value(labor_id));
    m_pending_labors[labor_id] = next_val;
}


bool Dwarf::labor_enabled(int labor_id) {
    return m_pending_labors.value(labor_id, false);
}

bool Dwarf::is_labor_state_dirty(int labor_id) {
    return m_labors[labor_id] != m_pending_labors[labor_id];
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
        //don't butcher if it's a pet, user will be notified via tooltip on column
        if(!m_is_pet)
            m_butcher^=mask;
    }else
        m_caged^=mask;
    return true;
}

int Dwarf::pending_changes() {
    int cnt = get_dirty_labors().size();
    if (m_nick_name != m_pending_nick_name)
        cnt++;
    if (m_custom_profession != m_pending_custom_profession)
        cnt++;
    if (m_flag1 != m_caged)
        cnt++;
    if (m_flag2 != m_butcher)
        cnt++;
    return cnt;
}

void Dwarf::clear_pending() {
    //update our header numbers before we refresh
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id >= 0 && is_labor_state_dirty(labor_id))
            m_df->update_labor_count(labor_id, labor_enabled(labor_id) ? -1 : 1);
    }

    refresh_data();    
}

void Dwarf::commit_pending() {    
    int addr = m_address + m_mem->dwarf_offset("labors");

    QByteArray buf(102, 0);
    m_df->read_raw(addr, 102, buf); // set the buffer as it is in-game
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id < 0)
            continue;
        // change values to what's pending
        buf[labor_id] = m_pending_labors.value(labor_id);
    }

    m_df->write_raw(addr, 102, buf.data());

    // We'll set the "recheck_equipment" flag because there was a labor change.
    recheck_equipment();

    if (m_pending_nick_name != m_nick_name){
        m_df->write_string(m_address + m_mem->dwarf_offset("nick_name"), m_pending_nick_name);
        m_df->write_string(m_first_soul + m_mem->soul_detail("name") + m_mem->dwarf_offset("nick_name"), m_pending_nick_name);
        if(m_hist_nickname != 0)
            m_df->write_string(m_hist_nickname, m_pending_nick_name);
        if(m_fake_nickname != 0)
            m_df->write_string(m_fake_nickname, m_pending_nick_name);
    }
    if (m_pending_custom_profession != m_custom_profession)
        m_df->write_string(m_address + m_mem->dwarf_offset("custom_profession"), m_pending_custom_profession);
    if (m_caged != m_flag1)
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags1"), 4, &m_caged);
    if (m_butcher != m_flag2)
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags2"), 4, &m_butcher);

    refresh_data();
}

void Dwarf::recheck_equipment(){
    // set the "recheck_equipment" flag if there was a labor change, or squad change
    BYTE recheck_equipment = m_df->read_byte(m_address + m_mem->dwarf_offset("recheck_equipment"));
    recheck_equipment |= 1;
    m_df->write_raw(m_address + m_mem->dwarf_offset("recheck_equipment"), 1, &recheck_equipment);
}


void Dwarf::set_nickname(const QString &nick) {
    m_pending_nick_name = nick;
    calc_names();
}

void Dwarf::set_custom_profession_text(const QString &prof_text) {
    m_pending_custom_profession = prof_text;
}

int Dwarf::apply_custom_profession(CustomProfession *cp) {        
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        //only turn off all labours if it's NOT a mask
        if(!cp->is_mask())
            set_labor(labor_id, false,false);
    }
    foreach(int labor_id, cp->get_enabled_labors()) {
        set_labor(labor_id, true,false); // only turn on what this prof has enabled...
    }
    m_pending_custom_profession = cp->get_name();    

    return get_dirty_labors().size();
}

QTreeWidgetItem *Dwarf::get_pending_changes_tree() {
    QVector<int> labors = get_dirty_labors();
    QTreeWidgetItem *d_item = new QTreeWidgetItem;
    d_item->setText(0, QString("%1 (%2)").arg(nice_name()).arg(labors.size()));
    d_item->setData(0, Qt::UserRole, id());
    if (m_caged != m_flag1) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Caged change to %1").arg(hexify(m_caged)));
        i->setIcon(0, QIcon(":img/book_edit.png"));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_butcher != m_flag2) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Butcher change to %1").arg(hexify(m_butcher)));
        i->setIcon(0, QIcon(":img/book_edit.png"));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_nick_name != m_nick_name) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString nick = m_pending_nick_name;
        if (nick.isEmpty())
            nick = tr("DEFAULT");
        i->setText(0, tr("Nickname change to %1").arg(nick));
        i->setIcon(0, QIcon(":img/book_edit.png"));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_custom_profession != m_custom_profession) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        QString prof = m_pending_custom_profession;
        if (prof.isEmpty())
            prof = tr("DEFAULT");
        i->setText(0, tr("Profession change to %1").arg(prof));
        i->setIcon(0, QIcon(":img/book_edit.png"));
        i->setData(0, Qt::UserRole, id());
    }
    foreach(int labor_id, labors) {
        Labor *l = GameDataReader::ptr()->get_labor(labor_id);
        MilitaryPreference *mp = GameDataReader::ptr()->get_military_preference(labor_id);

        if (!l || l->labor_id != labor_id) {
            // this may be a mil pref not a labor..
            if (!mp || mp->labor_id != labor_id) {
                LOGW << "somehow got a change to an unknown labor with id:" << labor_id;
                continue;
            }
        }

        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        if (l) {
            i->setText(0, l->name);
            if (labor_enabled(labor_id)) {
                i->setIcon(0, QIcon(":img/plus-circle.png"));
            } else {
                i->setIcon(0, QIcon(":img/minus-circle.png"));
            }
        } else if (mp) {
            i->setText(0, QString("Set %1 to %2").arg(mp->name).arg(mp->value_name(pref_value(labor_id))));
            i->setIcon(0, QIcon(":img/arrow-switch.png"));
        }
        i->setData(0, Qt::UserRole, id());
    }
    return d_item;
}

QString Dwarf::tooltip_text() {
    QSettings *s = DT->user_settings();
    GameDataReader *gdr = GameDataReader::ptr();
    QString skill_summary, trait_summary, roles_summary, preference_summary;
    int max_roles = s->value("options/role_count_tooltip",3).toInt();
    if(max_roles > m_sorted_role_ratings.count())
        max_roles = m_sorted_role_ratings.count();

    //in some mods animals, like golems, may have skills
    if(!m_skills.isEmpty() && s->value("options/tooltip_show_skills",true).toBool()){
        QVector<Skill> *skills = get_skills();
        qSort(skills->begin(),skills->end(),Skill::less_than_key());

        int max_level = s->value("options/min_tooltip_skill_level", true).toInt();
        for (int i = skills->size() - 1; i >= 0; --i) {
            if (skills->at(i).capped_level() < max_level) {
                continue;
            }
            skill_summary.append(QString("<li>%1</li>").arg(skills->at(i).to_string()));
        }
    }

    if(!m_is_animal){
        if(!m_traits.isEmpty() && s->value("options/tooltip_show_traits",true).toBool()){
            QStringList notes;
            for (int i = 0; i < m_traits.size(); ++i) {
                notes.clear();
                if (trait_is_active(i)){
                    Trait *t = gdr->get_trait(i);
                    if (!t)
                        continue;
                    int val = m_traits.value(i);
                    trait_summary.append(t->level_message(val));
                    QString temp = t->conflicts_messages(val);
                    if(!temp.isEmpty())
                        notes.append("<u>" + temp + "</u>");
                    temp = t->special_messages(val);
                    if(!temp.isEmpty())
                        notes.append(temp);

                    if(!notes.isEmpty())
                        trait_summary.append(" (" + notes.join(" and ") + ")");

                    trait_summary.append(", ");
                }
            }
            if(trait_summary.lastIndexOf(",") == trait_summary.length()-2)
                trait_summary.chop(2);
        }

        if(!m_sorted_role_ratings.isEmpty() && max_roles > 0 && s->value("options/tooltip_show_roles",true).toBool()){
            roles_summary.append("<ol style=\"margin-top:0px; margin-bottom:0px;\">");
            for(int i = 0; i < max_roles; i++){
                roles_summary += tr("<li>%1  (%2%)</li>").arg(m_sorted_role_ratings.at(i).first)
                        .arg(QString::number(m_sorted_role_ratings.at(i).second,'f',2));
            }
            roles_summary.append("</ol>");
        }

        if(!m_preferences.isEmpty() && s->value("options/tooltip_show_preferences",true).toBool()){
            preference_summary = "<b>Preferences: </b>";
            foreach(QString key, m_grouped_preferences.uniqueKeys()){
                preference_summary.append(m_grouped_preferences.value(key)->join(", ")).append(", ");
            }
            if(!preference_summary.isEmpty())
                preference_summary.chop(2);
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

    if(!m_is_animal && s->value("options/tooltip_show_noble",true).toBool())
        tt.append(tr("<b>Profession:</b> %1").arg(profession()));

    if(!m_is_animal && m_squad_id > -1 && s->value("options/tooltip_show_squad",true).toBool())
        tt.append(tr("<b>Squad:</b> %1").arg(m_squad_name));

    if(!m_is_animal && m_noble_position != "" && s->value("options/tooltip_show_noble",true).toBool())
        tt.append(tr("<b>Noble Position%1:</b> %2").arg(m_noble_position.indexOf(",") > 0 ? "s" : "").arg(m_noble_position));

    if(s->value("options/tooltip_show_happiness",true).toBool())
        tt.append(tr("<b>Happiness:</b> %1 (%2)").arg(happiness_name(m_happiness)).arg(m_raw_happiness));

    if(!m_is_animal && !m_thought_desc.isEmpty() && s->value("options/tooltip_show_thoughts",true).toBool())
        tt.append(tr("<p style=\"margin:0px;\"><b>Thoughts: </b>%1</p>").arg(m_thought_desc));

    if(!skill_summary.isEmpty())
        tt.append(tr("<h4 style=\"margin:0px;\"><b>Skills:</b></h4><ul style=\"margin:0px;\">%1</ul>").arg(skill_summary));

    if(!m_is_animal && s->value("options/tooltip_show_mood",false).toBool())
        tt.append(tr("<b>Highest Moodable Skill:</b> %1")
                  .arg(gdr->get_skill_name(m_highest_moodable_skill, true)));

    if(!trait_summary.isEmpty())
        tt.append(tr("<p style=\"margin:0px;\"><b>Traits:</b> %1</p>").arg(trait_summary));

    if(!preference_summary.isEmpty())
        tt.append(tr("<p style=\"margin:0px;\">%1</p>").arg(preference_summary));

    if(!roles_summary.isEmpty())
        tt.append(tr("<h4 style=\"margin:0px;\"><b>Top %1 Roles:</b></h4>%2").arg(max_roles).arg(roles_summary));

    if(m_is_animal)
        tt.append(tr("<p style=\"margin:0px;\"><b>Trained Level:</b> %1</p>").arg(get_animal_trained_descriptor(m_animal_type)));

    if(s->value("options/tooltip_show_caste",true).toBool() && caste_desc() != "")
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

    //TEST READING SYNDROMES ******
//    QList<int> syns;
//    QStringList syn_names;
//    QVector<VIRTADDR> active_syns = m_df->enumerate_vector(m_address + 0x918 - 0x4);
//    QVector<VIRTADDR> all_syns = m_df->enumerate_vector(m_df->get_memory_correction() + 0x1875384-0x4);
//    foreach(VIRTADDR syn, active_syns){
//        bool is_sick = m_df->read_byte(syn + 0x38); //only show buffs/good syndromes?
//        if(!is_sick){
//            int id = m_df->read_int(syn);
//            syns.append(m_df->read_int(syn));

//            VIRTADDR syn_ptr = all_syns.at(id);
//            QString name = m_df->read_string(syn_ptr);
//            if(!name.trimmed().isEmpty())
//                syn_names.append(name);
//        }
//    }
//    if(syn_names.length()>0){
//        tt += tr("<b>Buffs:</b> %1").arg(syn_names.join(", "));
//    }
    //******************************

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
    QByteArray data = m_df->get_data(m_address, 0xb90);
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
        f->write(QString("NAME: %1\n").arg(nice_name()).toAscii());
        f->write(QString("ADDRESS: %1\n").arg(hexify(m_address)).toAscii());
        QByteArray data = m_df->get_data(m_address, 0xb90);
        f->write(m_df->pprint(data).toAscii());
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
    foreach(Skill s, m_skills) {
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
    foreach(Skill s, m_skills) {
        if(s.raw_level() > 0)
            ret_val += s.raw_level();
    }
    return ret_val;
}

int Dwarf::total_assigned_labors(bool include_hauling) {
    // get the list of identified labors from game_data.ini
    int ret_val = 0;
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        if(!include_hauling && l->is_hauling)
            continue;
        if (m_labors[l->labor_id] > 0)
            ret_val++;    
    }
    return ret_val;
}

//calculates the caste weighted attribute ratings, taking into account potential gain
//this should always be done prior to the first role rating calculations so the values are already stored
void Dwarf::calc_attribute_ratings(){
    for(int i = 0; i < m_attributes.count(); i++){
        DwarfStats::get_att_caste_role_rating(m_attributes[i]);
    }
}

void Dwarf::calc_role_ratings(){
    m_role_ratings.clear();
    m_sorted_role_ratings.clear();

    foreach(Role *m_role, GameDataReader::ptr()->get_roles()){
        if(m_role){
            //if we have a script, use that
            if(m_role->script != ""){
                QScriptEngine m_engine;
                QScriptValue d_obj = m_engine.newQObject(this);
                m_engine.globalObject().setProperty("d", d_obj);
                float rating_total = m_engine.evaluate(m_role->script).toNumber(); //just show the raw value the script generates
                m_role_ratings.insert(m_role->name,rating_total);
            }else
            {
                m_role_ratings.insert(m_role->name, calc_role_rating(m_role)); //rating_total);
            }
        }
    }
    m_raw_role_ratings = m_role_ratings;
}

float Dwarf::calc_role_rating(Role *m_role){
    float rating_att = 0.0;
    float rating_trait = 0.0;
    float rating_skill = 0.0;
    float rating_prefs = 0.0;
    float rating_total = 0.0;

    float aspect_value = 0.0;

    //adjust our global weights here to 0 if the aspect count is <= 0
    float attrib_weight = m_role->attributes.count() <= 0 ? 0 : m_role->attributes_weight.weight;
    float skill_weight = m_role->skills.count() <= 0 ? 0 : m_role->skills_weight.weight;
    float trait_weight = m_role->traits.count() <= 0 ? 0 : m_role->traits_weight.weight;
    float pref_weight = m_role->prefs.count() <= 0 ? 0 : m_role->prefs_weight.weight;

    RoleAspect *a;

    //read the attributes, traits and skills, and calculate the ratings
    float total_weight = 0.0;
    float weight = 1.0;

    //**************** ATTRIBUTES ****************
    if(m_role->attributes.count()>0){

        int attrib_id = 0;
        aspect_value = 0;
        foreach(QString name, m_role->attributes.uniqueKeys()){
            a = m_role->attributes.value(name);
            weight = a->weight;

            name = name.toLower();
            //map the user's attribute name to enum
            if(name == "strength"){attrib_id = AT_STRENGTH;}
            else if(name == "agility"){attrib_id = AT_AGILITY;}
            else if(name == "toughness"){attrib_id = AT_TOUGHNESS;}
            else if(name == "endurance"){attrib_id = AT_ENDURANCE;}
            else if(name == "recuperation"){attrib_id = AT_RECUPERATION;}
            else if(name == "disease resistance"){attrib_id = AT_DISEASE_RESISTANCE;}
            else if(name == "analytical ability"){attrib_id = AT_ANALYTICAL_ABILITY;}
            else if(name == "focus"){attrib_id = AT_FOCUS;}
            else if(name == "willpower"){attrib_id = AT_WILLPOWER;}
            else if(name == "creativity"){attrib_id = AT_CREATIVITY;}
            else if(name == "intuition"){attrib_id = AT_INTUITION;}
            else if(name == "patience"){attrib_id = AT_PATIENCE;}
            else if(name == "memory"){attrib_id = AT_MEMORY;}
            else if(name == "linguistic ability"){attrib_id = AT_LINGUISTIC_ABILITY;}
            else if(name == "spatial sense"){attrib_id = AT_SPATIAL_SENSE;}
            else if(name == "musicality"){attrib_id = AT_MUSICALITY;}
            else if(name == "kinesthetic sense"){attrib_id = AT_KINESTHETIC_SENSE;}
            else if(name == "empathy"){attrib_id = AT_EMPATHY;}
            else if(name == "social awareness"){attrib_id = AT_SOCIAL_AWARENESS;}

//            aspect_value = DwarfStats::get_att_default_role_rating(
//                        GameDataReader::ptr()->get_attributes().value(attrib_id)->m_aspect_type
//                        , attribute(attrib_id));


            aspect_value = get_attribute(attrib_id).rating(true);
            if(aspect_value < 0)
                aspect_value = DwarfStats::get_att_caste_role_rating(m_attributes[attrib_id]);//get_attribute(attrib_id));

//            aspect_value = DwarfStats::get_att_caste_role_rating(static_cast<ATTRIBUTES_TYPE>(attrib_id),attribute(attrib_id));



            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_att += (aspect_value*weight);

            total_weight += weight;

        }
        rating_att = (rating_att / total_weight) * 100; //weighted average percentile
    }
    //********************************


    //**************** TRAITS ****************
    if(m_role->traits.count()>0)
    {
        total_weight = 0;
        aspect_value = 0;
        foreach(QString trait_id, m_role->traits.uniqueKeys()){
            a = m_role->traits.value(trait_id);
            weight = a->weight;
            aspect_value = DwarfStats::get_trait_role_rating(
                        GameDataReader::ptr()->get_trait(trait_id.toInt())->m_aspect_type
                        , trait(trait_id.toInt()));
            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_trait += (aspect_value * weight);

            total_weight += weight;
        }
        rating_trait = (rating_trait / total_weight) * 100;//weighted average percentile
    }
    //********************************


    //************ SKILLS ************
    float skill_rate_value = 0.0;
    float skill_rate_weight = DT->user_settings()->value("options/default_skill_rate_weight",0.25).toDouble();
    if(m_role->skills.count()>0){
        total_weight = 0;
        aspect_value = 0;
        skill_rate_value = 0;
        Skill s;
        foreach(QString skill_id, m_role->skills.uniqueKeys()){
            a = m_role->skills.value(skill_id);
            weight = a->weight;

            s = this->get_skill(skill_id.toInt());
            aspect_value = s.capped_level_precise() / 20.0f;

            if(DT->multiple_castes){
                //use a weighted average of the level and simulated xp rating
                skill_rate_value = s.get_simulated_rating();
                aspect_value = (aspect_value * (1.0f-skill_rate_weight)) + (skill_rate_value * skill_rate_weight);
            }

            if(aspect_value > 1.0)
                aspect_value = 1.0;
            if(a->is_neg)
                aspect_value = 1-aspect_value;
            rating_skill += (aspect_value*weight);

            total_weight += weight;

            //s = 0;
        }
        rating_skill = (rating_skill / total_weight) * 100;//weighted average percentile
    }
    //********************************

    //************ PREFERENCES ************
    if(m_role->prefs.count()>0){
        total_weight = 0;
        aspect_value = 0;
        Preference *dwarf_pref;
        int total_match_count = 0;
        int key = 0;
        int match_count;
        foreach(Preference *role_pref,m_role->prefs){
            total_match_count = 0;
            key = role_pref->get_pref_category();
            QMultiMap<int, Preference *>::iterator i = m_preferences.find(key);
            while(i != m_preferences.end() && i.key() == key){
                dwarf_pref = i.value();
                match_count = dwarf_pref->matches(role_pref);

                if(match_count > 0)
                    //LOGD << nice_name() << " " << dwarf_pref->get_name() << " matches " << role_pref->get_name();

                //if it's a weapon, and a match, ensure the dwarf can actually wield it as well
                if(match_count > 0 && role_pref->get_item_type() == WEAPON){
                    Weapon *w = m_df->get_weapon(capitalizeEach(dwarf_pref->get_name()));
                    if(!w || (m_body_size < w->single_grasp() && m_body_size < w->multi_grasp()))
                        match_count = 0;
                }
                total_match_count += match_count;
                i++;
            }

            if(total_match_count > 0)
                aspect_value = 1.0;
            else
                aspect_value = 0.0;

            weight = role_pref->pref_aspect->weight;
            if(role_pref->pref_aspect->is_neg)
                aspect_value = 1-aspect_value;

            rating_prefs += (aspect_value*weight);
            total_weight += weight;
        }        
        rating_prefs = (rating_prefs / total_weight) * 100;//weighted average percentile
    }

    //********************************

    if((attrib_weight + skill_weight + trait_weight + pref_weight) == 0){
        rating_total = 0;
    }else{
        rating_total = ((rating_att * attrib_weight)+(rating_skill * skill_weight)
                        +(rating_trait * trait_weight)+(rating_prefs * pref_weight))
                / (attrib_weight + skill_weight + trait_weight + pref_weight); //weighted average percentile total
    }

    return rating_total;
}

float Dwarf::get_role_rating(QString role_name, bool raw){
    if(raw)
        return m_raw_role_ratings.value(role_name);
    else
        return m_role_ratings.value(role_name);
}
void Dwarf::set_role_rating(QString role_name, float value){
    m_role_ratings.insert(role_name,value);
}
void Dwarf::update_rating_list(){
    //keep a sorted list of the ratings as well
    foreach(QString name, m_role_ratings.uniqueKeys()){        
        m_sorted_role_ratings << qMakePair(name,m_role_ratings.value(name));
    }
    qSort(m_sorted_role_ratings.begin(),m_sorted_role_ratings.end(), &Dwarf::sort_ratings);
    //refresh the tooltip as well
    //tooltip_text();
}

Reaction *Dwarf::get_reaction()
{
    if(m_current_sub_job_id.isEmpty())
        return 0;
    else
        return m_df->get_reaction(m_current_sub_job_id);
}

bool Dwarf::has_preference(QString pref_name, QString category, bool exactMatch){
    QRegExp str_search("(" + pref_name + ")",Qt::CaseInsensitive, QRegExp::RegExp);
    QString pref = "";

    if(category.isEmpty()){
        if(exactMatch){
            foreach(QString key, m_grouped_preferences.uniqueKeys()){
                return m_grouped_preferences.value(key)->contains(pref_name,Qt::CaseInsensitive);
            }
        }else{
            foreach(QString key, m_grouped_preferences.uniqueKeys()){
                pref = m_grouped_preferences.value(key)->join(", ");
                if(pref.contains(str_search))
                    return true;
            }
        }

        return false;
    }else{
        if(!m_grouped_preferences.keys().contains(category))
            return false;
        if(exactMatch){
            return m_grouped_preferences.value(category)->contains(pref_name,Qt::CaseInsensitive);
        }else{
            pref = m_grouped_preferences.value(category)->join(", ");
            if(pref.contains(str_search))
                return true;
        }
        return false;
    }
}
