#include "itemsubtype.h"

void ItemSubtype::read_data() {
    if(m_address){
        m_subType = m_df->read_short(m_address + m_mem->item_subtype_offset("sub_type"));

        QString mat_name;

        if(m_offset_mat != -1)
            mat_name = m_df->read_string(m_address + m_offset_mat);

        QStringList name_parts;
        if(m_offset_adj != -1)
            name_parts.append(m_df->read_string(m_address + m_offset_adj));
        name_parts.append(mat_name);
        name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name")));
        m_name = capitalizeEach(name_parts.join(" ")).simplified().trimmed();

        name_parts.removeLast();
        name_parts.append(m_df->read_string(m_address + m_mem->item_subtype_offset("name_plural")));
        m_name_plural = capitalizeEach(name_parts.join(" ")).simplified().trimmed();
    }
}
