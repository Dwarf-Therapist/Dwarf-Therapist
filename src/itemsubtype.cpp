#include "itemsubtype.h"

#include "item.h"

ItemSubtype::ItemSubtype(ITEM_TYPE itype, DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_address(address)
    , m_df(df)
    , m_mem(df->memory_layout())
    , m_iType(itype)
    , m_subType(-1)
{
    m_offset_adj = -1;
    m_offset_mat = -1;
    m_offset_preplural = -1;
    switch (itype) {
    case ARMOR:
    case PANTS:
        //armor can also have a preplural (eg. suits of leather armor) but it's unused for things like preferences
        m_offset_adj = m_mem->armor_subtype_offset("armor_adjective");
        m_offset_mat = m_mem->armor_subtype_offset("mat_name");
        break;
    case GLOVES:
    case AMMO:
    case WEAPON:
    case TRAPCOMP:
    case SHOES:
    case SHIELD:
    case HELM:
        m_offset_adj = m_mem->item_subtype_offset("adjective");
        break;
    case TOOL:
        m_offset_adj = m_mem->item_subtype_offset("tool_adjective");
        break;
    default:
        break;
    }

    set_item_type_flags(m_flags, itype);

    if(m_address){
        m_subType = m_df->read_short(m_mem->item_subtype_field(m_address, "sub_type"));

        QString mat_name;

        if(m_offset_mat != -1)
            mat_name = m_df->read_string(m_address + m_offset_mat);

        QStringList name_parts;
        if(m_offset_adj != -1)
            name_parts.append(m_df->read_string(m_address + m_offset_adj));
        name_parts.append(mat_name);
        name_parts.append(m_df->read_string(m_mem->item_subtype_field(m_address, "name")));
        m_name = capitalizeEach(name_parts.join(" ")).simplified().trimmed();

        name_parts.removeLast();
        name_parts.append(m_df->read_string(m_mem->item_subtype_field(m_address, "name_plural")));
        m_name_plural = capitalizeEach(name_parts.join(" ")).simplified().trimmed();
    }
}

ItemSubtype::~ItemSubtype() {
}

FlagArray ItemSubtype::item_type_flags(ITEM_TYPE type)
{
    FlagArray flags;
    set_item_type_flags(flags, type);
    return flags;
}

void ItemSubtype::set_item_type_flags(FlagArray &flags, ITEM_TYPE type)
{
    if (Item::is_trade_good(type))
        flags.set_flag(ITEM_TYPE_IS_TRADE_GOOD, true);
    if (Item::is_supplies(type)) {
        flags.set_flag(ITEM_TYPE_IS_EQUIPMENT, true);
        flags.set_flag(ITEM_TYPE_IS_SUPPLIES, true);
    }
    if (Item::is_melee_equipment(type)) {
        flags.set_flag(ITEM_TYPE_IS_EQUIPMENT, true);
        flags.set_flag(ITEM_TYPE_IS_MELEE_EQUIPMENT, true);
    }
    if (Item::is_ranged_equipment(type)) {
        flags.set_flag(ITEM_TYPE_IS_EQUIPMENT, true);
        flags.set_flag(ITEM_TYPE_IS_RANGED_EQUIPMENT, true);
    }
}
