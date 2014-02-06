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

#include "uniform.h"

Squad::Squad(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    refresh_data();
}

Squad::~Squad() {
    m_df = 0;
    m_uniforms.clear();
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

    //qDeleteAll(m_members);
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
    m_name = m_df->get_translated_word(m_address + m_mem->squad_offset("name"));
    QString alias = m_df->read_string(m_address + m_mem->squad_offset("alias"));
    if(alias != "")
        m_name = alias;
    TRACE << "Name:" << m_name;
}

void Squad::read_members() {
    members_addr = m_df->enumerate_vector(m_address + m_mem->squad_offset("members"));

    short carry_food = m_df->read_short(m_address+m_mem->squad_offset("carry_food"));
    short carry_water = m_df->read_short(m_address+m_mem->squad_offset("carry_water"));
    int carry_ammo = m_df->enumerate_vector(m_address+m_mem->squad_offset("ammunition")).count();

    //read the uniforms
    int position = 0;    
    Uniform *u;
    foreach(VIRTADDR addr, members_addr){
        u = new Uniform(m_df,this);

        m_members.append(m_df->read_int(addr));        
        read_equip_category(addr+m_mem->squad_offset("armor_vector"),ARMOR,u);
        read_equip_category(addr+m_mem->squad_offset("helm_vector"),HELM,u);
        read_equip_category(addr+m_mem->squad_offset("pants_vector"),PANTS,u);
        read_equip_category(addr+m_mem->squad_offset("gloves_vector"),GLOVES,u);
        read_equip_category(addr+m_mem->squad_offset("shoes_vector"),SHOES,u);
        read_equip_category(addr+m_mem->squad_offset("shield_vector"),SHIELD,u);
        read_equip_category(addr+m_mem->squad_offset("weapon_vector"),WEAPON,u);

        //add other items
        if(carry_ammo){
            u->add_uniform_item(addr+m_mem->squad_offset("quiver"),QUIVER);
            u->add_uniform_item(AMMO,-1,-1);
        }
        if(carry_food)
            u->add_uniform_item(addr+m_mem->squad_offset("backpack"),BACKPACK);
        if(carry_water)
            u->add_uniform_item(addr+m_mem->squad_offset("flask"),FLASK);

        m_uniforms.insert(position,u);
        position++;
    }
}

void Squad::read_equip_category(VIRTADDR vec_addr, ITEM_TYPE itype, Uniform *u){
    QVector<VIRTADDR> uniform_items = m_df->enumerate_vector(vec_addr);
    foreach(VIRTADDR uItem_addr, uniform_items){
        u->add_uniform_item(uItem_addr,itype,-1); //don't count the items yet
    }
    u->add_equip_count(itype,uniform_items.count());
}

int Squad::assigned_count(){
    //use the addresses to determine the assigned dwarfs because
    //if a dwarf dies in DF but isn't found, they're still assigned to the squad until found
    //so simply counting the m_members isn't sufficient
    int count = 0;
    foreach(VIRTADDR member_addr, members_addr) {
        if(m_df->read_int(member_addr) != -1)
            count++;
    }
    return count;
}

int Squad::assign_to_squad(Dwarf *d){
    int assigned = assigned_count();

    //users could potentially select more than 10 and assign to squad
    if(assigned==10)
        return -1;

    d->set_squad_id(m_id);
    d->set_squad_name(name());
    d->recheck_equipment();

    //add the dwarf to our dt vector for grouping etc
    //    m_members << d;
    m_members.append(d->id());
    //add the dwarf's hist id to the first available position in the squad
    int position = -1; //positions start at 0
    foreach(VIRTADDR member_addr, members_addr) {
        position += 1;
        if(m_df->read_int(member_addr) == -1){
            m_df->write_int(member_addr,d->historical_id());
            break;
        }
    }
    //save the position after looping through the mem addresses in the case of missing/dead dwarves still in the squad
    d->set_squad_position(position);
    //update the memory values immediately
    m_df->write_int(d->address() + m_df->memory_layout()->dwarf_offset("squad_id"), m_id);
    m_df->write_int(d->address() + m_df->memory_layout()->dwarf_offset("squad_position"), position);

    return position;
}

void Squad::remove_from_squad(Dwarf *d){
    m_df->write_int(d->address() + m_df->memory_layout()->dwarf_offset("squad_id"),-1);
    m_df->write_int(d->address() + m_df->memory_layout()->dwarf_offset("squad_position"),-1);

    foreach(VIRTADDR member_addr, members_addr) {
        if(m_df->read_int(member_addr) == d->historical_id()){
            m_df->write_int(member_addr,-1);
            break;
        }
    }
    for(int i = 0; i < members().count()-1; i++){
        if(members().at(i)==d->id()){//d){
            members().remove(i);
            break;
        }
    }

    d->set_squad_id(-1);
    d->set_squad_position(-1);
    d->set_squad_name("");
    d->recheck_equipment();
}

void Squad::rename_squad(QString alias){
    m_df->write_string(m_address + m_df->memory_layout()->squad_offset("alias"),alias);
}

