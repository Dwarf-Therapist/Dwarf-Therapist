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
#include <QtDebug>

Material::Material()
    : QObject(NULL)
    , m_index(-1)
    , m_address(0x0)
    , m_df(0x0)
    , m_mem(0x0)
{
}

Material::Material(DFInstance *df, VIRTADDR address, int index, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

Material::~Material() {
}

Material* Material::get_material(DFInstance *df, const VIRTADDR & address, int index) {
    return new Material(df, address, index);
}

void Material::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of materials called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of material data at" << hexify(m_address);

    read_material();
}

void Material::read_material() {
    if(!m_df)
        return;
    //read material names
    m_state_names.insert(GENERIC,m_df->read_string(m_address)); //the id name, but can be things like 'wood'
    m_state_names.insert(SOLID,m_df->read_string(m_address + m_mem->material_offset("solid_name")));
    m_state_names.insert(LIQUID,m_df->read_string(m_address + m_mem->material_offset("liquid_name")));
    m_state_names.insert(GAS,m_df->read_string(m_address + m_mem->material_offset("gas_name")));
    m_state_names.insert(POWDER,m_df->read_string(m_address + m_mem->material_offset("powder_name")));
    m_state_names.insert(PASTE,m_df->read_string(m_address + m_mem->material_offset("paste_name")));
    m_state_names.insert(PRESSED, m_df->read_string(m_address + m_mem->material_offset("pressed_name")));

}

QString Material::get_material_name(MATERIAL_STATES state){
    if(m_state_names.contains(state))
        return m_state_names.value(state);
    else
        return "unknown";
}
