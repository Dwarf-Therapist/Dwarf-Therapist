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


#include "item.h"
#include "gamedatareader.h"
#include "itemammo.h"

Item::Item(const Item &i)
    :QObject(i.parent())
{
    m_df = i.m_df;
    m_addr = i.m_addr;
    m_iType = i.m_iType;
    m_wear = i.m_wear;
    m_mat_type = i.m_mat_type;
    m_mat_idx = i.m_mat_idx;
    m_quality = i.m_quality;
    m_material_name = i.m_material_name;
    m_item_name = i.m_item_name;
    m_layer_name = i.m_layer_name;
    m_display_name = i.m_display_name;
    m_color_display = i.m_color_display;
    m_id = i.m_id;
    m_affection = i.m_affection;
    m_stack_size = i.m_stack_size;
    m_artifact_name = i.m_artifact_name;
}

Item::Item(DFInstance *df, ItemDefUniform *u, QObject *parent)
    :QObject(parent)
    ,m_df(df)
    ,m_addr(0x0)
    ,m_iType(u->item_type())
    ,m_wear(0)
    ,m_mat_type(u->mat_type())
    ,m_mat_idx(u->mat_index())
    ,m_quality(-1)
    ,m_id(u->id())
    ,m_affection(0)
    ,m_stack_size(0)
{
    if(m_id > 0){
        //find the actual item's address
        m_addr = m_df->get_item_address(m_iType,m_id);
        read_data();
    }else{
        QVector<VIRTADDR> item_defs = m_df->get_item_vector(m_iType);

        int mat_class = u->mat_class();
        if(mat_class >= 0){
            m_material_name = Material::get_mat_class_desc(mat_class);
        }else{
            if(m_mat_idx >= 0 || m_mat_type >= 0)
                m_material_name = m_df->find_material_name(m_mat_idx,m_mat_type,m_iType);
        }

        short subtype = u->item_subtype();
        if(!item_defs.empty() && (subtype >=0 && subtype < item_defs.count())){
            //get sub-type name
            m_item_name = m_df->read_string(item_defs.at(subtype) + m_df->memory_layout()->item_subtype_offset("name"));
        }else{
            //get base item name
            m_item_name = Item::get_item_name(m_iType);
            //check skill type
            if(u->indv_choice() <= 0 && u->job_skill() >= 0)
                m_item_name.append(QObject::tr(" of %1 skill type").arg(GameDataReader::ptr()->get_skill_name(u->job_skill())));

        }
    }
    //set the color to the missing uniform color, since we passed in a uniform itemdef
    m_color_display = Item::color_missing();
}

Item::Item(DFInstance *df, VIRTADDR item_addr, QObject *parent)
    :QObject(parent)
    ,m_df(df)
    ,m_addr(item_addr)
    ,m_wear(0)
    ,m_mat_type(-1)
    ,m_mat_idx(-1)
    ,m_quality(-1)
    ,m_id(-1)
    ,m_affection(0)
    ,m_stack_size(0)
{
    read_data();
}

Item::Item(ITEM_TYPE itype, QString name, QObject *parent)
    :QObject(parent)
    ,m_df(0x0)
    ,m_addr(0x0)
    ,m_iType(itype)
    ,m_wear(0)
    ,m_mat_type(-1)
    ,m_mat_idx(-1)
    ,m_quality(-1)
    ,m_id(-1)
    ,m_affection(0)
    ,m_stack_size(0)
{
    m_item_name = (!name.trimmed().isEmpty() ? name : QObject::tr("Unknown"));
    m_display_name = m_item_name;
    m_color_display = Item::color_uncovered();
}

Item::~Item(){
    m_df = 0;
    qDeleteAll(m_contained_items);
}

void Item::read_data(){
    if(m_addr){
        VIRTADDR item_vtable = m_df->read_addr(m_addr);
        m_iType = static_cast<ITEM_TYPE>(m_df->read_int(m_df->read_addr(item_vtable) + 0x1));

        m_id = m_df->read_int(m_addr+m_df->memory_layout()->item_offset("id"));
        m_stack_size = m_df->read_int(m_addr+m_df->memory_layout()->item_offset("stack_size"));
        m_wear = m_df->read_short(m_addr+m_df->memory_layout()->item_offset("wear"));
        m_mat_type = m_df->read_short(m_addr+m_df->memory_layout()->item_offset("mat_type"));
        m_mat_idx = m_df->read_int(m_addr+m_df->memory_layout()->item_offset("mat_index"));
        m_quality = m_df->read_short(m_addr+m_df->memory_layout()->item_offset("quality"));

        Material *m = m_df->find_material(m_mat_idx,m_mat_type);
        m_material_name = capitalizeEach(m_df->find_material_name(m_mat_idx,m_mat_type,m_iType));

        set_default_name(m);

        QVector<VIRTADDR> gen_refs = m_df->enumerate_vector(m_addr+m_df->memory_layout()->item_offset("general_refs"));
        foreach(VIRTADDR ref, gen_refs){
            VIRTADDR gen_ref_vtable = m_df->read_addr(ref);
            int ref_type = m_df->read_int(m_df->read_addr(gen_ref_vtable+m_df->memory_layout()->general_ref_offset("ref_type")) + 0x1);
            if(ref_type == 0 || ref_type == 1){
                int artifact_id = m_df->read_int(ref+m_df->memory_layout()->general_ref_offset("artifact_id"));
                if(artifact_id > 0){
                    m_artifact_name = m_df->get_item_name(ARTIFACTS,artifact_id);
                    break;
                }
            }else if(ref_type == 10 && m_iType == QUIVER){ //type of container item
                int item_id = m_df->read_int(ref+m_df->memory_layout()->general_ref_offset("item_id"));
                VIRTADDR ammo_addr = m_df->get_item_address(AMMO,item_id);
                m_contained_items.append(new ItemAmmo(m_df,ammo_addr));
            }
        }
    }
}

QString Item::display_name(bool colored){
    {
        if(m_display_name == ""){
            build_display_name();
        }
        if(colored){
            if(!m_color_display.isValid()){;
                m_color_display = QApplication::palette().shadow().color();
            }
            return QString("<font color=%1>%2</font>").arg(m_color_display.name()).arg(m_display_name);
        }else{
            return m_display_name;
        }
    }
}

void Item::set_default_name(Material *m){
    //only handling equipment for now
    if(m_iType == FLASK){
        if(m->flags().has_flag(47))
            m_item_name = QObject::tr("Flask");
        else if(m->flags().has_flag(49))
            m_item_name = QObject::tr("Vial");
        else
            m_item_name = QObject::tr("Waterskin");
    }else{
        m_item_name = get_item_name(m_iType);
    }
}

void Item::set_affection(int level)
{
    if(level > 0)
        m_affection = level;
}

void Item::build_display_name(){
    QString symbol_q;
    QString symbol_w;

    switch(m_quality){
    case 1:{
        symbol_q = QString("-");
    }break;
    case 2:{
        symbol_q = QString("+");
    }break;
    case 3:{
        symbol_q = QChar(0x002A);
    }break;
    case 4:{
        symbol_q = QChar(0x2261);
    }break;
    case 5:{
        symbol_q = QChar(0x263C);
    }break;
    case 6:{
        symbol_q = QChar('!'); //artifact?
    }break;
    default:
        symbol_q = "";
    }

    switch(m_wear){
    case 1:{
        symbol_w = "x";
    }break;
    case 2:{
        symbol_w = "X";
    }break;
    case 3:{
        symbol_w = "XX";
    }break;
    default:
        symbol_w = "";
    }

    //build the base display name
    m_display_name = QString("%4%3%1 %2%3%4")
            .arg(capitalizeEach(m_material_name))
            .arg(capitalizeEach(m_item_name))
            .arg(symbol_q)
            .arg(symbol_w);

    if(m_stack_size > 1)
        m_display_name.prepend(QString::number(m_stack_size) + " ");

    //only for armor
    if(m_layer_name != "")
        m_display_name.append(QString(" (%1)").arg(capitalizeEach(m_layer_name)));

    //weapons/shields that aren't artifacts (named) yet
    if(m_affection > 50 && m_affection < MAX_AFFECTION){
        m_display_name.append(QObject::tr(" (%1% Named)").arg(QString::number((float)m_affection / MAX_AFFECTION * 100.0f,'f',0)));
    }

    //weapons/shields that have become artifacts (named)
    if(m_artifact_name != ""){
        m_display_name = QString("%1 (%2)").arg(capitalizeEach(m_artifact_name)).arg(m_display_name);
        //override the color
        m_color_display = QColor(42,189,127);
    }

    //override the color if there's wear, as it indicates this is an actual item
    if(m_wear > 0)
        m_color_display = Item::color_wear();
}
