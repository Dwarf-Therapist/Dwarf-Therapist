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
#include "plant.h"
#include "dfinstance.h"
#include "material.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"

Plant::Plant(QObject *parent)
    : QObject(parent)
    , m_index(-1)
    , m_address(0x0)
    , m_df(0x0)
    , m_mem(0x0)
    , m_flags()
    , m_is_crop(false)
{
}

Plant::Plant(DFInstance *df, VIRTADDR address, int index, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_flags()
    , m_is_crop(false)
{
    load_data();
}

Plant::~Plant() {
    qDeleteAll(m_plant_mats);
    m_plant_mats.clear();
}

Plant* Plant::get_plant(DFInstance *df, const VIRTADDR & address, int index) {
    return new Plant(df, address, index);
}

void Plant::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of plants called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();
    TRACE << "Starting refresh of plant data at" << hexify(m_address);

    read_plant();
}

void Plant::read_plant() {
    m_plant_name = m_df->read_string(m_address + m_mem->plant_offset("name"));
    m_plant_name_plural = m_df->read_string(m_address + m_mem->plant_offset("name_plural"));
    m_leaf_name_plural = m_df->read_string(m_address + m_mem->plant_offset("name_leaf_plural"));
    m_seed_name_plural = m_df->read_string(m_address + m_mem->plant_offset("name_seed_plural"));

    m_flags = FlagArray(m_df,m_address+m_mem->plant_offset("flags"));
    if(m_flags.has_flag(P_SPRING) || m_flags.has_flag(P_SUMMER) || m_flags.has_flag(P_AUTUMN) || m_flags.has_flag(P_WINTER)){
        m_flags.set_flag(P_CROP,true);
    }
    if(m_flags.has_flag(P_EXTRACT_BARREL) || m_flags.has_flag(P_EXTRACT_STILL_VIAL) || m_flags.has_flag(P_EXTRACT_VIAL) || m_flags.has_flag(P_THREAD)){
        m_flags.set_flag(P_HAS_EXTRACTS,true);
    }
}

void Plant::load_materials(){
    if(!m_df)
        return;
    QVector<VIRTADDR> mats = m_df->enumerate_vector(m_address + m_mem->plant_offset("materials_vector"));
    int i = 0;
    foreach(VIRTADDR mat, mats){
        m_plant_mats.append(Material::get_material(m_df,mat,i,false,this));
        i++;
    }
}

QVector<Material*> Plant::get_plant_materials(){
    if(m_plant_mats.empty())
        load_materials();

    return m_plant_mats;
}

Material * Plant::get_plant_material(int index){
    if(m_plant_mats.empty())
        load_materials();

    if(index < m_plant_mats.count())
        return m_plant_mats.at(index);
    else
        return new Material(this);
}

int Plant::material_count(){
    if(m_plant_mats.empty())
        load_materials();
    return m_plant_mats.count();
}
