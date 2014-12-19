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
#include "viewcolumnsetcolors.h"

ViewColumnSetColors::ViewColumnSetColors(QObject *parent)
    : CellColors(parent)
    , m_defaults(0)
{
    use_defaults();
}

ViewColumnSetColors::ViewColumnSetColors(QSettings &s, QObject *parent)
    : CellColors(parent)
    , m_defaults(0)
{
    use_defaults();
    load_settings(s);
}

ViewColumnSetColors::~ViewColumnSetColors(){
    m_defaults = 0;
}

void ViewColumnSetColors::use_defaults(){
    CellColors::use_defaults();

    if(m_defaults == 0){
        m_defaults = new CellColors(*this);
    }
}

void ViewColumnSetColors::read_settings(){
    for(int idx=0;idx<m_colors.size();idx++){
        QColor def = DT->get_global_color(static_cast<GLOBAL_COLOR_TYPES>(idx));
        if(!m_colors.at(idx)->is_overridden()){
            m_colors[idx]->set_color(def);
        }
        m_defaults->colors().at(idx)->set_color(def);
        //m_defaults->set_color(idx,def,false);
    }
}

CellColors *ViewColumnSetColors::get_default_colors(){
    return m_defaults;
}
