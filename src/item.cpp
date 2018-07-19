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
#include "dfinstance.h"
#include "itemammo.h"
#include "itemsubtype.h"
#include "material.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"

const QList<ITEM_TYPE> Item::m_items_subtypes=Item::init_subtypes();
const QList<MATERIAL_FLAGS> Item::m_mat_cats=Item::init_mat_cats();
const QMap<Item::ITEM_STATE,QColor> Item::m_state_colors = Item::set_state_colors();

Item::Item(const Item &i)
    : QObject(i.parent())
{
    m_df = i.m_df;
    m_addr = i.m_addr;
    m_iType = i.m_iType;
    m_wear = i.m_wear;
    m_mat_type = i.m_mat_type;
    m_mat_idx = i.m_mat_idx;
    m_quality = i.m_quality;
    m_material_name = i.m_material_name;
    m_material_name_base = i.m_material_name_base;
    m_material_flags = i.m_material_flags;
    m_item_name = i.m_item_name;
    m_item_name_plural = i.m_item_name_plural;
    m_layer_name = i.m_layer_name;
    m_display_name = i.m_display_name;
    m_color_display = i.m_color_display;
    m_id = i.m_id;
    m_affection = i.m_affection;
    m_stack_size = i.m_stack_size;
    m_artifact_name = i.m_artifact_name;
    m_size_prefix = i.m_size_prefix;
    m_maker_race = i.m_maker_race;
}

Item::Item(DFInstance *df, VIRTADDR item_addr, QObject *parent)
    : QObject(parent)
    , m_df(df)
    , m_addr(item_addr)
    , m_wear(0)
    , m_mat_type(-1)
    , m_mat_idx(-1)
    , m_quality(-1)
    , m_id(-1)
    , m_affection(0)
    , m_stack_size(0)
    , m_size_prefix("")
    , m_maker_race(-1)
{
    read_data();
}

Item::Item(ITEM_TYPE itype, QString name, QObject *parent)
    : QObject(parent)
    , m_df(0x0)
    , m_addr(0x0)
    , m_iType(itype)
    , m_wear(0)
    , m_mat_type(-1)
    , m_mat_idx(-1)
    , m_quality(-1)
    , m_id(-1)
    , m_affection(0)
    , m_stack_size(0)
    , m_size_prefix("")
    , m_maker_race(-1)
{
    m_item_name = (!name.trimmed().isEmpty() ? name : tr("Unknown"));
    m_item_name_plural = m_item_name;
    m_display_name = m_item_name;
    m_color_display = Item::color_uncovered();
}

Item::~Item(){
    m_df = 0;
    qDeleteAll(m_contained_items);
}

const QList<ITEM_TYPE> Item::init_subtypes(){
    QList<ITEM_TYPE> tmp;
    tmp << SHOES << PANTS << ARMOR << GLOVES << HELM << WEAPON << AMMO << TRAPCOMP << SHIELD << TOOL << INSTRUMENT;
    return tmp;
}
const QList<MATERIAL_FLAGS> Item::init_mat_cats(){
    QList<MATERIAL_FLAGS> tmp;
    tmp << IS_METAL << LEATHER << SILK << YARN << THREAD_PLANT << BONE << SHELL;
    return tmp;
}

void Item::read_data(){
    if(m_addr){
        VIRTADDR item_vtable = m_df->read_addr(m_addr);

        m_iType = static_cast<ITEM_TYPE>(m_df->read_int(m_df->read_addr(item_vtable) + m_df->VM_TYPE_OFFSET()));

        m_id = m_df->read_int(m_df->memory_layout()->item_field(m_addr, "id"));
        m_stack_size = m_df->read_int(m_df->memory_layout()->item_field(m_addr, "stack_size"));
        m_wear = m_df->read_short(m_df->memory_layout()->item_field(m_addr, "wear"));
        m_mat_type = m_df->read_short(m_df->memory_layout()->item_field(m_addr, "mat_type"));
        m_mat_idx = m_df->read_int(m_df->memory_layout()->item_field(m_addr, "mat_index"));
        m_maker_race = m_df->read_short(m_df->memory_layout()->item_field(m_addr, "maker_race"));
        m_quality = m_df->read_short(m_df->memory_layout()->item_field(m_addr, "quality"));

        init_defaults();

        QVector<VIRTADDR> gen_refs = m_df->enumerate_vector(m_df->memory_layout()->item_field(m_addr, "general_refs"));
        foreach(VIRTADDR ref, gen_refs){
            VIRTADDR gen_ref_vtable = m_df->read_addr(ref);
            int ref_type = m_df->read_int(m_df->read_addr(m_df->memory_layout()->general_ref_field(gen_ref_vtable, "ref_type")) + m_df->VM_TYPE_OFFSET());
            if(ref_type == 0 || ref_type == 1){
                LOGD << "reading type:" << ref_type << "(artifact name)";
                int artifact_id = m_df->read_int(m_df->memory_layout()->general_ref_field(ref, "artifact_id"));
                if(artifact_id){
                    m_artifact_name = m_df->get_artifact_name(ARTIFACTS,artifact_id);
                    break;
                }
            }else if(ref_type == 10 && m_iType == QUIVER){ //type of container item, could be expanded to show food and drink
                LOGD << "reading type:" << ref_type << "(container)";
                int item_id = m_df->read_int(m_df->memory_layout()->general_ref_field(ref, "item_id"));
                VIRTADDR ammo_addr = m_df->get_item_address(AMMO,item_id);
                if(ammo_addr){
                    ItemAmmo *ia = new ItemAmmo(m_df,ammo_addr);
                    bool appended = false;
                    foreach(Item *i, m_contained_items){
                        if(i->equals(*ia)){
                            i->add_to_stack(ia->get_stack_size());
                            appended = true;
                            break;
                        }
                    }
                    if(!appended){
                        m_contained_items.append(ia);
                    }else{
                        delete ia;
                    }
                }else{
                    LOGE << "found unknown ammo!";
                }
            }
        }
    }
}

void Item::init_defaults(){
    m_material_name = capitalizeEach(m_df->find_material_name(m_mat_idx,m_mat_type,m_iType));
    Material *m = m_df->find_material(m_mat_idx,m_mat_type);
    set_default_name(m);
    if(m){
        m_material_flags = FlagArray(m->flags());
        foreach(MATERIAL_FLAGS mf, m_mat_cats){
            if(m->flags().has_flag(mf)){
                m_material_name_base = Material::get_material_flag_desc(mf);
                break;
            }
        }
    }
}

QString Item::display_name(bool colored){
    {
        if(m_display_name == ""){
            build_display_name();
        }
        if(colored && m_color_display.isValid()){
            return QString("<font color=%1>%2</font>").arg(m_color_display.name()).arg(m_display_name);
        }else{
            return m_display_name;
        }
    }
}

bool Item::equals(const Item &i) const{
    return (i.m_quality == m_quality && i.m_material_name == m_material_name &&
            i.m_iType == m_iType && i.item_subtype() == item_subtype());//i.m_item_name == m_item_name);
}


void Item::set_default_name(Material *m){
    //only handling equipment for now
    ITEM_TYPE name_type = m_iType;
    if(m && m_iType == FLASK){
        if(!m->flags().has_flag(47)){
            if(m->flags().has_flag(49)){
                name_type = VIAL;
            }else{
                name_type = WATERSKIN;
            }
        }
    }
    m_item_name = get_item_name(name_type);
    m_item_name_plural = get_item_name_plural(name_type);
}

void Item::set_affection(int level)
{
    if(level > 0)
        m_affection = level;
}

void Item::build_display_name(){
    QString symbol_q = get_quality_symbol();
    QString symbol_w;

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
    m_display_name = QString("%5%4%1%2 %3%4%5")
            .arg(m_size_prefix)
            .arg(capitalizeEach(m_material_name))
            .arg(item_name((m_stack_size > 1 ? true : false),false,false))
            .arg(symbol_q)
            .arg(symbol_w);

    if(m_stack_size > 1)
        m_display_name.prepend(QString::number(m_stack_size) + " ");

    //only for armor
    if(m_layer_name != "")
        m_display_name.append(QString(" (%1)").arg(capitalizeEach(m_layer_name)));

    //weapons/shields that aren't artifacts (named) yet, seems around 300 (3%) they become attached?
    if(m_affection >= 300 && m_affection < MAX_AFFECTION){
        m_display_name.append(tr(" (%1% Named)").arg(QString::number((float)m_affection / MAX_AFFECTION * 100.0f,'f',0)));
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

QString Item::get_quality_symbol(){
    QString symbol_q;
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
    return symbol_q;
}

QString Item::item_name(bool plural, bool mat, bool generic_mat){
    //if there's a subtype, use that name instead
    QString i_name;
    if(get_subType()){
        if(plural){
            i_name = get_subType()->name_plural();
        }else{
            i_name = get_subType()->name();
        }
    }
    if(i_name.isEmpty()){
        if(plural){
            i_name = m_item_name_plural;
        }else{
            i_name = m_item_name;
        }
    }

    QString mat_name = m_material_name_base;
    if(!m_material_name.isEmpty() && (!generic_mat || mat_name.isEmpty())){
        mat_name = m_material_name;
    }

    return capitalizeEach(QString("%1 %2")
                          .arg(mat && !mat_name.isEmpty() ? mat_name : "")
                          .arg(i_name)
                          .trimmed());
}

QMap<Item::ITEM_STATE,QColor> Item::set_state_colors(){
    QMap<ITEM_STATE,QColor> m;
    m.insert(IS_TATTERED,color_wear());
    QColor tmp = color_wear();
    tmp.setAlpha(190);
    m.insert(IS_THREADBARE,tmp);
    tmp.setAlpha(135);
    m.insert(IS_WORN,tmp);
    tmp.setAlpha(0);
    m.insert(IS_CLOTHED,tmp);
    m.insert(IS_UNCOVERED,color_uncovered());
    m.insert(IS_MISSING,color_missing());
    return m;
}
