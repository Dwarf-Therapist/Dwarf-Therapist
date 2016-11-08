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
#ifndef MOOD_H
#define MOOD_H

#include "utils.h"
#include <QObject>
#include <QSettings>
#include <QColor>

class Mood : public QObject
{
    Q_OBJECT

public:

    Mood(QObject *parent=0)
        : QObject(parent)
        , m_name("")
        , m_name_colored("")
        , m_desc("")
        , m_desc_colored("")
        , m_color(QColor(Qt::black))
    {}

    Mood(QSettings &s, QObject *parent = 0);

    QString get_mood_name(bool colored = false){
        if(colored){
            return m_name_colored;
        }else{
            return m_name;
        }
    }

    QString get_mood_desc(bool colored = false){
        if(colored){
            return m_desc_colored;
        }else{
            return m_desc;
        }
    }

    QColor get_mood_color() {return m_color;}

private:
    QString m_name;
    QString m_name_colored;
    QString m_desc;
    QString m_desc_colored;
    QColor m_color;

};

#endif // MOOD_H
