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
#ifndef PROFESSION_H
#define PROFESSION_H

#include <QString>
#include <QStringList>
#include <QSettings>

class Profession {
public:
    Profession()
        : m_id(-1)
        , m_name("UNKNOWN PROFESSION")
        , m_female_name(m_name)
        , m_is_military(false)
        , m_can_assign_labors(true)
        , m_can_assign_military(true)
    {
    }

    Profession(const QSettings &s)
        : m_id(s.value("id", -1).toInt())
        , m_name(s.value("name", "UNKNOWN PROFESSION").toString())
        , m_female_name(s.value("female_name", m_name).toString())
        , m_is_military(s.value("is_military", false).toBool())
        , m_can_assign_labors(s.value("can_assign_labors", true).toBool())
        , m_can_assign_military(s.value("can_assign_military", true).toBool())
    {
    }

    short id() const {return m_id;}
    QString name(bool male = true) const {
        return male ? m_name : m_female_name;
    }
    bool is_military() const {return m_is_military;}
    bool can_assign_labors() const {return m_can_assign_labors;}
    bool can_assign_military()  const {return m_can_assign_military;}

    bool operator<(const Profession &other) const {
        return m_name < other.m_name;
    }

private:
    short m_id;
    QString m_name;
    QString m_female_name;
    bool m_is_military;
    bool m_can_assign_labors;
    bool m_can_assign_military;
};

#endif // PROFESSION_H
