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
#ifdef _WINDOWS
#include <QtGui>
#include <QtDebug>
#include <windows.h>
#include <psapi.h>

#include "dfinstance.h"
#include "dfinstancewindows.h"
#include "defines.h"
#include "dwarf.h"
#include "utils.h"
#include "gamedatareader.h"
#include "memorylayout.h"
#include "cp437codec.h"
#include "win_structs.h"


DFInstanceWindows::DFInstanceWindows(QObject* parent)
	: DFInstance(parent)	
{
}

DFInstanceWindows::~DFInstanceWindows() {
	if (m_proc) {
		CloseHandle(m_proc);
	}
}

int DFInstanceWindows::calculate_checksum() {
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

QVector<int> DFInstanceWindows::enumerate_vector(int address) {
        TRACE << "beginning vector enumeration at" << address;
        QVector<int> addresses;
        uint bytes_read = 0;
        int start = read_int32(address + 4, bytes_read);
        TRACE << "start of vector" << start;
        int end = read_int32(address + 8, bytes_read);
        TRACE << "end of vector" << end;

        int entries = (end - start) / sizeof(int);
        LOGD << "there appears to be" << entries << "entries in this vector";

        Q_ASSERT(start >= 0);
        Q_ASSERT(end >= 0);
        Q_ASSERT(end >= start);
        Q_ASSERT((end - start) % 4 == 0);
        Q_ASSERT(start % 4 == 0);
        Q_ASSERT(end % 4 == 0);
        //Q_ASSERT(entries < 2000);

        int count = 0;
        for( int ptr = start; ptr < end; ptr += 4 ) {
                TRACE << "reading address" << count << "at" << ptr;
                int addr = read_int32(ptr, bytes_read);
                TRACE << bytes_read << "bytes were read OK";
                if (bytes_read == sizeof(int)) {
                        TRACE << "read pointer size ok, adding address" << addr;
                        addresses.append(addr);
                }
                count++;
        }
        TRACE << "FOUND" << count << "addresses in vector";
        return addresses;
}

QString DFInstance::read_string(const uint &addr) {
	int len = read_int(addr + STRING_LENGTH_OFFSET);
	int cap = read_int(addr + STRING_CAP_OFFSET);
	int buffer_addr = addr + STRING_BUFFER_OFFSET;
	if (cap >= 16)
		buffer_addr = read_int(buffer_addr);

	Q_ASSERT_X(len <= cap, "read_string", "Length must be less than or equal to capacity!");
	Q_ASSERT_X(len >= 0, "read_string", "Length must be >=0!");
	Q_ASSERT_X(len < (1 << 16), "read_string", "String must be of sane length!");
	
	char *buffer = new char[len];
	read_raw(buffer_addr, len, buffer);
	
	CP437Codec *codec = new CP437Codec;
	QString ret_val = codec->toUnicode(buffer, len);
	delete[] buffer;
	return ret_val;
}

int DFInstanceWindows::write_string(int address, QString str) {
	uint bytes_read = 0;
	int cap = read_int32(address + STRING_CAP_OFFSET, bytes_read);
	int buffer_addr = address + STRING_BUFFER_OFFSET;
	if( cap >= 16 )
		buffer_addr = read_int32(buffer_addr, bytes_read);

	int len = qMin<int>(str.length(), cap);
	write_int32(address + STRING_LENGTH_OFFSET, len);
	return write_raw(buffer_addr, len, str.toAscii().data());
}

short DFInstanceWindows::read_short(int start_address, uint &bytes_read) {
	char cval[2];
	memset(cval, 0, 2);
	bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)start_address, &cval, 2, (DWORD*)&bytes_read);
	return static_cast<short>((int)cval[0] + (int)(cval[1] >> 8));
}

ushort DFInstanceWindows::read_ushort(int start_address, uint &bytes_read) {
	ushort val = 0;
	bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)start_address, &val, sizeof(ushort), (DWORD*)&bytes_read);
	//qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << hex << val;
	return val;
}

int DFInstanceWindows::read_int32(int start_address, uint &bytes_read) {
	int val = 0;
	bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)start_address, &val, sizeof(int), (DWORD*)&bytes_read);
	//qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << hex << val;
	return val;
}

int DFInstanceWindows::write_int32(int start_address, int val) {
	uint bytes_written = 0;
	WriteProcessMemory(m_proc, (LPVOID)start_address, &val, sizeof(int), (DWORD*)&bytes_written);
	//qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << hex << val;
	return bytes_written;
}

//returns # of bytes read
char DFInstanceWindows::read_char(int start_address, uint &bytes_read) {
	char val = 0;
	bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)start_address, &val, sizeof(char), (DWORD*)&bytes_read);
	//qDebug() << "Read from " << start_address << "OK?" << ok <<  " bytes read: " << bytes_read << " VAL " << val;
	return val;
}

int DFInstanceWindows::read_raw(uint addr, int bytes, void *buffer) {
	memset(buffer, 0, bytes);
	DWORD bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, (void*)buffer, sizeof(uchar) * bytes, &bytes_read);
	return bytes_read;
}

int DFInstanceWindows::write_raw(int start_address, int bytes, void *buffer) {
	DWORD bytes_written = 0;
	WriteProcessMemory(m_proc, (LPVOID)start_address, (void*)buffer, sizeof(uchar) * bytes, &bytes_written);
	Q_ASSERT(bytes_written == bytes);
	return bytes_written;
}

bool DFInstanceWindows::find_running_copy() {
	LOGD << "attempting to find running copy of DF by window handle";

	HWND hwnd = FindWindow(NULL, L"Dwarf Fortress");
	if (!hwnd) {
		QMessageBox::warning(0, tr("Warning"),
			tr("Unable to locate a running copy of Dwarf "
			"Fortress, are you sure it's running?"));
		LOGW << "can't find running copy";
		return false;
	}
	LOGD << "found copy with HWND: " << hwnd;

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0)
		return false;
	LOGD << "PID of process is: " << pid;
	m_pid = pid;
	m_hwnd = hwnd;

	m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
						 | PROCESS_VM_READ
						 | PROCESS_VM_OPERATION
						 | PROCESS_VM_WRITE, false, m_pid);

	PROCESS_MEMORY_COUNTERS pmc;
	GetProcessMemoryInfo(m_proc, &pmc, sizeof(pmc));
	m_memory_size = pmc.WorkingSetSize;
	LOGD << "working set size: " << dec << m_memory_size / (1024.0f * 1024.0f) << "MB";

	PVOID peb_addr = GetPebAddress(m_proc);
	LOGD << "PEB is at: " << hex << peb_addr;

	QString connection_error = tr("I'm sorry. I'm having trouble connecting to DF. "
		"I can't seem to locate the PEB address of the process. \n\n"
		"Please re-launch DF and try again.");

	if (peb_addr == 0){
		QMessageBox::critical(0, tr("Connection Error"), connection_error);
		qCritical() << "PEB address came back as 0";
		m_is_ok = false;
	} else {
		PEB peb;
		DWORD bytes = 0;
		if (ReadProcessMemory(m_proc, (PCHAR)peb_addr, &peb, sizeof(PEB), &bytes)) {
			LOGD << "read" << bytes << "bytes BASE ADDR is at: " << hex << peb.ImageBaseAddress;
			m_base_addr = (int)peb.ImageBaseAddress;
		} else {
			QMessageBox::critical(0, tr("Connection Error"), connection_error);
			qCritical() << "unable to read remote PEB!" << GetLastError();
			m_is_ok = false;
		}
		if (m_is_ok) {
			int checksum = calculate_checksum();
			LOGD << "DF's checksum is:" << hex << checksum;
			//GameDataReader::ptr()->set_game_checksum(checksum);

			m_layout = new MemoryLayout(checksum);
			if (!m_layout->is_valid()) {
				QMessageBox::critical(0, tr("Unidentified Version"),
					tr("I'm sorry but I don't know how to talk to this version of DF!"));
				LOGC << "unable to identify version from checksum:" << hex << checksum;
				//m_is_ok = false;
			}
		}

		m_memory_correction = (int)m_base_addr - 0x0400000;
		LOGD << "memory correction " << m_memory_correction;
	}
	return true;
}
#endif
