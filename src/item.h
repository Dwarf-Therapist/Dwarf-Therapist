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

#include "flagarray.h"
#include "global_enums.h"
#include "utils.h"

#include <QObject>
#include <QColor>

class DFInstance;
class ItemSubtype;
class Material;

class Item : public QObject {
    Q_OBJECT
public:
    Item(const Item &i);
    Item(DFInstance *df, VIRTADDR item_addr, QObject *parent = 0);
    Item(ITEM_TYPE itype,QString name, QObject *parent = 0);
    virtual ~Item();

    static const int MAX_AFFECTION = 10000;

    typedef enum {
        IS_MISSING = -2,
        IS_UNCOVERED = -1,
        IS_CLOTHED = 0,
        IS_WORN = 1,
        IS_THREADBARE = 2,
        IS_TATTERED = 3
    } ITEM_STATE;

    static const QString get_item_name_plural(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]=tr("N/A");
        m[BAR]=tr("Metals/Fuels/Soaps");
        m[SMALLGEM]=tr("Cut gemstones");
        m[BLOCKS]=tr("Blocks");
        m[ROUGH]=tr("Rough Gemstones");
        m[BOULDER]=tr("Boulders");
        m[WOOD]=tr("Woods");
        m[DOOR]=tr("Doors");
        m[FLOODGATE]=tr("Floodgates");
        m[BED]=tr("Beds");
        m[CHAIR]=tr("Chairs/Thrones");
        m[CHAIN]=tr("Chains");
        m[FLASK]=tr("Flasks");
        m[VIAL]=tr("Vials");
        m[WATERSKIN]=tr("Waterskins");
        m[GOBLET]=tr("Goblets");
        m[INSTRUMENT]=tr("Instruments");
        m[TOY]=tr("Toys");
        m[WINDOW]=tr("Windows");
        m[CAGE]=tr("Cages");
        m[BARREL]=tr("Barrels");
        m[BUCKET]=tr("Buckets");
        m[ANIMALTRAP]=tr("Animal Traps");
        m[TABLE]=tr("Tables");
        m[COFFIN]=tr("Coffins");
        m[STATUE]=tr("Statues");
        m[CORPSE]=tr("Corpses");
        m[WEAPON]=tr("Weapons");
        m[ARMOR]=tr("Armors (Chest)");
        m[SHOES]=tr("Armors (Feet)");
        m[SHIELD]=tr("Armors (Shields)");
        m[HELM]=tr("Armors (Head)");
        m[GLOVES]=tr("Armors (Hands)");
        m[BOX]=tr("Boxes");
        m[BAG]=tr("Bags");
        m[BIN]=tr("Bins");
        m[ARMORSTAND]=tr("Armor Stands");
        m[WEAPONRACK]=tr("Weapon Racks");
        m[CABINET]=tr("Cabinets");
        m[FIGURINE]=tr("Figurines");
        m[AMULET]=tr("Amulets");
        m[SCEPTER]=tr("Scepters");
        m[AMMO]=tr("Ammunitions");
        m[CROWN]=tr("Crowns");
        m[RING]=tr("Rings");
        m[EARRING]=tr("Earrings");
        m[BRACELET]=tr("Bracelets");
        m[GEM]=tr("Large Gems");
        m[ANVIL]=tr("Anvils");
        m[CORPSEPIECE]=tr("Corpse Bodyparts");
        m[REMAINS]=tr("Remains");
        m[MEAT]=tr("Meats");
        m[FISH]=tr("Fishes");
        m[FISH_RAW]=tr("Raw Fishes");
        m[VERMIN]=tr("Vermins");
        m[IS_PET]=tr("Pets");
        m[SEEDS]=tr("Seeds");
        m[PLANT]=tr("Plants");
        m[SKIN_TANNED]=tr("Tanned Hides");
        m[LEAVES_FRUIT]=tr("Leaves");
        m[THREAD]=tr("Threads");
        m[CLOTH]=tr("Cloths");
        m[TOTEM]=tr("Totems");
        m[PANTS]=tr("Armors (Legs)");
        m[BACKPACK]=tr("Backpacks");
        m[QUIVER]=tr("Quivers");
        m[CATAPULTPARTS]=tr("Catapult Parts");
        m[BALLISTAPARTS]=tr("Ballista Parts");
        m[SIEGEAMMO]=tr("Siege Ammunitions");
        m[BALLISTAARROWHEAD]=tr("Ballista Ammunitions");
        m[TRAPPARTS]=tr("Mechanisms");
        m[TRAPCOMP]=tr("Trap Components");
        m[DRINK]=tr("Alcohols");
        m[POWDER_MISC]=tr("Flours/Sugars/Powders");
        m[CHEESE]=tr("Cheeses");
        m[FOOD]=tr("Prepared Foods");
        m[LIQUID_MISC]=tr("Honeys/Syrups/Milks/Oils");
        m[COIN]=tr("Coins");
        m[GLOB]=tr("Fats");
        m[ROCK]=tr("Rocks");
        m[PIPE_SECTION]=tr("Pipes");
        m[HATCH_COVER]=tr("Hatch Covers");
        m[GRATE]=tr("Grates");
        m[QUERN]=tr("Querns");
        m[MILLSTONE]=tr("Millstones");
        m[SPLINT]=tr("Splints");
        m[CRUTCH]=tr("Crutches");
        m[TRACTION_BENCH]=tr("Traction Benches");
        m[ORTHOPEDIC_CAST]=tr("Casts");
        m[TOOL]=tr("Tools");
        m[SLAB]=tr("Slabs");
        m[EGG]=tr("Eggs");
        m[BOOK]=tr("Books");
        m[SHEET]=tr("Sheets");
        m[SUPPLIES]=tr("Supplies");
        m[MELEE_EQUIPMENT]=tr("Weapon & Shield");
        m[RANGED_EQUIPMENT]=tr("Quiver & Ammo");
        return m.value(type, "N/A");
    }

    static const QString get_item_clothing_name(const ITEM_TYPE &type){
        QMap<ITEM_TYPE,QString> m;
        m[ARMOR]=tr("Clothing (Chest)");
        m[SHOES]=tr("Clothing (Feet)");
        m[HELM]=tr("Clothing (Head)");
        m[GLOVES]=tr("Clothing (Hands)");
        m[PANTS]=tr("Clothing (Legs)");
        return m.value(type,get_item_name_plural(type));
    }

    static const QString get_item_generic_name(const ITEM_TYPE &type){
        QMap<ITEM_TYPE,QString> m;
        m[ARMOR]=tr("Armor/Clothing");
        m[SHOES]=tr("Footwear");
        m[HELM]=tr("Headwear");
        m[GLOVES]=tr("Handwear");
        m[PANTS]=tr("Legwear");
        return m.value(type, get_item_name_plural(type));
    }

    static const QString get_item_name(const ITEM_TYPE &type) {
        QMap<ITEM_TYPE, QString> m;
        m[NONE]=tr("N/A");
        m[BAR]=tr("Bar");
        m[SMALLGEM]=tr("Cut Gemstone");
        m[BLOCKS]=tr("Block");
        m[ROUGH]=tr("Rough Gemstone");
        m[BOULDER]=tr("Boulder");
        m[WOOD]=tr("Wood");
        m[DOOR]=tr("Door");
        m[FLOODGATE]=tr("Floodgate");
        m[BED]=tr("Bed");
        m[CHAIR]=tr("Chair");
        m[CHAIN]=tr("Chain");
        m[FLASK]=tr("Flask");
        m[VIAL]=tr("Vial");
        m[WATERSKIN]=tr("Waterskin");
        m[GOBLET]=tr("Goblet");
        m[INSTRUMENT]=tr("Instrument");
        m[TOY]=tr("Toy");
        m[WINDOW]=tr("Window");
        m[CAGE]=tr("Cage");
        m[BARREL]=tr("Barrel");
        m[BUCKET]=tr("Bucket");
        m[ANIMALTRAP]=tr("Animal Trap");
        m[TABLE]=tr("Table");
        m[COFFIN]=tr("Coffin");
        m[STATUE]=tr("Statue");
        m[CORPSE]=tr("Corpse");
        m[WEAPON]=tr("Weapon");
        m[ARMOR]=tr("Armor");
        m[SHOES]=tr("Boot");
        m[SHIELD]=tr("Shield");
        m[HELM]=tr("Helm");
        m[GLOVES]=tr("Gauntlet");
        m[BOX]=tr("Box");
        m[BIN]=tr("Bin");
        m[ARMORSTAND]=tr("Armor Stand");
        m[WEAPONRACK]=tr("Weapon Rack");
        m[CABINET]=tr("Cabinet");
        m[FIGURINE]=tr("Figurine");
        m[AMULET]=tr("Amulet");
        m[SCEPTER]=tr("Scepter");
        m[AMMO]=tr("Ammunition");
        m[CROWN]=tr("Crown");
        m[RING]=tr("Ring");
        m[EARRING]=tr("Earring");
        m[BRACELET]=tr("Bracelet");
        m[GEM]=tr("Large Gem");
        m[ANVIL]=tr("Anvil");
        m[CORPSEPIECE]=tr("Corpse Bodypart");
        m[REMAINS]=tr("Remain");
        m[MEAT]=tr("Meat");
        m[FISH]=tr("Fish");
        m[FISH_RAW]=tr("Raw Fish");
        m[VERMIN]=tr("Vermin");
        m[IS_PET]=tr("Pet");
        m[SEEDS]=tr("Seed");
        m[PLANT]=tr("Plant");
        m[SKIN_TANNED]=tr("Tanned Hide");
        m[LEAVES_FRUIT]=tr("Leaf");
        m[THREAD]=tr("Thread");
        m[CLOTH]=tr("Cloth");
        m[TOTEM]=tr("Totem");
        m[PANTS]=tr("Greave");
        m[BACKPACK]=tr("Backpack");
        m[QUIVER]=tr("Quiver");
        m[CATAPULTPARTS]=tr("Catapult Part");
        m[BALLISTAPARTS]=tr("Ballista Part");
        m[SIEGEAMMO]=tr("Siege Ammunition");
        m[BALLISTAARROWHEAD]=tr("Ballista Ammunition");
        m[TRAPPARTS]=tr("Mechanism");
        m[TRAPCOMP]=tr("Trap Component");
        m[DRINK]=tr("Alcohol");
        m[POWDER_MISC]=tr("Powder");
        m[CHEESE]=tr("Cheese");
        m[FOOD]=tr("Prepared Food");
        m[LIQUID_MISC]=tr("Syrup/Milk/Oil");
        m[COIN]=tr("Coin");
        m[GLOB]=tr("Fat");
        m[ROCK]=tr("Rock");
        m[PIPE_SECTION]=tr("Pipe");
        m[HATCH_COVER]=tr("Hatch Cover");
        m[GRATE]=tr("Grate");
        m[QUERN]=tr("Quern");
        m[MILLSTONE]=tr("Millstone");
        m[SPLINT]=tr("Splint");
        m[CRUTCH]=tr("Crutch");
        m[TRACTION_BENCH]=tr("Traction Bench");
        m[ORTHOPEDIC_CAST]=tr("Cast");
        m[TOOL]=tr("Tool");
        m[SLAB]=tr("Slab");
        m[EGG]=tr("Egg");
        m[BOOK]=tr("Book");
        m[SHEET]=tr("Sheet");
        m[SUPPLIES]=tr("Other Equipment");
        return m.value(type, "N/A");
    }

    static const QList<ITEM_TYPE> items_with_subtypes(){return m_items_subtypes;}

    static bool has_subtypes(const ITEM_TYPE &i_type){
        return m_items_subtypes.contains(i_type);
    }

    static bool is_trade_good(const ITEM_TYPE &i_type){
        if(i_type == BRACELET || i_type == RING || i_type == SCEPTER|| i_type == CROWN ||
                i_type == FIGURINE || i_type == AMULET || i_type == EARRING){
            return true;
        }else{
            return false;
        }
    }

    static bool is_armor_type(const ITEM_TYPE &i_type, const bool &include_shield = false){
        if(i_type == ARMOR || i_type == GLOVES || i_type == HELM || i_type == PANTS || i_type == SHOES ||
                (include_shield && i_type == SHIELD)){
            return true;
        }else{
            return false;
        }
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
    static const QString uncovered_group_name(){return tr("Uncovered");}
    //the category for missing uniform items
    static const QString missing_group_name() {return tr("Missing");}

    static const QColor color_clothed() {return QColor(69,148,21);}
    static const QColor color_missing() {return QColor(57,130,227);}
    static const QColor color_uncovered() {return QColor(227,22,18);}
    static const QColor color_wear() {return QColor(240,116,0,255);}
    static const QColor get_color(ITEM_STATE i_status){return m_state_colors.value(i_status,color_clothed());}

    VIRTADDR address() {return m_addr;}

    int id(){return m_id;}

    int wear(){return m_wear;}
    int quality(){return m_quality;}

    ITEM_TYPE item_type(){return m_iType;}
    void item_type(ITEM_TYPE newType){m_iType = newType;}

    int mat_index(){return m_mat_idx;}
    short mat_type(){return m_mat_type;}

    virtual short item_subtype() const {return -1;}
    virtual short melee_skill(){return -1;}
    virtual short ranged_skill(){return -1;}

    virtual ItemSubtype * get_subType() {return 0;}

    QString display_name(bool colored = false);
    bool equals(const Item &) const;

    void set_affection(int level);

    QList<Item*> contained_items() {return m_contained_items;}
    int get_stack_size(){return m_stack_size;}
    void add_to_stack(int num){
        m_stack_size+=num;
    }
    QString get_material_name(){return m_material_name;}
    QString get_material_name_base(){return m_material_name_base;}
    short get_quality(){return m_quality;}
    FlagArray mat_flags(){return m_material_flags;}
    QString item_name(bool plural, bool mat, bool generic_mat);

protected:
    DFInstance *m_df;
    VIRTADDR m_addr;
    ITEM_TYPE m_iType;
    short m_wear;
    short m_mat_type;
    int m_mat_idx;
    short m_quality;
    QString m_material_name;
    QString m_material_name_base;
    FlagArray m_material_flags;
    QString m_item_name;
    QString m_item_name_plural;
    QString m_layer_name;
    QString m_display_name;
    QColor m_color_display;
    int m_id;
    int m_affection;
    int m_stack_size;
    QString m_artifact_name;
    QString m_size_prefix;
    int m_maker_race;
    QList<Item*> m_contained_items;

    void read_data();
    void init_defaults();
    void set_name(ItemSubtype *sub);
    void set_default_name(Material *m);
    void build_display_name();
    QString get_quality_symbol();
    static QMap<Item::ITEM_STATE,QColor> set_state_colors();

private:
    static const QList<ITEM_TYPE> m_items_subtypes;
    static const QList<ITEM_TYPE> init_subtypes();
    static const QList<MATERIAL_FLAGS> m_mat_cats;
    static const QList<MATERIAL_FLAGS> init_mat_cats();
    static const QMap<ITEM_STATE,QColor> m_state_colors;

};
#endif // ITEM_H
