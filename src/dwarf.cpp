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
#include "defines.h"
#include "gamedatareader.h"
#include "customprofession.h"
#include "memorylayout.h"
#include "dwarftherapist.h"

Dwarf::Dwarf(DFInstance *df, const uint &addr, QObject *parent)
	: QObject(parent)
	, m_df(df)
    , m_address(addr)
{
	refresh_data();
}

void Dwarf::refresh_data() {
	MemoryLayout *mem = m_df->memory_layout();

	m_first_name = m_df->read_string(m_address + mem->dwarf_offset("first_name"));
	if (m_first_name.size() > 1)
		m_first_name[0] = m_first_name[0].toUpper();
	
	m_id = m_df->read_int(m_address + mem->dwarf_offset("id"));
	m_nick_name = m_df->read_string(m_address + mem->dwarf_offset("nick_name"));
	m_pending_nick_name = m_nick_name;
	m_last_name = read_last_name(m_address + mem->dwarf_offset("last_name"));
	m_custom_profession = m_df->read_string(m_address + mem->dwarf_offset("custom_profession"));
	m_pending_custom_profession = m_df->read_string(m_address + mem->dwarf_offset("custom_profession"));
	m_race_id = m_df->read_int(m_address + mem->dwarf_offset("race"));
	m_skills = read_skills(m_address + mem->dwarf_offset("skills"));
	m_profession = read_profession(m_address + mem->dwarf_offset("profession"));
	m_strength = m_df->read_int(m_address + mem->dwarf_offset("strength"));
	m_toughness = m_df->read_int(m_address + mem->dwarf_offset("toughness"));
	m_agility = m_df->read_int(m_address + mem->dwarf_offset("agility"));
	read_labors(m_address + mem->dwarf_offset("labors"));
	
	// NEW
	char sex = m_df->read_char(m_address + mem->dwarf_offset("sex"));
	m_is_male = (int)sex == 1;

	m_money = m_df->read_int(m_address + mem->dwarf_offset("money"));
	m_raw_happiness = m_df->read_int(m_address +mem->dwarf_offset("happiness"));
	m_happiness = happiness_from_score(m_raw_happiness);
	
	//qDebug() << nice_name() << "SEX" << (m_is_male ? "M" : "F") << " MONEY" << m_money << "ADDR" << hex << m_address;

	calc_nice_name();
	/*
	if (m_first_name == "Dumed") {
		QVector<int> vector_offsets = m_df->find_likely_vectors(m_address, 4096);
		//read_prefs(m_address + mem->dwarf_offset("likes"));
		read_prefs(m_address + 0x2a4);
		read_prefs(m_address + 0x51c);
		read_prefs(m_address + 0xaac);
		read_prefs(m_address + 0xd24);
	}*/
}

Dwarf::~Dwarf() {
}

QString Dwarf::profession() {
	if (!m_pending_custom_profession.isEmpty())
		return m_pending_custom_profession;
	if (!m_custom_profession.isEmpty())
		return m_custom_profession;
	return m_profession;
}

void Dwarf::calc_nice_name() {
	if (m_pending_nick_name.isEmpty())
		m_nice_name = QString("%1 %2").arg(m_first_name, m_last_name);
	else
		m_nice_name = QString("'%1' %2").arg(m_pending_nick_name, m_last_name);
}

Dwarf::DWARF_HAPPINESS Dwarf::happiness_from_score(int score) {
	int chunk = score / 30; // yes, integer division
	if (chunk > 5)
		chunk = 5;
	return (DWARF_HAPPINESS)(DH_MISERABLE + chunk);
}

QString Dwarf::happiness_name(DWARF_HAPPINESS happiness) {
	switch(happiness) {
		case DH_MISERABLE: return tr("Miserable");
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

    uint dwarf_race_index = df->memory_layout()->address("dwarf_race_index");
    int dwarf_race_id = df->read_int(dwarf_race_index + df->get_memory_correction());
	TRACE << "Dwarf Race ID is" << dwarf_race_id;
    //LOGD << "\n" << df->pprint(df->get_data(addr, 0x400), 0);
	
	/*
	if ((df->read_int(addr + mem->dwarf_offset("flags1")) & mem->flags("flags1.invalidate")) > 0) {
		return 0;
	}
	if ((df->read_int(addr + mem->dwarf_offset("flags2")) & mem->flags("flags2.invalidate")) > 0) {
		return 0;
	}
	*/
	if ((df->read_int(addr + mem->dwarf_offset("race"))) != dwarf_race_id) {
		return 0;
	}
	return new Dwarf(df, addr, df);
}

QString Dwarf::read_last_name(const uint &addr) {
	QString out;
	for (int i = 0; i < 7; i++) {
        uint word = m_df->read_uint(addr + i * 4);
        if(word == 0xFFFFFFFF)
			break;
        out.append(DT->get_dwarf_word(word));
	}
	if (out.size() > 1) {
		out[0] = out[0].toUpper();
	}
	return out;
}

void Dwarf::read_prefs(const uint &addr) {
	//0x051C vector at this offset, not sure what it is
    QVector<uint> addrs = m_df->enumerate_vector(addr);
	foreach(int addr, addrs) {
		//short val0 = m_df->read_short(addr, bytes_read);
		//short val1 = m_df->read_short(addr + 0x02, bytes_read);
		//short val2 = m_df->read_short(addr + 0x04, bytes_read);
		short is_rock = m_df->read_short(addr + 0x04);
		short obj_type = m_df->read_short(addr + 0x06);
		short materiel = m_df->read_short(addr + 0x08);
		int when_possible = (int)m_df->read_char(addr + 0x0A);
		//short val5 = m_df->read_short(addr + 0x0A, bytes_read);
		//short val6 = m_df->read_short(addr + 0x0C, bytes_read);
		//short val7 = m_df->read_short(addr + 0x0E, bytes_read);
		//short val8 = m_df->read_short(addr + 0x10, bytes_read);
		//short val9 = m_df->read_short(addr + 0x12, bytes_read);
		//short valA = m_df->read_short(addr + 0x14, bytes_read);
		//short valB = m_df->read_short(addr + 0x16, bytes_read);
		int x = 0;
	}
}

QVector<Skill> Dwarf::read_skills(const uint &addr) {
	QVector<Skill> skills(0);
    foreach(uint entry, m_df->enumerate_vector(addr)) {
        short type = m_df->read_short(entry);
        uint experience = m_df->read_int(entry + 8);
        short rating = m_df->read_short(entry + 4);
		Skill s(type, experience, rating);
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
	return Skill(skill_id, 0, 0);
}

short Dwarf::get_rating_by_skill(int skill_id) {
	short retval = 0;
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
	return get_rating_by_skill(l->skill_id);
}

QString Dwarf::read_profession(const uint &addr) {
	if (!m_custom_profession.isEmpty()) {
		return m_custom_profession; 
	}
	
	char buffer[1];
    m_df->read_raw(addr, 1, &buffer[0]);
	return GameDataReader::ptr()->get_profession_name((int)buffer[0]);
}

void Dwarf::read_labors(const uint &addr) {
    // read a big array of labors in one read, then pick and choose
	// the values we care about
	uchar buf[102];
	memset(buf, 0, 102);
    m_df->read_raw(addr, 102, &buf);

	// get the list of identified labors from game_data.ini
	GameDataReader *gdr = GameDataReader::ptr();
	foreach(Labor *l, gdr->get_ordered_labors()) {
		bool enabled = buf[l->labor_id] > 0;
		m_labors[l->labor_id] = enabled;
		m_pending_labors[l->labor_id] = enabled;
	}
	// special cases
	/*
	int num_weapons_offset = gdr->get_int_for_key("military_prefs/0/id", 10);
	m_pending_num_weapons = buf[num_weapons_offset];
	m_num_weapons = m_pending_num_weapons;
	*/
}

bool Dwarf::is_labor_enabled(int labor_id) {
	return m_pending_labors[labor_id];
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
	m_pending_labors[labor_id] = !m_pending_labors[labor_id];
	return true;
}

void Dwarf::set_labor(int labor_id, bool enabled) {
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
		// change values to what's pending
		buf[labor_id] = m_pending_labors.value(labor_id, false) ? 1 : 0;
	}

	m_df->write_raw(addr, 102, &buf);
	if (m_pending_nick_name != m_nick_name)
		m_df->write_string(m_address + mem->dwarf_offset("nick_name"), m_pending_nick_name);
	if (m_pending_custom_profession != m_custom_profession)
		m_df->write_string(m_address + mem->dwarf_offset("custom_profession"), m_pending_custom_profession);
	refresh_data();
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
	d_item->setText(0, nice_name() + "(" + QString::number(labors.size()) + ")");
	d_item->setData(0, Qt::UserRole, id());
	if (m_pending_nick_name != m_nick_name) {
		QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
		i->setText(0, "nickname change to " + m_pending_nick_name);
		i->setIcon(0, QIcon(":img/book_edit.png"));
		i->setData(0, Qt::UserRole, id());
	}
	if (m_pending_custom_profession != m_custom_profession) {
		QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
		QString prof = m_pending_custom_profession;
		if (prof.isEmpty())
			prof = "DEFAULT";
		i->setText(0, "profession change to " + prof);
		i->setIcon(0, QIcon(":img/book_edit.png"));
		i->setData(0, Qt::UserRole, id());
	}
	foreach(int labor_id, labors) {
		Labor *l = GameDataReader::ptr()->get_labor(labor_id);
		if (l->labor_id != labor_id) {
			LOGW << "somehow got a change to an unknown labor with id:" << labor_id;
			continue;
		}
		
		QTreeWidgetItem *i = new QTreeWidgetItem(d_item);
		i->setText(0, l->name);
		if (is_labor_enabled(labor_id)) {
			i->setIcon(0, QIcon(":img/add.png"));
		} else {
			i->setIcon(0, QIcon(":img/delete.png"));
		}
		i->setData(0, Qt::UserRole, id());
	}
	return d_item;
}
