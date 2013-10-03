#ifndef BODYPARTDAMAGE_H
#define BODYPARTDAMAGE_H


#include "truncatingfilelogger.h"
#include "bodypart.h"

class BodyPartDamage
{
public:

    BodyPartDamage()
        :m_bp(0x0)
        ,m_bp_status(0)
        ,m_bp_req(0)
        ,m_bone_dmg(false)
        ,m_muscle_dmg(false)
        ,m_skin_dmg(false)
    {
        m_has_rot = false;
        m_is_missing = false;
    }

    BodyPartDamage(BodyPart *bp, quint32 bp_status, quint32 bp_req)
        :m_bp(bp)
        ,m_bp_status(bp_status)
        ,m_bp_req(bp_req)
        ,m_bone_dmg(false)
        ,m_muscle_dmg(false)
        ,m_skin_dmg(false)
    {

        if(has_flag(0x10,bp_status))
            m_muscle_dmg = true; //muscle loss
        if(has_flag(0x20,bp_status))
            m_muscle_dmg = true; //muscle damage
        if(has_flag(0x40,bp_status))
           m_bone_dmg = true; //bone loss
        if(has_flag(0x80,bp_status))
            m_bone_dmg = true; //bone damage
        if(has_flag(0x100,bp_status))
            m_skin_dmg = true; //skin damage

        m_has_rot = (has_flag(0x00000080,m_bp_req));

        m_is_missing = (has_flag(0x2,m_bp_status));
//        m_needs_cleaning = (has_flag(0x0004,m_bp_req));
//        m_needs_sutures = (has_flag(0x0010,m_bp_req));
    }

    virtual ~BodyPartDamage(){
       m_bp = 0;
    }

    bool has_skin_dmg() {return m_skin_dmg;}
    bool has_muscle_dmg() {return m_muscle_dmg;}
    bool has_bone_dmg() {return m_bone_dmg;}

    bool has_rot() {return m_has_rot;}
    bool is_missing() {return m_is_missing;}

    bool old_motor_nerve_dmg(){return has_flag(0x200,m_bp_status);}
    bool old_sensory_nerve_dmg(){return has_flag(0x400,m_bp_status);}

    BodyPart *body_part() {return m_bp;}

private:
    BodyPart *m_bp;

    quint32 m_bp_status;
    quint32 m_bp_req; //holds flags for treatments (immobilize, suture, etc. but is also the only place we can check for rot)
    bool m_bone_dmg;
    bool m_muscle_dmg;
    bool m_skin_dmg;

    bool m_has_rot;
    bool m_is_missing;
//    bool m_needs_cleaning;
//    bool m_needs_sutures;
};

#endif // BODYPARTDAMAGE_H
