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
#ifndef VERSION_H
#define VERSION_H

#include <QString>
#include "defines.h"

struct Version {
	int major;
	int minor;
	int patch;

	Version()
		: major(VERSION_MAJOR)
		, minor(VERSION_MINOR)
		, patch(VERSION_PATCH)
	{}
	Version(int major, int minor, int patch) 
		: major(major)
		, minor(minor)
		, patch(patch)
	{}

	QString to_string() const {return QString("%1.%2.%3").arg(major).arg(minor).arg(patch);}

	bool operator==(const Version &other) const {
		return major == other.major && minor == other.minor && patch == other.patch;
	}
	
	bool operator<(const Version &other) const {
		if (major == other.major) {
			if (minor == other.minor) {
				return patch < other.patch;
			} else {
				return minor < other.minor;
			}
		} else {
			return major < other.major;
		}
	}
};

#endif