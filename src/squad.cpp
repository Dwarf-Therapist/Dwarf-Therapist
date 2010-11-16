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
#include "word.h"
#include "dwarfmodel.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "dwarftherapist.h"
#include "mainwindow.h"
#include "truncatingfilelogger.h"

Squad::Squad(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    refresh_data();
}

Squad::~Squad() {
}

Squad* Squad::get_squad(DFInstance *df, const VIRTADDR & address) {
    return new Squad(df, address);
}

void Squad::refresh_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "refresh of squad called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of squad data at" << hexify(m_address);

    m_members.clear();

    read_id();
    read_name();
    read_members();
}

void Squad::read_id() {
    m_id = m_df->read_int(m_address + m_mem->squad_offset("id"));
    TRACE << "ID:" << m_id;
}

void Squad::read_name() {
    m_name = read_chunked_name(m_address + m_mem->squad_offset("name"));
    TRACE << "Name:" << m_name;
}

void Squad::read_members() {
    DwarfModel * dm = DT->get_main_window()->get_model();

    VIRTADDR member_vector = m_address + m_mem->squad_offset("members");
    QVector<VIRTADDR> members = m_df->enumerate_vector(member_vector);
    TRACE << "Squad" << m_id << "has" << members.size() << "members.";
    foreach(VIRTADDR member_addr, members) {
        int ref_id = m_df->read_int(member_addr);
        if(ref_id != -1) {
            foreach(Dwarf * d, dm->get_dwarves()) {
                if(d->get_squad_ref_id() == ref_id) {
                    TRACE << "Squad member ref_id" << ref_id << "refers to" << d->nice_name();
                    m_members << d;
                    d->m_squad_name = name();
                    break;
                }
            }
        } else {
            m_members << NULL;
        }
    }
}

//! used by read_last_name to find word chunks
Word * Squad::read_word(uint offset) {
    Word * result = NULL;
    uint word_id = m_df->read_int(m_address +
        m_mem->squad_offset("name") + offset);
    if(word_id != 0xFFFFFFFF) {
        result = DT->get_word(word_id);
    }
    return result;
}

QString Squad::read_chunked_name(const VIRTADDR &addr) {
    QString result = "The";

    //7 parts e.g.  ffffffff ffffffff 000006d4
    //      ffffffff ffffffff 000002b1 ffffffff

    //Unknown
    Word * word = read_word(0x00);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Unknown
    word = read_word(0x04);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Verb
    word = read_word(0x08);
    if(word) {
        result.append(" " + capitalize(word->adjective()));
    }

    //Unknown
    word = read_word(0x0C);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Unknown
    word = read_word(0x10);
    if(word)
        result.append(" " + capitalize(word->base()));

    //Noun
    word = read_word(0x14);
    bool singular = false;
    if(word) {
        if(word->plural_noun().isEmpty()) {
            result.append(" " + capitalize(word->noun()));
            singular = true;
        } else {
            result.append(" " + capitalize(word->plural_noun()));
        }
    }

    //of verb
    word = read_word(0x18);
    if(word) {
        if(singular) {
            result.append(" of " + capitalize(word->verb()));
        } else {
            result.append(" of " + capitalize(word->present_participle_verb()));
        }
    }

    return result.trimmed();
}

QString Squad::capitalize(const QString & word) {
    QString result = word;
    if(!result.isEmpty()) {
        result = result.toLower();
        result[0] = result[0].toUpper();
    }
    return result;
}
