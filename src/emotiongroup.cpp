#include "emotiongroup.h"

void EmotionGroup::add_detail(Dwarf *d, UnitEmotion *ue) {
    EMOTION_TYPE e_type = ue->get_emotion_type();
    int total_count = ue->get_count();
    if(m_details.contains(e_type)){
        m_details[e_type].count += total_count;
        m_details[e_type].unit_ids.insert(d->nice_name(),d->id());
    }else{
        emotion_count ec;
        ec.count = total_count;
        ec.unit_ids.insert(d->nice_name(),d->id());
        m_details.insert(e_type,ec);
    }

    if(ue->get_stress_effect() > 0){
        if(!m_stress_ids.contains(d->id()))
            m_stress_ids.append(d->id());
        m_stress_count += total_count;
    }else if(ue->get_stress_effect() < 0){
        if(!m_eustress_ids.contains(d->id()))
            m_eustress_ids.append(d->id());
        m_eustress_count += total_count;
    }else{
        if(!m_unaffected_ids.contains(d->id()))
            m_unaffected_ids.append(d->id());
        m_unaffected_count += total_count;
    }
}
