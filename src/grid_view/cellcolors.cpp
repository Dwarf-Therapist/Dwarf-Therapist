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
#include "cellcolors.h"

#include "defines.h"
#include "utils.h"
#include "viewcolumn.h"
#include "dwarftherapist.h"

CellColors::CellColors(QObject *parent)
    : QObject(parent)
    , m_override_cell_colors(false)
{
    //connect(DT,SIGNAL(settings_changed()),SLOT(read_settings()));
}

CellColors::CellColors(const CellColors &cc)
    : QObject(cc.parent())
{
    m_colors = cc.m_colors;
    m_override_cell_colors = cc.m_override_cell_colors;
}

void CellColors::load_settings(QSettings &s){
    m_override_cell_colors = s.value("overrides_cell_colors",false).toBool();
    if(m_override_cell_colors){
        set_color(0,s.value("active_color").value<QColor>());
        set_color(1,s.value("pending_color").value<QColor>());
        set_color(2,s.value("disabled_color").value<QColor>());
    }
}

void CellColors::inherit_colors(const CellColors &cc){
    int idx = 0;
    QPair<bool,QColor> c;
    foreach(c, cc.m_colors){
        if(!m_override_cell_colors || !m_colors.at(idx).first){
            set_color(idx,cc.get_color(idx));
        }
        idx++;
    }
}

void CellColors::use_defaults(){
    m_colors.clear();
    m_colors.append(qMakePair(false,DT->get_global_color(GCOL_ACTIVE)));
    m_colors.append(qMakePair(false,DT->get_global_color(GCOL_PENDING)));
    m_colors.append(qMakePair(false,DT->get_global_color(GCOL_DISABLED)));
}

QColor CellColors::get_default_color(int idx) const{
    return DT->get_global_color(static_cast<GLOBAL_COLOR_TYPES>(idx));
}

void CellColors::write_to_ini(QSettings &s){
    //only write values if they've overridden the defaults
    if(m_override_cell_colors){
        s.setValue("overrides_cell_colors",m_override_cell_colors);
        if(m_colors.at(0).first)
            s.setValue("active_color",m_colors.at(0).second);
        if(m_colors.at(1).first)
            s.setValue("pending_color",m_colors.at(1).second);
        if(m_colors.at(2).first)
            s.setValue("disabled_color",m_colors.at(2).second);
    }
}

QColor CellColors::get_color(int idx) const {
    return m_colors.at(idx).second;
}

void CellColors::set_color(int idx, QColor c){
    QColor def = get_default_color(idx);
    QPair<bool,QColor> c_pair = m_colors.takeAt(idx);
    if(c.isValid()){
        c_pair.first = (c != def);
        c_pair.second = c;
    }else{
        c_pair.first = false;
        c_pair.second = def;
    }
    m_colors.insert(idx,c_pair);
}
