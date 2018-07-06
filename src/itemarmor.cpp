#include "itemarmor.h"
#include "races.h"

void ItemArmor::read_def() {
    if(m_addr){
        m_armor_def = new ItemArmorSubtype(m_iType,m_df,m_df->read_addr(m_df->memory_layout()->item_field(m_addr, "item_def")),this);
        if(m_armor_def){

            QString layer_name = m_armor_def->get_layer_name();
            if(layer_name != ""){
                m_layer_name = layer_name;
            }

            if(m_armor_def->armor_flags().has_flag(ItemArmorSubtype::ARMOR_SHAPED)){
                m_layer_name.append(tr("[S]"));
            }
        }
        if(m_maker_race >= 0 && m_maker_race != m_df->dwarf_race_id()){
            Race *r_maker = m_df->get_race(m_maker_race);
            if(r_maker){
                int item_size = r_maker->adult_size();
                int our_size = item_size;
                Race *r_fort = m_df->get_race(m_df->dwarf_race_id());
                if(r_fort){
                    our_size = r_fort->adult_size();
                }
                if(item_size > our_size){
                    m_size_prefix = tr("Large ");
                }else if(item_size < our_size){
                    m_size_prefix = tr("Small ");
                }
            }
        }
    }
}
