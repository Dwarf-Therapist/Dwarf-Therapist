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
#ifndef MILITARY_PREFERENCE_H
#define MILITARY_PREFERENCE_H

#include <QtGui>
#include "defines.h"

struct PreferenceValue {
    short id;
    QString name;
    QString symbol;
};

class MilitaryPreference : public QObject {
    Q_OBJECT
public:
    MilitaryPreference(QSettings &s, QObject *parent = 0)
        : QObject(parent)
        , name(s.value("name", "UNKNOWN PREFERENCE").toString())
        , labor_id(s.value("id", -1).toInt())
        , skill_id(s.value("skill", -1).toInt())
    {
        int values = s.beginReadArray("values");
        for (int i = 0; i < values; ++i) {
            s.setArrayIndex(i);
            PreferenceValue *pv = new PreferenceValue;
            pv->id = s.value("id", -1).toInt();
            pv->name = s.value("name").toString();
            pv->symbol = s.value("symbol", pv->name).toString();
            if (pv->id != -1 || !pv->name.isEmpty()) {
                m_values << pv;
            } else {
                delete pv;
            }
        }
        s.endArray();
    }
    
    virtual ~MilitaryPreference() {
        foreach(PreferenceValue *pv, m_values) {
            delete pv;
        }
        m_values.clear();
    }

    QString value_symbol(const short &val_id) {
        foreach(PreferenceValue *pv, m_values) {
            if (pv->id == val_id)
                return pv->symbol;
        }
        return QString("%1: Unknown Symbol").arg(name);
    }

    QString value_name(const short &val_id) {
        foreach(PreferenceValue *pv, m_values) {
            if (pv->id == val_id)
                return pv->name;
        }
        return QString("%1: Unknown Value").arg(name);
    }

    // return the next value in the list, should wrap around
    short next_val(const short &old_val) {
        int count = 0;
        foreach (PreferenceValue *pv, m_values) {
            if (pv->id == old_val) {
                if (count + 1 >= m_values.size()) {
                    return m_values.at(0)->id;
                } else {
                    return m_values.at(count + 1)->id;
                }
            }
            count++;
        }
        LOGW << "Military Preference:" << name << "was unable to find a suitable next value after" << old_val;
        return old_val; // that sucks...
    }

    QString name;
    int labor_id;
    int skill_id;
    QList<PreferenceValue*> m_values;
};

#endif

