#include <QtDebug>
#include <QApplication>
#include <QProgressDialog>
#include <QByteArrayMatcher>
#include <windows.h>
#include <psapi.h>

#include "win_structs.h"
#include "dfinstance.h"
#include "dwarf.h"


DFInstance::DFInstance(DWORD pid, HWND hwnd, QObject* parent)
    :QObject(parent)
    ,m_pid(pid)
    ,m_hwnd(hwnd)
    ,m_proc(0)
    ,m_memory_correction(0)
    ,m_stop_scan(false)
{
    m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
                         | PROCESS_VM_READ
                         | PROCESS_VM_OPERATION
                         | PROCESS_VM_WRITE, false, m_pid);

    PROCESS_MEMORY_COUNTERS pmc;
    GetProcessMemoryInfo(m_proc, &pmc, sizeof(pmc));
    m_memory_size = pmc.WorkingSetSize;
    qDebug() << "process working set size: " << m_memory_size;

    PVOID peb_addr = GetPebAddress(m_proc);
    qDebug() << "PEB is at: " << hex << peb_addr;

	PEB peb;
	DWORD bytes = 0;
	if (ReadProcessMemory(m_proc, (PCHAR)peb_addr, &peb, sizeof(PEB), &bytes)) {
		qDebug() << "read " << bytes << "bytes BASE ADDR is at: " << hex << peb.ImageBaseAddress;
		m_base_addr = (int)peb.ImageBaseAddress;
	} else {
		qCritical() << "unable to read remote PEB!" << GetLastError();
	}

	calculate_checksum();

    m_memory_correction = (int)m_base_addr - 0x0400000;
    qDebug() << "memory correction " << m_memory_correction;

	
	
	QVector<int> creatures = enumerate_vector(0x013ab3cc);
	if (creatures.size() > 0) {
		for (int offset=0; offset < creatures.size(); ++offset) {
			Dwarf *d = Dwarf::get_dwarf(this, creatures[offset]);
			if (d) {
				qDebug() << "CREATURE" << offset << d->to_string();
			} else {
				qWarning() << "BOGUS CREATURE" << offset;
			}
		}
	}
	qDebug() << "finished reading creature vector";
}

DFInstance::~DFInstance() {
    if (m_proc) {
        CloseHandle(m_proc);
    }
}

int DFInstance::calculate_checksum() {
	uint bytes_read = 0;
	char expect_M = read_char(m_base_addr, bytes_read);
	char expect_Z = read_char(m_base_addr + 0x1, bytes_read);

	if (expect_M != 'M' || expect_Z != 'Z') {
		qWarning() << "invalid executable";
	}
	uint pe_header = m_base_addr + read_int32(m_base_addr + 30 * 2, bytes_read);
	char expect_P = read_char(pe_header, bytes_read);
	char expect_E = read_char(pe_header + 0x1, bytes_read);
	if (expect_P != 'P' || expect_E != 'E') {
		qWarning() << "PE header invalid";
	}

	int timestamp = read_int32(pe_header + 4 + 2 * 2, bytes_read);
	return timestamp;
}

QByteArray DFInstance::encode_int(uint addr) {
    char bytes[4];
    bytes[0] = (char)addr;
    bytes[1] = (char)(addr >> 8);
    bytes[2] = (char)(addr >> 16);
    bytes[3] = (char)(addr >> 24);

    QByteArray arr(bytes, 4);
    return arr;
}

QString DFInstance::get_base_address() {
    return QString("%1").arg((int)m_base_addr);
}

QString DFInstance::read_string(int address) {
    uint bytes_read = 0;
    int len = read_int32(address + STRING_LENGTH_OFFSET, bytes_read);
    int cap = read_int32(address + STRING_CAP_OFFSET, bytes_read);
    int buffer_addr = address + STRING_BUFFER_OFFSET;
    if (cap >= 16)
        buffer_addr = read_int32(buffer_addr, bytes_read);

	Q_ASSERT(len <= cap);
	Q_ASSERT(len >= 0);
	Q_ASSERT(len < (1 << 16));
    
    char *buffer = new char[len];
    bytes_read = read_raw(buffer_addr, len, buffer);
	QString ret_val = QString::fromAscii(buffer, bytes_read);
    delete[] buffer;
    return ret_val;
}

int DFInstance::read_int32(int start_address, uint &bytes_read) {
    int val = 0;
    bytes_read = 0;
    ReadProcessMemory(m_proc, (LPCVOID)start_address, &val, sizeof(int), (DWORD*)&bytes_read);
    //qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << hex << val;
    return val;
}

//returns # of bytes read
char DFInstance::read_char(int start_address, uint &bytes_read) {
    char val = 0;
	bytes_read = 0;
    ReadProcessMemory(m_proc, (LPCVOID)start_address, &val, sizeof(char), (DWORD*)&bytes_read);
    //qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << val;
    return val;
}

int DFInstance::read_raw(int start_address, int bytes, char *buffer) {
	memset(buffer, 0, bytes);
	DWORD bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)start_address, (void*)buffer, sizeof(char) * bytes, &bytes_read);
	return bytes_read;
}

QVector<int> DFInstance::scan_mem_find_all(QByteArray &needle, int start_address, int end_address) {
	QVector<int> addresses;
	bool ok;
	do {
		int addr = scan_mem(needle, start_address, end_address, ok);
		if (ok && addr != 0) {
			addresses.push_back(addr);
			start_address = addr+1;
		} else {
			break;
		}
	} while (start_address < end_address);
	return addresses;
}

int DFInstance::scan_mem(QByteArray &needle, int start_address, int end_address, bool &ok) {
    m_stop_scan = false;
    ok = true;
    if (end_address <= start_address) {
        qWarning() << "start address must be lower than end_address";
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
            emit scan_progress(total_steps);
            return ptr + idx;
        }
        emit scan_progress(i);
    }
    //delete[] buffer;
    ok = false;
    return 0;
}

QVector<int> DFInstance::enumerate_vector(int address) {
	QVector<int> addresses;
	uint bytes_read = 0;
    int start = read_int32(address + 4, bytes_read);
    int end = read_int32(address + 8, bytes_read);

	Q_ASSERT(end >= start);
	Q_ASSERT((end - start) % 4 == 0);
	Q_ASSERT(start % 4 == 0);
	Q_ASSERT(end % 4 == 0);

	for( int ptr = start; ptr < end; ptr += 4 ) {
		int addr = read_int32(ptr, bytes_read);
		if (bytes_read == sizeof(int)) {
			addresses.append(addr);
		}
	}
	return addresses;
}

uint DFInstance::find_creature_vector() {
	QByteArray needle("FirstCreature");
    QVector<int> v_names = scan_mem_find_all(needle, 0, m_memory_size + m_base_addr);
    if (v_names.size() > 0) {
		for (int i=0; i < v_names.size(); ++i) {
			int dwarf_nickname = v_names[i];
			QString nick = read_string(dwarf_nickname);
			//qDebug() << "first creature found at" << dwarf_nickname;

			int dwarf = dwarf_nickname - 0x001C - 4; // get the pointer to this dwarf object
			//qDebug() << "dwarf pointer 0: " << hex << dwarf;
			Dwarf *d = Dwarf::get_dwarf(this, dwarf);
			if (d) {
				qDebug() << "FOUND A DWARF!" << d->to_string();
			}

			QByteArray needle = encode_int(dwarf);
			QVector<int> addresses = scan_mem_find_all(needle, 0, m_memory_size);
			if (addresses.size() > 0) {
				for (int j = 0; j < addresses.size(); ++j) {
					uint addr = addresses[j];
					//qDebug() << "possible dwarf pointer match at" << hex << addr;
				
					uint dwarf_list = addr;
					needle = encode_int(dwarf_list);

					QVector<int> list_ptr_matches = scan_mem_find_all(needle, 0x01300000 + m_memory_correction, 0x01700000 + m_memory_correction);
					if (list_ptr_matches.size() > 0) {
						for (int k = 0; k < list_ptr_matches.size(); ++k) {
							qDebug() << "+++++ Possible CreatureVector +++++" << hex << list_ptr_matches[k] - 0x4;
							QVector<int> creatures = enumerate_vector(list_ptr_matches[k] - 0x4);
							if (creatures.size() > 0) {
								for (int offset=0; offset < creatures.size(); ++offset) {
									Dwarf *d = Dwarf::get_dwarf(this, creatures[offset]);
									if (d) {
										qDebug() << "CREATURE" << offset << d->to_string();
									} else {
										qWarning() << "BOGUS CREATURE" << offset;
									}
								}
							}
						}
					}
				}
			}
		}
    } else {
        qWarning() << "unable to find first creature";
    }
	qDebug() << "all done with creature scan";
    return 9;
}


DFInstance* DFInstance::find_running_copy(QObject *parent) {
    HWND hwnd = FindWindow(NULL, L"Dwarf Fortress");
    if (!hwnd) {
        qWarning() << "can't find running copy";
        return 0;
    }
    qDebug() << "found copy with HWND: " << hwnd;

    DWORD pid;
    GetWindowThreadProcessId(hwnd, &pid);
    qDebug() << "PID is: " << pid;

    return new DFInstance(pid, hwnd, parent);
}
