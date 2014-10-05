#ifndef BODYPART_H
#define BODYPART_H

#include "truncatingfilelogger.h"
#include "flagarray.h"
#include "bodypartlayer.h"
#include "races.h"
#include "memorylayout.h"

class DFInstance;

class BodyPart
{
public:

    BodyPart()
        : m_df(0x0)
        , m_race(0x0)
        , bp_addr(0x0)
        , m_body_part_id(-1)
        , m_parent_id(0)
    {
        m_token = "UNK";
        m_bp_name = "Unknown";
//        m_has_rot = false;
    }

    BodyPart(DFInstance *df, Race *r, VIRTADDR bp_addr, int bp_id)
        : m_df(df)
        , m_race(r)
        , bp_addr(bp_addr)
        , m_body_part_id(bp_id)
        , m_parent_id(0)
    {
        m_token = m_df->read_string(bp_addr);
        build_bp_name();

        m_layers_addr = m_df->enumerate_vector(bp_addr + m_df->memory_layout()->health_offset("layers_vector"));
        m_parent_id = m_df->read_short(bp_addr + m_df->memory_layout()->health_offset("parent_id"));
    }

    virtual ~BodyPart(){
       m_df = 0;
       m_race = 0;
       m_layers.clear();
    }

    BodyPartLayer get_layer(short id){
        if(id >= 0 && id < m_layers_addr.count()){
            if(!m_layers.contains(id))
                m_layers.insert(id, BodyPartLayer(m_layers_addr.at(id),id,m_df,m_race));

                return m_layers.value(id);
        }else{
            return BodyPartLayer(0,-1,m_df,m_race);
        }
    }

    QHash<int, BodyPartLayer> get_layers() {
        int idx = 0;
        foreach(VIRTADDR addr, m_layers_addr){
            if(!m_layers.contains(idx)){
                m_layers.insert(idx, BodyPartLayer(addr,idx,m_df,m_race));
            }
            idx++;
        }
        return m_layers;
    }

    QString name() {return m_bp_name;}
    int id() {return m_body_part_id;}
    QString token() {return m_token;}
    int parent() {return m_parent_id;}

private:
    DFInstance *m_df;
    Race *m_race;
    VIRTADDR bp_addr;
    int m_body_part_id;
    int m_parent_id;

    QString m_bp_name;
    QString m_token;
    QHash<int, BodyPartLayer> m_layers;
    QVector<VIRTADDR> m_layers_addr;

    void build_bp_name(){
        int bp_count = m_df->read_int(bp_addr + m_df->memory_layout()->health_offset("number"));
        QVector<VIRTADDR> sing_names = m_df->enumerate_vector(bp_addr + m_df->memory_layout()->health_offset("names_vector"));
        QVector<VIRTADDR> plural_names = m_df->enumerate_vector(bp_addr + m_df->memory_layout()->health_offset("names_plural_vector"));

        QString bp_name = m_df->read_string(sing_names.at(0));
        QString bp_name_plural = m_df->read_string(plural_names.at(0));

        if(bp_count > 1)
            bp_name = bp_name_plural;

        if(bp_name.contains(",")){
            QStringList pieces = bp_name.split(",");
            bp_name = QString("%1, %2").arg(pieces.at(1).trimmed()).arg(pieces.at(0).trimmed());
        }
        m_bp_name =  bp_name;
    }

};
#endif // BODYPART_H
