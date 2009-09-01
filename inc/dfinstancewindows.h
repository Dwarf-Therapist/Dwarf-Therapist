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
#ifndef DFINSTANCE_WINDOWS_H
#define DFINSTANCE_WINDOWS_H
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "dfinstance.h"
#include "dwarf.h"

class MemoryLayout;

class DFInstanceWindows : public DFInstance {
	Q_OBJECT
public:
	DFInstanceWindows(QObject *parent=0);
	~DFInstanceWindows();
	typedef QVector<int> AddressVector;

	// factory ctor
	bool find_running_copy();

        QVector<int> enumerate_vector(int address);
	char read_char(int start_address, uint &bytes_read);
	short read_short(int start_address, uint &bytes_read);
	ushort read_ushort(int start_address, uint &bytes_read);
	int read_int32(int start_address, uint &bytes_read);
	int read_raw(uint addr, int bytes, void *buffer);
    QString read_string(const uint &addr);
	
	// Writing
	int write_raw(int start_address, int bytes, void *buffer);
	int write_string(int start_address, QString str);
	int write_int32(int start_address, int val);


protected:
	// handy util methods
	int calculate_checksum();

	HWND m_hwnd;
	HANDLE m_proc;
};

#endif // DFINSTANCE_H
#endif // OS check
