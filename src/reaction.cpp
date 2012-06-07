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
#include "reaction.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include <QtDebug>

Reaction::Reaction(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_tag(QString::null)
    , m_name(QString::null)
    , m_skill(QString::null)
    , m_skill_id(0)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

Reaction::~Reaction() {
}

Reaction* Reaction::get_reaction(DFInstance *df, const VIRTADDR & address) {
    return new Reaction(df, address);
}

void Reaction::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of Reactions called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of Reaction data at" << hexify(m_address);

    read_reaction();
}

void Reaction::read_reaction() {
    m_df->attach();
    m_tag = m_df->read_string(m_address);
    m_name = capitalize(m_df->read_string(m_address + m_df->memory_layout()->job_detail("reaction")));
    m_skill_id = m_df->read_short(m_address + m_df->memory_layout()->job_detail("reaction_skill"));
    m_skill = get_skill_name(m_skill_id);
    //LOGD << "Reaction " << m_name << " at " << hexify(m_address) << " tag " << m_tag;
    m_df->detach();
}


