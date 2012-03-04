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
#include "dwarf.h"
#include "dfinstance.h"
#include "skill.h"
#include "labor.h"
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
#include "utils.h"

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
    , m_show_full_name(false)
    , m_total_xp(0)
    , m_migration_wave(0)
    , m_raw_profession(-1)
    , m_can_set_labors(false)
    , m_current_job_id(-1)
    , m_squad_ref_id(-1)
    , m_squad_name(QString::null)
    , m_flag1(0)
    , m_flag2(0)
    , m_age(0)
{
    read_settings();
    refresh_data();
    connect(DT, SIGNAL(settings_changed()), SLOT(read_settings()));

    // setup context actions
    m_actions.clear();
    QAction *show_details = new QAction(tr("Show Details..."), this);
    connect(show_details, SIGNAL(triggered()), SLOT(show_details()));
    m_actions << show_details;
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
    m_traits.clear();
    foreach(QAction *a, m_actions) {
        a->deleteLater();
    }
    m_actions.clear();
    m_skills.clear();
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

    // read everything we need
    read_id();
    read_curse();
    read_sex();
    read_caste();
    read_race();
    read_first_name();
    read_last_name();
    read_nick_name();
    read_mood();
    calc_names();
    read_profession();
    read_labors();
    read_happiness();
    read_current_job();
    read_souls();
    read_squad_ref_id();
    read_turn_count();

    m_flag1 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags1"));
    m_flag2 = m_df->read_addr(m_address + m_mem->dwarf_offset("flags2"));
    m_pending_flag1 = m_flag1;
    m_pending_flag2 = m_flag2;
    m_age = m_df->current_year() - m_df->read_short(m_address + m_mem->dwarf_offset("birth_year"));

    TRACE << "finished refresh of dwarf data for dwarf:" << m_nice_name
            << "(" << m_translated_name << ")";
}

/*******************************************************************************
  DATA POPULATION METHODS
*******************************************************************************/

void Dwarf::read_id() {
    m_id = m_df->read_int(m_address + m_mem->dwarf_offset("id"));
    //m_id = m_address; // HACK: this will allow dwarfs in the list even when
    // the id offset isn't know for this version
    TRACE << "ID:" << m_id;
}

void Dwarf::read_sex() {
    // TODO: actually break down this caste
    BYTE sex = m_df->read_byte(m_address + m_mem->dwarf_offset("sex"));
    m_is_male = (int)sex == 1;
    TRACE << "MALE:" << m_is_male;
}

void Dwarf::read_mood(){
    m_mood_id = m_df->read_short(m_address + m_mem->dwarf_offset("mood"));
}

void Dwarf::read_curse(){
    m_curse_name = m_df->read_string(m_address + m_mem->dwarf_offset("curse"));
}

void Dwarf::read_caste() {
    m_caste_id = m_df->read_short(m_address + m_mem->dwarf_offset("caste"));
    TRACE << "CASTE:" << m_caste_id;
}

void Dwarf::read_race() {
    m_race_id = m_df->read_int(m_address + m_mem->dwarf_offset("race"));
    TRACE << "RACE ID:" << m_race_id;
}

void Dwarf::read_first_name() {
    m_first_name = m_df->read_string(m_address +
                                     m_mem->dwarf_offset("first_name"));
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

void Dwarf::read_last_name() {
    VIRTADDR addr = m_address + m_mem->dwarf_offset("last_name");

    //Generic
    bool use_generic = false;
    if (DT->user_settings()->value("options/use_generic_names", false).toBool()) {
        use_generic = true;
    }

    m_last_name = read_chunked_name(addr, use_generic);
    m_translated_last_name = read_chunked_name(addr);
}


void Dwarf::read_nick_name() {
    m_nick_name = m_df->read_string(m_address +
                                    m_mem->dwarf_offset("nick_name"));
    TRACE << "\tNICKNAME:" << m_nick_name;
    m_pending_nick_name = m_nick_name;
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
    if (is_animal())
    {
        if (m_nice_name.trimmed()=="")
        {
            m_nice_name = "Stray " + race_name(m_race_id);
            m_translated_name = m_nice_name;
        }
        else
        {
            m_nice_name += " " + race_name(m_race_id);
            m_translated_name += " " + race_name(m_race_id);
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

bool Dwarf::is_animal()
{
    return (m_df->dwarf_race_id()!=m_race_id);
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
    TRACE << "reading profession for" << nice_name() << m_raw_profession <<
            prof_name;
    TRACE << "EFFECTIVE PROFESSION:" << m_profession;
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
}

void Dwarf::read_current_job() {
    // TODO: jobs contain info about materials being used, if we ever get the
    // material list we could show that in here
    VIRTADDR addr = m_address + m_mem->dwarf_offset("current_job");
    VIRTADDR current_job_addr = m_df->read_addr(addr);

    m_current_sub_job_id.clear();

    TRACE << "Current job addr: " << hex << current_job_addr;

    if (current_job_addr != 0) {
        m_current_job_id = m_df->read_word(current_job_addr +
                                     m_df->memory_layout()->job_detail("id"));
        DwarfJob *job = GameDataReader::ptr()->get_job(m_current_job_id);
        if (job) {
            m_current_job = job->description;

            int sub_job_offset = m_df->memory_layout()->job_detail("sub_job_id");
            if(sub_job_offset != -1) {
                m_current_sub_job_id = m_df->read_string(current_job_addr + sub_job_offset);
                if(!job->reactionClass.isEmpty() && !m_current_sub_job_id.isEmpty()) {
                    RawObjectPtr reaction = GameDataReader::ptr()->
                            get_reaction(job->reactionClass, m_current_sub_job_id);
                    if(!reaction.isNull()) {
                        m_current_job = capitalize(reaction->get_value("NAME", m_current_job));
                        TRACE << "Sub job: " << m_current_sub_job_id << m_current_job;
                    }
                }
            }
        } else {
            m_current_job = tr("Unknown job");
        }
    } else {
        bool is_on_break = false;
        MemoryLayout* layout = m_df->memory_layout();
        uint states_offset = layout->dwarf_offset("states");
        if (states_offset) {
            VIRTADDR states_addr = m_address + states_offset;
            QVector<uint> entries = m_df->enumerate_vector(states_addr);
            short on_break_value = layout->job_detail("on_break_flag");
            foreach(uint entry, entries) {
                if (m_df->read_short(entry) == on_break_value) {
                    is_on_break = true;
                    break; // no pun intended
                }
            }
        }
        m_current_job = is_on_break ? tr("On Break") : tr("No Job");
    }
    TRACE << "CURRENT JOB:" << m_current_job_id << m_current_sub_job_id << m_current_job;
}

void Dwarf::read_souls() {
    VIRTADDR soul_vector = m_address + m_mem->dwarf_offset("souls");
    QVector<VIRTADDR> souls = m_df->enumerate_vector(soul_vector);
    if (souls.size() != 1) {
        LOGW << nice_name() << "has" << souls.size() << "souls!";
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
QString Dwarf::caste_name(short id) {
    QString tmp_name = GameDataReader::ptr()->get_caste_name(id);
    tmp_name = tmp_name.toLower().remove("female_").remove("male_").replace('_',' ');
    if (tmp_name.size()>0)
        tmp_name[0]=tmp_name[0].toUpper();
    if (tmp_name=="Male" || tmp_name=="Female")
        tmp_name="No caste";
    return tmp_name;
}

QString Dwarf::race_name(int id) {
    return GameDataReader::ptr()->get_race_name(id);
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

QString Dwarf::attribute_level_name(ATTRIBUTES_TYPE attribute, short val)
{
    QString msg;
    switch (attribute) {
        case AT_STRENGTH:
            msg = GameDataReader::ptr()->get_attribute_level_name("Strength", val);
            break;
        case AT_AGILITY:
            msg = GameDataReader::ptr()->get_attribute_level_name("Agility", val);
            break;
        case AT_TOUGHNESS:
            msg = GameDataReader::ptr()->get_attribute_level_name("Toughness", val);
            break;
        case AT_ENDURANCE:
            msg = GameDataReader::ptr()->get_attribute_level_name("Endurance", val);
            break;
        case AT_RECUPERATION:
            msg = GameDataReader::ptr()->get_attribute_level_name("Recuperation", val);
            break;
        case AT_DISEASE_RESISTANCE:
            msg = GameDataReader::ptr()->get_attribute_level_name("Disease Resistance", val);
            break;
    }
    return msg;
}

Dwarf *Dwarf::get_dwarf(DFInstance *df, const VIRTADDR &addr) {
    MemoryLayout *mem = df->memory_layout();
    TRACE << "attempting to load dwarf at" << addr << "using memory layout"
            << mem->game_version();

    quint32 flags1 = df->read_addr(addr + mem->dwarf_offset("flags1"));
    quint32 flags2 = df->read_addr(addr + mem->dwarf_offset("flags2"));
    quint32 flags3 = df->read_addr(addr + mem->dwarf_offset("flags3"));
    WORD race_id = df->read_word(addr + mem->dwarf_offset("race"));

    if (race_id != df->dwarf_race_id()) { // animals and other non-dwarves loaded here
//        TRACE << "Ignoring creature with race ID of " << hex << race_id;
//        return 0;
        Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
        TRACE << "examining dwarf at" << hex << addr;
        TRACE << "FLAGS1 :" << hexify(flags1);
        TRACE << "FLAGS2 :" << hexify(flags2);
        TRACE << "FLAGS3 :" << hexify(flags3);
        TRACE << "RACE   :" << hexify(race_id);

        if (mem->is_complete()) {
            QHash<uint, QString> flags = mem->valid_flags_1();

            quint32 temp;
            flags = mem->invalid_flags_1();
            foreach(uint flag, flags.uniqueKeys()) {
                QString reason = flags[flag];
                temp = flags1 & flag;
                if ((flags1 & flag) == flag) {
                    TRACE << "Ignoring" << unverified_dwarf->nice_name()
                            << "who appears to be" << reason;
                    delete unverified_dwarf;
                    return 0;
                }
                //else
                //{
                //    LOGD << unverified_dwarf->race_name((int)race_id) << flag << " " << flags1;
                //}
            }

            flags = mem->invalid_flags_2();
            foreach(uint flag, flags.uniqueKeys()) {
                QString reason = flags[flag];
                temp = flags2 & flag;
                if ((flags2 & flag) == flag) {
                    TRACE << "Ignoring" << unverified_dwarf->nice_name()
                            << "who appears to be" << reason;
                    delete unverified_dwarf;
                    return 0;
                }
            }

            flags = mem->invalid_flags_3();
            foreach(uint flag, flags.uniqueKeys()) {
                QString reason = flags[flag];
                temp = flags3 & flag;
                if ((flags3 & flag) == flag) {
                    TRACE << "Ignoring" << unverified_dwarf->nice_name()
                            << "who appears to be" << reason;
                    delete unverified_dwarf;
                    return 0;
                }
            }
        }
        return unverified_dwarf;

    }
    Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
    TRACE << "examining dwarf at" << hex << addr;
    TRACE << "FLAGS1 :" << hexify(flags1);
    TRACE << "FLAGS2 :" << hexify(flags2);
    TRACE << "FLAGS3 :" << hexify(flags3);
    TRACE << "RACE   :" << hexify(race_id);

    if (mem->is_complete()) {
        QHash<uint, QString> flags = mem->valid_flags_1();

        //valid_flags_1 0x80000000 is for important historical figures, which with the changes to migrants is probably
        //not a good idea to check any longer
//        foreach(uint flag, flags.uniqueKeys()) {
//            QString reason = flags[flag];
//            if ((flags1 & flag) != flag) {
//                LOGD << "Ignoring" << unverified_dwarf->nice_name() <<
//                        "who appears to be" << reason;
//                delete unverified_dwarf;
//                return 0;
//            }
//        }

        //if a dwarf has gone crazy (berserk=7,raving=6)
        int m_mood = unverified_dwarf->m_mood_id;
        if(m_mood==7 || m_mood==6){
            LOGD << "Ignoring" << unverified_dwarf->nice_name()
                 << "who appears to have lost their mind.";
            delete unverified_dwarf;
            return 0;
        }

        flags = mem->invalid_flags_1();
        foreach(uint flag, flags.uniqueKeys()) {
            QString reason = flags[flag];
            if ((flags1 & flag) == flag) {
                LOGD << "Ignoring" << unverified_dwarf->nice_name()
                        << "who appears to be" << reason;
                delete unverified_dwarf;
                return 0;
            }
        }

        //the only valid_flag_2 was one to check damage to vision and then assume death??
        //as expected this wasn't really filter out creatures and after adding the killed flag to invalid_flags_1
        //this has become obsolete
//        flags = mem->valid_flags_2();
//        foreach(uint flag, flags.uniqueKeys()) {
//            QString reason = flags[flag];
//            if ((flags2 & flag) != flag) {
//                LOGD << "Ignoring" << unverified_dwarf->nice_name() <<
//                        "who appears to be" << reason;
//                delete unverified_dwarf;
//                return 0;
//            }
//        }

        flags = mem->invalid_flags_2();
        foreach(uint flag, flags.uniqueKeys()) {
            QString reason = flags[flag];
            if ((flags2 & flag) == flag) {
                LOGD << "Ignoring" << unverified_dwarf->nice_name()
                        << "who appears to be" << reason;
                delete unverified_dwarf;
                return 0;
            }
        }

        flags = mem->invalid_flags_3();
        foreach(uint flag, flags.uniqueKeys()) {
            QString reason = flags[flag];
            if ((flags3 & flag) == flag) {
                LOGD << "Ignoring" << unverified_dwarf->nice_name()
                        << "who appears to be" << reason;
                delete unverified_dwarf;
                return 0;
            }
        }

        // HACK: ugh... so ugly, but this seems to be the best way to filter
        // out kidnapped babies
        short baby_id = -1;
        foreach(Profession *p, GameDataReader::ptr()->get_professions()) {
            if (p->name(true) == "Baby") {
                baby_id = p->id();
                break;
            }
        }
        if (unverified_dwarf->raw_profession() == baby_id) {
            //kidnapped flag? seems like it (0x200 is actually the 'rider' flag and sometimes flags unkidnapped babies)
            //changed to 0x100 - 'left the map' instead. check both or just add this as an invalid flag?
            if ((flags1 & 0x100) == 0x100) {
                LOGD << "Ignoring" << unverified_dwarf->nice_name() <<
                        "who appears to be a kidnapped baby";
                delete unverified_dwarf;
                return 0;
            }
        }

    }
    return unverified_dwarf;
}


bool Dwarf::get_flag_value(int bit)
{
    quint32 flg=m_pending_flag1;
    if(bit>31)
    {
        bit-=32;
        flg=m_pending_flag2;
    }
    quint32 mask = 1;
    for (int i=0;i<bit;i++)
        mask<<=1;
    return ((flg & mask)==mask);
}

void Dwarf::read_squad_ref_id() {
    m_squad_ref_id = m_df->read_int(m_address + m_mem->dwarf_offset("squad_ref_id"));
    TRACE << "Squad Reference ID:" << m_squad_ref_id;
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
    int last_used = 0;
    int rust = 0;
    int rust_counter = 0;
    int demotion_counter = 0;
    foreach(VIRTADDR entry, entries) {
        /* type, level, experience, last used counter, rust, rust counter,
        demotion counter
        */
        type = m_df->read_short(entry);
        rating = m_df->read_short(entry + 0x04);
        xp = m_df->read_int(entry + 0x08);
        last_used =m_df->read_int(entry + 0x0C);
        rust = m_df->read_int(entry + 0x10);
        rust_counter = m_df->read_int(entry + 0x14);
        demotion_counter = m_df->read_int(entry + 0x18);

        TRACE   << "reading skill at" << hex << entry << "type" << dec << type
                << "rating" << rating << "xp:" << xp << "last_used:"
                << last_used << "rust:" << rust << "rust counter:"
                << rust_counter << "demotions:" << demotion_counter;
        Skill s(type, xp, rating);
        m_total_xp += s.actual_exp();
        m_skills.append(s);
    }
}

void Dwarf::read_traits() {
    VIRTADDR addr = m_first_soul + m_mem->soul_detail("traits");
    m_traits.clear();
    for (int i = 0; i < 30; ++i) {
        short val = m_df->read_short(addr + i * 2);
        int deviation = abs(val - 50); // how far from the norm is this trait?
        if (deviation <= 5) {
            val = -1; // this will cause median scores to not be treated as "active" traits
        }
        m_traits.insert(i, val);
    }
}

void Dwarf::read_attributes() {

    m_attributes.clear();
    //read the physical attributes
    VIRTADDR addr = m_address + m_mem->dwarf_offset("physical_attrs");
    for(int i=0; i<6; i++){

        m_attributes.insert(i, (int)m_df->read_int(addr));
        addr+=0x1c;
    }
    //read the mental attributes, but append to our array (augment the key by the number of physical attribs)
    int phys_size = m_attributes.size();
    addr = m_first_soul + m_mem->soul_detail("mental_attrs");
    for(int i=0; i<13; i++){
        m_attributes.insert(i+phys_size, (int)m_df->read_int(addr));
        addr+=0x1c;
    }
}

const Skill Dwarf::get_skill(int skill_id) {
    foreach(Skill s, m_skills) {
        if (s.id() == skill_id) {
            return s;
        }
    }
    return Skill(skill_id, 0, -1);
}

short Dwarf::get_rating_by_skill(int skill_id) {
    short retval = -1;
    foreach(Skill s, m_skills) {
        if (s.id() == skill_id) {
            retval = s.rating();
            break;
        }
    }
    return retval;
}

short Dwarf::get_rating_by_labor(int labor_id) {
    GameDataReader *gdr = GameDataReader::ptr();
    Labor *l = gdr->get_labor(labor_id);
    if (l)
        return get_rating_by_skill(l->skill_id);
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
    set_labor(labor_id, !m_pending_labors[labor_id]);
    return true;
}

void Dwarf::set_labor(int labor_id, bool enabled) {
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

    if (enabled) { // user is turning a labor on, so we must turn off exclusives
        foreach(int excluded, l->get_excluded_labors()) {
            TRACE << "LABOR" << labor_id << "excludes" << excluded;
            m_pending_labors[excluded] = false;
        }
    }
    m_pending_labors[labor_id] = enabled;
}

bool Dwarf::toggle_flag_bit(int bit) {
    if (bit!=49)
        return false;
    if (!m_nice_name.startsWith("Stray"))
        return false;
    int n=bit;
    if(bit>31)
    {
        n=bit-32;
    }
    quint32 mask = 1;
    for (int i=0;i<n;i++)
        mask<<=1;
    if (bit>31)
        m_pending_flag2^=mask;
    else
        m_pending_flag1^=mask;
    return true;
}

int Dwarf::pending_changes() {
    int cnt = get_dirty_labors().size();
    if (m_nick_name != m_pending_nick_name)
        cnt++;
    if (m_custom_profession != m_pending_custom_profession)
        cnt++;
    if (m_flag1 != m_pending_flag1)
        cnt++;
    if (m_flag2 != m_pending_flag2)
        cnt++;
    return cnt;
}

void Dwarf::clear_pending() {
    refresh_data();
}

void Dwarf::commit_pending() {
    MemoryLayout *mem = m_df->memory_layout();
    int addr = m_address + mem->dwarf_offset("labors");

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
    BYTE recheck_equipment = m_df->read_byte(m_address +
                                     mem->dwarf_offset("recheck_equipment"));
    recheck_equipment |= 1;
    m_df->write_raw(m_address + mem->dwarf_offset("recheck_equipment"), 1,
                    &recheck_equipment);

    if (m_pending_nick_name != m_nick_name)
        m_df->write_string(m_address + mem->dwarf_offset("nick_name"), m_pending_nick_name);
    if (m_pending_custom_profession != m_custom_profession)
        m_df->write_string(m_address + mem->dwarf_offset("custom_profession"), m_pending_custom_profession);
    if (m_pending_flag1 != m_flag1)
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags1"), 4, &m_pending_flag1);
    if (m_pending_flag2 != m_flag2)
        m_df->write_raw(m_address + m_mem->dwarf_offset("flags2"), 4, &m_pending_flag2);
    refresh_data();
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
        set_labor(labor_id, false); // turn off everything...
    }
    foreach(int labor_id, cp->get_enabled_labors()) {
        set_labor(labor_id, true); // only turn on what this prof has enabled...
    }
    m_pending_custom_profession = cp->get_name();
    return get_dirty_labors().size();
}

QTreeWidgetItem *Dwarf::get_pending_changes_tree() {
    QVector<int> labors = get_dirty_labors();
    QTreeWidgetItem *d_item = new QTreeWidgetItem;
    d_item->setText(0, QString("%1 (%2)").arg(nice_name()).arg(labors.size()));
    d_item->setData(0, Qt::UserRole, id());
    if (m_pending_flag1 != m_flag1) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Flag1 change to %1").arg(hexify(m_pending_flag1)));
        i->setIcon(0, QIcon(":img/book_edit.png"));
        i->setData(0, Qt::UserRole, id());
    }
    if (m_pending_flag2 != m_flag2) {
        QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
        i->setText(0, tr("Flag2 change to %1").arg(hexify(m_pending_flag2)));
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
                i->setIcon(0, QIcon(":img/add.png"));
            } else {
                i->setIcon(0, QIcon(":img/delete.png"));
            }
        } else if (mp) {
            i->setText(0, QString("Set %1 to %2").arg(mp->name).arg(mp->value_name(pref_value(labor_id))));
            i->setIcon(0, QIcon(":img/arrow_switch.png"));
        }
        i->setData(0, Qt::UserRole, id());
    }
    return d_item;
}

QString Dwarf::tooltip_text() {
    QString skill_summary, trait_summary;
    QVector<Skill> *skills = get_skills();
    qSort(*skills);

    QSettings *s = DT->user_settings();
    bool show_dabbling = s->value("options/show_dabbling_in_tooltips", true)
                         .toBool();
    for (int i = skills->size() - 1; i >= 0; --i) {
        // check options to see if we should show dabbling skills
        if (skills->at(i).rating() < 1 && !show_dabbling) {
            continue;
        }
        skill_summary.append(QString("<li>%1</li>").arg(skills->at(i).to_string()));
    }
    GameDataReader *gdr = GameDataReader::ptr();
    for (int i = 0; i <= m_traits.size(); ++i) {
        if (m_traits.value(i) == -1)
            continue;
        Trait *t = gdr->get_trait(i);
        if (!t)
            continue;
        trait_summary.append(QString("%1").arg(t->level_message(m_traits.value(i))));
        if (i < m_traits.size() - 1) // second to last
            trait_summary.append(", ");
    }

    return tr("<b><font size=5>%1</font><br/><font size=3>(%2)</font></b><br/><br/>"
        "<b>Happiness:</b> %3 (%4)<br/>"
        "<b>Profession:</b> %5<br/><br/>"
        "<b>Skills:</b><ul>%6</ul><br/>"
        "<b>Traits:</b> %7<br/>")
        .arg(m_nice_name)
        .arg(m_translated_name)
        .arg(happiness_name(m_happiness))
        .arg(m_raw_happiness)
        .arg(profession())
        .arg(skill_summary)
        .arg(trait_summary);
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
    Skill highest(0, 0, 0);
    foreach(Skill s, m_skills) {
        if (s.rating() > highest.rating()) {
            highest = s;
        }
    }
    return highest;
}

int Dwarf::total_skill_levels() {
    int ret_val = 0;
    foreach(Skill s, m_skills) {
        ret_val += s.rating();
    }
    return ret_val;
}

int Dwarf::total_assigned_labors() {
    // get the list of identified labors from game_data.ini
    int ret_val = 0;
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        if (m_labors[l->labor_id] > 0)
            ret_val++;
    }
    return ret_val;
}

int Dwarf::get_attribute(int attribute)
{
    //int mean_value = 0; //set the mean value to 0 so that we're working with the raw values rather than a value adjusted to the mean
    //**finally i'm not using the mean at all
    //LOGD << "mean " << mean_value << " attribute " << attribute << " caste " << m_caste_id;
    int ret_value;
    int value = m_attributes.value(attribute);

    //modify the return 'rating' values so they're spread more from 1 to 15 so that when painting with graphics it shows more of a difference
    //(ie, shows the diamond graphic on that draw type for high values, but still show a tiny square for low values)

    //a lot of the attributes have specific ranges and a gap in them somewhere, so we'll add an additional range for that gap value
    //usually the level 5 (rating 8) as the gap is between 4 (rating 7) and 6 (rating 9)
    if(attribute==AT_AGILITY)
    {
        if(value==NULL){
            ret_value=1;
        }else if(value<151){
            ret_value=3;
        }else if(value<401){
            ret_value=5;
        }else if(value<651){
            ret_value=7;
        }else if(value<1150){
            ret_value=8;
        }else if(value<1400){
            ret_value=9;
        }else if(value<1650){
            ret_value=11;
        }else if(value<1890){
            ret_value=13;
        }else{
            ret_value=15;
        }
    }

    if (attribute==AT_STRENGTH || attribute==AT_TOUGHNESS || attribute==AT_ANALYTICAL_ABILITY ||
            attribute==AT_CREATIVITY || attribute==AT_PATIENCE || attribute==AT_MEMORY)
    {
        if(value==251){
            ret_value=1;
        }else if(value<501){
            ret_value=3;
        }else if(value<751){
            ret_value=5;
        }else if(value<1001){
            ret_value=7;
        }else if(value<1500){
            ret_value=8;
        }else if(value<1750){
            ret_value=9;
        }else if(value<2000){
            ret_value=11;
        }else if(value<2250){
            ret_value=13;
        }else{
            ret_value=15;
        }
    }

    if( attribute==AT_SPATIAL_SENSE || attribute==AT_FOCUS)
    {
        if(value==543){
            ret_value=1;
        }else if(value<793){
            ret_value=3;
        }else if(value<1043){
            ret_value=5;
        }else if(value<1293){
            ret_value=7;
        }else if(value<1792){
            ret_value=8;
        }else if(value<2042){
            ret_value=9;
        }else if(value<2292){
            ret_value=11;
        }else if(value<2542){
            ret_value=13;
        }else{
            ret_value=15;
        }
    }

    if (attribute==AT_ENDURANCE || attribute==AT_RECUPERATION || attribute==AT_DISEASE_RESISTANCE ||
            attribute==AT_INTUITION || attribute==AT_WILLPOWER || attribute==AT_KINESTHETIC_SENSE ||
            attribute==AT_LINGUISTIC_ABILITY || attribute==AT_MUSICALITY || attribute==AT_EMPATHY ||
            attribute==AT_SOCIAL_AWARENESS)
    {
        if(value==0){
            ret_value=1;
        }else if(value<253){
            ret_value=3;
        }else if(value<501){
            ret_value=5;
        }else if(value<751){
            ret_value=7;
        }else if(value<1250){
            ret_value=8;
        }else if(value<1500){
            ret_value=9;
        }else if(value<1750){
            ret_value=11;
        }else if(value<2000){
            ret_value=13;
        }else{
            ret_value=15;
        }
    }

    return ret_value;
}
