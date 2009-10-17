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
#include "squad.h"
#include "dwarf.h"
#include "dwarfmodel.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "dwarftherapist.h"

Squad::Squad(Dwarf *leader, DFInstance *df, QObject *parent)
    : QObject(parent)
    , m_leader(leader)
    , m_parent_squad(0)
{
    m_members.clear();

    MemoryLayout *mem = df->memory_layout();
    for (int i = 0; i < 24; i+=4) {
        int word_offset = df->read_int(leader->address() + mem->dwarf_offset("squad_name") + i);
        if (word_offset == 0 || word_offset == 0xFFFFFFFF)
            continue;
        m_name += DT->get_dwarf_word(word_offset);
        m_generic_name += DT->get_generic_word(word_offset);
    }
    
    
    //Squad *m_parent_squad;
    //QString m_name;
    //QString m_generic_name;
    //bool m_standing_down;
    //short m_carried_rations; //0, 1 or 2
    //bool m_carries_water;
    //SQUAD_SLEEP_LOCATION m_sleep_location; // room, ground, or barracks
    //bool m_chases_opponents;
    //bool m_harasses_animals;

}

Squad::~Squad() {
    m_members.clear();
}

void Squad::add_member(Dwarf *d) {
    if (d && !m_members.contains(d))
        m_members << d;
}