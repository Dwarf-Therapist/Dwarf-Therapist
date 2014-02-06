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

    static const int MAX_AFFECTION = 1000;

    static const QString get_item_name_plural(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]="N/A";
        m[BAR]="Metal, fuel or soap bars";
        m[SMALLGEM]="Cut gemstones";
        m[BLOCKS]="Blocks";
        m[ROUGH]="Rough gemstones";
        m[BOULDER]="Boulders";
        m[WOOD]="Wood";
        m[DOOR]="Doors";
        m[FLOODGATE]="Floodgates";
        m[BED]="Beds";
        m[CHAIR]="Chairs and thrones";
        m[CHAIN]="Chains";
        m[FLASK]="Flasks";
        m[GOBLET]="Goblets";
        m[INSTRUMENT]="Instruments";
        m[TOY]="Toys";
        m[WINDOW]="Windows";
        m[CAGE]="Cages";
        m[BARREL]="Barrels";
        m[BUCKET]="Buckets";
        m[ANIMALTRAP]="Animal traps";
        m[TABLE]="Tables";
        m[COFFIN]="Coffins";
        m[STATUE]="Statues";
        m[CORPSE]="Corpses";
        m[WEAPON]="Weapons";
        m[ARMOR]="Armor (Chest)";
        m[SHOES]="Armor (Feet)";
        m[SHIELD]="Armor (Shields)";
        m[HELM]="Armor (Head)";
        m[GLOVES]="Armor (Hands)";
        m[BOX]="Boxes";
        m[BIN]="Bins";
        m[ARMORSTAND]="Armor stands";
        m[WEAPONRACK]="Weapon racks";
        m[CABINET]="Cabinets";
        m[FIGURINE]="Figurines";
        m[AMULET]="Amulets";
        m[SCEPTER]="Scepters";
        m[AMMO]="Ammunition";
        m[CROWN]="Crowns";
        m[RING]="Rings";
        m[EARRING]="Earrings";
        m[BRACELET]="Bracelets";
        m[GEM]="Large gems";
        m[ANVIL]="Anvils";
        m[CORPSEPIECE]="Corpse bodyparts";
        m[REMAINS]="Remains";
        m[MEAT]="Meat";
        m[FISH]="Fish";
        m[FISH_RAW]="Raw fish";
        m[VERMIN]="Vermin";
        m[IS_PET]="Pets";
        m[SEEDS]="Seeds";
        m[PLANT]="Plants";
        m[SKIN_TANNED]="Tanned hide";
        m[LEAVES]="Leaves";
        m[THREAD]="Thread";
        m[CLOTH]="Cloth";
        m[TOTEM]="Totems";
        m[PANTS]="Armor (Legs)";
        m[BACKPACK]="Backpacks";
        m[QUIVER]="Quivers";
        m[CATAPULTPARTS]="Catapult parts";
        m[BALLISTAPARTS]="Ballista parts";
        m[SIEGEAMMO]="Siege ammunition";
        m[BALLISTAARROWHEAD]="Ballista ammunition";
        m[TRAPPARTS]="Mechanisms";
        m[TRAPCOMP]="Trap components";
        m[DRINK]="Alcohol";
        m[POWDER_MISC]="Flour, sugar and powders";
        m[CHEESE]="Cheese";
        m[FOOD]="Prepared food";
        m[LIQUID_MISC]="Honey, syrup, milk, oils, etc.";
        m[COIN]="Coins";
        m[GLOB]="Fat";
        m[ROCK]="Rocks";
        m[PIPE_SECTION]="Pipes";
        m[HATCH_COVER]="Hatch covers";
        m[GRATE]="Grates";
        m[QUERN]="Querns";
        m[MILLSTONE]="Millstones";
        m[SPLINT]="Splints";
        m[CRUTCH]="Crutches";
        m[TRACTION_BENCH]="Traction benches";
        m[ORTHOPEDIC_CAST]="Casts";
        m[TOOL]="Tools";
        m[SLAB]="Slabs";
        m[EGG]="Eggs";
        m[BOOK]="Books";
        m[SUPPLIES]=QObject::tr("Supplies");
        m[MELEE_EQUIPMENT]=QObject::tr("Weapon & Shield");
        m[RANGED_EQUIPMENT]=QObject::tr("Quiver & Ammo");
        return m.value(type, "N/A");
    }

    static const QString get_item_name(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]="N/A";
        m[BAR]="Bar";
        m[SMALLGEM]="Cut gemstone";
        m[BLOCKS]="Block";
        m[ROUGH]="Rough gemstone";
        m[BOULDER]="Bouldes";
        m[WOOD]="Wood";
        m[DOOR]="Door";
        m[FLOODGATE]="Floodgate";
        m[BED]="Bed";
        m[CHAIR]="Chair";
        m[CHAIN]="Chain";
        m[FLASK]="Flask";
        m[GOBLET]="Goblet";
        m[INSTRUMENT]="Instrument";
        m[TOY]="Toy";
        m[WINDOW]="Window";
        m[CAGE]="Cage";
        m[BARREL]="Barrel";
        m[BUCKET]="Bucket";
        m[ANIMALTRAP]="Animal trap";
        m[TABLE]="Table";
        m[COFFIN]="Coffin";
        m[STATUE]="Statue";
        m[CORPSE]="Corpse";
        m[WEAPON]="Weapon";
        m[ARMOR]="Armor";
        m[SHOES]="Boot";
        m[SHIELD]="Shield";
        m[HELM]="Helm";
        m[GLOVES]="Gauntlet";
        m[BOX]="Box";
        m[BIN]="Bin";
        m[ARMORSTAND]="Armor stand";
        m[WEAPONRACK]="Weapon rack";
        m[CABINET]="Cabinet";
        m[FIGURINE]="Figurine";
        m[AMULET]="Amulet";
        m[SCEPTER]="Scepter";
        m[AMMO]="Ammunition";
        m[CROWN]="Crown";
        m[RING]="Ring";
        m[EARRING]="Earring";
        m[BRACELET]="Bracelet";
        m[GEM]="Large gem";
        m[ANVIL]="Anvil";
        m[CORPSEPIECE]="Corpse bodypart";
        m[REMAINS]="Remain";
        m[MEAT]="Meat";
        m[FISH]="Fish";
        m[FISH_RAW]="Raw fish";
        m[VERMIN]="Vermin";
        m[IS_PET]="Pet";
        m[SEEDS]="Seed";
        m[PLANT]="Plant";
        m[SKIN_TANNED]="Tanned hide";
        m[LEAVES]="Leave";
        m[THREAD]="Thread";
        m[CLOTH]="Cloth";
        m[TOTEM]="Totem";
        m[PANTS]="Greaves";
        m[BACKPACK]="Backpack";
        m[QUIVER]="Quiver";
        m[CATAPULTPARTS]="Catapult part";
        m[BALLISTAPARTS]="Ballista part";
        m[SIEGEAMMO]="Siege ammunition";
        m[BALLISTAARROWHEAD]="Ballista ammunition";
        m[TRAPPARTS]="Mechanism";
        m[TRAPCOMP]="Trap component";
        m[DRINK]="Alcohol";
        m[POWDER_MISC]="Powder";
        m[CHEESE]="Cheese";
        m[FOOD]="Prepared food";
        m[LIQUID_MISC]="Syrup/milk/oil";
        m[COIN]="Coin";
        m[GLOB]="Fat";
        m[ROCK]="Rock";
        m[PIPE_SECTION]="Pipe";
        m[HATCH_COVER]="Hatch cover";
        m[GRATE]="Grate";
        m[QUERN]="Quern";
        m[MILLSTONE]="Millstone";
        m[SPLINT]="Splint";
        m[CRUTCH]="Crutch";
        m[TRACTION_BENCH]="Traction bench";
        m[ORTHOPEDIC_CAST]="Cast";
        m[TOOL]="Tool";
        m[SLAB]="Slab";
        m[EGG]="Egg";
        m[BOOK]="Book";
        m[SUPPLIES]="Other Equipment";
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

    void set_affection(int level);

    QList<Item*> contained_items() {return m_contained_items;}

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

};
#endif // ITEM_H
