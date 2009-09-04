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

#include <QtGui>
#include <QtDebug>
#include "defines.h"
#include "dfinstance.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "cp437codec.h"
#include "dwarftherapist.h"

DFInstance::DFInstance(QObject* parent)
	:QObject(parent)
	,m_pid(0)
	,m_memory_correction(0)
	,m_stop_scan(false)
	,m_is_ok(true)
	,m_layout(0)
    ,m_attach_count(0)
{
	/*QTimer *df_check_timer = new QTimer(this);
	connect(df_check_timer, SIGNAL(timeout()), SLOT(heartbeat()));
	df_check_timer->start(1000); // every second
        */
}

uint DFInstance::write_string(const uint &addr, const QString &str) {
	int cap = read_int(addr + STRING_CAP_OFFSET);
	int buffer_addr = addr + STRING_BUFFER_OFFSET;
	if( cap >= 16 )
		buffer_addr = read_int(buffer_addr);

	int len = qMin<int>(str.length(), cap);
	write_int(addr + STRING_LENGTH_OFFSET, len);
	return write_raw(buffer_addr, len, str.toAscii().data());
}

QVector<uint> DFInstance::scan_mem_find_all(const QByteArray &needle, const uint &start_address, const uint &end_address) {
	QVector<uint> addresses;
	uint ptr = start_address;
	bool ok;
	do {
		int addr = scan_mem(needle, ptr, end_address, ok);
		if (ok && addr != 0) {
			addresses.push_back(addr);
			ptr = addr + 1;
		} else {
			break;
		}
	} while (ptr < end_address);
	return addresses;
}

uint DFInstance::scan_mem(const QByteArray &needle, const uint &start_address, const uint &end_address, bool &ok) {
	m_stop_scan = false;
	ok = true;
	if (end_address <= start_address) {
        LOGW << "start address must be lower than end_address";
		ok = false;
		return 0;
	}
	//qDebug() << "starting scan from" << hex << start_address << "to" << end_address << "for" << needle;

	int step = 0x1000;
	int bytes_read = 0;
	char buffer[0x1000];
	int ptr = start_address;

	int total_steps = (end_address - start_address) / step;
	if ((end_address - start_address) % step) {
		total_steps++;
	}
	emit scan_total_steps(total_steps);
	emit scan_progress(0);

	QByteArrayMatcher matcher(needle);
	for (int i = 0; i < total_steps; ++i) {
		if (m_stop_scan) {
			break;
		}
		ptr = start_address + (i * step);
		bytes_read = read_raw(ptr, step, (char*)&buffer[0]);
		
		int idx = matcher.indexIn(QByteArray((char*)&buffer[0], bytes_read));
		if (idx != -1) {
			//qDebug() << "FOUND" << needle << "at" << hex << ptr + idx;
			ok = true;
			//delete[] buffer;
			emit scan_progress(total_steps-1); // don't finish it all the way
			return ptr + idx;
		}
		emit scan_progress(i);
		qApp->processEvents();
	}
	//delete[] buffer;
	ok = false;
	return 0;
}

bool DFInstance::looks_like_vector_of_pointers(const uint &addr) {
	int start = read_int(addr + 0x4);
	int end = read_int(addr + 0x8);
	int entries = (end - start) / sizeof(int);

	return start >=0 && 
		   end >=0 && 
		   end >= start && 
		   (end-start) % 4 == 0 &&
		   start % 4 == 0 &&
		   end % 4 == 0 &&
		   entries > 6 &&
		   entries < 12;
	
}

void DFInstance::find_language_tables() {
	GameDataReader *gdr = GameDataReader::ptr();
	int target_total_words = gdr->get_int_for_key("ram_guesser/total_words_per_table", 10);
	QString first_generic_word = gdr->get_string_for_key("ram_guesser/first_generic_word");
	QString first_dwarf_word = gdr->get_string_for_key("ram_guesser/first_dwarf_word");

	emit scan_message(tr("Scanning for vectors with %1 entries").arg(target_total_words));
	foreach(uint vec_addr, this->find_vectors(target_total_words)) {
		//LOGD << "FOUND VECTOR AT" << hex << vec_addr << "WITH" << dec << target_total_words << "ENTRIES!";
		foreach(uint word_ptr, enumerate_vector(vec_addr)) {
			QString first_entry = read_string(word_ptr);
			QString final_addr = QString("0x%1").arg(vec_addr - m_memory_correction, 8, 16, QChar('0'));
			if (first_entry == first_generic_word) {
				LOGI << "GENERIC WORD TABLE FOUND:" << final_addr;
			} else if (first_entry == first_dwarf_word) {
				LOGI << "DWARF WORD TABLE FOUND:" << final_addr;
			}

			break;
		}
	}
	LOGD << "FINISHED SCANNING FOR WORD TABLES";
}


uint DFInstance::find_creature_vector() {
	GameDataReader *gdr = GameDataReader::ptr();
	int low_cutoff = gdr->get_int_for_key("ram_guesser/creature_vector_low_cutoff") + m_memory_correction;
	int high_cutoff = gdr->get_int_for_key("ram_guesser/creature_vector_high_cutoff")+ m_memory_correction;
	int dwarf_nickname_offset = 0x001C; //m_layout->dwarf_offset("nick_name");
	QByteArray custom_nickname("FirstCreature");
	QByteArray custom_profession("FirstProfession");

	QByteArray skillpattern_miner = encode_skillpattern(0, 3340, 4);
	QByteArray skillpattern_metalsmith = encode_skillpattern(29, 0, 2);
	QByteArray skillpattern_swordsman = encode_skillpattern(40, 462, 3);
	QByteArray skillpattern_pump_operator = encode_skillpattern(65, 462, 1);

	int creature_vector_address = -1;
	emit scan_message(tr("Scanning for known nickname"));
	QByteArray needle(custom_nickname);
	int dwarf = 0;
	foreach(int nickname, scan_mem_find_all(needle, 0, 0x0FFFFFFF)) {
		qDebug() << "FOUND NICKNAME" << hex << nickname;
		emit scan_message(tr("Scanning for dwarf objects"));
		int possible_dwarf_address = nickname - dwarf_nickname_offset - 4;
		qDebug() << "DWARF POINTER SHOULD BE AT:" << hex << possible_dwarf_address;
        needle = encode(possible_dwarf_address); // should be the address of this dwarf
		foreach(int dwarf, scan_mem_find_all(needle, 0, 0x0FFFFFFF)) {
			//qDebug() << "FOUND DWARF" << hex << dwarf;
			emit scan_message(tr("Scanning for dwarf vector pointer"));
            needle = encode(dwarf); // since this is the first dwarf, it should also be the dwarf vector
			foreach(int vector_ptr, scan_mem_find_all(needle, low_cutoff, high_cutoff)) {
				creature_vector_address = vector_ptr - 0x4 - m_memory_correction;
				qDebug() << "FOUND CREATURE VECTOR" << hex << creature_vector_address;
			}
		}
	}

	needle = custom_profession;
	foreach(int prof, scan_mem_find_all(needle, dwarf, dwarf + 0x1000)) {
		qDebug() << "Custom Profession Offset" << prof - dwarf - 4;
	}

	return creature_vector_address;

	QVector<QByteArray> patterns;
	patterns.append(skillpattern_miner);
	patterns.append(skillpattern_metalsmith);
	patterns.append(skillpattern_swordsman);
	patterns.append(skillpattern_pump_operator);

	QVector< QVector<int> > addresses;
	addresses.reserve(patterns.size());
	for (int i = 0; i < 4; ++i) {
		QVector<int> lst;
		emit scan_message("scanning for skill pattern" + QString::number(i));
		foreach(int skill, scan_mem_find_all(patterns[i], 0, m_memory_size + m_base_addr - m_memory_correction)) {
			qDebug() << "FOUND SKILL PATTERN" << i << by_char(patterns[i]) << "AT" << hex << skill;
			lst.append(skill);
		}
		addresses.append(lst);
		break;
	}
	/*
				List<int>[] addresses = new List<int>[config.SkillPatterns.Length];
				for( int i = 0; i < config.SkillPatterns.Length; i++ ) {
					List<int> list = new List<int>();
					foreach( int skill in FindPattern( config.SkillPatterns[i] ) )
						list.Add( skill );
					addresses[i] = list;
				}
				foreach( int addr in addresses[0] ) {
					foreach( int start in Find( addr ) ) {
						byte[] buffer = memoryAccess.ReadMemory( start, config.SkillPatterns.Length*4 );
						foreach( int[] p in CrossProduct( addresses, 0 ) ) {
							Matcher matcher = new Matcher( BytesHelper.ToBytes( p ) );
							if( matcher.Matches( ref buffer, 0, buffer.Length ) ) {
								foreach( int listPointers in Find( start, dwarf, dwarf + 0x2000 ) ) {
									ReportAddress( "Offset", "Creature.SkillVector", listPointers - dwarf - 4 );
								}
							}
						}
					}
				}
				foreach( int labors in FindPattern( config.LaborsPattern, dwarf, dwarf + 0x2000 ) ) {
					ReportAddress( "Offset", "Creature.Labors", labors - dwarf );
				}
	*/
	qDebug("all done with creature scan");
	return creature_vector_address;
}

uint DFInstance::find_dwarf_race_index() {
	int dwarf_race_index = -1;
	emit scan_message(tr("Scanning for dwarf race index"));
	QByteArray needle("FirstCreature");
	int dwarf = 0;
	foreach(int nickname, scan_mem_find_all(needle, 0, 0x0FFFFFFF)) {
		dwarf = nickname - 0x1C - 4;
		int race = read_int(dwarf + 0x8C);
		emit scan_message(tr("Scanning for 'A group of '"));
		needle = "A group of";
		foreach(int group_of, scan_mem_find_all(needle, 0, 0x0FFFFFFF)) {
			QByteArray tmp(4, NULL);
			tmp[3] = 0x68;
            tmp.append(encode(group_of));
			emit scan_message(tr("Scanning for refs to 'A group of '"));
			foreach(int ref_to_group_of, scan_mem_find_all(tmp, 0, 0x7FFFFFFF)) {
				QByteArray tmp2(4, 0);
				tmp[1] = 0x66;
				tmp[2] = 0x39;
				emit scan_message(tr("Scanning for refs to race index"));
				foreach(int ref_to_race, scan_mem_find_all(tmp, ref_to_group_of, ref_to_group_of + 60)) {
					int race_ptr = ref_to_race + 4;
					int race_ptr_addr = read_int(race_ptr);
					LOGD << "POSSIBLE RACE INDEX:" << hex << race_ptr_addr;
					dwarf_race_index = race_ptr_addr;
				}
			}
		}
	}
	return dwarf_race_index;


	/*
	int currentRaceIndex = 0;
	foreach( int nickname in Find( config.CreatureNick ) ) {
		int dwarf = nickname - 0x001C - 4;
		currentRaceIndex = memoryAccess.ReadInt32( dwarf + 0x8C );

		foreach( int aGroupOf in Find( "A group of " ) ) {
			foreach( int refToAGroupOf in Find( BytesHelper.Concat( new byte?[] { null, null, null, 0x68 }, BytesHelper.ToBytes(aGroupOf) ) ) ) {
				foreach( int refToRaceIndex in Find( new byte?[] { null, 0x66, 0x39, null }, refToAGroupOf, refToAGroupOf + 60 ) ) {
					int pointAddress = refToRaceIndex + 4;
					int raceIndexPointer = memoryAccess.ReadInt32( pointAddress );
					if( raceIndexPointer > 0x1600000 )
						continue;

					try {
						int intRaceIndex = memoryAccess.ReadInt32( raceIndexPointer );
						if( intRaceIndex == currentRaceIndex )
							ReportAddress( "Address", "DwarvenRaceIndex", raceIndexPointer );
					} catch( Exception ) { }
				}
			}
		}
	}
	*/
}

QVector<Dwarf*> DFInstance::load_dwarves() {
	int creature_vector = m_layout->address("creature_vector");
    TRACE << "starting with creature vector" << creature_vector;
	QVector<Dwarf*> dwarves;
	TRACE << "adjusted creature vector" << creature_vector + m_memory_correction;
    QVector<uint> creatures = enumerate_vector(creature_vector + m_memory_correction);
	TRACE << "FOUND" << creatures.size() << "creatures";
	if (creatures.size() > 0) {
		for (int offset=0; offset < creatures.size(); ++offset) {
			Dwarf *d = Dwarf::get_dwarf(this, creatures[offset]);
			if (d) {
				dwarves.append(d);
				TRACE << "FOUND DWARF" << offset << d->nice_name();
			} else {
				TRACE << "FOUND OTHER CREATURE" << offset;
			}
		}
	}
	LOGI << "found" << dwarves.size() << "dwarves out of" << creatures.size() << "creatures";
	return dwarves;
}

void DFInstance::heartbeat() {
	// simple read attempt that will fail if the DF game isn't running a fort, or isn't running at all
    QVector<uint> creatures = enumerate_vector(m_layout->address("creature_vector") + m_memory_correction);
	if (creatures.size() < 1) {
		// no game loaded, or process is gone
		emit connection_interrupted();
	}
}

bool DFInstance::is_valid_address(const uint &addr) {
	bool valid = false;
	QPair<uint, uint> region;
	foreach(region, m_regions) {
		if (addr >= region.first && addr <= region.second) {
			valid = true;
			break;
		}
	}
	return valid;
}

QByteArray DFInstance::get_data(const uint &addr, const uint &size) {
	char *buffer = new char[size];
	memset(buffer, 0, size);
	read_raw(addr, size, buffer);
	QByteArray data(buffer, size);
	delete[] buffer;
	return data;
}

//! ahhh convenience
QString DFInstance::pprint(const uint &addr, const uint &size) {
	return pprint(get_data(addr, size), addr);
}

QString DFInstance::pprint(const QByteArray &ba, const uint &start_addr) {
	QString out = "   ADDR  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | TEXT\n";
	out.append("------------------------------------------------------------------------\n");
	int lines = ba.size() / 16;
	if (ba.size() % 16)
		lines++;

	for(int i = 0; i < lines; ++i) {
		uint offset = start_addr + i * 16;
		out.append(QString("0x%1").arg(offset, 8, 16, QChar('0')));
		out.append(" | ");
		for (int c = 0; c < 16; ++c) {
			out.append(ba.mid(i*16 + c, 1).toHex());
			out.append(" ");
		}
		out.append("| ");
		for (int c = 0; c < 16; ++c) {
			QByteArray tmp = ba.mid(i*16 + c, 1);
			if (tmp.at(0) == 0)
				out.append(".");
			else if (tmp.at(0) <= 126 && tmp.at(0) >= 32)
				out.append(tmp);
			else
				out.append(tmp.toHex());
		}
		//out.append(ba.mid(i*16, 16).toPercentEncoding());
		out.append("\n");
	}
	return out;
}

QVector<uint> DFInstance::find_vectors(const uint &num_entries, const uint &fuzz/* =0 */, const uint &entry_size/* =4 */) {
	QVector<uint> vectors;

	emit scan_total_steps(m_regions.size());
	emit scan_progress(0);

	/* 
	glibc++ does vectors like so...
	|4bytes      | 4bytes    | 4bytes
	START_ADDRESS|END_ADDRESS|END_ALLOCATOR

	MSVC++ does vectors like so...
	| 4bytes     | 4bytes      | 4 bytes   | 4bytes
	ALLOCATOR    |START_ADDRESS|END_ADDRESS|END_ALLOCATOR
	*/
	
	uint int1 = 0; // holds the start val
	uint int2 = 0; // holds the end val 

	// iterate over all known memory segments
	QPair<uint, uint> addr_pair;
	int count = 0;
	foreach(addr_pair, m_regions) {
		// size in bytes of this segment
		uint size = addr_pair.second - addr_pair.first;
		//qDebug() << "SCANNING REGION" << hex << addr_pair.first << "-" << addr_pair.second << "BYTES:" << dec << size;

		// this buffer will hold the entire memory segment (may need to toned down a bit)
		char *buffer = new (std::nothrow) char[size];
		if (buffer == 0) {
			LOGC << "unable to allocate char buffer of" << size << "bytes!";
			continue;
		}
		memset(buffer, 0, size); // 0 out the buffer

		// this may read multiple times to populate the entire region in our buffer
		uint bytes_read = read_raw(addr_pair.first, size, buffer);
		if (bytes_read < size) {
			LOGW << "tried to read" << size << "bytes starting at" << hex << addr_pair.first << "but only got" << dec << bytes_read;
			continue;
		}

		// we now have this entire memory segment inside buffer. So lets step through it looking for things that look like vectors of pointers.
		// we read a uint into int1 and 4 bytes later we read another uint into int2. If int1 is a vector head, then int2 will be larger than int2, 
		// evenly divisible by 4, and the difference between the two (divided by four) will tell us how many pointers are in this array. We can also
		// check to make sure int1 and int2 reside in valid memory regions. If all of this adds up, then odds are pretty good we've found a vector 
		for (uint i = 0; i < size; i += 4) {
			memcpy(&int1, buffer + i, 4);
			memcpy(&int2, buffer + i + 4, 4);
			if (int2 >= int1 && is_valid_address(int1) && is_valid_address(int2)) {
				uint bytes = int2 - int1;
				uint entries = bytes / entry_size;
				int diff = entries - num_entries;
				if (qAbs(diff) <= fuzz) {
#ifdef Q_WS_WIN
					// on windows the pointer is 4 bytes in front of the start address
					uint vector_address = addr_pair.first + i - 4;
#endif
#ifdef Q_WS_X11
					// on linux the pointer is the start address
					uint vector_address = addr_pair.first + i;
#endif
					QVector<uint> addrs = enumerate_vector(vector_address);
					diff = addrs.size() - num_entries;
					if (qAbs(diff) <= fuzz) {
						vectors << vector_address;
					}
				}
				if (m_stop_scan)
					break;
			}
		}
		delete[] buffer;

		emit scan_progress(count++);
		DT->processEvents();
		if (m_stop_scan)
			break;
	}
	return vectors;
}
