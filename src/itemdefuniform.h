/*
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
#ifndef ITEMDEFUNIFORM_H
#define ITEMDEFUNIFORM_H

#include <QObject>
#include "utils.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "flagarray.h"

class ItemDefUniform : public QObject {
    Q_OBJECT
public:
    ItemDefUniform(DFInstance *df, VIRTADDR address, QObject *parent = 0)
        : QObject(parent)
        , m_address(address)
        , m_df(df)
        , m_mem(df->memory_layout())
        , m_iType(NONE)
        , m_subType(-1)
        , m_matType(-1)
        , m_mat_index(-1)
        , m_mat_flag(MAT_NONE)
        , m_mat_generic_name("")
        , m_id(-1)
        , m_indv_choice(false)
        , m_stack_size(1)
    {
        read_data();
    }

    ItemDefUniform(ITEM_TYPE itype, int item_id, QObject *parent = 0)
        : QObject(parent)
        , m_address(0x0)
        , m_df(0x0)
        , m_mem(0x0)
        , m_iType(itype)
        , m_subType(-1)
        , m_matType(-1)
        , m_mat_index(-1)
        , m_mat_flag(MAT_NONE)
        , m_mat_generic_name("")
        , m_id(item_id)
        , m_indv_choice(false)
        , m_stack_size(1)
    {
    }

    ItemDefUniform(ITEM_TYPE itype, short sub_type, short job_skill, QObject *parent = 0)
        : QObject(parent)
        , m_address(0x0)
        , m_df(0x0)
        , m_mem(0x0)
        , m_iType(itype)
        , m_subType(sub_type)
        , m_matType(-1)
        , m_mat_index(-1)
        , m_mat_flag(MAT_NONE)
        , m_id(-1)
        , m_indv_choice(false)
        , m_stack_size(1)
    {
        add_job_skill(job_skill);
    }

    ItemDefUniform(ITEM_TYPE itype, short sub_type, QList<int> job_skills, QObject *parent = 0)
        : QObject(parent)
        , m_address(0x0)
        , m_df(0x0)
        , m_mem(0x0)
        , m_iType(itype)
        , m_subType(sub_type)
        , m_matType(-1)
        , m_mat_index(-1)
        , m_mat_flag(MAT_NONE)
        , m_id(-1)
        , m_indv_choice(false)
        , m_job_skills(job_skills)
        , m_stack_size(1)
    {
    }

    ItemDefUniform(const ItemDefUniform &uItem)
        : QObject(uItem.parent())
        , m_address(uItem.m_address)
        , m_df(uItem.m_df)
        , m_mem(uItem.m_mem)
        , m_iType(uItem.m_iType)
        , m_subType(uItem.m_subType)
        , m_matType(uItem.m_matType)
        , m_mat_index(uItem.m_mat_index)
        , m_mat_flag(uItem.m_mat_flag)
        , m_mat_generic_name(uItem.m_mat_generic_name)
        , m_id(uItem.m_id)
        , m_indv_choice(uItem.m_indv_choice)
        , m_job_skills(uItem.m_job_skills)
        , m_stack_size(uItem.m_stack_size)
    {
    }

    virtual ~ItemDefUniform(){
        m_df = 0;
        m_mem = 0;
    }

    typedef enum{
        MC_UNKNOWN=-2,
        MC_NONE=-1,
        MC_LEATHER=1,
        MC_CLOTH=2,
        MC_WOOD=3,
        MC_STONE=5,
        MC_METAL_AMMO=13,
        MC_METAL_AMMO2=14,
        MC_METAL_ARMOR=16,
        MC_GEM=17,
        MC_BONE=18,
        MC_SHELL=19,
        MC_PEARL=20,
        MC_TOOTH=21,
        MC_HORN=22,
        MC_PLANT_FIBER=27,
        MC_SILK=28,
        MC_YARN=29
    } MATERIAL_CLASS;

    static const QString get_mat_class_desc(const int mat_class){
        QMap<int, QString> m;
        m[MC_LEATHER]=tr("Leather");
        m[MC_CLOTH]=tr("Cloth");
        m[MC_WOOD]=tr("Wooden");
        m[MC_STONE]=tr("Stone");
        m[MC_METAL_AMMO]=tr("Metal");
        m[MC_METAL_AMMO2]=tr("Metal");
        m[MC_METAL_ARMOR]=tr("Metal");
        m[MC_GEM]=tr("Gem");
        m[MC_BONE]=tr("Bone");
        m[MC_SHELL]=tr("Shell");
        m[MC_PEARL]=tr("Pearl");
        m[MC_TOOTH]=tr("Ivory/Tooth");
        m[MC_HORN]=tr("Horn/Hoof");
        m[MC_PLANT_FIBER]=tr("Plant Fiber");
        m[MC_SILK]=tr("Silk");
        m[MC_YARN]=tr("Yarn/Wool/Fur");
        return m.value(mat_class, "???");
    }

    VIRTADDR address() {return m_address;}
    ITEM_TYPE item_type() const {return m_iType;}
    void item_type(ITEM_TYPE newType){m_iType = newType;}
    short item_subtype() {return m_subType;}
    short mat_type() {return m_matType;}
    int mat_index() {return m_mat_index;}
    QString generic_mat_name(){return m_mat_generic_name;}
    MATERIAL_FLAGS mat_flag() {return m_mat_flag;}
    int id(){return m_id;}
    bool indv_choice(){return m_indv_choice;}
    QList<int> job_skills(){return m_job_skills;}

    int get_stack_size(){return m_stack_size;}
    void add_to_stack(int num){m_stack_size+=num;}

private:
    VIRTADDR m_address;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    ITEM_TYPE m_iType;
    short m_subType;
    short m_matType;
    int m_mat_index;
    MATERIAL_FLAGS m_mat_flag;
    QString m_mat_generic_name;
    int m_id;
    bool m_indv_choice;
    QList<int> m_job_skills;
    int m_stack_size;
    //static const QHash<MATERIAL_CLASS,MATERIAL_FLAGS> m_class_mats;

    void read_data(){
        if(m_address){
            m_id = m_df->read_int(m_address);

            m_iType = static_cast<ITEM_TYPE>(m_df->read_short(m_df->memory_layout()->squad_field(m_address, "uniform_spec_item_type")));
            m_subType = m_df->read_short(m_df->memory_layout()->squad_field(m_address, "uniform_spec_item_subtype"));
            m_matType = m_df->read_short(m_df->memory_layout()->squad_field(m_address, "uniform_spec_mat_type"));
            m_mat_index = m_df->read_int(m_df->memory_layout()->squad_field(m_address, "uniform_spec_mat_index"));
            /*
             * mat_class here is actually referencing different vectors of materials in the historical_entity.resources
             * for now, we cheat this by mapping the material types to matching material flags and compare with those
             * but a more comprehensive solution would be to add all the required resources.x vector offsets, and search within them for a material match
             */
            MATERIAL_CLASS mat_class = static_cast<MATERIAL_CLASS>(m_df->read_short(m_df->memory_layout()->squad_field(m_address, "uniform_spec_mat_class")));
            if(class_mats().contains(mat_class)){
                m_mat_flag = class_mats().value(mat_class);
            }
            if(mat_class != MC_NONE && mat_class != MC_UNKNOWN){
                m_mat_generic_name = ItemDefUniform::get_mat_class_desc(mat_class);
            }
            //individual choice is stored in a bit array, first bit (any) second (melee) third (ranged)
            //currently we only care if one is set or not. it may be ok just to check for a weapon type as well
            m_indv_choice = m_df->read_addr(m_df->memory_layout()->squad_field(m_address, "uniform_indiv_choice")) & 0x7;
        }
    }
    static const QHash<MATERIAL_CLASS,MATERIAL_FLAGS> &class_mats(){
        static QHash<MATERIAL_CLASS,MATERIAL_FLAGS> ret;
        ret.insert(MC_LEATHER,LEATHER);
        ret.insert(MC_CLOTH,THREAD_PLANT);
        ret.insert(MC_WOOD,IS_WOOD);
        ret.insert(MC_STONE,IS_STONE);
        ret.insert(MC_METAL_AMMO,IS_METAL);
        ret.insert(MC_METAL_AMMO2,IS_METAL);
        ret.insert(MC_METAL_ARMOR,IS_METAL);
        ret.insert(MC_GEM,IS_GEM);
        ret.insert(MC_BONE,BONE);
        ret.insert(MC_SHELL,SHELL);
        ret.insert(MC_PEARL,PEARL);
        ret.insert(MC_TOOTH,TOOTH);
        ret.insert(MC_HORN,HORN);
        ret.insert(MC_PLANT_FIBER,THREAD_PLANT);
        ret.insert(MC_SILK,SILK);
        ret.insert(MC_YARN,YARN);
        return ret;
    }

    void add_job_skill(int skill_id){
        if(skill_id >= 0 && !m_job_skills.contains(skill_id)){
            m_job_skills.append(skill_id);
        }
    }
};

#endif // ITEMDEFUNIFORM_H
