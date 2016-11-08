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

#include "thought.h"

#include <QSettings>
#include <QList>
#include <QColor>

const QList<QColor> Thought::m_base_colors = Thought::set_base_colors();

Thought::Thought(int id, QObject *parent)
    : QObject(parent)
    , m_title(QString("%1 - Unknown").arg(QString::number(id)))
    , m_description("This is an unknown thought, please report it!")
    , m_effect(0)
    , m_subtype(-1)
    , m_id(id)
{
}

Thought::Thought(int id, QSettings &s, QObject *parent)
    : QObject(parent)
    , m_title(s.value("title", "Unknown").toString())
    , m_description(s.value("thought", m_title).toString())
    , m_effect(s.value("value", 0).toInt())
    , m_subtype(s.value("subthoughts_type",-1).toInt())
    , m_id(id)
{

    if(m_effect == 0){
        m_color = c_neu();
    }
    else if(m_effect > 0){
        m_color = c_pos();
    }else{
        m_color = c_neg();
    }
    //-1000 to +1000
    //(x - from_min) * (to_max - to_min) / (from_max - from_min) + to_min
    int alpha = (((m_effect + 50) * (255-75)) / 100) + 75;
    if(alpha > 255)
        alpha = 255;
    else if(alpha < 0)
        alpha = 0;

    if(m_effect < 0)
        alpha = 255 - alpha;

    m_color.setAlpha(alpha);

}
