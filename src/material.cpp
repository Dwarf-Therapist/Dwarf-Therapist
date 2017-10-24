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
#include "material.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"

Material::Material(QObject *parent)
    : QObject(parent)
    , m_index(-1)
    , m_address(0x0)
    , m_df(0x0)
    , m_mem(0x0)
    , m_flags()
    , m_inorganic(false)
    , m_is_generated(false)
{
}

Material::Material(DFInstance *df, VIRTADDR address, int index, bool inorganic, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_flags()
    , m_inorganic(inorganic)
    , m_is_generated(false)
{
    load_data();
}

Material::~Material() {
    m_state_names.clear();
}

Material* Material::get_material(DFInstance *df, const VIRTADDR & address, int index, bool inorganic, QObject *parent) {
    return new Material(df, address, index, inorganic, parent);
}

void Material::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of materials called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();

    //if passed in an inorganic material, we have to offset to the material.common first
    if(m_inorganic){
        //additionally, check the inorganic flags, specifically for the 1st bit (GENERATED) so they can be ignored in roles
        int offset = m_mem->material_offset("inorganic_flags");
        if(offset != -1){
            FlagArray inorganic_flags;
            inorganic_flags = FlagArray(m_df,m_address + offset);
            m_is_generated = inorganic_flags.has_flag(1);
        }
        m_address += m_mem->material_offset("inorganic_materials_vector");
    }

    TRACE << "Starting refresh of material data at" << hexify(m_address);

    read_material();
}

void Material::read_material() {
    if(!m_df)
        return;

    //read material names
    m_state_names.insert(SOLID,m_df->read_string(m_address + m_mem->material_offset("solid_name")));
    m_state_names.insert(LIQUID,m_df->read_string(m_address + m_mem->material_offset("liquid_name")));
    m_state_names.insert(GAS,m_df->read_string(m_address + m_mem->material_offset("gas_name")));
    m_state_names.insert(POWDER,m_df->read_string(m_address + m_mem->material_offset("powder_name")));
    m_state_names.insert(PASTE,m_df->read_string(m_address + m_mem->material_offset("paste_name")));
    m_state_names.insert(PRESSED, m_df->read_string(m_address + m_mem->material_offset("pressed_name")));

    //QString id = m_df->read_string(m_address);
    m_prefix = m_df->read_string(m_address + m_mem->material_offset("prefix"));
    for (auto addr: m_df->enumerate_vector(m_address + m_mem->material_offset("reaction_class"))) {
        QString reaction = m_df->read_string(addr);
        if (reaction.isEmpty() || !m_reactions.insert(reaction).second) {
            LOGW << "Invalid reaction class" << reaction;
        }
    }
    m_flags = FlagArray(m_df,m_address + m_mem->material_offset("flags"));

    //int material_value = m_df->read_int(m_address + 0x244);
}

QString Material::get_material_name(MATERIAL_STATES state) const {
    if(m_state_names.contains(state)) {
        QString state_name = m_state_names.value(state);
        return m_prefix.isEmpty ()
               ? state_name
               : QString("%1 %2").arg(m_prefix).arg(state_name);
    }
    else
        return "";
}

