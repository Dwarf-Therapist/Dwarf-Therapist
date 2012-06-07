/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert

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
#ifndef HIST_ENTITY_H
#define HIST_ENTITY_H
#include <QtGui>
#include "utils.h"
#include "global_enums.h"

class DFInstance;
class MemoryLayout;

class FortressEntity : public QObject {
    Q_OBJECT
private:
    //this could be fleshed out into it's own class if we ever want to use more of what's in a position
    //for now, we just want the position names for display purposes
    struct position{
        QString name_female;
        QString name_male;
        QString name;
        QColor highlight;
    };

    VIRTADDR m_address;
    DFInstance * m_df;
    MemoryLayout * m_mem;

    //key is the historical figure id, and x positions as value(s)
    QMultiHash<int,position> m_nobles;

    //squads this fortress has
    QVector<VIRTADDR> m_squads;

    void read_entity();
    void read_colors();

public:    
    FortressEntity(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~FortressEntity();
    static FortressEntity* get_entity(DFInstance *df, const VIRTADDR &address);

    typedef enum{
        MULTIPLE=0,
        BOOKKEEPER=1,
        ROYALTY=2,
        BROKER=3,
        LAW=4,
        CHAMPION=5,
        CHIEF_MEDICAL_DWARF=6,
        LEADER=7,
        HAMMERER=8,
        MANAGER=9,
        MILITIA=10,
        MONARCH=11
    } NOBLE_COLORS;

    static NOBLE_COLORS get_color_type(const QString &raw) {
        QMap<QString, NOBLE_COLORS> m;
        m["BOOKKEEPER"] = BOOKKEEPER;
        m["BROKER"] = BROKER;
        m["BARON"] = ROYALTY;
        m["DUKE"] = ROYALTY;
        m["COUNT"] = ROYALTY;
        m["CAPTAIN_OF_THE_GUARD"] = LAW;
        m["CHAMPION"] = CHAMPION;
        m["CHIEF_MEDICAL_DWARF"] = CHIEF_MEDICAL_DWARF;
        m["EXPEDITION_LEADER"] = LEADER;
        m["MAYOR"] = LEADER;
        m["HAMMERER"] = HAMMERER;
        m["MANAGER"] = MANAGER;
        m["MILITIA_CAPTAIN"] = MILITIA;
        m["MILITIA_COMMANDER"] = MILITIA;
        m["MONARCH"] = MONARCH;
        return m.value(raw.toUpper());
    }

    void load_data();
    VIRTADDR address() {return m_address;}
    QString get_noble_positions(int hist_id, bool is_male);
    QColor get_noble_color(int hist_id);
    QColor default_noble_color;
    QHash<NOBLE_COLORS, QColor> noble_colors;
    bool squad_is_active(int id) {return m_squads.contains(id);}


};

#endif
