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
#ifndef LABOR_H
#define LABOR_H

#include <QtGui>

class Labor : public QObject {
	Q_OBJECT
public:
	Labor(QString name, int id, int skill, int list_order, QObject *parent = 0) 
		: QObject(parent)
		, name(name)
		, labor_id(id)
		, skill_id(skill)
		, list_order(list_order)
	{}
	Labor(Labor &other)
		: QObject(other.parent())
		, name(other.name)
		, labor_id(other.labor_id)
		, list_order(other.list_order)
	{}

	int operator<(const Labor &other) {
		return other.list_order < list_order;
	}

	QString name;
	int labor_id;
	int skill_id;
	int list_order;
};

#endif

