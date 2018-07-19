/*
 *
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
#include "unitwound.h"

#include "bodypart.h"
#include "bodypartdamage.h"
#include "bodypartlayer.h"
#include "dfinstance.h"
#include "flagarray.h"
#include "healthcategory.h"
#include "memorylayout.h"
#include "unithealth.h"

class HealthInfo;

UnitWound::UnitWound()
    : m_df(0x0)
    , m_addr(0x0)
    , m_unitHealth(0x0)
    , m_caste_flags()
    , m_severed(false)
    , m_mortal(false)
    , m_stuck(false)
    , m_diagnosed(false)
    , m_sutured(false)
    , m_infection(false)
    , m_is_critical(false)
{
}
UnitWound::UnitWound(DFInstance *df, VIRTADDR base_addr, FlagArray caste_flags, UnitHealth *uh)
    : m_df(df)
    , m_addr(base_addr)
    , m_unitHealth(uh)
    , m_caste_flags(caste_flags)
    , m_severed(false)
    , m_mortal(false)
    , m_stuck(false)
    , m_diagnosed(false)
    , m_sutured(false)
    , m_infection(false)
    , m_is_critical(false)
{
    read_wound();
}
UnitWound::UnitWound(DFInstance *df, int body_part_id, UnitHealth *uh)
    : m_df(df)
    , m_addr(0x0)
    , m_unitHealth(uh)
    , m_caste_flags()
    , m_severed(false)
    , m_mortal(false)
    , m_stuck(false)
    , m_diagnosed(false)
    , m_sutured(false)
    , m_infection(false)
    , m_is_critical(false)
{
    BodyPartDamage bp = m_unitHealth->get_body_part(body_part_id);
    BodyPartDamage parent;
    if(bp.body_part()->parent() > 0){
        parent = m_unitHealth->get_body_part(bp.body_part()->parent());
        if(parent.is_missing())
            return;
    }

    wounded_part_details wpd;
    wpd.body_part_id = body_part_id;
    wpd.body_part_name = bp.body_part()->name();

    //check for old wound flags?
    if(bp.is_missing()){
        add_detail(wpd,eHealth::HI_SEVERED, false, true);
    }else{
        add_detail(wpd,eHealth::HI_NERVE, bp.old_motor_nerve_dmg(),bp.old_sensory_nerve_dmg());
    }

    wpd.diagnosed = true;
    wpd.is_old_wound = true;
    wpd.visible = true;
    if(wpd.wnd_info.size() > 0)
        m_wounded_parts.append(wpd);
}

UnitWound::~UnitWound(){
    m_bp_info.clear();
    m_df = 0;
    m_unitHealth = 0;
}

void UnitWound::read_wound(){
    MemoryLayout *mem = m_df->memory_layout();

    QList<short> desc_index;
    quint32 general_flags = m_df->read_addr(mem->wound_field(m_addr, "general_flags"));

    if(general_flags & 1)
        m_severed = true;
    if(general_flags & (1 << 1))
        m_mortal = true;
    if(general_flags & (1 << 2))
        m_stuck = true;
    if(general_flags & (1 << 3))
        m_diagnosed = true;
    if(general_flags & (1 << 4))
        m_sutured = true;
    if(m_caste_flags.has_flag(GETS_WOUND_INFECTIONS) && general_flags & (1 << 5))
        m_infection = true;

    m_is_critical = m_severed || m_mortal;

    QVector<VIRTADDR> addr_wounded_parts = m_df->enumerate_vector(mem->wound_field(m_addr, "parts"));

    foreach(VIRTADDR wounded_part, addr_wounded_parts){
        wounded_part_details wpd;

        wpd.body_part_id = m_df->read_short(mem->wound_field(wounded_part, "id"));
        short layer_id = m_df->read_short(mem->wound_field(wounded_part, "layer"));

        wpd.wound_flags1 = m_df->read_addr(mem->wound_field(wounded_part, "flags1"));
        wpd.wound_flags2 = m_df->read_addr(mem->wound_field(wounded_part, "flags2"));

        VIRTADDR addr_effect = mem->wound_field(wounded_part, "effects_vector");
        wpd.effect_types = m_df->enumerate_vector_short(addr_effect);

        wpd.cur_pen = m_df->read_short(mem->wound_field(wounded_part, "cur_pen"));
        wpd.pen_max = m_df->read_short(mem->wound_field(wounded_part, "max_pen"));

        BodyPartDamage bp = m_unitHealth->get_body_part(wpd.body_part_id);

        wpd.body_part_name = bp.body_part()->name();
        wpd.layer_name = bp.body_part()->get_layer(layer_id).name();

        wpd.diagnosed = wpd.wound_flags1 & 0x20000000;
        wpd.visible = false;

        if(wpd.wound_flags1 & 0x04820004)
            wpd.is_scarred = true;
        else
            wpd.is_scarred = false;

        if(m_severed){
            add_detail(wpd,eHealth::HI_SEVERED,true);
        }else{
            if(m_infection){
                add_detail(wpd,eHealth::HI_INFECTION,true);
            }

            // wpd.strained_amount = m_df->read_int(wounded_part + 0x10);
            //        if(wpd.strained_amount > 0)
            //            wpd.wound_details.append(WI_STRAINED);

            if(wpd.cur_pen > 0){
                //if a body part is pierced, and there's fluid/gas/whatever that has the TISSUE_LEAKS tag, then underlying layers leak, or are emptied
                //this only applies to mods, and usually fortress animals/creatures at that (masterwork golems). this also still doesn't process all related layers, as they're not mapped.
                //it's only doing a check on the layers for this bodypart. any layers above/below that may have also been pierced are ignored as it's probably not THAT important
                foreach(BodyPartLayer bpl, bp.body_part()->get_layers()){
                    if(m_unitHealth->layer_status_flags.at(bpl.global_layer_id()) & 1){
                        //gone
                        wounded_part_details fluid_gone;
                        fluid_gone.body_part_name = QString("%1's %2").arg(bp.body_part()->name(),bpl.tissue_name());
                        add_detail(fluid_gone,eHealth::HI_SEVERED,false,true);
                        m_wounded_parts.append(fluid_gone);
                    }
                }

                if(bp.body_part()->get_layer(layer_id).tissue_type() == eHealth::TT_BONE){
                    //bone damage/loss is different in that it's usually a smashed type, but reports as broken or fractured
                    desc_index.clear();
                    if(!m_unitHealth->required_diagnosis() || (m_unitHealth->required_diagnosis() && wpd.diagnosed)){
                        if(wpd.wound_flags1 & 0x80000000)
                            desc_index.append(0); //compound fracture
                        if(wpd.wound_flags1 & 0x10000000)
                            desc_index.append(1); //overlapping fracture
                    }

                    if(bp.has_bone_dmg() && wpd.cur_pen > 0){
                        if(wpd.cur_pen == wpd.pen_max)
                            desc_index.append(2); //broken
                        else
                            desc_index.append(3); //fractured
                    }
                    if(desc_index.size() > 0){
                        wpd.wnd_info.insert(eHealth::HI_FRACTURE,desc_index);
                        wpd.visible = true;
                        m_is_critical = true;
                    }
                }else{ //other tissue (fat, skin, muscle) damage seems to have the same descriptors

                    //suturing or bandaging a wound repairs lacerations...or does it
                    //it might be exclusively related to whether or not the wound has completely healed.. (ie. cur_pen)
                    //                if((muscle_dmg || skin_dmg) && wpd.cur_pen > 0
                    //                   && !m_sutured && !wpd.is_scarred
                    //                   && !m_unitHealth->bp_statuses.at(wpd.body_part_id & 0x00002000) //bandaged
                    //                && !m_unitHealth->bp_statuses.at(wpd.body_part_id & 0x00004000) //cast
                    //                && !m_unitHealth->bp_statuses.at(wpd.body_part_id & 0x00001000) //splint
                    //                ){
                    short idx = -1;
                    desc_index.clear();
                    if(wpd.wound_flags1 & 0x2) //smashed
                        idx = 0;
                    else if(wpd.wound_flags1 & 0x4) //torn
                        idx = 2;
                    else if(wpd.wound_flags1 & 0x1) //cut
                        idx = 4;
                    else if(wpd.wound_flags1 & 0x80000) //broken
                        idx = 6;
                    else if(wpd.wound_flags1 & 0x200000) //gouged
                        idx = 8;

                    //cur_pen < max = cut/torn/smashed OPEN, else cut/torn/smashed APART
                    if(wpd.cur_pen < wpd.pen_max)
                        idx++;
                    if(idx >= 0){
                        desc_index.append(idx);
                        wpd.wnd_info.insert(eHealth::HI_LACERATION,desc_index);
                        wpd.visible = true;
                    }
                }
            }

            //if((wound_flags1 & 0x4) == 0x4)
            //    wnd_desc.append("Huge Dent");

            add_detail(wpd,eHealth::HI_TENDON,
                       wpd.wound_flags1 & 0x00000040,wpd.wound_flags1 & 0x00000020,wpd.wound_flags1 & 0x00000010);

            add_detail(wpd,eHealth::HI_LIGAMENT,
                       wpd.wound_flags1 & 0x00000200,wpd.wound_flags1 & 0x00000100,wpd.wound_flags1 & 0x00000080);

            add_detail(wpd,eHealth::HI_NERVE,
                       wpd.wound_flags1 & 0x00000400,wpd.wound_flags1 & 0x00000800);

            if(!m_sutured){//(!wpd.is_scarred){
                add_detail(wpd,eHealth::HI_ARTERY, wpd.wound_flags1 & 0x00004000,wpd.wound_flags1 & 0x40000000);
             }

            add_detail(wpd,eHealth::HI_GUTTED, wpd.wound_flags1 & 0x00008000);

            add_detail(wpd,eHealth::HI_SETTING, wpd.wound_flags2 & 0x00000001);
            add_detail(wpd,eHealth::HI_GELDED, wpd.wound_flags2 & 0x00000004);

            if(!m_caste_flags.has_flag(NO_PAIN)){
                wpd.pain = m_df->read_int(mem->wound_field(wounded_part, "pain"));
                add_detail(wpd,eHealth::HI_PAIN, (wpd.pain > 50),(wpd.pain > 25),(wpd.pain > 0));
            }

            desc_index.clear();
            foreach(short eType, wpd.effect_types){
                //for these, the index needs to match with the severity
                switch(eType){
                case 0: //bruised
                    desc_index.append(6);
                    break;
                case 3: //burnt
                    desc_index.append(1);
                    break;
                case 2: //frostbite
                    desc_index.append(4);
                    break;
                case 4: //melted
                    desc_index.append(0);
                    m_is_critical = true;
                    break;
                case 6: //frozen
                    desc_index.append(5);
                    break;
                case 8: //necrotic
                {
                    //desc_index.append(3);
                    //it seems wounds that become necrotic are shown as rotten, but not inoperable rot
                    //so we'll put it in with the rot category for now
                    add_detail(wpd,eHealth::HI_ROT,false,true);
                }
                    break;
                case 9: //blistered
                    desc_index.append(2);
                    break;
                }

            }
            if(desc_index.size() > 0){
                wpd.wnd_info.insert(eHealth::HI_OTHER,desc_index);
                wpd.visible = true;
            }

            add_detail(wpd,eHealth::HI_ROT,bp.has_rot());

            /* individual body part values
            wpd.nauseous = m_df->read_int(wounded_part + 0x58);
            wpd.dizziness = m_df->read_int(wounded_part + 0x5c);
            wpd.paralysis = m_df->read_int(wounded_part + 0x60);
            wpd.numbness = m_df->read_int(wounded_part + 0x64);
            wpd.swelling = m_df->read_int(wounded_part + 0x68);
            wpd.impaired = m_df->read_int(wounded_part + 0x6c);
            */

            //            int jammed_fracture_layer = m_df->read_int(wounded_part + 0x74);
            //            if(jammed_fracture_layer > 0)

        }

        //apparently severed parts can still bleed...
        wpd.bleeding = m_df->read_int(mem->wound_field(wounded_part, "bleeding"));
        if(wpd.bleeding){
            add_detail(wpd,eHealth::HI_BLEEDING,(wpd.bleeding >= 3),(wpd.bleeding < 3));
            m_is_critical = true;
        }

        m_wounded_parts.append(wpd);
    }
}

void UnitWound::add_detail(wounded_part_details &wpd, eHealth::H_INFO id, bool idx0, bool idx1, bool idx2){
    //the boolean values correspond to display indexes within the category/group
    //so for example for bleeding, idx0 = heavy bleeding, and idx1 = bleeding
    if(!idx0 && !idx1 && !idx2)
        return;

    if(((m_unitHealth->required_diagnosis() && wpd.diagnosed)|| !m_unitHealth->required_diagnosis()) || id == eHealth::HI_SEVERED) {
        QList<short> desc_index;
        if(m_unitHealth->get_display_categories().value(id)->allows_multiple()){
            if(idx0)
                desc_index.append(0);
            if(idx1)
                desc_index.append(1);
            if(idx2)
                desc_index.append(2);
        }else{
            if(idx0)
                desc_index.append(0);
            else if(idx1)
                desc_index.append(1);
            else if(idx2)
                desc_index.append(2);
        }
        if(desc_index.size() > 0){
            wpd.visible = true;
            wpd.wnd_info.insert(id,desc_index);
        }
    }
}

QHash<QString,QList<HealthInfo*> > UnitWound::get_wound_details()
{
    //go through all wounded parts, and group everything into a new structure by bodypart
    //additionally, add the wound info to the unit's status, treatment and wounds section where/when applicable
    if(m_bp_info.size() <= 0 && m_wounded_parts.size() > 0){
        foreach(wounded_part_details wpd, m_wounded_parts){
            QList<HealthInfo*> body_infos = m_bp_info.take(wpd.body_part_name);
            foreach(eHealth::H_INFO hs, wpd.wnd_info.uniqueKeys()){
                QList<short> desc_indexes = wpd.wnd_info.value(hs);
                bool visible = wpd.visible;
                m_unitHealth->add_info(hs, desc_indexes,visible); //add to the top level statuses, treatments and wounds
                if(visible){ //only if visible (ie. diagnosed +- option)
                    m_unitHealth->add_info(hs,desc_indexes, body_infos);
                }
            }
            if(body_infos.size() > 0)
                m_bp_info.insert(wpd.body_part_name,body_infos);
        }
    }
    return m_bp_info;
}
