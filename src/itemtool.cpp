#include "itemtool.h"
#include "itemtoolsubtype.h"

ItemTool::ItemTool(const Item &baseItem)
    : Item(baseItem)
    , m_tool_def(0)
{
    read_def();
}

ItemTool::ItemTool(DFInstance *df, VIRTADDR item_addr)
    : Item(df,item_addr)
    , m_tool_def(0)
{
    read_def();
}

ItemTool::~ItemTool()
{
    delete m_tool_def;
}

short ItemTool::item_subtype() const {
    return m_tool_def->subType();
}

ItemSubtype * ItemTool::get_subType(){
    return m_tool_def;
}

void ItemTool::read_def(){
    if(m_addr){
        m_tool_def = new ItemToolSubtype(m_df, m_df->read_addr(m_addr + m_df->memory_layout()->item_offset("item_def")), this);
    }
}

