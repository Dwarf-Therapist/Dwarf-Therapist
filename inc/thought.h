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
#ifndef THOUGHT_H
#define THOUGHT_H

#include <QObject>
#include <QSettings>
#include <QColor>

class Thought : public QObject
{
    Q_OBJECT

private:
    QString m_title;
    QString m_description;
    int m_effect;
    int m_subtype;
    short m_id;
    QColor m_color;

    static const QList<QColor> m_base_colors;

    static QList<QColor> set_base_colors(){
        QList<QColor> temp;
        temp.append(QColor(61,145,64)); //positive
        temp.append(QColor(99,99,99)); //neutral
        temp.append(QColor(176,23,31)); //negative
        return temp;
    }

    static const QColor c_pos(){return m_base_colors.at(0);}
    static const QColor c_neu(){return m_base_colors.at(1);}
    static const QColor c_neg(){return m_base_colors.at(2);}

public:
    Thought(int id, QObject *parent = 0);
    Thought(int id, QSettings &s, QObject *parent = 0);

    QString title() {return m_title;}
    QString desc() {return m_description;}
    int effect() const {return m_effect;}
    int subtype() const {return m_subtype;}
    short id() const {return m_id;}
    QColor color() const {return m_color;}

};
#endif // THOUGHT_H


