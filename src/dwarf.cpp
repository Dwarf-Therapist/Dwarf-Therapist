#include "dwarf.h"

Dwarf::Dwarf(DFInstance *df, int address, QObject *parent)
	: QObject(parent)
	, m_df(df)
	, m_address(address)
{
	uint bytes_read = 0;
	m_first_name = df->read_string(address);
	m_first_name[0] = m_first_name[0].toUpper();
	m_nick_name = df->read_string(address + 0x001C);
	m_race_id = df->read_int32(address + 0x008C, bytes_read);
	//QString last_name = df->read_string(dwarf + 0x0038);
	m_last_name = "foo";
	m_custom_profession = df->read_string(address + 0x006C);
}

Dwarf::~Dwarf()
{

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

	return new Dwarf(df, address);
}

QString Dwarf::nice_name() {
	if (m_nick_name.isEmpty()) {
		return QString("%1 %2").arg(m_first_name, m_last_name);
	} else {
		return QString("%1 \"%2\" %3").arg(m_first_name, m_nick_name, m_last_name);
	}
}

QString Dwarf::to_string() {
	return QString("<%1, %4>").arg(nice_name(), m_custom_profession);
}
