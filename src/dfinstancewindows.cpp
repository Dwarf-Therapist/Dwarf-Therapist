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
#include "memorysegment.h"


DFInstanceWindows::DFInstanceWindows(QObject* parent)
	: DFInstance(parent)
	, m_proc(0)
{}

DFInstanceWindows::~DFInstanceWindows() {
	if (m_proc) {
		CloseHandle(m_proc);
	}
}

uint DFInstanceWindows::calculate_checksum() {
	char expect_M = read_char(m_base_addr);
	char expect_Z = read_char(m_base_addr + 0x1);

	if (expect_M != 'M' || expect_Z != 'Z') {
		qWarning() << "invalid executable";
	}
	uint pe_header = m_base_addr + read_int(m_base_addr + 30 * 2);
	char expect_P = read_char(pe_header);
	char expect_E = read_char(pe_header + 0x1);
	if (expect_P != 'P' || expect_E != 'E') {
		qWarning() << "PE header invalid";
	}

	uint timestamp = read_uint(pe_header + 4 + 2 * 2);
	return timestamp;
}

QVector<uint> DFInstanceWindows::enumerate_vector(const uint &addr) {
	TRACE << "beginning vector enumeration at" << addr;
	QVector<uint> addresses;
	uint start = read_uint(addr + 4);
	TRACE << "start of vector" << start;
	uint end = read_uint(addr + 8);
	TRACE << "end of vector" << end;

	uint entries = (end - start) / sizeof(uint);
	TRACE << "there appears to be" << entries << "entries in this vector";

	Q_ASSERT(start >= 0);
	Q_ASSERT(end >= 0);
	Q_ASSERT(end >= start);
	Q_ASSERT((end - start) % 4 == 0);
	Q_ASSERT(start % 4 == 0);
	Q_ASSERT(end % 4 == 0);
	//Q_ASSERT(entries < 2000);

	int count = 0;
	for (uint ptr = start; ptr < end; ptr += 4 ) {
		uint a = read_uint(ptr);
		addresses.append(a);
		count++;
	}
	TRACE << "FOUND" << count << "addresses in vector";
	return addresses;
}

QString DFInstanceWindows::read_string(const uint &addr) {
	int len = read_int(addr + STRING_LENGTH_OFFSET);
	int cap = read_int(addr + STRING_CAP_OFFSET);
	uint buffer_addr = addr + STRING_BUFFER_OFFSET;
	if (cap >= 16)
		buffer_addr = read_uint(buffer_addr);
	
	if (len > cap || len < 0 || len > 1024) {
		// probaby not really a string
		LOGW << "Tried to read a string at" << hex << addr << "but it was totally not a string...";
		return QString();
	}
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

uint DFInstanceWindows::write_string(const uint &addr, const QString &str) {
	int cap = read_int(addr + STRING_CAP_OFFSET);
	uint buffer_addr = addr + STRING_BUFFER_OFFSET;
	if( cap >= 16 )
		buffer_addr = read_uint(buffer_addr);

	int len = qMin<int>(str.length(), cap);
	write_int(addr + STRING_LENGTH_OFFSET, len);
	return write_raw(buffer_addr, len, str.toAscii().data());
}

short DFInstanceWindows::read_short(const uint &addr) {
	char cval[2];
	memset(cval, 0, 2);
	ReadProcessMemory(m_proc, (LPCVOID)addr, &cval, 2, 0);
	return static_cast<short>((int)cval[0] + (int)(cval[1] >> 8));
}

ushort DFInstanceWindows::read_ushort(const uint &addr) {
	ushort val = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, &val, sizeof(ushort), 0);
	return val;
}

int DFInstanceWindows::read_int(const uint &addr) {
	int val = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, &val, sizeof(int), 0);
	return val;
}

uint DFInstanceWindows::read_uint(const uint &addr) {
	uint val = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, &val, sizeof(uint), 0);
	return val;
}

uint DFInstanceWindows::write_int(const uint &addr, const int &val) {
	uint bytes_written = 0;
	WriteProcessMemory(m_proc, (LPVOID)addr, &val, sizeof(int), (DWORD*)&bytes_written);
	return bytes_written;
}

char DFInstanceWindows::read_char(const uint &addr) {
	char val = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, &val, sizeof(char), 0);
	return val;
}

uint DFInstanceWindows::read_raw(const uint &addr, const uint &bytes, void *buffer) {
	memset(buffer, 0, bytes);
	uint bytes_read = 0;
	ReadProcessMemory(m_proc, (LPCVOID)addr, (void*)buffer, sizeof(uchar) * bytes, (DWORD*)&bytes_read);
	/*if (!ok || bytes_read != bytes)
		LOGW << "ERROR: tried to get" << bytes << "bytes from" << hex << addr << "but only got" 
			 << dec << bytes_read << "Windows System Error(" << dec << GetLastError() << ")";*/
	return bytes_read;
}

uint DFInstanceWindows::write_raw(const uint &addr, const uint &bytes, void *buffer) {
	uint bytes_written = 0;
	WriteProcessMemory(m_proc, (LPVOID)addr, (void*)buffer, sizeof(uchar) * bytes, (DWORD*)&bytes_written);
	Q_ASSERT(bytes_written == bytes);
	return bytes_written;
}

bool DFInstanceWindows::find_running_copy() {
	LOGD << "attempting to find running copy of DF by window handle";
	m_is_ok = false;

	HWND hwnd = FindWindow(L"OpenGL", L"Dwarf Fortress");
	if (!hwnd)
		hwnd = FindWindow(L"SDL_app", L"Dwarf Fortress");
	if (!hwnd)
		hwnd = FindWindow(NULL, L"Dwarf Fortress");

	if (!hwnd) {
		QMessageBox::warning(0, tr("Warning"),
			tr("Unable to locate a running copy of Dwarf "
			"Fortress, are you sure it's running?"));
		LOGW << "can't find running copy";
		return m_is_ok;
	}
	LOGD << "found copy with HWND: " << hwnd;

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if (pid == 0) {
		return m_is_ok;
	}
	LOGD << "PID of process is: " << pid;
	m_pid = pid;
	m_hwnd = hwnd;

	m_proc = OpenProcess(PROCESS_QUERY_INFORMATION
						 | PROCESS_VM_READ
						 | PROCESS_VM_OPERATION
						 | PROCESS_VM_WRITE, false, m_pid);
	
	//m_proc = OpenProcess(PROCESS_ALL_ACCESS, false, m_pid);

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
	} else {
		PEB peb;
		DWORD bytes = 0;
		if (ReadProcessMemory(m_proc, (PCHAR)peb_addr, &peb, sizeof(PEB), &bytes)) {
			LOGD << "read" << bytes << "bytes BASE ADDR is at: " << hex << peb.ImageBaseAddress;
			m_base_addr = (int)peb.ImageBaseAddress;
		} else {
			QMessageBox::critical(0, tr("Connection Error"), connection_error);
			qCritical() << "unable to read remote PEB!" << GetLastError();
		}
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
			m_is_ok = false;
		}
	}

	if (!m_is_ok) // time to bail
		return m_is_ok;

	m_memory_correction = (int)m_base_addr - 0x0400000;
	LOGD << "memory correction " << m_memory_correction;

	// scan pages
	uint start = 0;
	int accepted = 0;
	int rejected = 0;
	LOGD << "ENUMERATING MEMORY SEGMENTS FROM" << hex << start << "TO" << m_base_addr + m_memory_size;
	while (start < 0x7FFFFFFF) {
		MEMORY_BASIC_INFORMATION mbi;
		int sz = VirtualQueryEx(m_proc, (void*)start, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
		if (sz != sizeof(MEMORY_BASIC_INFORMATION)) {
			// incomplete data returned. increment start and move on...
			start += 0x1000;
			continue;
		}

		if (mbi.State == MEM_COMMIT 
			//&& !(mbi.Protect & PAGE_GUARD)
			&& (mbi.Protect & PAGE_EXECUTE_READ || 
				mbi.Protect & PAGE_EXECUTE_READWRITE || 
				mbi.Protect & PAGE_READONLY ||
				mbi.Protect & PAGE_READWRITE ||
				mbi.Protect & PAGE_WRITECOPY)) {
			TRACE << QString("FOUND READABLE COMMITED MEMORY SEGMENT FROM 0x%1-0x%2 SIZE: %3KB FLAGS:%4")
					.arg((uint)mbi.BaseAddress, 8, 16, QChar('0'))
					.arg((uint)mbi.BaseAddress + mbi.RegionSize, 8, 16, QChar('0'))
					.arg(mbi.RegionSize / 1024.0)
					.arg(mbi.Protect, 0, 16);
			MemorySegment *segment = new MemorySegment("", (uint)mbi.BaseAddress, (uint)(mbi.BaseAddress) + mbi.RegionSize);
			segment->is_guarded = mbi.Protect & PAGE_GUARD;
			m_regions << segment;
			accepted++;
		} else {
			TRACE << QString("REJECTING MEMORY SEGMENT AT 0x%1 SIZE: %2KB FLAGS:%3")
				.arg((uint)mbi.BaseAddress, 8, 16, QChar('0')).arg(mbi.RegionSize / 1024.0)
				.arg(mbi.Protect, 0, 16);
			rejected++;
		}
		if (mbi.RegionSize)
			start += mbi.RegionSize;
		else
			start += 0x1000; //skip ahead 1k
	}
	m_lowest_address = 0xFFFFFFFF;
	m_highest_address = 0;
	foreach(MemorySegment *seg, m_regions) {
		if (seg->start_addr < m_lowest_address)
			m_lowest_address = seg->start_addr;
		if (seg->end_addr > m_highest_address)
			m_highest_address = seg->end_addr;
	}
	LOGD << "MEMORY SEGMENT SUMMARY: accepted" << accepted << "rejected" << rejected << "total" << accepted + rejected;
	
	m_heartbeat_timer->start(1000); // check every second for disconnection
	m_is_ok = true;
	return m_is_ok;
}
#endif
