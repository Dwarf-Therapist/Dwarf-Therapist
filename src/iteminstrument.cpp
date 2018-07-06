#include "iteminstrument.h"
#include "itemsubtype.h"


ItemInstrument::ItemInstrument(const Item &baseItem)
    : Item(baseItem)
    , m_Instrument_def(0)
{
    read_def();
}

ItemInstrument::ItemInstrument(DFInstance *df, VIRTADDR item_addr)
    : Item(df,item_addr)
    , m_Instrument_def(0)
{
    read_def();
}

ItemInstrument::~ItemInstrument()
{
    delete m_Instrument_def;
}

short ItemInstrument::item_subtype() const {
    return m_Instrument_def->subType();
}

ItemSubtype * ItemInstrument::get_subType(){
    return m_Instrument_def;
}

void ItemInstrument::read_def(){
    if(m_addr){
        m_Instrument_def = new ItemSubtype(m_iType,m_df, m_df->read_addr(m_df->memory_layout()->item_field(m_addr, "item_def")), this);
    }
}
