#ifndef BODYPARTLAYER_H
#define BODYPARTLAYER_H

#include "truncatingfilelogger.h"
#include "flagarray.h"
#include "races.h"
#include "global_enums.h"
#include "memorylayout.h"
#include "dfinstance.h"

class BodyPartLayer{

public:

    BodyPartLayer()
        : m_addr(0x0)
        , m_df(0)
        , m_race(0x0)
        , m_layer_id(-1)
        , m_layer_name("??")
        , m_tissue_name("??")
        , m_tissue_id(-1)
        , m_t_type(eHealth::TT_OTHER)
        , m_global_layer_id(-1)
    {
    }

    BodyPartLayer(VIRTADDR layer_addr, int id, DFInstance *df, Race *r)
        : m_addr(layer_addr)
        , m_df(df)
        , m_race(r)
        , m_layer_id(id)
        , m_t_type(eHealth::TT_OTHER)
    {

        MemoryLayout *mem = m_df->memory_layout();

        m_layer_name = m_df->read_string(m_addr); //layer within the body part, fat, skin, muscle, bone etc.
        if(!m_layer_name.isEmpty())
            m_layer_name = capitalize(m_layer_name.toLower());

        m_global_layer_id = m_df->read_int(m_addr + mem->health_offset("layer_global_id"));

        //get the tissue this layer is made from. this id is an index reference in the race's tissue list
        m_tissue_id = m_df->read_int(m_addr + mem->health_offset("layer_tissue"));

        if(m_race){
            VIRTADDR t_addr = m_race->get_tissue_address(m_tissue_id);
            if(t_addr){
                m_tissue_name = m_df->read_string(t_addr + mem->health_offset("tissue_name"));
                m_tissue_flags = FlagArray(m_df,t_addr+mem->health_offset("tissue_flags"));

//                Material *m = m_df->find_material(m_df->read_int(t_addr+0xb4), m_df->read_short(t_addr+0xb0));
//                short state_id = m_df->read_short(t_addr + 0xf0);
//                if(state_id < 0)
//                    state_id = 0;
//                MATERIAL_STATES ms = static_cast<MATERIAL_STATES>(state_id);
//                QString name = m->get_material_name(ms);

                //check for tissue types we need
                //structural and tissue_anchor(19)? settable(20)?
                if(m_tissue_flags.has_flag(4) && m_tissue_flags.has_flag(20)){
                    m_t_type = eHealth::TT_BONE;
                }else if(m_tissue_flags.has_flag(3) && m_tissue_flags.has_flag(14)){ //scars and connects
                    if(m_tissue_flags.has_flag(7) && m_tissue_flags.has_flag(0)) //muscle and thickens
                        m_t_type = eHealth::TT_MUSCLE;
                    else if(m_tissue_flags.has_flag(1)) //thickens on disuse
                        m_t_type = eHealth::TT_FAT;
                    else
                        m_t_type = eHealth::TT_SKIN;
                }
            }
        }
    }

    virtual ~BodyPartLayer(){
       m_df = 0;
       m_race = 0;
    }

    int id(){return m_layer_id;}
    QString name(){return m_layer_name;}

    QString tissue_name() {return m_tissue_name;}
    eHealth::TISSUE_TYPE tissue_type(){return m_t_type;}

    int tissue_id(){return m_tissue_id;}
    int global_layer_id() {return m_global_layer_id;}

private:
    VIRTADDR m_addr;
    DFInstance *m_df;
    Race *m_race;
    int m_layer_id;
    QString m_layer_name;
    QString m_tissue_name;

    int m_tissue_id;
    FlagArray m_tissue_flags;
    eHealth::TISSUE_TYPE m_t_type;

    int m_global_layer_id;
};

#endif // BODYPARTLAYER_H
