#ifndef BODYPARTDAMAGE_H
#define BODYPARTDAMAGE_H

#include "truncatingfilelogger.h"

class BodyPart;

class BodyPartDamage
{
public:

    BodyPartDamage()
        : m_bp(0x0)
        , m_bp_status(0)
        , m_bp_req(0)
        , m_bone_dmg(false)
        , m_muscle_dmg(false)
        , m_skin_dmg(false)
    {
        m_has_rot = false;
        m_is_missing = false;
    }

    BodyPartDamage(BodyPart *bp, quint32 bp_status, quint32 bp_req)
        : m_bp(bp)
        , m_bp_status(bp_status)
        , m_bp_req(bp_req)
        , m_bone_dmg(false)
        , m_muscle_dmg(false)
        , m_skin_dmg(false)
    {
        m_muscle_dmg = bp_status & 0x30;
        m_bone_dmg = bp_status & 0xc0;
        m_skin_dmg = bp_status & 0x100;

        m_has_rot = m_bp_req & 0x80;

        m_is_missing = m_bp_status & 0x2;
    }

    virtual ~BodyPartDamage(){
       m_bp = 0;
    }

    inline bool has_skin_dmg() {return m_skin_dmg;}
    inline bool has_muscle_dmg() {return m_muscle_dmg;}
    inline bool has_bone_dmg() {return m_bone_dmg;}

    inline bool has_rot() {return m_has_rot;}
    inline bool is_missing() {return m_is_missing;}

    inline bool old_motor_nerve_dmg(){return m_bp_status & 0x200;}
    inline bool old_sensory_nerve_dmg(){return m_bp_status & 0x400;}

    inline BodyPart *body_part() {return m_bp;}

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
