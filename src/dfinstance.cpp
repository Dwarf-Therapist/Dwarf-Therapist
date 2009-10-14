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
	,m_heartbeat_timer(new QTimer(this))
{
	connect(m_heartbeat_timer, SIGNAL(timeout()), SLOT(heartbeat()));
	// let subclasses start the timer, since we don't want to be checking before we're connected
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

QVector<Dwarf*> DFInstance::load_dwarves() {
	QVector<Dwarf*> dwarves;
	if (!m_is_ok) {
		LOGW << "not connected";
		return dwarves;
	}
    attach();
	int creature_vector = m_layout->address("creature_vector");
    TRACE << "starting with creature vector" << creature_vector;
	
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
    detach();

    /*TEST RELATIONSHIPS
	int relations_found = 0;
    foreach(Dwarf *d, dwarves) {
        // get all bytes for this dwarf...
        qDebug() << "Scanning for ID references in" << d->nice_name() << "(" << hex << d->id() << ")";
        QByteArray data = get_data(d->address(), 0x7e8);
        foreach(Dwarf *other_d, dwarves) {
            if (other_d == d)
                continue; // let's hope there are no pointers to ourselves...
            int offset = 0;
            while (offset < data.size()) {
                int idx = data.indexOf(encode(other_d->id()), offset);
                if (idx != -1) {
                    qDebug() << "\t\tOFFSET:" << hex << idx << "Found ID of" << other_d->nice_name() << "(" << other_d->id() << ")";
					relations_found++;
                    offset = idx + 1;
                } else {
                    offset = data.size();
                }
            }
        }
    }
	LOGD << "relations found" << relations_found;
	*/

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
	QString out = "    ADDR   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | TEXT\n";
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
				if ((uint)qAbs(diff) <= fuzz) {
					uint vector_address = seg->start_addr + i - VECTOR_POINTER_OFFSET;
					QVector<uint> addrs = enumerate_vector(vector_address);
					diff = addrs.size() - num_entries;
					if ((uint)qAbs(diff) <= fuzz) {
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
