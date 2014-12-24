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
#include "viewcolumn.h"
#include "dwarftherapist.h"
#include "cellcolordef.h"

CellColors::CellColors(QObject *parent)
    : QObject(parent)
    , m_override_cell_colors(false)
{
}

CellColors::CellColors(const CellColors &cc)
    : QObject(cc.parent())
{
    m_color_defs = cc.m_color_defs;
    m_override_cell_colors = cc.m_override_cell_colors;
}

CellColors::~CellColors(){
    m_color_defs.clear();
}

void CellColors::load_settings(QSettings &s){
    m_override_cell_colors = s.value("overrides_cell_colors",false).toBool();
    if(m_override_cell_colors){
        set_color(0,s.value("active_labor").value<QColor>());
        set_color(1,s.value("pending_color").value<QColor>());
        set_color(2,s.value("disabled_color").value<QColor>());
    }
}

void CellColors::inherit_colors(const CellColors &cc){
    int idx = 0;
    foreach(QSharedPointer<CellColorDef> c, cc.m_color_defs){
        //if we're not overridding or this specific color isn't overridden, then inherit
        if(!m_override_cell_colors || !c->is_overridden()){
            m_color_defs[idx].swap(c);
        }
        idx++;
    }
}

void CellColors::use_defaults(){
    //by default use the global color scheme
    m_color_defs.clear();
    m_color_defs.append(DT->get_global_color(GCOL_ACTIVE));
    m_color_defs.append(DT->get_global_color(GCOL_PENDING));
    m_color_defs.append(DT->get_global_color(GCOL_DISABLED));
}

QColor CellColors::get_default_color(int idx) const{
    return DT->get_global_color(static_cast<GLOBAL_COLOR_TYPES>(idx))->color();
}

QSharedPointer<CellColorDef> CellColors::get_default_color_def(int idx){
    return DT->get_global_color(static_cast<GLOBAL_COLOR_TYPES>(idx));
}

void CellColors::write_to_ini(QSettings &s){
    //only write values if they've overridden the defaults
    if(m_override_cell_colors){
        s.setValue("overrides_cell_colors",m_override_cell_colors);
        foreach(QSharedPointer<CellColorDef> c, m_color_defs){
            s.setValue(c->key(),c->color());
        }
    }
}

QColor CellColors::get_color(int idx) const {
    return m_color_defs.at(idx)->color();
}

QVector<QSharedPointer<CellColorDef> > CellColors::get_color_defs(){
    return m_color_defs;
}

void CellColors::set_color(int idx, QColor c){
    if(c.isValid()){
        if(c != get_default_color(idx)){
            //new color is valid and not the default color
            if(m_color_defs.at(idx)->is_overridden()){
                //current color def is already custom, update the color def's color
                m_color_defs.at(idx)->set_color(c);
            }else{
                //current color def is not custom, so it's the default. create a new def by copying the existing one
                QSharedPointer<CellColorDef> cpy = QSharedPointer<CellColorDef>(new CellColorDef(*m_color_defs.at(idx)));
                cpy->set_color(c);
                cpy->set_overridden(true);
                m_color_defs[idx].swap(cpy);
            }
        }else{
            //new color is the default color
            if(m_color_defs.at(idx)->is_overridden()){
                //current color def is custom, replace it with the default color def
                QSharedPointer<CellColorDef> def = get_default_color_def(idx);
                m_color_defs[idx].swap(def);
            }else{
                //current color is already default, do nothing
            }
        }
    }else{
        //invalid color, do nothing
    }
}
