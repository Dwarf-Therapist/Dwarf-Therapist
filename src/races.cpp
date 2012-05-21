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
#include "races.h"
#include "caste.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include <QtDebug>

Race::Race(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_name(QString::null)
    , m_description(QString::null)
    , m_name_plural(QString::null)
    , m_adjective(QString::null)
    , m_baby_name(QString::null)
    , m_baby_name_plural(QString::null)
    , m_child_name(QString::null)
    , m_child_name_plural(QString::null)
    , m_castes_vector(0)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

Race::~Race() {
}

Race* Race::get_race(DFInstance *df, const VIRTADDR & address) {
    return new Race(df, address);
}

void Race::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of Races called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of Race data at" << hexify(m_address);

    read_race();
}

void Race::read_race() {
    m_df->attach();
    m_id = m_df->read_int(m_address);
    m_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("name_singular")));
    TRACE << "RACE " << m_name << " at " << hexify(m_address);
    m_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("name_plural")));
    m_adjective = capitalize(m_df->read_string(m_address + m_mem->race_offset("adjective")));
    m_baby_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("baby_name_singular")));
    m_baby_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("baby_name_plural")));
    m_child_name = capitalize(m_df->read_string(m_address + m_mem->race_offset("child_name_singular")));
    m_child_name_plural = capitalize(m_df->read_string(m_address + m_mem->race_offset("child_name_plural")));
    m_pref_string_vector = m_address + m_mem->race_offset("pref_string_vector");
    m_pop_ratio_vector = m_address + m_mem->race_offset("pop_ratio_vector");
    m_castes_vector = m_address + m_mem->race_offset("castes_vector");
    //m_description = m_df->read_string(m_address + m_mem->caste_offset("caste_descr"));
    QVector<VIRTADDR> castes = m_df->enumerate_vector(m_castes_vector);
    TRACE << "RACE " << m_name << " with " << castes.size() << "castes";

    if (!castes.empty()) {
        Caste *c = 0;
        int i = 0;
        foreach(VIRTADDR caste_addr, castes) {
            c = Caste::get_caste(m_df, caste_addr, m_name);
            if (c != 0) {
                m_castes[i] = c;
                TRACE << "FOUND CASTE " << hexify(caste_addr);
            }
            i++;
        }        
    }
    m_df->detach();
}


