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
#ifndef CELLCOLORS_H
#define CELLCOLORS_H

#include <QObject>
#include <QSettings>
#include "defines.h"
#include "utils.h"
#include "viewcolumn.h"
#include "dwarftherapist.h"

class CellColors : public QObject {
    Q_OBJECT
public:
    CellColors(QObject *parent=0)
        : QObject(parent)
    {
        //load global default settings
        load_defaults();
        m_override_cell_colors = false;
    }

    CellColors(QSettings &s, QObject *parent=0)
        : QObject(parent)
    {
        m_override_cell_colors = s.value("overrides_cell_colors",false).toBool();
        if(m_override_cell_colors){
            m_active = s.value("active_color").value<QColor>();
            m_pending = s.value("pending_color").value<QColor>();
            m_disabled = s.value("disabled_color").value<QColor>();
        }else{
            load_defaults();
        }
    }

    CellColors(const CellColors &c)
        : QObject(c.parent())
        , m_override_cell_colors(c.m_override_cell_colors)
        , m_active(c.m_active)
        , m_pending(c.m_pending)
        , m_disabled(c.m_disabled)
    {}

    bool overrides_cell_colors() {return m_override_cell_colors;}
    void set_overrides_cell_colors(bool val) {m_override_cell_colors = val;}

    void copy_colors(const CellColors &colors){
        m_active = colors.active_color();
        m_pending = colors.pending_color();
        m_disabled = colors.disabled_color();
    }

    QColor active_color() const {return m_active;}
    void set_active_color(QColor c){m_active = c;}

    QColor pending_color() const {return m_pending;}
    void set_pending_color(QColor c){m_pending = c;}

    QColor disabled_color() const {return m_disabled;}
    void set_disabled_color(QColor c){m_disabled = c;}

    void set_state_color(ViewColumn::CELL_STATE state, QColor new_color){
        switch(state){
        case ViewColumn::STATE_ACTIVE:
        {
            m_active = new_color;
        }
            break;
        case ViewColumn::STATE_PENDING:
        {
            m_pending = new_color;
        }
            break;
        case ViewColumn::STATE_DISABLED:
        {
            m_disabled = new_color;
        }
            break;
        default:
        {
        }
            break;
        }
    }

public slots:
    void load_defaults(){
        m_active = DT->get_global_color(GCOL_ACTIVE);
        m_pending = DT->get_global_color(GCOL_PENDING);
        m_disabled = DT->get_global_color(GCOL_DISABLED);
    }

private:
    bool m_override_cell_colors;
    QColor m_active;
    QColor m_pending;
    QColor m_disabled;

};
#endif // CELLCOLORS_H
