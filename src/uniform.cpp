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

Uniform::Uniform(DFInstance *df,QObject *parent)
    :QObject(parent)
    ,m_df(df)
    ,m_first_check(true)
{
}

Uniform::~Uniform()
{
    m_df = 0;

    QHashIterator<ITEM_TYPE,QList<ItemDefUniform*> > iter_items(m_uniform_items);
    iter_items.toBack();
    while (iter_items.hasPrevious()) {
        iter_items.previous();
        QList<ItemDefUniform*> list = m_uniform_items.take(iter_items.key());
        qDeleteAll(list);
        list.clear();
    }
    m_uniform_items.clear();
    m_missing_items.clear();

    m_equip_counts.clear();
}

void Uniform::add_uniform_item(ITEM_TYPE itype, short sub_type, short job_skill, int count){
    ItemDefUniform *uItem = new ItemDefUniform(itype,sub_type,job_skill,this);    
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

    uItem->add_to_stack(count-1); //uniform items always assume 1

    QList<ItemDefUniform*> items = m_uniform_items.take(itype);
    items.append(uItem);
    if(itype == SHOES || itype == GLOVES){
        items.append(new ItemDefUniform(*uItem));        
    }
    if(items.length() > 0){
        m_uniform_items.insert(itype,items);
    }
    if(count > 0)
        add_equip_count(itype,count);

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

int Uniform::get_required_count(ITEM_TYPE itype){
    //return m_equip_counts.value(itype,-1);
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

    if(itype == NONE){
        foreach(QList<ItemDefUniform*> items, m_missing_items.values()){
            //count += items.count();
            foreach(ItemDefUniform *u, items){
                if(u->get_stack_size() > 0)
                    count += u->get_stack_size();
            }
        }
    }else{
        foreach(ITEM_TYPE missing_type, m_missing_items.uniqueKeys()){
            if(missing_type == itype || Item::type_in_group(itype,missing_type)){
//                count += m_missing_items.value(missing_type).count();
                foreach(ItemDefUniform *u, m_missing_items.value(missing_type)){
                    if(u->get_stack_size() > 0)
                        count += u->get_stack_size();
                }
            }
        }
    }
    return count;
}

float Uniform::get_uniform_rating(ITEM_TYPE itype){
    float rating = 100;
    if(m_first_check)
        return rating;

    float original = 0;
    float missing = get_missing_equip_count(itype);

    if(itype != NONE){
        original = (float)m_equip_counts.value(itype);
    }else{
        foreach(ITEM_TYPE i,m_equip_counts.uniqueKeys()){
            if(i <= NUM_OF_ITEM_TYPES)
                original += m_equip_counts.value(i,0);
        }
    }

    if(original <= 0)
        original = 1;
    rating = (original-missing) / original * 100.0f;

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
            int req_count = get_required_count(item_inv->item_type());
            for(idx = m_missing_items.value(itype).count()-1; idx >= 0; idx--){
//                if(req_count <= 0)
//                    break;
                ItemDefUniform *u = m_missing_items.value(itype).at(idx);
//            foreach(ItemDefUniform *u, m_missing_items.value(itype)){
                if((u->id() > -1 && u->id() == item_inv->id()) || u->indv_choice() ||
                        (
                            (u->item_subtype() < 0 || u->item_subtype() == item_inv->item_subtype()) &&
                            (u->mat_type() < 0 || u->mat_type()==item_inv->mat_type()) &&
                            (u->mat_index() < 0 || u->mat_index()==item_inv->mat_index()) &&
                            (u->job_skill() < 0 || (u->job_skill() == item_inv->melee_skill() || u->job_skill() == item_inv->ranged_skill()))
                            )
                        )
                {
//                    if(item_inv->get_stack_size() >= req_count || req_count <= 0){
//                        u->add_to_stack(-req_count);
//                        if(u->get_stack_size() <= 0)
//                            m_missing_items.values(itype).removeAt(idx);
//                    }else{
//                        m_missing_items.values(itype).removeAt(idx);
//                    }
                    QList<ItemDefUniform*> uItems = m_missing_items.take(itype);
                    if(req_count <= 0){
                        uItems.removeAt(idx);
                    }else{
                        uItems.at(idx)->add_to_stack(-item_inv->get_stack_size());
                        if(uItems.at(idx)->get_stack_size() <= 0)
                            uItems.removeAt(idx);
                        req_count -= item_inv->get_stack_size();
                    }
                    if(uItems.count() > 0)
                        m_missing_items.insert(itype,uItems);
//                    QList<ItemDefUniform*> uItems = m_missing_items.take(itype);
//                    uItems.removeAt(idx);
//                    if(uItems.count() > 0)
//                        m_missing_items.insert(itype,uItems);
//                    break;
                }
//                idx++;
            }
        }
    }
}
