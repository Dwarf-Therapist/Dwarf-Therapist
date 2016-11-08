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
#ifndef EMOTION_H
#define EMOTION_H

#include "global_enums.h"
#include <QObject>
#include <QColor>
#include <QMap>

class QSettings;

class Emotion : public QObject
{
    Q_OBJECT

private:
    EMOTION_TYPE m_id;
    QString m_name;
    int m_divider;
    QColor m_color;

    static const QMap<int,QColor> m_base_colors;

    static QMap<int,QColor> set_base_colors(){
        QMap<int,QColor> temp;
        temp.insert(6,QColor(135,106,24)); //brown
        temp.insert(7,QColor(123,123,123)); //gray
        temp.insert(8,QColor(33,33,33)); //dark gray
        temp.insert(9,QColor(45,120,153)); //blue
        temp.insert(10,QColor(40,194,23)); //green
        temp.insert(11,QColor(19,145,132)); //cyan
        temp.insert(12,QColor(255,0,0)); //red
        temp.insert(13,QColor(179,39,197)); //magenta
        temp.insert(14,QColor(184,178,9)); //yellow
        return temp;
    }

    static const QColor get_color(const int id) {return m_base_colors.value(id);}

public:
    Emotion(QObject *parent = 0);
    Emotion(int id, QSettings &s, QObject *parent = 0);

    EMOTION_TYPE id() const {return m_id;}
    QString get_name() const {return m_name;}
    int get_divider() const {return m_divider;}
    QColor get_color() const {return m_color;}
};
#endif // EMOTION_H
