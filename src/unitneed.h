/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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
#ifndef UNIT_NEED_H
#define UNIT_NEED_H

#include <QColor>
#include <QCoreApplication>
#include <QString>

#include "utils.h"

class DFInstance;
class Dwarf;
class HistFigure;

class UnitNeed
{
    Q_DECLARE_TR_FUNCTIONS(UnitNeed)
public:
    UnitNeed(VIRTADDR address, DFInstance *df, Dwarf *d);

    enum DEGREE {
        BADLY_DISTRACTED = 0,
        DISTRACTED,
        UNFOCUSED,
        NOT_DISTRACTED,
        UNTROUBLED,
        LEVEL_HEADED,
        UNFETTERED,
        DEGREE_COUNT
    };

    int id() const { return m_id; }
    int deity_id() const { return m_deity_id; }
    int focus_level() const { return m_focus_level; }
    int need_level() const { return m_need_level; }
    DEGREE focus_degree() const { return m_focus_degree; };
    QString adjective() const;
    QString description() const;

    static QColor degree_color(int degree, bool tooltip = false);

private:
    Dwarf *m_dwarf;
    int m_id;
    int m_deity_id;
    QString m_deity_name;
    int m_focus_level;
    int m_need_level;
    DEGREE m_focus_degree;
};

#endif
