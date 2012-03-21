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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QtGui>

class Attribute : public QObject {
    Q_OBJECT
public:
    Attribute(QSettings &s, QObject *parent = 0);
    QString name;
    int id;
    //QHash<int, QString> m_levels;
    //QList<float> m_limits;

    struct level{
        QString description;
        int rating;
        int limit;
    };

    QList<level> m_levels;

    typedef enum {
        AT_STRENGTH = 0,
        AT_AGILITY=1,
        AT_TOUGHNESS=2,
        AT_ENDURANCE=3,
        AT_RECUPERATION=4,
        AT_DISEASE_RESISTANCE=5,
        AT_ANALYTICAL_ABILITY=6,
        AT_FOCUS=7,
        AT_WILLPOWER=8,
        AT_CREATIVITY=9,
        AT_INTUITION=10,
        AT_PATIENCE=11,
        AT_MEMORY=12,
        AT_LINGUISTIC_ABILITY=13,
        AT_SPATIAL_SENSE=14,
        AT_MUSICALITY=15,
        AT_KINESTHETIC_SENSE=16,
        AT_EMPATHY=17,
        AT_SOCIAL_AWARENESS=18
    } ATTRIBUTES_TYPE;

protected:
    int find_attribute_limit(int id, int level_index);

};

#endif // ATTRIBUTE_H
