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
#include "belief.h"
#include "gamedatareader.h"
#include <QtWidgets>

//personality facets
Belief::Belief(int id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    this->m_id = id;
    name = s.value("name", "UNKNOWN").toString();

    m_level_string[-50] = s.value("level_0", "").toString();
    m_level_string[-40] = s.value("level_1", "").toString();
    m_level_string[-25] = s.value("level_2", "").toString();
    m_level_string[-10] = s.value("level_3", "").toString();
    m_level_string[11] = s.value("level_4", "").toString();
    m_level_string[26] = s.value("level_5", "").toString();
    m_level_string[41] = s.value("level_6", "").toString();
}

QString Belief::level_message(const short &val){
    QString ret_val;
    QMapIterator<int,QString> i(m_level_string);
    i.toBack();
    while(i.hasPrevious()){
        i.previous();
        if(val >= i.key()){
            ret_val = i.value();
            break;
        }
    }
    return ret_val;
}
