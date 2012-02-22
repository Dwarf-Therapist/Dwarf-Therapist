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
    Attribute(QSettings &s, QObject *parent = 0)
        : QObject(parent)
        , name(s.value("name", "UNKNOWN ATTRIBUTE").toString())
    {
        int levels = s.beginReadArray("levels");
        for (int i = 0; i < levels; ++i) {
            s.setArrayIndex(i);
            int level = s.value("level_id", -1).toInt();
            QString level_name = s.value("level_name", "").toString();
            if (level != -1)
                m_levels.insert(level, level_name);
        }
        s.endArray();
    }
    QString name;
    QHash<int, QString> m_levels;
    typedef enum {
        AT_STRENGTH = 0,
        AT_AGILITY,
        AT_TOUGHNESS,
        AT_ENDURANCE,
        AT_RECUPERATION,
        AT_DISEASE_RESISTANCE
    } ATTRIBUTES_TYPE;

};

#endif // ATTRIBUTE_H
