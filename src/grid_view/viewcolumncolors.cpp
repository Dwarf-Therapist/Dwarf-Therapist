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
#include "viewcolumncolors.h"
#include "viewcolumnset.h"
#include "viewcolumnsetcolors.h"

ViewColumnColors::ViewColumnColors(QObject *parent)
    : CellColors(parent)
    , m_set(0)
{
    use_defaults();
}
ViewColumnColors::ViewColumnColors(ViewColumnSet *set, QObject *parent)
    : CellColors(parent)
    , m_set(set)
{
    use_defaults();
}
ViewColumnColors::ViewColumnColors(QSettings &s, ViewColumnSet *set, QObject *parent)
    : CellColors(parent)
    , m_set(set)
{
    use_defaults();
    load_settings(s);
}
ViewColumnColors::~ViewColumnColors(){
    m_set = 0;
}

void ViewColumnColors::use_defaults(){
    //use the set's colors
    if(m_set){
        m_color_defs.clear();
        foreach(QSharedPointer<CellColorDef> c, m_set->get_colors()->get_color_defs()){
            m_color_defs.append(c);
        }

//        CellColors *cc = m_set->get_colors();
//        qDeleteAll(m_colors);
//        m_colors.clear();
//        foreach(CellColorDef *ccd, cc->get_colors()){
//            m_colors.append(new CellColorDef(*ccd,this));
//        }
    }else{
        CellColors::use_defaults();
    }
}

QColor ViewColumnColors::get_default_color(int idx) const{
    if(m_set){
        return m_set->get_colors()->get_color(idx);
    }else{
        return CellColors::get_default_color(idx);
    }
}

QSharedPointer<CellColorDef> ViewColumnColors::get_default_color_def(int idx){
    if(m_set){
        return m_set->get_colors()->get_color_defs().at(idx);
    }else{
        return CellColors::get_default_color_def(idx);
    }
}

void ViewColumnColors::read_settings(){
    if(m_set){
        inherit_colors(*m_set->get_colors());
    }
}
