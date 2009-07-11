#include "dwarf.h"
#include "dfinstance.h"
#include "skill.h"
#include <QVector>

Dwarf::Dwarf(DFInstance *df, int address, QObject *parent)
	: QObject(parent)
	, m_df(df)
	, m_address(address)
    //, m_skills(QVector<Skill>)
{
	/*
	<Offset Name="Creature.FirstName" Value="0x0000" /> 
	<Offset Name="Creature.NickName" Value="0x001C" /> 
	<Offset Name="Creature.LastName" Value="0x0038" /> 
	<Offset Name="Creature.CustomProfession" Value="0x006c" /> 
	<Offset Name="Creature.Profession" Value="0x0088" /> 
	<Offset Name="Creature.Race" Value="0x008C" /> 
	<Offset Name="Creature.Flags1" Value="0x00FC" /> 
	<Offset Name="Creature.Flags2" Value="0x0100" /> 
	<Offset Name="Creature.ID" Value="0x010C" /> 
	<Offset Name="Creature.Strength" Value="0x04f0" /> 
	<Offset Name="Creature.Agility" Value="0x04f4" /> 
	<Offset Name="Creature.Toughness" Value="0x04f8" /> 
	<Offset Name="Creature.SkillVector" Value="0x0504" /> 
	<Offset Name="Creature.Labors" Value="0x0544" /> 
	*/
	uint bytes_read = 0;

	m_first_name = df->read_string(address);
	if (m_first_name.size() > 1)
		m_first_name[0] = m_first_name[0].toUpper();
	
	m_nick_name = df->read_string(address + 0x001C);
    m_last_name = read_last_name(address + 0x0038);
	m_custom_profession = df->read_string(address + 0x006C);
	m_race_id = df->read_int32(address + 0x008C, bytes_read);
    m_skills = read_skills(address + 0x0504);
}

Dwarf::~Dwarf() {
}

Dwarf *Dwarf::get_dwarf(DFInstance *df, int address) {
	uint bytes_read = 0;
	if ((df->read_int32(address + 0x00FC, bytes_read) & 0x08C0) > 0) {
		return 0;
	}
	
	//if( ( memoryAccess.ReadInt32( address + memoryLayout["Creature.Flags1"] ) & memoryLayout["Creature.Flags1.Invalidate"] ) > 0)
    //	return false;

	if ((df->read_int32(address + 0x0100, bytes_read) & 0x0080) > 0) {
		return 0;
	}
	
	//if( ( memoryAccess.ReadInt32( address + memoryLayout["Creature.Flags2"] ) & memoryLayout["Creature.Flags2.Invalidate"] ) > 0 )
	//	return false;

	if ((df->read_int32(address + 0x008C, bytes_read)) != 166) {
		return 0;
	}
	//if( memoryAccess.ReadInt32( address + memoryLayout["Creature.Race"] ) != actualDwarfRaceId )
	//	return false;

	return new Dwarf(df, address, df);
}

QString Dwarf::to_string() {
    return QString("%1, %4").arg(nice_name(), m_custom_profession);
}

QString Dwarf::nice_name() {
	if (m_nick_name.isEmpty()) {
		return QString("%1 %2").arg(m_first_name, m_last_name);
	} else {
		return QString("%1 \"%2\" %3").arg(m_first_name, m_nick_name, m_last_name);
	}
}

QString Dwarf::read_last_name(int address) {
    // TODO: move to config
    int word_table = 0x0058;
    uint bytes_read = 0;
    int actual_lang_table = m_df->read_int32(0x013f15c8 + 4, bytes_read);
    int translations_ptr = m_df->read_int32(0x013f15f8 + 4, bytes_read);
    int translation_ptr = m_df->read_int32(translations_ptr, bytes_read);
    int actual_dwarf_translation_table = m_df->read_int32(translation_ptr + word_table, bytes_read);

    QString out;

	for (int i = 0; i < 7; i++) {
        int word = m_df->read_int32(address + i * 4, bytes_read);
        if(bytes_read == 0 || word == -1)
			break;
        //Q_ASSERT(word < 10000);
        int addr = m_df->read_int32(actual_dwarf_translation_table + word * 4, bytes_read);
        out += m_df->read_string(addr);
	}
    if (out.size() > 1) {
        out[0] = out[0].toUpper();
    }
    return out;
}


QVector<Skill> Dwarf::read_skills(int address) {
    QVector<Skill> skills(0);
    uint bytes_read = 0;
	foreach(int addr, m_df->enumerate_vector(address)) {
        short type = m_df->read_short(addr, bytes_read);
        short experience = m_df->read_short(addr + 2, bytes_read);
        short rating = m_df->read_short(addr + 4, bytes_read);
		Skill s(type, rating, experience);
        skills.append(s);
    }
	return skills;
}

/*protected Labors ReadLabors( int address )
{
    byte[] labors = memoryAccess.ReadMemory( address, 102 );
    return new Labors( labors );
}*/
