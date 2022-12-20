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

#include "uniform.h"
#include "item.h"
#include "itemuniform.h"
#include "itemdefuniform.h"

Uniform::Uniform(DFInstance *df, QObject *parent)
    : QObject(parent)
    , m_df(df)
    , m_first_check(true)
{
}

Uniform::~Uniform()
{
    foreach (const QList<ItemDefUniform*> &list, m_uniform_items) {
        foreach (ItemDefUniform* def, list) {
            delete def;
        }
    }
}

void Uniform::add_uniform_item(ITEM_TYPE itype, short sub_type, short job_skill, int count){
    ItemDefUniform *uItem = new ItemDefUniform(itype,sub_type,job_skill,this);
    add_uniform_item(itype,uItem,count);
}

void Uniform::add_uniform_item(ITEM_TYPE itype, short sub_type, QList<int> job_skills, int count){
    ItemDefUniform *uItem = new ItemDefUniform(itype,sub_type,job_skills,this);
    add_uniform_item(itype,uItem,count);
}

void Uniform::add_uniform_item(VIRTADDR addr, ITEM_TYPE itype, int count){
    ItemDefUniform *uItem;
    //backpacks, quivers and flasks are assigned specific items, so pass the id number rather than an address
    if(!Item::is_supplies(itype) && !Item::is_ranged_equipment(itype)){
        uItem = new ItemDefUniform(m_df,addr,this);
    }else{
        uItem = new ItemDefUniform(itype,m_df->read_int(addr),this);
    }
    add_uniform_item(itype,uItem,count);
}

void Uniform::add_uniform_item(ITEM_TYPE itype, ItemDefUniform *uItem, int count){
    //if there's no item type, this indicates that it's a specifically chosen item, so reset the item type so we can find it later
    if(uItem->item_type() == NONE)
        uItem->item_type(itype);

    uItem->add_to_stack(count-1); //uniform item stack size start at 1

    m_uniform_items[itype].append(uItem);
    //assigning boots to a uniform only lists one item, but
    if (uItem->id() <= 0 && (itype == SHOES || itype == GLOVES))
        m_uniform_items[itype].append(new ItemDefUniform(*uItem));
    if (count > 0)
        add_equip_count(itype,count);

    uItem = 0;
}

void Uniform::add_equip_count(ITEM_TYPE itype, int count){
    int current_count = m_equip_counts.value(itype);
    if(itype == SHOES || itype == GLOVES)
        count *=2;

    m_equip_counts.insert(itype,current_count+count);

    //add group counts as well
    if(Item::is_melee_equipment(itype))
        add_equip_count(MELEE_EQUIPMENT,count);
    else if(Item::is_supplies(itype))
        add_equip_count(SUPPLIES,count);
    else if(Item::is_ranged_equipment(itype))
        add_equip_count(RANGED_EQUIPMENT,count);
}

void Uniform::clear(){
    m_equip_counts.clear();
    m_missing_items.clear();
    m_first_check = true;
}

int Uniform::get_remaining_required(ITEM_TYPE itype){
    int count = 0;
    foreach(ITEM_TYPE key, m_uniform_items.uniqueKeys()){
        QList<ItemDefUniform*> items = m_uniform_items.value(key);
        foreach(ItemDefUniform *u, items){
            if(u->get_stack_size() > 0 && (u->item_type() == itype || Item::type_in_group(itype,u->item_type())))
                count += u->get_stack_size();
        }
    }
    return count;
}

int Uniform::get_missing_equip_count(ITEM_TYPE itype){
    int count = 0;
    if(m_missing_items.count() == 0)
        return count;
    if(m_missing_counts.count() <= 0){
        load_missing_counts();
    }
    return m_missing_counts.value(itype);
}

void Uniform::load_missing_counts(){
    //ensure we have a NONE and group counts
    m_missing_counts.insert(NONE,0);
    m_missing_counts.insert(MELEE_EQUIPMENT,0);
    m_missing_counts.insert(RANGED_EQUIPMENT,0);
    m_missing_counts.insert(SUPPLIES,0);

    foreach(ITEM_TYPE itype, m_missing_items.uniqueKeys()){
        foreach(ItemDefUniform *item_miss, m_missing_items.value(itype)){
            int missing_count = m_missing_counts.value(itype,0);
            int item_qty = item_miss->get_stack_size();
            if(item_qty > 0){
                missing_count += item_qty;
                m_missing_counts[NONE] += item_qty;
                //add group counts as well
                if(Item::is_melee_equipment(itype))
                    m_missing_counts[MELEE_EQUIPMENT] += item_qty;
                else if(Item::is_supplies(itype))
                    m_missing_counts[SUPPLIES] += item_qty;
                else if(Item::is_ranged_equipment(itype))
                    m_missing_counts[RANGED_EQUIPMENT] += item_qty;
            }
            m_missing_counts.insert(itype,missing_count);
        }
    }
}

float Uniform::get_uniform_rating(ITEM_TYPE itype){
    float rating = 100;
    if(m_first_check)
        return rating;

    float required = 0;
    float missing = get_missing_equip_count(itype);

    if(itype != NONE){
        required = (float)m_equip_counts.value(itype);
    }else{
        foreach(ITEM_TYPE i,m_equip_counts.uniqueKeys()){
            if(i < NUM_OF_ITEM_TYPES)
                required += m_equip_counts.value(i,0);
        }
    }

    if(required <= 0)
        required = 1;
    rating = (required-missing) / required * 100.0f;

    if(rating > 100)
        rating = 100.0f;

    return rating;
}

void Uniform::check_uniform(QString category_name, Item *item_inv){
    if(m_uniform_items.count() <= 0)
        return;
    //make a copy of our full uniform list to a missing list to remove items from
    if(m_first_check){
        m_missing_items = m_uniform_items;
        m_first_check = false;
    }
    ITEM_TYPE itype = item_inv->item_type();
    //no address indicates a placeholder item for uncovered/missing stuff
    if(item_inv->address() != 0 && category_name != Item::missing_group_name() && category_name != Item::uncovered_group_name()){
        //check our uniform's item list
        if(m_missing_items.value(itype).count() > 0){
            int idx = 0;
            //get the qty of this item type still required, based on the stack size
            int req_count = get_remaining_required(item_inv->item_type());
            int inv_qty = item_inv->get_stack_size();
            for(idx = m_missing_items.value(itype).count()-1; idx >= 0; idx--){
                //if the inventory quantity has been used up, or the required item count for the uniform met, quit
                if(inv_qty <= 0 || req_count <= 0)
                    break;

                ItemDefUniform *item_uni_miss = m_missing_items.value(itype).at(idx); //get a missing item of the same type
                Item *item_miss = new ItemUniform(m_df,item_uni_miss,this);

                bool match = false;

                //check for a specific item id
                if(item_miss->id() > -1){
                    if(item_miss->id() == item_inv->id()){
                        match = true;
                    }else{
                        continue;
                    }
                }else{
                    match = (item_uni_miss->indv_choice() ||
                             (
                                 (item_uni_miss->item_subtype() < 0 || item_uni_miss->item_subtype() == item_inv->item_subtype()) &&
                                 (
                                     (item_uni_miss->mat_flag() == MAT_NONE && item_miss->mat_type() < 0 && item_miss->mat_index() < 0) ||
                                     (item_miss->mat_type() < 0 && item_miss->mat_index() < 0 && item_inv->mat_flags().has_flag(item_uni_miss->mat_flag())) ||
                                      //QString::compare(item_miss->get_material_name(),item_inv->get_material_name_base(),Qt::CaseInsensitive)==0) ||
                                     ((item_miss->mat_type() >= 0 || item_miss->mat_index() >= 0) &&
                                      item_miss->mat_type() == item_inv->mat_type() && item_miss->mat_index() == item_inv->mat_index())
                                     ) &&
                                 (item_uni_miss->job_skills().count() == 0 || (item_uni_miss->job_skills().contains(item_inv->melee_skill()) || item_uni_miss->job_skills().contains(item_inv->ranged_skill())))
                                 )
                             );
                }
                delete item_miss;
                if(match){
                    QList<ItemDefUniform*> missing_items = m_missing_items.take(itype);

                    req_count -= inv_qty; //reduce the required count
                    missing_items.at(idx)->add_to_stack(-inv_qty); //reduce the uniform item's stack
                    int curr_qty = missing_items.at(idx)->get_stack_size(); //after applying the inventory quantity
                    if(curr_qty <= 0){
                        inv_qty = abs(curr_qty); //update the leftover inventory quantity
                        missing_items.removeAt(idx); //item isn't missing, remove it
                    }

                    if(missing_items.count() > 0)
                        m_missing_items.insert(itype,missing_items);
                }
            }
        }
    }
}
