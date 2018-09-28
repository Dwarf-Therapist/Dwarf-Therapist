#include "itemsubtype.h"

#include "item.h"

ItemSubtype::ItemSubtype(ITEM_TYPE itype, QObject *parent)
    : QObject(parent)
    , m_iType(itype)
    , m_subType(-1)
{
    set_item_type_flags(m_flags, itype);
}

ItemSubtype::ItemSubtype(ITEM_TYPE itype, DFInstance *df, VIRTADDR address, QObject *parent)
    : QObject(parent)
    , m_iType(itype)
    , m_subType(-1)
    , m_flags(df, df->memory_layout()->item_subtype_field(address, "base_flags"))
{
    auto mem = df->memory_layout();
    int offset_adj = -1;
    int offset_mat = -1;
    switch (itype) {
    case ARMOR:
    case PANTS:
        //armor can also have a preplural (eg. suits of leather armor) but it's unused for things like preferences
        offset_adj = mem->armor_subtype_offset("armor_adjective");
        offset_mat = mem->armor_subtype_offset("mat_name");
        break;
    case GLOVES:
    case AMMO:
    case WEAPON:
    case TRAPCOMP:
    case SHOES:
    case SHIELD:
    case HELM:
        offset_adj = mem->item_subtype_offset("adjective");
        break;
    case TOOL:
        offset_adj = mem->item_subtype_offset("tool_adjective");
        break;
    default:
        break;
    }

    set_item_type_flags(m_flags, itype);

    if(address){
        m_subType = df->read_short(mem->item_subtype_field(address, "sub_type"));

        QString mat_name;

        if(offset_mat != -1)
            mat_name = df->read_string(address + offset_mat);

        QStringList name_parts;
        if(offset_adj != -1)
            name_parts.append(df->read_string(address + offset_adj));
        name_parts.append(mat_name);
        name_parts.append(df->read_string(mem->item_subtype_field(address, "name")));
        m_name = capitalizeEach(name_parts.join(" ")).simplified().trimmed();

        name_parts.removeLast();
        name_parts.append(df->read_string(mem->item_subtype_field(address, "name_plural")));
        m_name_plural = capitalizeEach(name_parts.join(" ")).simplified().trimmed();
    }
    else {
        LOGE << "Null address for ItemSubtype";
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
