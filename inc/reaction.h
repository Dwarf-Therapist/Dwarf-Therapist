/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert

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
#ifndef REACTION_H
#define REACTION_H

#include <QtGui>
#include "utils.h"

class DFInstance;
class MemoryLayout;

class Reaction : public QObject {
    Q_OBJECT
public:
    Reaction(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Reaction();

    static Reaction* get_reaction(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this race in the remote DF process
    VIRTADDR address() {return m_address;}

    QString name() {return m_name;}
    QString skill() {return m_skill;}
    QString tag() {return m_tag;}

    void load_data();

private:
    typedef enum {
        RS_NONE = -1,
        RS_MINING,
        RS_WOODCUTTING,
        RS_CARPENTRY,
        RS_DETAILSTONE,
        RS_MASONRY,
        RS_ANIMALTRAIN,
        RS_ANIMALCARE,
        RS_DISSECT_FISH,
        RS_DISSECT_VERMIN,
        RS_PROCESSFISH,
        RS_BUTCHER,
        RS_TRAPPING,
        RS_TANNER,
        RS_WEAVING,
        RS_BREWING,
        RS_ALCHEMY,
        RS_CLOTHESMAKING,
        RS_MILLING,
        RS_PROCESSPLANTS,
        RS_CHEESEMAKING,
        RS_MILK,
        RS_COOK,
        RS_PLANT,
        RS_HERBALISM,
        RS_FISH,
        RS_SMELT,
        RS_EXTRACT_STRAND,
        RS_FORGE_WEAPON,
        RS_FORGE_ARMOR,
        RS_FORGE_FURNITURE,
        RS_CUTGEM,
        RS_ENCRUSTGEM,
        RS_WOODCRAFT,
        RS_STONECRAFT,
        RS_METALCRAFT,
        RS_GLASSMAKER,
        RS_LEATHERWORK,
        RS_BONECARVE,
        RS_AXE,
        RS_SWORD,
        RS_DAGGER,
        RS_MACE,
        RS_HAMMER,
        RS_SPEAR,
        RS_CROSSBOW,
        RS_SHIELD,
        RS_ARMOR,
        RS_SIEGECRAFT,
        RS_SIEGEOPERATE,
        RS_BOWYER,
        RS_PIKE,
        RS_WHIP,
        RS_BOW,
        RS_BLOWGUN,
        RS_THROW,
        RS_MECHANICS,
        RS_MAGIC_NATURE,
        RS_SNEAK,
        RS_DESIGNBUILDING,
        RS_DRESS_WOUNDS,
        RS_DIAGNOSE,
        RS_SURGERY,
        RS_SET_BONE,
        RS_SUTURE,
        RS_CRUTCH_WALK,
        RS_WOOD_BURNING,
        RS_LYE_MAKING,
        RS_SOAP_MAKING,
        RS_POTASH_MAKING,
        RS_DYER,
        RS_OPERATE_PUMP,
        RS_SWIMMING,
        RS_PERSUASION,
        RS_NEGOTIATION,
        RS_JUDGING_INTENT,
        RS_APPRAISAL,
        RS_ORGANIZATION,
        RS_RECORD_KEEPING,
        RS_LYING,
        RS_INTIMIDATION,
        RS_CONVERSATION,
        RS_COMEDY,
        RS_FLATTERY,
        RS_CONSOLE,
        RS_PACIFY,
        RS_TRACKING,
        RS_KNOWLEDGE_ACQUISITION,
        RS_CONCENTRATION,
        RS_DISCIPLINE,
        RS_SITUATIONAL_AWARENESS,
        RS_WRITING,
        RS_PROSE,
        RS_POETRY,
        RS_READING,
        RS_SPEAKING,
        RS_COORDINATION,
        RS_BALANCE,
        RS_LEADERSHIP,
        RS_TEACHING,
        RS_MELEE_COMBAT,
        RS_RANGED_COMBAT,
        RS_WRESTLING,
        RS_BITE,
        RS_GRASP_STRIKE,
        RS_STANCE_STRIKE,
        RS_DODGING,
        RS_MISC_WEAPON,
        RS_KNAPPING,
        RS_MILITARY_TACTICS,
        RS_SHEARING,
        RS_SPINNING,
        RS_POTTERY,
        RS_GLAZING,
        RS_PRESSING,
        RS_BEEKEEPING,
        RS_WAX_WORKING
    } REACTION_SKILL;

    static QString get_skill_name(const short &skill_id) {
        QMap<short, QString> m;
        m[RS_NONE] = "NONE";
        m[RS_MINING] = "MINING";
        m[RS_WOODCUTTING] = "WOODCUTTING";
        m[RS_CARPENTRY] = "CARPENTRY";
        m[RS_DETAILSTONE] = "DETAILSTONE";
        m[RS_MASONRY] = "MASONRY";
        m[RS_ANIMALTRAIN] = "ANIMALTRAIN";
        m[RS_ANIMALCARE] = "ANIMALCARE";
        m[RS_DISSECT_FISH] = "DISSECT_FISH";
        m[RS_DISSECT_VERMIN] = "DISSECT_VERMIN";
        m[RS_PROCESSFISH] = "PROCESSFISH";
        m[RS_BUTCHER] = "BUTCHER";
        m[RS_TRAPPING] = "TRAPPING";
        m[RS_TANNER] = "TANNER";
        m[RS_WEAVING] = "WEAVING";
        m[RS_BREWING] = "BREWING";
        m[RS_ALCHEMY] = "ALCHEMY";
        m[RS_CLOTHESMAKING] = "CLOTHESMAKING";
        m[RS_MILLING] = "MILLING";
        m[RS_PROCESSPLANTS] = "PROCESSPLANTS";
        m[RS_CHEESEMAKING] = "CHEESEMAKING";
        m[RS_MILK] = "MILK";
        m[RS_COOK] = "COOK";
        m[RS_PLANT] = "PLANT";
        m[RS_HERBALISM] = "HERBALISM";
        m[RS_FISH] = "FISH";
        m[RS_SMELT] = "SMELT";
        m[RS_EXTRACT_STRAND] = "EXTRACT_STRAND";
        m[RS_FORGE_WEAPON] = "FORGE_WEAPON";
        m[RS_FORGE_ARMOR] = "FORGE_ARMOR";
        m[RS_FORGE_FURNITURE] = "FORGE_FURNITURE";
        m[RS_CUTGEM] = "CUTGEM";
        m[RS_ENCRUSTGEM] = "ENCRUSTGEM";
        m[RS_WOODCRAFT] = "WOODCRAFT";
        m[RS_STONECRAFT] = "STONECRAFT";
        m[RS_METALCRAFT] = "METALCRAFT";
        m[RS_GLASSMAKER] = "GLASSMAKER";
        m[RS_LEATHERWORK] = "LEATHERWORK";
        m[RS_BONECARVE] = "BONECARVE";
        m[RS_AXE] = "AXE";
        m[RS_SWORD] = "SWORD";
        m[RS_DAGGER] = "DAGGER";
        m[RS_MACE] = "MACE";
        m[RS_HAMMER] = "HAMMER";
        m[RS_SPEAR] = "SPEAR";
        m[RS_CROSSBOW] = "CROSSBOW";
        m[RS_SHIELD] = "SHIELD";
        m[RS_ARMOR] = "ARMOR";
        m[RS_SIEGECRAFT] = "SIEGECRAFT";
        m[RS_SIEGEOPERATE] = "SIEGEOPERATE";
        m[RS_BOWYER] = "BOWYER";
        m[RS_PIKE] = "PIKE";
        m[RS_WHIP] = "WHIP";
        m[RS_BOW] = "BOW";
        m[RS_BLOWGUN] = "BLOWGUN";
        m[RS_THROW] = "THROW";
        m[RS_MECHANICS] = "MECHANICS";
        m[RS_MAGIC_NATURE] = "MAGIC_NATURE";
        m[RS_SNEAK] = "SNEAK";
        m[RS_DESIGNBUILDING] = "DESIGNBUILDING";
        m[RS_DRESS_WOUNDS] = "DRESS_WOUNDS";
        m[RS_DIAGNOSE] = "DIAGNOSE";
        m[RS_SURGERY] = "SURGERY";
        m[RS_SET_BONE] = "SET_BONE";
        m[RS_SUTURE] = "SUTURE";
        m[RS_CRUTCH_WALK] = "CRUTCH_WALK";
        m[RS_WOOD_BURNING] = "WOOD_BURNING";
        m[RS_LYE_MAKING] = "LYE_MAKING";
        m[RS_SOAP_MAKING] = "SOAP_MAKING";
        m[RS_POTASH_MAKING] = "POTASH_MAKING";
        m[RS_DYER] = "DYER";
        m[RS_OPERATE_PUMP] = "OPERATE_PUMP";
        m[RS_SWIMMING] = "SWIMMING";
        m[RS_PERSUASION] = "PERSUASION";
        m[RS_NEGOTIATION] = "NEGOTIATION";
        m[RS_JUDGING_INTENT] = "JUDGING_INTENT";
        m[RS_APPRAISAL] = "APPRAISAL";
        m[RS_ORGANIZATION] = "ORGANIZATION";
        m[RS_RECORD_KEEPING] = "RECORD_KEEPING";
        m[RS_LYING] = "LYING";
        m[RS_INTIMIDATION] = "INTIMIDATION";
        m[RS_CONVERSATION] = "CONVERSATION";
        m[RS_COMEDY] = "COMEDY";
        m[RS_FLATTERY] = "FLATTERY";
        m[RS_CONSOLE] = "CONSOLE";
        m[RS_PACIFY] = "PACIFY";
        m[RS_TRACKING] = "TRACKING";
        m[RS_KNOWLEDGE_ACQUISITION] = "KNOWLEDGE_ACQUISITION";
        m[RS_CONCENTRATION] = "CONCENTRATION";
        m[RS_DISCIPLINE] = "DISCIPLINE";
        m[RS_SITUATIONAL_AWARENESS] = "SITUATIONAL_AWARENESS";
        m[RS_WRITING] = "WRITING";
        m[RS_PROSE] = "PROSE";
        m[RS_POETRY] = "POETRY";
        m[RS_READING] = "READING";
        m[RS_SPEAKING] = "SPEAKING";
        m[RS_COORDINATION] = "COORDINATION";
        m[RS_BALANCE] = "BALANCE";
        m[RS_LEADERSHIP] = "LEADERSHIP";
        m[RS_TEACHING] = "TEACHING";
        m[RS_MELEE_COMBAT] = "MELEE_COMBAT";
        m[RS_RANGED_COMBAT] = "RANGED_COMBAT";
        m[RS_WRESTLING] = "WRESTLING";
        m[RS_BITE] = "BITE";
        m[RS_GRASP_STRIKE] = "GRASP_STRIKE";
        m[RS_STANCE_STRIKE] = "STANCE_STRIKE";
        m[RS_DODGING] = "DODGING";
        m[RS_MISC_WEAPON] = "MISC_WEAPON";
        m[RS_KNAPPING] = "KNAPPING";
        m[RS_MILITARY_TACTICS] = "MILITARY_TACTICS";
        m[RS_SHEARING] = "SHEARING";
        m[RS_SPINNING] = "SPINNING";
        m[RS_POTTERY] = "POTTERY";
        m[RS_GLAZING] = "GLAZING";
        m[RS_PRESSING] = "PRESSING";
        m[RS_BEEKEEPING] = "BEEKEEPING";
        m[RS_WAX_WORKING] = "WAX_WORKING";        
        return m.value(skill_id, "UNKNOWN");
    }

    VIRTADDR m_address;
    QString m_tag;
    QString m_name;
    QString m_skill;
    short m_skill_id;    

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_reaction();
};

#endif // REACTION_H
