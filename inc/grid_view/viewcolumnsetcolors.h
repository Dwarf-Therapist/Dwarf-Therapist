#ifndef VIEWCOLUMNSETCOLORS_H
#define VIEWCOLUMNSETCOLORS_H

#include "cellcolors.h"
#include "dwarftherapist.h"

class ViewColumnSetColors : public CellColors {
    Q_OBJECT
public:
    ViewColumnSetColors(QObject *parent=0)
        : CellColors(parent)
        , m_defaults(0)
    {
        use_defaults();
    }

    ViewColumnSetColors(QSettings &s, QObject *parent=0)
        : CellColors(parent)
        , m_defaults(0)
    {
        use_defaults();
        load_settings(s);
    }

    ~ViewColumnSetColors(){
        m_defaults = 0;
    }

    void use_defaults(){
        m_colors.clear();
        m_colors.append(qMakePair(false,DT->get_global_color(GCOL_ACTIVE)));
        m_colors.append(qMakePair(false,DT->get_global_color(GCOL_PENDING)));
        m_colors.append(qMakePair(false,DT->get_global_color(GCOL_DISABLED)));

        if(m_defaults == 0){
            m_defaults = new CellColors(*this);
        }
    }

    void read_settings(){
        QPair<bool,QColor> c_pair;
        int idx = 0;
        foreach(c_pair, m_colors){
            QColor def =DT->get_global_color(static_cast<GLOBAL_COLOR_TYPES>(idx));
            if(!c_pair.first){ //!overridden
                set_color(idx,def);
            }
            m_defaults->set_color(idx, def);
            idx++;
        }
    }

    CellColors *get_default_colors(){
        return m_defaults;
    }

private:
    CellColors *m_defaults;
};
#endif // VIEWCOLUMNSETCOLORS_H
