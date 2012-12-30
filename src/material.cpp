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

Material::Material(QObject *parent)
    : QObject(parent)
    , m_index(-1)
    , m_address(0x0)
    , m_df(0x0)
    , m_mem(0x0)    
    , m_inorganic(false)
{
}

Material::Material(DFInstance *df, VIRTADDR address, int index, bool inorganic, QObject *parent)
    : QObject(parent)
    , m_index(index)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_inorganic(inorganic)
{
    load_data();
}

Material::~Material() {
    delete(m_flags);
    m_flags = 0;

    m_state_names.clear();
}

Material* Material::get_material(DFInstance *df, const VIRTADDR & address, int index, bool inorganic) {
    return new Material(df, address, index, inorganic);
}

void Material::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        LOGW << "load of materials called but we're not connected";
        return;
    }
    // make sure our reference is up to date to the active memory layout
    m_mem = m_df->memory_layout();

    //if passed in an inorganic material, we have to offset to the material.common first
    if(m_inorganic)
        m_address += m_mem->material_offset("inorganic_materials_vector");

    m_flag_address = m_address + m_mem->material_offset("flags");


    TRACE << "Starting refresh of material data at" << hexify(m_address);

    read_material();
}

void Material::read_material() {
    if(!m_df)
        return;

    //read material names
    //m_state_names.insert(GENERIC,m_df->read_string(m_address)); //the id name, but can be things like 'wood'
    m_state_names.insert(SOLID,m_df->read_string(m_address + m_mem->material_offset("solid_name")));
    m_state_names.insert(LIQUID,m_df->read_string(m_address + m_mem->material_offset("liquid_name")));
    m_state_names.insert(GAS,m_df->read_string(m_address + m_mem->material_offset("gas_name")));
    m_state_names.insert(POWDER,m_df->read_string(m_address + m_mem->material_offset("powder_name")));
    m_state_names.insert(PASTE,m_df->read_string(m_address + m_mem->material_offset("paste_name")));
    m_state_names.insert(PRESSED, m_df->read_string(m_address + m_mem->material_offset("pressed_name")));

  /*
    QString m_generic = m_df->read_string(m_address);
    QString m_solid = m_df->read_string(m_address + m_mem->material_offset("solid_name"));
    QString m_liquid = m_df->read_string(m_address + m_mem->material_offset("liquid_name"));
    QString m_gas = m_df->read_string(m_address + m_mem->material_offset("gas_name"));
    QString m_powder = m_df->read_string(m_address + m_mem->material_offset("powder_name"));
    QString m_paste = m_df->read_string(m_address + m_mem->material_offset("paste_name"));
    QString m_pressed = m_df->read_string(m_address + m_mem->material_offset("pressed_name"));

    QString m_adj_solid = m_df->read_string(m_address + 0x014c);
    QString m_adj_liquid = m_df->read_string(m_address + 0x0168);
    QString m_adj_gas = m_df->read_string(m_address + 0x0184);
    QString m_adj_powder = m_df->read_string(m_address + 0x1a0);
    QString m_adj_paste = m_df->read_string(m_address + 0x1bc);
    QString m_adj_pressed = m_df->read_string(m_address + 0x1d8);


    QString m_prefix = m_df->read_string(m_address + 0x3dc);
    QString m_stone = m_df->read_string(m_address + 0x54);
    QString m_gem = m_df->read_string(m_address + 0x1c);
    QString m_gem2 = m_df->read_string(m_address + 0x38);


*/


    QString generic_state_name = m_df->read_string(m_address);
//    foreach(VIRTADDR temp, templates){
//        QString mat_generic = m_df->read_string(m_address);
//        if(!mat_generic.isEmpty()){
//            mat_generic += "_TEMPLATE";
//            if(mat_generic == m_df->read_string(temp)){
//                generic_state_name = m_df->read_string(temp + m_mem->material_offset("solid_name"));
//                break;
//            }
//        }
//    }
    if(!generic_state_name.isEmpty()){
        VIRTADDR template_addr = m_df->get_material_template(generic_state_name + "_TEMPLATE");
        if(template_addr){
            QString template_state = m_df->read_string(template_addr + m_mem->material_offset("solid_name"));
            if(template_state != generic_state_name)
                generic_state_name = template_state;
        }
    }

    if(generic_state_name.toLower() == "thread"){
        generic_state_name = m_state_names[SOLID] + tr(" fabric");
    }

    //int value = m_df->read_int(m_address + 0x244);

    m_state_names.insert(GENERIC, generic_state_name);

    m_flags = new FlagArray(m_df, m_flag_address);

//    //read the material flags
//    //get the array from the pointer
//    VIRTADDR flags_addr = m_df->read_addr(m_flag_address);
//    //size of the byte array
//    quint32 size_in_bytes = m_df->read_addr(m_flag_address + 0x4);

//    m_flags = new QBitArray(size_in_bytes * 8);

//    BYTE b;
//    int position;
//    for(int i = 0; i < size_in_bytes; i++){
//        position = 7;
//        b = m_df->read_byte(flags_addr);
//        if(b > 0){
//            for(int t=128; t>0; t = t/2){
//                if(b & t)
//                    m_flags->setBit(i*8 + position, true);
//                position--;
//            }
//        }
//        flags_addr += 0x1;
//    }


//    if(m_solid.toLower().indexOf("dye") > 0 || has_flag(POWDER_MISC_PLANT) || has_flag(CHEESE_PLANT)){
//        QString val;
//        for(int i =0; i < m_flags->size(); i++){
//            val.append(m_flags->at(i) ? "1," : "0,");
//        }
//        val.chop(1);
//        LOGW << m_solid << "," << val;
//    }
}

//bool Material::has_flag(MATERIAL_FLAGS f){
//    if((int)f < m_flags->count()){
//        return m_flags->at(f);
//    }else{
//        return false;
//    }
//}

QString Material::get_material_name(MATERIAL_STATES state){
    if(m_state_names.contains(state))
        return m_state_names.value(state);
    else
        return "Unknown";
}

