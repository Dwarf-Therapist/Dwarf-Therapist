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
#include "itemuniform.h"
#include "gamedatareader.h"
#include "itemdefuniform.h"
#include "itemgenericsubtype.h"

ItemUniform::ItemUniform(const Item &baseItem)
    :Item(baseItem)
    , m_item_def(0)
{
    read_def();
}

ItemUniform::ItemUniform(DFInstance *df, VIRTADDR item_addr)
    :Item(df,item_addr)
    , m_item_def(0)
{
    read_def();
}

ItemUniform::ItemUniform(DFInstance *df, ItemDefUniform *u, QObject *parent)
    : Item(df,0x0,parent)
    , m_item_def(0)
    , m_uniform_def(u)
{
    m_iType = u->item_type();
    m_mat_type = u->mat_type();
    m_mat_idx = u->mat_index();
    m_id = u->id();
    m_stack_size = u->get_stack_size();

    //set the color to the missing uniform color, since we passed in a uniform itemdef
    m_color_display = Item::color_missing();

    if(m_id > 0){
        //find the actual item's address
        m_addr = m_df->get_item_address(m_iType,m_id);
        if(m_addr){
            read_data();
            read_def();
            return;
        }
    }

    read_def();
}

ItemUniform::~ItemUniform(){
    delete m_item_def;
    m_uniform_def = 0;
}

short ItemUniform::item_subtype() const {return m_item_def->subType();}
ItemSubtype * ItemUniform::get_subType() {return m_item_def;}

void ItemUniform::read_def(){
    if(m_addr > 0){
        m_item_def = new ItemGenericSubtype(m_iType,m_df,m_df->read_addr(m_addr+m_df->memory_layout()->item_offset("item_def")),this);
    }else{
        if(m_uniform_def->mat_flag() != MAT_NONE){
            mat_flags().set_flag(m_uniform_def->mat_flag(),true);
        }

        init_defaults();

        if(m_uniform_def->mat_flag() != MAT_NONE){
            m_material_name = m_uniform_def->generic_mat_name();
        }

        short subtype = m_uniform_def->item_subtype();
        QVector<VIRTADDR> item_defs = m_df->get_itemdef_vector(m_iType);
        if(!item_defs.empty() && (subtype >=0 && subtype < item_defs.count())){
            //get sub-type name
            m_item_def = new ItemGenericSubtype(m_iType,m_df,item_defs.at(subtype),this);
        }else{
            //build a simple subtype and manually set the name
            m_item_def = new ItemGenericSubtype(m_iType,m_df,0,this);
            //get base item name
            QString name = Item::get_item_name(m_iType);
            QString name_plural = Item::get_item_name_plural(m_iType);
            //check skill type
            if(!m_uniform_def->indv_choice() && m_uniform_def->job_skills().count() > 0){
                QStringList skill_names;
                foreach(int id, m_uniform_def->job_skills()){
                    skill_names.append(GameDataReader::ptr()->get_skill_name(id));
                }
                QString skill_desc = tr(" (%1 Skill)").arg(skill_names.join("/"));
                name.append(skill_desc);
                name_plural.append(skill_desc);
            }
            if(m_id > 0){
                //couldn't find the item, it's been claimed by another unit or couldn't be equipped
                QString prefix = tr("Specific ");
                name.prepend(prefix);
                name_plural.prepend(prefix);
            }
            m_item_def->name(name);
            m_item_def->name_plural(name_plural);
        }
    }
    if(!m_item_def->name().isEmpty()){
        m_item_name = m_item_def->name();
        m_item_name_plural = m_item_def->name_plural();
    }
}
