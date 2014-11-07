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

#include "emotion.h"

const QMap<int,QColor> Emotion::m_base_colors = Emotion::set_base_colors();

Emotion::Emotion(QObject *parent)
    : QObject(parent)
    , m_id(EM_NONE)
    , m_name("")
    , m_divider(1)
    , m_color(QColor(Qt::black))
{
}

Emotion::Emotion(int id, QSettings &s, QObject *parent)
    : QObject(parent)
{
    m_id = static_cast<EMOTION_TYPE>(id);
    m_name = s.value("emotion","").toString();
    m_divider = s.value("divider",1).toInt();
    m_color = get_color(s.value("color",7).toInt());
}
