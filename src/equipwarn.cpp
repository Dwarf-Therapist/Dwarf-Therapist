#include "equipwarn.h"
#include "dwarf.h"

EquipWarn::EquipWarn(QObject *parent)
    : QObject(parent)
    , m_total_count(0)
{
}

void EquipWarn::add_detail(Dwarf *d, warn_info wi){
    if(m_details.contains(wi.key)){
        m_details[wi.key].count += wi.count;
        m_details[wi.key].unit_ids.insert(d->nice_name(),d->id());
    }else{
        warn_count wc;
        wc.count = wi.count;
        wc.unit_ids.insert(d->nice_name(),d->id());
        m_details.insert(wi.key,wc);
    }

    if(!m_wear_counts.contains(wi.key.second)){
        m_wear_counts[wi.key.second] = wi.count;
    }else{
        m_wear_counts[wi.key.second] += wi.count;
    }
    m_total_count += wi.count;
}
