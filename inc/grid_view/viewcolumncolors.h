#ifndef VIEWCOLUMNCOLORS_H
#define VIEWCOLUMNCOLORS_H

#include "cellcolors.h"
#include "viewcolumnset.h"

class ViewColumnColors : public CellColors {
    Q_OBJECT
public:
    ViewColumnColors(ViewColumnSet *set, QObject *parent=0)
        : CellColors(parent)
        , m_set(set)
    {
        use_defaults();
    }
    ViewColumnColors(QSettings &s, ViewColumnSet *set, QObject *parent=0)
        : CellColors(parent)
        , m_set(set)
    {
        use_defaults();
        load_settings(s);
    }
    ~ViewColumnColors(){
        m_set = 0;
    }

    void use_defaults(){
        CellColors *cc = m_set->get_colors();
        m_colors.clear();
        m_colors.append(qMakePair(false,cc->active_color()));
        m_colors.append(qMakePair(false,cc->pending_color()));
        m_colors.append(qMakePair(false,cc->disabled_color()));
    }

    QColor get_default_color(int idx) const{
        return m_set->get_colors()->get_color(idx);
    }

    void read_settings(){
        inherit_colors(m_set->get_colors());
    }

private:
    ViewColumnSet *m_set;

};

#endif // VIEWCOLUMNCOLORS_H
