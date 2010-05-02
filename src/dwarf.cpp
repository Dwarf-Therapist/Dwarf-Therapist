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
    , m_address(addr)
    , m_race_id(-1)
    , m_happiness(DH_MISERABLE)
    , m_raw_happiness(0)
    , m_money(0)
    , m_is_male(true)
    , m_show_full_name(false)
    , m_total_xp(0)
    , m_migration_wave(0)
    , m_raw_profession(-1)
    , m_can_set_labors(false)
    , m_strength(-1)
    , m_agility(-1)
    , m_toughness(-1)
    , m_current_job_id(-1)
    , m_squad_leader_id(-1)
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
}


Dwarf::~Dwarf() {
    m_traits.clear();
    foreach(QAction *a, m_actions) {
        a->deleteLater();
    }
    m_actions.clear();
    m_skills.clear();
}

void Dwarf::refresh_data() {
    MemoryLayout *mem = m_df->memory_layout();
    TRACE << QString("Starting refresh of dwarf data at 0x%1").arg(m_address, 8, 16, QChar('0'));

    m_id = m_df->read_int(m_address + mem->dwarf_offset("id"));
    //m_id = m_address; // HACK: this will allow dwarfs in the list even when
    // the id offset isn't know for this version
    TRACE << "\tID:" << m_id;
    char sex = m_df->read_char(m_address + mem->dwarf_offset("sex"));
    m_is_male = (int)sex == 1;
    TRACE << "\tMALE?" << m_is_male;

    m_first_name = m_df->read_string(m_address + mem->dwarf_offset("first_name"));
    if (m_first_name.size() > 1)
        m_first_name[0] = m_first_name[0].toUpper();
    TRACE << "\tFIRSTNAME:" << m_first_name;
    m_nick_name = m_df->read_string(m_address + mem->dwarf_offset("nick_name"));
    TRACE << "\tNICKNAME:" << m_nick_name;
    m_pending_nick_name = m_nick_name;
    m_last_name = read_last_name(m_address + mem->dwarf_offset("last_name"));
    TRACE << "\tLASTNAME:" << m_last_name;
    m_translated_last_name = read_last_name(m_address + mem->dwarf_offset("last_name"), true);
    calc_names();
    m_custom_profession = m_df->read_string(m_address + mem->dwarf_offset("custom_profession"));
    TRACE << "\tCUSTOM PROF:" << m_custom_profession;
    m_pending_custom_profession = m_df->read_string(m_address + mem->dwarf_offset("custom_profession"));
    m_race_id = m_df->read_int(m_address + mem->dwarf_offset("race"));
    TRACE << "\tRACE ID:" << m_race_id;
    m_profession = read_profession(m_address + mem->dwarf_offset("profession"));
    TRACE << "\tPROFESSION:" << m_profession;
    m_strength = m_df->read_int(m_address + mem->dwarf_offset("strength"));
    TRACE << "\tSTRENGTH:" << m_strength;
    m_toughness = m_df->read_int(m_address + mem->dwarf_offset("toughness"));
    TRACE << "\tTOUGHNESS:" << m_toughness;
    m_agility = m_df->read_int(m_address + mem->dwarf_offset("agility"));
    TRACE << "\tAGILITY:" << m_agility;
    read_labors(m_address + mem->dwarf_offset("labors"));
    m_money = m_df->read_int(m_address + mem->dwarf_offset("money"));
    TRACE << "\tMONEY:" << m_money;
    m_raw_happiness = m_df->read_int(m_address +mem->dwarf_offset("happiness"));
    TRACE << "\tRAW HAPPINESS:" << m_raw_happiness;
    m_happiness = happiness_from_score(m_raw_happiness);
    TRACE << "\tHAPPINESS:" << happiness_name(m_happiness);
    read_current_job(m_address + mem->dwarf_offset("current_job"));
    TRACE << "\tCURRENT JOB:" << m_current_job_id << m_current_job;
    m_squad_leader_id = m_df->read_int(m_address + mem->dwarf_offset("squad_leader_id"));
    TRACE << "\tSQUAD LEADER ID:" << m_squad_leader_id;
    m_squad_name = read_squad_name(m_address + mem->dwarf_offset("squad_name"));
    TRACE << "\tSQUAD NAME:" << m_squad_name;
    m_generic_squad_name = read_squad_name(m_address + mem->dwarf_offset("squad_name"), true);
    TRACE << "\tGENERIC SQUAD NAME:" << m_generic_squad_name;

    /* Search for a souls vector, where each soul contains a vector of skills
    QVector<uint> vectors = m_df->find_vectors_in_range(1, m_address, 0xc00);
    foreach(uint vec_addr, vectors) {
        QVector<uint> entries = m_df->enumerate_vector(vec_addr);
        uint soul = entries.at(0);
        LOGD << nice_name() << "Found vector of" << entries.size()
                << "valid entries at" << hex << vec_addr << "Offset:"
                << (vec_addr - m_address) << "SOULPTR:" << soul;
        //LOGD << m_df->pprint(soul, 0x50);
        foreach(uint skill_vec, m_df->find_vectors_in_range(50, soul, 0x250)) {
            QVector<uint> skills = m_df->enumerate_vector(skill_vec);
            LOGD << nice_name() << "souls offset:" << hex
                    << (vec_addr - m_address) << "skills offset in soul:"
                    << (skill_vec - entries.at(0)) << "total skills:"
                    << dec << skills.size();

            //m_skills = read_skills(skill_vec);
            //foreach(uint skill, skills) {
            //    LOGD << nice_name() << "\n" << m_df->pprint(skill, 0x20);
            //}
        }
    }
    */

    QVector<uint> souls = m_df->enumerate_vector(m_address + mem->dwarf_offset("souls"));
    foreach(uint soul, souls) {
        //LOGD << "SOUL FOUND AT" << hex << soul << m_df->pprint(m_df->get_data(soul, 0x500), 0);
        QVector<uint> skills = m_df->enumerate_vector(soul + mem->dwarf_offset("skills"));
        //LOGD << nice_name() << "Soul contains" << skills.size() << "skills";
        m_skills = read_skills(soul + mem->dwarf_offset("skills"));
        read_traits(soul + mem->dwarf_offset("traits"));
        TRACE << "\tTRAITS:" << m_traits.size();
    }

    //ushort position = m_df->read_ushort(m_address + mem->dwarf_offset("position"));
    //LOGD << nice_name() << "POSITION:" << position;

    TRACE << "finished refresh of dwarf data for dwarf:" << m_nice_name << "(" << m_translated_name << ")";
}

void Dwarf::read_settings() {
    /* This is pretty fucked up. There is no good way to let the options for
       full name, or show dabbling to just update existing cells. Will have to
       send signals when this stuff changes, or just bit the bullet and subclass
       the QStandardItem for the name items in the main model
       */
    QSettings *s = DT->user_settings();
    bool new_show_full_name = s->value("options/show_full_dwarf_names", false).toBool();
    if (new_show_full_name != m_show_full_name) {
        calc_names();
        emit name_changed();
    }
    m_show_full_name = new_show_full_name;
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

Dwarf *Dwarf::get_dwarf(DFInstance *df, const uint &addr) {
    MemoryLayout *mem = df->memory_layout();
    TRACE << "attempting to load dwarf at" << addr << "using memory layout" << mem->game_version();

    uint dwarf_race_index = mem->address("dwarf_race_index");
    int dwarf_race_id = df->read_int(dwarf_race_index + df->get_memory_correction());

    uint flags1 = df->read_uint(addr + mem->dwarf_offset("flags1"));
    uint flags2 = df->read_uint(addr + mem->dwarf_offset("flags2"));
    int race_id = df->read_int(addr + mem->dwarf_offset("race"));

    if (race_id != dwarf_race_id) { // we only care about dwarfs
        TRACE << "Ignoring non-dwarf creature with racial ID of " << hexify(race_id);
        return 0;
    }
    Dwarf *unverified_dwarf = new Dwarf(df, addr, df);
    /*LOGD << "examining dwarf at" << hex << addr;
    LOGD << "FLAGS1 :" << hexify(flags1);
    LOGD << "FLAGS2 :" << hexify(flags2);
    LOGD << "RACE   :" << hexify(race_id);
    LOGD << "NAME   :" << df->read_string(addr + mem->dwarf_offset("first_name"));
    LOGD << "INVADER FILTER:" << hexify(0x1000 | 0x2000 | 0x20000 | 0x80000 | 0xc0000 | 0x8C0);
    LOGD << "OTHER   FILTER:" << hexify(0x80 | 0x40000);
    */

    QHash<uint, QString> flags = mem->valid_flags_1();
    foreach(uint flag, flags.uniqueKeys()) {
        QString reason = flags[flag];
        if ((flags1 & flag) != flag) {
            LOGD << "Ignoring" << unverified_dwarf->nice_name() << "who appears to be" << reason;
            delete unverified_dwarf;
            return 0;
        }
    }

    flags = mem->invalid_flags_1();
    foreach(uint flag, flags.uniqueKeys()) {
        QString reason = flags[flag];
        if ((flags1 & flag) == flag) {
            LOGD << "Ignoring" << unverified_dwarf->nice_name() << "who appears to be" << reason;
            delete unverified_dwarf;
            return 0;
        }
    }

    flags = mem->invalid_flags_2();
    foreach(uint flag, flags.uniqueKeys()) {
        QString reason = flags[flag];
        if ((flags2 & flag) == flag) {
            LOGD << "Ignoring" << unverified_dwarf->nice_name() << "who appears to be" << reason;
            delete unverified_dwarf;
            return 0;
        }
    }

    //HACK: ugh... so ugly, but this seems to be the best way to filter out kidnapped babies
    short baby_id = -1;
    foreach(Profession *p, GameDataReader::ptr()->get_professions()) {
        if (p->name(true) == "Baby") {
            baby_id = p->id();
            break;
        }
    }
    if (unverified_dwarf->raw_profession() == baby_id) {
        if ((flags1 & 0x200) != 0x200) {
            // kidnapped flag? seems like it
            LOGD << "Ignoring" << unverified_dwarf->nice_name() << "who appears to be a kidnapped baby";
            delete unverified_dwarf;
            return 0;
        }
    }

    return unverified_dwarf;
}

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

QString Dwarf::read_last_name(const uint &addr, bool use_generic) {
    // last name reading taken from patch by Zhentar (issue 189)
    QString first, second, third;

    first.append(word_chunk(m_df->read_uint(addr), use_generic));
    first.append(word_chunk(m_df->read_uint(addr + 0x4), use_generic));
    second.append(word_chunk(m_df->read_uint(addr + 0x8), use_generic));
    second.append(word_chunk(m_df->read_uint(addr + 0x14), use_generic));
    third.append(word_chunk(m_df->read_uint(addr + 0x18), use_generic));

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

void Dwarf::read_traits(const uint &addr) {
    m_traits.clear();
    for (int i = 0; i < 30; ++i) {
        short val = m_df->read_short(addr + i * 2);
        int deviation = abs(val - 50); // how far from the norm is this trait?
        if (deviation <= 10) {
            val = -1; // this will cause median scores to not be treated as "active" traits
        }
        m_traits.insert(i, val);
    }
}

void Dwarf::read_prefs(const uint &addr) {
    /*
Looks like this as a dump where the start of each line is the raw 4 * 6 bytes of each like struct
5th byte appears to point at a massive vector with 14169 entries, possible object vector?
    the 5th byte of the first and second entry both point at the same massive vector (at least same # of entries)
---------------------------------------------------------------------------------
00 00 ff ff | ff ff 01 00 | c3 00 01 00 | 67 52 cb 39 | f0 0f 6e 0c | 19 00 00 00 (stone: realgar)
00 00 ff ff | ff ff 02 00 | 03 00 01 00 | 7c dd da 15 | f0 12 6e 0c | 19 00 00 00 (metal: bismuth)
00 00 ff ff | ff ff 01 00 | 73 00 01 00 | 4d 0a 43 0b | f0 15 6e 0c | 19 00 00 00 (stone(gem): rhodolite)
00 00 ff ff | ff ff 0d 00 | ff ff 01 00 | aa 16 49 24 | f0 18 6e 0c | 19 00 00 00 (wood: mango tree)
00 00 ff ff | ff ff 0a 00 | 5f 00 00 00 | 23 d2 54 2f | f0 1b 6e 0c | 19 00 00 00 (body part?: horn)
00 00 ff ff | ff ff 0b 00 | 72 00 00 00 | 08 83 c8 3e | f0 1e 6e 0c | 19 00 00 00 (body part?: cave lobster shell)
04 00 3f 00 | 01 00 ff ff | ff ff 01 00 | 69 47 00 2b | f0 21 6e 0c | 19 00 00 00 (color: light brown)
04 00 56 00 | ff ff ff ff | ff ff 01 00 | 16 c2 0b 12 | f0 24 6e 0c | 19 00 00 00 (donkeys: for stubborness)
02 00 2f 00 | ff ff 37 00 | ff ff 00 00 | 9f a3 02 0c | f0 27 6e 0c | 19 00 00 00 (cave spiders: for mystery)
02 00 4c 00 | ff ff 0a 00 | 66 00 01 00 | 54 5f d7 2f | f0 2a 6e 0c | 19 00 00 00 (when possible: Dwarven Wine)
02 00 4a 00 | ff ff 20 00 | 03 00 01 00 | 45 1c 7f 3c | f0 2d 6e 0c | 19 00 00 00 (when possible: Dwarven wheat flour)
03 00 01 00 | ff ff ff ff | ff ff 00 00 | 42 b2 36 26 | f0 30 6e 0c | 21 00 00 00 (detests purring maggots)
    */
    /*
    uint stone_vector = 0x08f9cab0;
    QVector<uint> stones = m_df->enumerate_vector(stone_vector);
    foreach(uint stone, stones) {
        LOGD << "material at" << hex << stone << m_df->read_string(stone);
    }
    */
    QVector<uint> addrs = m_df->enumerate_vector(addr);
    LOGD << "reading prefs for" << m_nice_name << "found" << addrs.size() << "preference entries";
    foreach(int addr, addrs) {
        short type = m_df->read_short(addr + 0x06);
        short sub_type = m_df->read_short(addr + 0x08);
        LOGD << "\tTYPE" << type << hex << type << "SUBTYPE" << dec << sub_type << hex << sub_type;
        /*
        switch (type) {
            case 1: // stone
                LOGD << "\tSTONE TYPE" << m_df->read_string(stones.at(sub_type));
                break;
            case 2: // metal
                LOGD << "\tMETAL TYPE" << "UNKNOWN";
                break;
            case 0xd: // wood (tree)
                LOGD << "\tWOOD TYPE UNKNOWN";
                break;
        }
        */
        //int when_possible = (int)m_df->read_char(addr + 0x0A);
    }
}

QVector<Skill> Dwarf::read_skills(const uint &addr) {
    m_total_xp = 0;
    QVector<Skill> skills(0);
    QVector<uint> entries = m_df->enumerate_vector(addr);
    TRACE << "Reading skills for" << nice_name() << "found:" << entries.size();
    short type = 0;
    short rating = 0;
    int xp = 0;
    int last_used = 0;
    int rust = 0;
    int rust_counter = 0;
    int demotion_counter = 0;
    foreach(uint entry, entries) {
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
        skills.append(s);
    }
    return skills;
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

QString Dwarf::read_profession(const uint &addr) {
    char buffer[1];
    m_df->read_raw(addr, 1, &buffer[0]);
    m_raw_profession = (short)buffer[0];
    Profession *p = GameDataReader::ptr()->get_profession(m_raw_profession);
    QString prof_name = tr("Unknown Profession %1").arg(m_raw_profession);
    if (p) {
        m_can_set_labors = p->can_assign_labors();
        prof_name = QString("(%1) %2").arg(m_raw_profession).arg(p->name(m_is_male));
    } else {
        LOGE << tr("Read unknown profession with id '%1' for dwarf '%2'")
                .arg(m_raw_profession).arg(m_nice_name);
        m_can_set_labors = false;
    }
    if (!m_custom_profession.isEmpty()) {
        return m_custom_profession;
    } else {
        return prof_name;
    }
}

void Dwarf::read_current_job(const uint &addr) {
    // TODO: jobs contain info about materials being used, if we ever get the
    // material list we could show that in here
    uint current_job_addr = m_df->read_uint(addr);
    if (current_job_addr != 0) {
        m_current_job_id = m_df->read_ushort(current_job_addr + m_df->memory_layout()->offset("current_job_id"));
        DwarfJob *job = GameDataReader::ptr()->get_job(m_current_job_id);
        if (job)
            m_current_job = job->description;
        else
            m_current_job = tr("Unknown job");
    } else {
        m_current_job = tr("No Job");
    }

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

void Dwarf::read_labors(const uint &addr) {
    // read a big array of labors in one read, then pick and choose
    // the values we care about
    uchar buf[150];
    memset(buf, 0, 150);
    m_df->read_raw(addr, 150, &buf);

    // get the list of identified labors from game_data.ini
    GameDataReader *gdr = GameDataReader::ptr();
    foreach(Labor *l, gdr->get_ordered_labors()) {
        if (l->is_weapon && l->labor_id < 0) // unarmed
            continue;
        bool enabled = buf[l->labor_id] > 0;
        m_labors[l->labor_id] = enabled;
        m_pending_labors[l->labor_id] = enabled;
    }
    // also store prefs in this structure
    foreach(MilitaryPreference *mp, gdr->get_military_preferences()) {
        m_labors[mp->labor_id] = static_cast<ushort>(buf[mp->labor_id]);
        m_pending_labors[mp->labor_id] = static_cast<ushort>(buf[mp->labor_id]);
    }
}

bool Dwarf::labor_enabled(int labor_id) {
    if (labor_id < 0) {// unarmed
        bool uses_weapon = false;
        foreach(Labor *l, GameDataReader::ptr()->get_ordered_labors()) {
            if (l->is_weapon && l->labor_id > 0) {
                if (m_pending_labors[l->labor_id]) {
                    uses_weapon = true;
                    break;
                }
            }
        }
        return !uses_weapon;
    } else {
        return m_pending_labors.value(labor_id, false);
    }
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

    if (!m_can_set_labors && !DT->labor_cheats_allowed() && !l->is_weapon) {
        LOGD << "IGNORING SET LABOR OF ID:" << labor_id << "TO:" << enabled << "FOR:" << m_nice_name << "PROF_ID" << m_raw_profession
             << "PROF_NAME:" << profession() << "CUSTOM:" << m_pending_custom_profession;
        return;
    }

    if (enabled) { // user is turning a labor on, so we must turn off exclusives
        foreach(int excluded, l->get_excluded_labors()) {
            LOGD << "LABOR" << labor_id << "excludes" << excluded;
            m_pending_labors[excluded] = false;
        }
    }
    if (enabled && l->is_weapon) { // weapon type labors are automatically exclusive
        foreach(Labor *l, GameDataReader::ptr()->get_ordered_labors()) {
            if (l && l->is_weapon)
                m_pending_labors[l->labor_id] = false;
        }
    }
    m_pending_labors[labor_id] = enabled;
}

int Dwarf::pending_changes() {
    int cnt = get_dirty_labors().size();
    if (m_nick_name != m_pending_nick_name)
        cnt++;
    if (m_custom_profession != m_pending_custom_profession)
        cnt++;
    return cnt;
}

void Dwarf::clear_pending() {
    refresh_data();
}

void Dwarf::commit_pending() {
    MemoryLayout *mem = m_df->memory_layout();
    int addr = m_address + mem->dwarf_offset("labors");

    uchar buf[102];
    memset(buf, 0, 102);
    m_df->read_raw(addr, 102, &buf); // set the buffer as it is in-game
    foreach(int labor_id, m_pending_labors.uniqueKeys()) {
        if (labor_id < 0)
            continue;
        // change values to what's pending
        buf[labor_id] = m_pending_labors.value(labor_id);
    }

    m_df->write_raw(addr, 102, &buf);
    if (m_pending_nick_name != m_nick_name)
        m_df->write_string(m_address + mem->dwarf_offset("nick_name"), m_pending_nick_name);
    if (m_pending_custom_profession != m_custom_profession)
        m_df->write_string(m_address + mem->dwarf_offset("custom_profession"), m_pending_custom_profession);
    refresh_data();
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
    te->setText(m_df->pprint(m_df->get_data(m_address, 0xb90), 0));
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
        f->write(m_df->pprint(m_df->get_data(m_address, 0xb90), 0).toAscii());
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

/************************************************************************/
/* SQUAD STUFF                                                          */
/************************************************************************/
Dwarf *Dwarf::get_squad_leader() {
    // Oh I will be...
    if (m_squad_leader_id <= 0) {
        // I'll be squad leader!
        return 0;
    } else {
        return DT->get_dwarf_by_id(m_squad_leader_id); // Tony Danza
    }
}

QString Dwarf::read_squad_name(const uint &addr, bool use_generic) {
    QString out;
    for (int i = 0; i < 24; i+=4) {
        int word_offset = m_df->read_int(addr + i);
        if (word_offset == 0 || word_offset == 0xFFFFFFFF)
            continue;
        if (use_generic)
            out += DT->get_generic_word(word_offset);
        else
            out += DT->get_dwarf_word(word_offset);
    }
    if (out.size())
        out[0] = out.at(0).toUpper();
    return out;
}

QString Dwarf::squad_name() {
    if (m_squad_leader_id > 0) // follow the chain up
        return get_squad_leader()->squad_name();
    else
        return m_squad_name;
}

void Dwarf::add_squad_member(Dwarf *d) {
    if (d && !m_squad_members.contains(d))
        m_squad_members << d;
}
