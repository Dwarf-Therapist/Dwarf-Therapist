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
#include "fortressentity.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include <QtDebug>
#include "gamedatareader.h"

QColor FortressEntity::default_noble_color = QColor(255,153,0,180);

/*
 *it seems this is more of a civilization entity instead of a fortress entity.
 *it's a historical entity, but the name, for example, appears to reference the civilization, rather than the fortress
 */

FortressEntity::FortressEntity(DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
{
    load_data();
}

FortressEntity::~FortressEntity()
{
    m_noble_colors.clear();
    m_nobles.clear();
}

FortressEntity* FortressEntity::get_entity(DFInstance *df, const VIRTADDR & address) {
    return new FortressEntity(df, address);
}

void FortressEntity::load_data() {
    if (!m_df || !m_df->memory_layout() || !m_df->memory_layout()->is_valid()) {
        //not connected
        return;
    }
    //refresh mem layout
    m_mem = m_df->memory_layout();

    read_entity();
}

void FortressEntity::read_entity(){
    //read the position/noble colors
    load_noble_colors();

    m_df->attach();
    //civ name
//    m_name = m_df->get_language_word(m_address + 0x14);
//    m_translated_name = m_df->get_translated_word(m_address + 0x14);

    //load squads
    m_squads = m_df->enumerate_vector(m_address + m_mem->hist_entity_offset("squads"));

    QVector<VIRTADDR> entities = m_df->enumerate_vector(m_df->get_memory_correction() + m_mem->address("historical_entities_vector"));
    QHash<int, position> positions;
    QString unk_name = tr("Unknown");
    position pos_unk = {unk_name,unk_name,unk_name,m_noble_colors.value(MULTIPLE)};

    int assign_pos_id;
    int position_id;
    int hist_id;
    QString raw_name;

    foreach(VIRTADDR ent, entities){
        //don't bother searching in non-civilization entities,
        //however as some reports have other races as nobles this is the only filtering we can do
        //make sure to include the fortress positions as well
        if(m_df->read_int(ent) == 0 || ent == m_address){

            //load assignments and positions, mapping them to historical figure ids
            QVector<VIRTADDR> addr_positions = m_df->enumerate_vector(ent + m_mem->hist_entity_offset("positions")); //positions in the fortress
            QVector<VIRTADDR> addr_assignments = m_df->enumerate_vector(ent + m_mem->hist_entity_offset("assignments")); //assignments to positions


            positions.clear();
            foreach(VIRTADDR pos, addr_positions){
                position_id = m_df->read_int(pos + m_mem->hist_entity_offset("position_id"));
                position p;
                p.name = m_df->read_string(pos + m_mem->hist_entity_offset("position_name"));
                p.name_female = m_df->read_string(pos + m_mem->hist_entity_offset("position_female_name"));
                p.name_male = m_df->read_string(pos + m_mem->hist_entity_offset("position_male_name"));
                raw_name = m_df->read_string(pos);
                p.highlight = m_noble_colors.value(get_color_type(raw_name));
                positions.insert(position_id,p);
            }

            //may be better to check all the different responsibility flags and other flags like succession/appointed etc
            //to get profiles of the different nobility types
            foreach(VIRTADDR assign, addr_assignments){
                assign_pos_id = m_df->read_int(assign + m_mem->hist_entity_offset("assign_position_id")); //position for the assignment
                hist_id = m_df->read_int(assign + m_mem->hist_entity_offset("assign_hist_id")); //dwarf assigned                
                if(hist_id > 0){                    
                    position p = positions.value(assign_pos_id, pos_unk);
                    m_nobles.insert(hist_id,p);
                }
            }
        }
    }    
    //"historical_entity","2","0x8b0","0x80","static-array","resources.values","int32_t",""
    VIRTADDR beliefs_addr = m_address + 0x8b0;
    for(int i = 0; i < GameDataReader::ptr()->get_total_belief_count();i++){
        short val = m_df->read_short(beliefs_addr + i * 4);
        if(val > 100)
            val = 100;
        m_beliefs.insert(i, val);
    }

    m_df->detach();
}

void FortressEntity::refresh_squads(){
    m_squads = m_df->enumerate_vector(m_address + m_mem->hist_entity_offset("squads"));
}

void FortressEntity::load_noble_colors(){
    QSettings *u = DT->user_settings();

    QColor c;
    u->beginGroup("options");
    u->beginGroup("colors");
    for(int i = 0; i<12; i++){
        c = u->value(QString("nobles/%1").arg(i), default_noble_color).value<QColor>();
        m_noble_colors.insert(static_cast<NOBLE_COLORS>(i),c); //i matches noble enum
    }
    u->endGroup();
    u->endGroup();
}

QString FortressEntity::get_noble_positions(int hist_id, bool is_male){
    //return a string of all the positions this dwarf holds in the fortress
    QStringList names;
    position p;
    QMultiHash<int, position>::iterator i = m_nobles.find(hist_id);
    while(i != m_nobles.end() && i.key()==hist_id){
        p = i.value();
        if(is_male && p.name_male != "")
            names.append(p.name_male);
        else if(!is_male && p.name_female != "")
            names.append(p.name_female);
        else
            names.append(p.name);
        i++;
    }
    return capitalizeEach(names.join(", "));
}

QColor FortressEntity::get_noble_color(int hist_id){
    QList<position> p = m_nobles.values(hist_id);
    if(p.size() > 1)
        return m_noble_colors.value(MULTIPLE);
    else
        return p[0].highlight;

    return default_noble_color;
}
