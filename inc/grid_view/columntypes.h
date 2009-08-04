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
#ifndef COLUMN_TYPES_H
#define COLUMN_TYPES_H

#include <QString>

typedef enum {
	CT_DEFAULT,
	CT_SPACER,
	CT_SKILL,
	CT_LABOR,
	CT_HAPPINESS
} COLUMN_TYPE;

static inline COLUMN_TYPE get_column_type(const QString &name) {
	if (name.toLower() == "spacer" || name.toLower() == "space") {
		return CT_SPACER;
	} else if (name.toLower() == "labor") {
		return CT_LABOR;
	} else if (name.toLower() == "skill") {
		return CT_SKILL;
	} else if (name.toLower() == "happiness") {
		return CT_HAPPINESS;
	}
	return CT_DEFAULT;
}

static inline QString get_column_type(const COLUMN_TYPE &type) {
	switch (type) {
		case CT_SPACER:		return "SPACER";
		case CT_SKILL:		return "SKILL";
		case CT_LABOR:		return "LABOR";
		case CT_HAPPINESS:	return "HAPPINESS";
	}
	return "UNKNOWN";
}

#endif;
