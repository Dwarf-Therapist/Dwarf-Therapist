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
#include "memorysegment.h"

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

//! Convenience method for finding a pointer to an address
QVector<uint> DFInstance::scan_for_pointer(const uint &addr) {
#ifdef Q_WS_WIN
	QByteArray needle = encode(addr - 0x4); // windows needs to backup 4 bytes
#else
	QByteArray needle = encode(addr); // windows needs to backup 4 bytes
#endif
	return scan_mem(needle);
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

QVector<uint> DFInstance::scan_mem(const QByteArray &needle) {
	m_stop_scan = false;
	QVector<uint> addresses;
	QByteArrayMatcher matcher(needle);

	int step = 0x1000;
	char *buffer = new char[step];
	if (!buffer) {
		qCritical() << "unable to allocate char buffer of" << step << "bytes!";
		return addresses;
	}
	int count = 0;
	emit scan_total_steps(m_regions.size());
	emit scan_progress(0);

	uint bytes_scanned = 0;
	QTime timer;
	timer.start();
	foreach(MemorySegment *seg, m_regions) {	
		int steps = seg->size / step;
		if (seg->size % step)
			steps++;
		
		for(uint ptr = seg->start_addr; ptr < seg->end_addr; ptr += step) {
			if (ptr + step > seg->end_addr)
				step = seg->end_addr - (ptr + step);

			memset(buffer, 0, step);
			int bytes_read = read_raw(ptr, step, buffer);
			if (bytes_read < step && !seg->is_guarded) {
				LOGW << "tried to read" << step << "bytes starting at" << hex << ptr << "but only got" << dec << bytes_read;
				continue;
			}

			int idx = matcher.indexIn(QByteArray(buffer, bytes_read));
			if (idx != -1) {
				addresses << (uint)(ptr + idx);
			}
			if (m_stop_scan)
				break;
			
		}
		bytes_scanned += seg->size;
		emit scan_progress(count++);
		DT->processEvents();
		if (m_stop_scan)
			break;
	}
	LOGD << QString("Scanned %L1KB in %L2ms").arg(bytes_scanned / 1024 * 1024).arg(timer.elapsed());
	delete[] buffer;
	return addresses;
}

bool DFInstance::looks_like_vector_of_pointers(const uint &addr) {
	int start = read_int(addr + 0x4);
	int end = read_int(addr + 0x8);
	int entries = (end - start) / sizeof(int);
	LOGD << "LOOKS LIKE VECTOR? unverified entries:" << entries;

	return start >=0 && 
		   end >=0 && 
		   end >= start && 
		   (end-start) % 4 == 0 &&
		   start % 4 == 0 &&
		   end % 4 == 0 &&
		   entries < 10000;
	
}

void DFInstance::find_language_tables() {
	GameDataReader *gdr = GameDataReader::ptr();
	int target_total_words = gdr->get_int_for_key("ram_guesser/total_words_per_table", 10);
	int target_total_langs = gdr->get_int_for_key("ram_guesser/total_languages", 10);
	QString first_generic_word = gdr->get_string_for_key("ram_guesser/first_generic_word");
	QString first_dwarf_word = gdr->get_string_for_key("ram_guesser/first_dwarf_word");

	// First find all vectors that have the correct number of entries to be lang tables
	uint dwarf_lang_table = 0;
	QVector<uint> translations_vectors;
	/*
	emit scan_message(tr("Scanning for vectors with %1 entries").arg(target_total_words));
	foreach(uint addr, find_vectors(target_total_words)) {
		foreach(uint vec_addr, enumerate_vector(addr)) {
			QString first_entry = read_string(vec_addr);
			if (first_entry == first_generic_word) {
				qDebug() << "FOUND LANGUAGE TABLE" << hex << addr;
			} else if (first_entry == first_dwarf_word) {
				qDebug() << "FOUND DWARF TABLE" << hex << addr;
				dwarf_lang_table = addr;
			} else {
				break;
			}
			break;
		}
	}
	*/
	dwarf_lang_table = 0x90478b4;
	uint dwarf_translation = 0;
	uint word_table_offset = 0;
	if (dwarf_lang_table) {
		for(uint i = dwarf_lang_table - 0x100; i < dwarf_lang_table; i+=4) {
			if (read_string(i) == "DWARF") {
				dwarf_translation = i - VECTOR_POINTER_OFFSET;
				word_table_offset = dwarf_lang_table - dwarf_translation;
				LOGD << "WORD TABLE OFFSET" << hex << word_table_offset;
				//LOGD << "FOUND 'DWARF'" << hex << dwarf_lang_table - i << "bytes in front of dwarf word table!";

				//now find a pointer to this guy...
				LOGD << "DWARF TRANS OBJECT" << hex << dwarf_translation;
				foreach (uint trans_ptr, scan_mem(encode(dwarf_translation + 4))) {
					foreach (uint trans_vec_ptr, scan_mem(encode(trans_ptr))) {
						translations_vectors << trans_vec_ptr - VECTOR_POINTER_OFFSET;
					}
				}
				break;
			}
		}
	}

	
	//translations_vectors << dwarf_translation << dwarf_translation - 4;
	
	
	// A translation entry is basically the race name (i.e. "DWARF") followed by a vector of words shorty afterwards.
	// So we find langs like so...
	// 1) look for a null terminated string "DWARF\0"
	// 2) find a pointer to that buffer (this is the std::string object)
	// 3) find a pointer to the std::string (the translation object)
	// 4) find a pointer to the translation object (an entry in the translations vector)
	/*
	emit scan_message(tr("Scanning for DWARF buffer"));
	
	QByteArray needle("DWARF");
	foreach(uint str_buf, scan_mem(needle)) { // std::string internal buffer
		str_buf -= STRING_BUFFER_OFFSET;
		if (read_string(str_buf) != "DWARF") // can match "DWARF_LIASON" and the like so ignore those...
			continue;
		emit scan_message(tr("Investigating 0x%1").arg(str_buf, 8, 16, QChar('0')));
		foreach(uint str, scan_mem(encode(str_buf))) { // std::string
			str -= STRING_BUFFER_OFFSET;
			emit scan_message(tr("Scanning for DWARF translation table"));
			foreach(uint str_ptr, scan_mem(encode(str))) { // translation table name attribute?
				emit scan_message(tr("Scanning for DWARF string object pointer"));
				foreach(uint str_ptr_ptr, scan_mem(encode(str_ptr - STRING_BUFFER_OFFSET))) { // entry in translations vector
					uint trans_vec = str_ptr_ptr - STRING_BUFFER_OFFSET;
					translations_vectors << trans_vec;
					qDebug() << "Found possible translations vector" << hex << trans_vec;
				}
			}
		}
	}
	*/
	qDebug() << "Verifying possible vectors";
	foreach(uint vec, translations_vectors) {
		LOGD << "Verifying" << hex << vec;

		QVector<uint> langs = enumerate_vector(vec);
		LOGD << "ENTRIES" << langs.size();
		if (langs.size() == target_total_langs) {
			if (read_string(langs.at(0)) == "DWARF") {
				LOGD << "+++ VERIFIED +++";
				LOGD << "\tTRANSLATIONS VECTOR AT" << hex << vec;
				LOGD << "\tDWARF TRANS OBJECT" << hex << langs.at(0) << "WORD TABLE" << dwarf_lang_table;
				LOGD << "\tOFFSET FROM WORD TABLE" << hex << (int)(dwarf_lang_table - langs.at(0));
				LOGD << "\tFINAL ADDRESS" << QString("0x%1").arg(vec - m_memory_correction, 8, 16, QChar('0'));
				break;
			}
		}
	}
	LOGD << "FINISHED SCANNING FOR WORD TABLES";
}


uint DFInstance::find_creature_vector() {
	return 0;
}

uint DFInstance::find_dwarf_race_index() {
	return 0;
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
	foreach(MemorySegment *seg, m_regions) {
		if (seg->contains(addr)) {
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
	m_stop_scan = false;
	QVector<uint> vectors;

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

	// progress reporting
	uint total_bytes = 0;
	uint bytes_scanned = 0;
	foreach(MemorySegment *seg, m_regions) {
		total_bytes += seg->size;
	}
	uint report_every_n_bytes = total_bytes / 100;
	emit scan_total_steps(100);
	emit scan_progress(0);
	// iterate over all known memory segments
	foreach(MemorySegment *seg, m_regions) {
		//LOGD << "SCANNING REGION" << hex << seg->start_addr << "-" << seg->end_addr << "BYTES:" << dec << seg->size;

		// this buffer will hold the entire memory segment (may need to toned down a bit)
		char *buffer = new (std::nothrow) char[seg->size];
		if (buffer == 0) {
			LOGC << "unable to allocate char buffer of" << seg->size << "bytes!";
			continue;
		}
		memset(buffer, 0, seg->size); // 0 out the buffer

		// this may read multiple times to populate the entire region in our buffer
		uint bytes_read = read_raw(seg->start_addr, seg->size, buffer);
		if (bytes_read < seg->size) {
			LOGW << "tried to read" << seg->size << "bytes starting at" << hex 
				 << seg->start_addr << "but only got" << dec << bytes_read;
			continue;
		}

		// we now have this entire memory segment inside buffer. So lets step through it looking for things that look like vectors of pointers.
		// we read a uint into int1 and 4 bytes later we read another uint into int2. If int1 is a vector head, then int2 will be larger than int2, 
		// evenly divisible by 4, and the difference between the two (divided by four) will tell us how many pointers are in this array. We can also
		// check to make sure int1 and int2 reside in valid memory regions. If all of this adds up, then odds are pretty good we've found a vector 
		for (uint i = 0; i < seg->size; i += 4) {
			memcpy(&int1, buffer + i, 4);
			memcpy(&int2, buffer + i + 4, 4);
			if (int2 >= int1 && is_valid_address(int1) && is_valid_address(int2)) {
				uint bytes = int2 - int1;
				uint entries = bytes / entry_size;
				int diff = entries - num_entries;
				if (qAbs(diff) <= fuzz) {
					uint vector_address = seg->start_addr + i - VECTOR_POINTER_OFFSET;
					QVector<uint> addrs = enumerate_vector(vector_address);
					diff = addrs.size() - num_entries;
					if (qAbs(diff) <= fuzz) {
						vectors << vector_address;
					}
				}
				if (m_stop_scan)
					break;
			}
			bytes_scanned += 4;
			if (i % 400 == 0)
				emit scan_progress(bytes_scanned / report_every_n_bytes);
		}
		delete[] buffer;
		DT->processEvents();
		if (m_stop_scan)
			break;
	}
	emit scan_progress(100);
	return vectors;
}
