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
#ifndef EMOTIONGROUP_H
#define EMOTIONGROUP_H

#include "dwarf.h"
#include <QObject>

class EmotionGroup : public QObject
{
    Q_OBJECT

public:
    EmotionGroup(QObject *parent = 0)
        : QObject(parent)
        , m_stress_count(0)
        , m_unaffected_count(0)
        , m_eustress_count(0)
    {
    }

    struct emotion_count{
        int count;
        QMap<QString,QVariant> unit_ids;
    };

    int get_stress_count() {return m_stress_count;}
    int get_unaffected_count() {return m_unaffected_count;}
    int get_eustress_count() {return m_eustress_count;}

    int get_stress_unit_count() {return m_stress_ids.count();}
    int get_unaffected_unit_count() {return m_unaffected_ids.count();}
    int get_eustress_unit_count() {return m_eustress_ids.count();}

    QHash<EMOTION_TYPE, emotion_count> get_details() {return m_details;}
    void add_detail(Dwarf *d, UnitEmotion *ue);

    int get_total_occurrances(){
        return m_stress_count + m_eustress_count + m_unaffected_count;
    }

private:
    QHash<EMOTION_TYPE, emotion_count> m_details;

    int m_stress_count;
    int m_unaffected_count;
    int m_eustress_count;

    QList<int> m_stress_ids;
    QList<int> m_eustress_ids;
    QList<int> m_unaffected_ids;
};

#endif // EMOTIONGROUP_H
