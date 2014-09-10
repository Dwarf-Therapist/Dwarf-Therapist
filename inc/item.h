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
#ifndef ITEM_H
#define ITEM_H

#include <QtCore>
#include <QObject>
#include "truncatingfilelogger.h"
#include "global_enums.h"
#include "material.h"
#include "itemdefuniform.h"

class Item : public QObject  {
    Q_OBJECT
public:
    Item(const Item &i);
    Item(DFInstance *df, VIRTADDR item_addr, QObject *parent = 0);
    //uniform item ctor
    Item(DFInstance *df, ItemDefUniform *u, QObject *parent = 0);
    //placeholder item ctor
    Item(ITEM_TYPE itype,QString name, QObject *parent = 0);
    virtual ~Item();

    static const int MAX_AFFECTION = 10000;

    static const QString get_item_name_plural(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]=QObject::tr("N/A");
        m[BAR]=QObject::tr("Metals/Fuels/Soaps");
        m[SMALLGEM]=QObject::tr("Cut gemstones");
        m[BLOCKS]=QObject::tr("Blocks");
        m[ROUGH]=QObject::tr("Rough Gemstones");
        m[BOULDER]=QObject::tr("Boulders");
        m[WOOD]=QObject::tr("Woods");
        m[DOOR]=QObject::tr("Doors");
        m[FLOODGATE]=QObject::tr("Floodgates");
        m[BED]=QObject::tr("Beds");
        m[CHAIR]=QObject::tr("Chairs/Thrones");
        m[CHAIN]=QObject::tr("Chains");
        m[FLASK]=QObject::tr("Flasks");
        m[GOBLET]=QObject::tr("Goblets");
        m[INSTRUMENT]=QObject::tr("Instruments");
        m[TOY]=QObject::tr("Toys");
        m[WINDOW]=QObject::tr("Windows");
        m[CAGE]=QObject::tr("Cages");
        m[BARREL]=QObject::tr("Barrels");
        m[BUCKET]=QObject::tr("Buckets");
        m[ANIMALTRAP]=QObject::tr("Animal Traps");
        m[TABLE]=QObject::tr("Tables");
        m[COFFIN]=QObject::tr("Coffins");
        m[STATUE]=QObject::tr("Statues");
        m[CORPSE]=QObject::tr("Corpses");
        m[WEAPON]=QObject::tr("Weapons");
        m[ARMOR]=QObject::tr("Armors (Chest)");
        m[SHOES]=QObject::tr("Armors (Feet)");
        m[SHIELD]=QObject::tr("Armors (Shields)");
        m[HELM]=QObject::tr("Armors (Head)");
        m[GLOVES]=QObject::tr("Armors (Hands)");
        m[BOX]=QObject::tr("Boxes/Bags");
        m[BIN]=QObject::tr("Bins");
        m[ARMORSTAND]=QObject::tr("Armor Stands");
        m[WEAPONRACK]=QObject::tr("Weapon Racks");
        m[CABINET]=QObject::tr("Cabinets");
        m[FIGURINE]=QObject::tr("Figurines");
        m[AMULET]=QObject::tr("Amulets");
        m[SCEPTER]=QObject::tr("Scepters");
        m[AMMO]=QObject::tr("Ammunitions");
        m[CROWN]=QObject::tr("Crowns");
        m[RING]=QObject::tr("Rings");
        m[EARRING]=QObject::tr("Earrings");
        m[BRACELET]=QObject::tr("Bracelets");
        m[GEM]=QObject::tr("Large Gems");
        m[ANVIL]=QObject::tr("Anvils");
        m[CORPSEPIECE]=QObject::tr("Corpse Bodyparts");
        m[REMAINS]=QObject::tr("Remains");
        m[MEAT]=QObject::tr("Meats");
        m[FISH]=QObject::tr("Fishes");
        m[FISH_RAW]=QObject::tr("Raw Fishes");
        m[VERMIN]=QObject::tr("Vermins");
        m[IS_PET]=QObject::tr("Pets");
        m[SEEDS]=QObject::tr("Seeds");
        m[PLANT]=QObject::tr("Plants");
        m[SKIN_TANNED]=QObject::tr("Tanned Hides");
        m[LEAVES_FRUIT]=QObject::tr("Leaves");
        m[THREAD]=QObject::tr("Threads");
        m[CLOTH]=QObject::tr("Cloths");
        m[TOTEM]=QObject::tr("Totems");
        m[PANTS]=QObject::tr("Armors (Legs)");
        m[BACKPACK]=QObject::tr("Backpacks");
        m[QUIVER]=QObject::tr("Quivers");
        m[CATAPULTPARTS]=QObject::tr("Catapult Parts");
        m[BALLISTAPARTS]=QObject::tr("Ballista Parts");
        m[SIEGEAMMO]=QObject::tr("Siege Ammunitions");
        m[BALLISTAARROWHEAD]=QObject::tr("Ballista Ammunitions");
        m[TRAPPARTS]=QObject::tr("Mechanisms");
        m[TRAPCOMP]=QObject::tr("Trap Components");
        m[DRINK]=QObject::tr("Alcohols");
        m[POWDER_MISC]=QObject::tr("Flours/Sugars/Powders");
        m[CHEESE]=QObject::tr("Cheeses");
        m[FOOD]=QObject::tr("Prepared Foods");
        m[LIQUID_MISC]=QObject::tr("Honeys/Syrups/Milks/Oils");
        m[COIN]=QObject::tr("Coins");
        m[GLOB]=QObject::tr("Fats");
        m[ROCK]=QObject::tr("Rocks");
        m[PIPE_SECTION]=QObject::tr("Pipes");
        m[HATCH_COVER]=QObject::tr("Hatch Covers");
        m[GRATE]=QObject::tr("Grates");
        m[QUERN]=QObject::tr("Querns");
        m[MILLSTONE]=QObject::tr("Millstones");
        m[SPLINT]=QObject::tr("Splints");
        m[CRUTCH]=QObject::tr("Crutches");
        m[TRACTION_BENCH]=QObject::tr("Traction Benches");
        m[ORTHOPEDIC_CAST]=QObject::tr("Casts");
        m[TOOL]=QObject::tr("Tools");
        m[SLAB]=QObject::tr("Slabs");
        m[EGG]=QObject::tr("Eggs");
        m[BOOK]=QObject::tr("Books");
        m[SUPPLIES]=QObject::tr("Supplies");        
        m[MELEE_EQUIPMENT]=QObject::tr("Weapon & Shield");
        m[RANGED_EQUIPMENT]=QObject::tr("Quiver & Ammo");
        return m.value(type, "N/A");
    }

    static const QString get_item_name(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]=QObject::tr("N/A");
        m[BAR]=QObject::tr("Bar");
        m[SMALLGEM]=QObject::tr("Cut Gemstone");
        m[BLOCKS]=QObject::tr("Block");
        m[ROUGH]=QObject::tr("Rough Gemstone");
        m[BOULDER]=QObject::tr("Boulder");
        m[WOOD]=QObject::tr("Wood");
        m[DOOR]=QObject::tr("Door");
        m[FLOODGATE]=QObject::tr("Floodgate");
        m[BED]=QObject::tr("Bed");
        m[CHAIR]=QObject::tr("Chair");
        m[CHAIN]=QObject::tr("Chain");
        m[FLASK]=QObject::tr("Flask");
        m[GOBLET]=QObject::tr("Goblet");
        m[INSTRUMENT]=QObject::tr("Instrument");
        m[TOY]=QObject::tr("Toy");
        m[WINDOW]=QObject::tr("Window");
        m[CAGE]=QObject::tr("Cage");
        m[BARREL]=QObject::tr("Barrel");
        m[BUCKET]=QObject::tr("Bucket");
        m[ANIMALTRAP]=QObject::tr("Animal Trap");
        m[TABLE]=QObject::tr("Table");
        m[COFFIN]=QObject::tr("Coffin");
        m[STATUE]=QObject::tr("Statue");
        m[CORPSE]=QObject::tr("Corpse");
        m[WEAPON]=QObject::tr("Weapon");
        m[ARMOR]=QObject::tr("Armor");
        m[SHOES]=QObject::tr("Boot");
        m[SHIELD]=QObject::tr("Shield");
        m[HELM]=QObject::tr("Helm");
        m[GLOVES]=QObject::tr("Gauntlet");
        m[BOX]=QObject::tr("Box");
        m[BIN]=QObject::tr("Bin");
        m[ARMORSTAND]=QObject::tr("Armor Stand");
        m[WEAPONRACK]=QObject::tr("Weapon Rack");
        m[CABINET]=QObject::tr("Cabinet");
        m[FIGURINE]=QObject::tr("Figurine");
        m[AMULET]=QObject::tr("Amulet");
        m[SCEPTER]=QObject::tr("Scepter");
        m[AMMO]=QObject::tr("Ammunition");
        m[CROWN]=QObject::tr("Crown");
        m[RING]=QObject::tr("Ring");
        m[EARRING]=QObject::tr("Earring");
        m[BRACELET]=QObject::tr("Bracelet");
        m[GEM]=QObject::tr("Large Gem");
        m[ANVIL]=QObject::tr("Anvil");
        m[CORPSEPIECE]=QObject::tr("Corpse Bodypart");
        m[REMAINS]=QObject::tr("Remain");
        m[MEAT]=QObject::tr("Meat");
        m[FISH]=QObject::tr("Fish");
        m[FISH_RAW]=QObject::tr("Raw Fish");
        m[VERMIN]=QObject::tr("Vermin");
        m[IS_PET]=QObject::tr("Pet");
        m[SEEDS]=QObject::tr("Seed");
        m[PLANT]=QObject::tr("Plant");
        m[SKIN_TANNED]=QObject::tr("Tanned Hide");
        m[LEAVES_FRUIT]=QObject::tr("Leaf");
        m[THREAD]=QObject::tr("Thread");
        m[CLOTH]=QObject::tr("Cloth");
        m[TOTEM]=QObject::tr("Totem");
        m[PANTS]=QObject::tr("Greave");
        m[BACKPACK]=QObject::tr("Backpack");
        m[QUIVER]=QObject::tr("Quiver");
        m[CATAPULTPARTS]=QObject::tr("Catapult Part");
        m[BALLISTAPARTS]=QObject::tr("Ballista Part");
        m[SIEGEAMMO]=QObject::tr("Siege Ammunition");
        m[BALLISTAARROWHEAD]=QObject::tr("Ballista Ammunition");
        m[TRAPPARTS]=QObject::tr("Mechanism");
        m[TRAPCOMP]=QObject::tr("Trap Component");
        m[DRINK]=QObject::tr("Alcohol");
        m[POWDER_MISC]=QObject::tr("Powder");
        m[CHEESE]=QObject::tr("Cheese");
        m[FOOD]=QObject::tr("Prepared Food");
        m[LIQUID_MISC]=QObject::tr("Syrup/Milk/Oil");
        m[COIN]=QObject::tr("Coin");
        m[GLOB]=QObject::tr("Fat");
        m[ROCK]=QObject::tr("Rock");
        m[PIPE_SECTION]=QObject::tr("Pipe");
        m[HATCH_COVER]=QObject::tr("Hatch Cover");
        m[GRATE]=QObject::tr("Grate");
        m[QUERN]=QObject::tr("Quern");
        m[MILLSTONE]=QObject::tr("Millstone");
        m[SPLINT]=QObject::tr("Splint");
        m[CRUTCH]=QObject::tr("Crutch");
        m[TRACTION_BENCH]=QObject::tr("Traction Bench");
        m[ORTHOPEDIC_CAST]=QObject::tr("Cast");
        m[TOOL]=QObject::tr("Tool");
        m[SLAB]=QObject::tr("Slab");
        m[EGG]=QObject::tr("Egg");
        m[BOOK]=QObject::tr("Book");
        m[SUPPLIES]=QObject::tr("Other Equipment");
        return m.value(type, "N/A");
    }

    static bool is_armor_type(const ITEM_TYPE &i_type){
        if(i_type == ARMOR || i_type == GLOVES || i_type == HELM || i_type == PANTS || i_type == SHOES || i_type == SHIELD)
            return true;
        else
            return false;
    }
    static bool is_supplies(const ITEM_TYPE &i_type){
        if(i_type == BACKPACK || i_type == CRUTCH || i_type == FLASK)
            return true;
        else
            return false;
    }
    static bool is_melee_equipment(const ITEM_TYPE &i_type){
        if(i_type == SHIELD || i_type == WEAPON)
            return true;
        else
            return false;
    }
    static bool is_ranged_equipment(const ITEM_TYPE &i_type){
        if(i_type == QUIVER || i_type == AMMO)
            return true;
        else
            return false;
    }

    static bool type_in_group(ITEM_TYPE group_type, ITEM_TYPE item){
        if(group_type == MELEE_EQUIPMENT && is_melee_equipment(item))
            return true;
        else if(group_type == SUPPLIES && is_supplies(item))
            return true;
        else if(group_type == RANGED_EQUIPMENT && is_ranged_equipment(item))
            return true;
        else
            return false;
    }

    //the category name for body parts not covered by clothing
    static const QString uncovered_group_name(){return QObject::tr("Uncovered");}
    //the category for missing uniform items
    static const QString missing_group_name() {return QObject::tr("Missing");}

    static const QColor color_missing() {return QColor(57,130,227);}
    static const QColor color_wear() {return QColor(240,116,0,255);}
    static const QColor color_uncovered() {return QColor(227,22,18);}

    VIRTADDR address() {return m_addr;}

    int id(){return m_id;}

    int wear(){return m_wear;}
    int quality(){return m_quality;}

    ITEM_TYPE item_type(){return m_iType;}
    void item_type(ITEM_TYPE newType){m_iType = newType;}

    int mat_index(){return m_mat_idx;}
    short mat_type(){return m_mat_type;}    

    virtual short item_subtype(){return -1;}
    virtual short melee_skill(){return -1;}
    virtual short ranged_skill(){return -1;}

    QString display_name(bool colored = false);
    bool equals(const Item &);

    void set_affection(int level);

    QList<Item*> contained_items() {return m_contained_items;}
    int get_stack_size(){return m_stack_size;}
    void add_to_stack(int num){m_stack_size+=num;}
    QString get_material_name(){return m_material_name;}
    short get_quality(){return m_quality;}

protected:
    DFInstance *m_df;
    VIRTADDR m_addr;
    ITEM_TYPE m_iType;
    short m_wear;
    short m_mat_type;
    int m_mat_idx;    
    short m_quality;
    QString m_material_name;
    QString m_item_name;
    QString m_layer_name;
    QString m_display_name;
    QColor m_color_display;
    int m_id;
    int m_affection;
    int m_stack_size;
    QString m_artifact_name;
    QList<Item*> m_contained_items;

    void read_data();
    void set_default_name(Material *m);
    void build_display_name();
    QString get_quality_symbol();

};
#endif // ITEM_H
