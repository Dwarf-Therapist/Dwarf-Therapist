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

class Item : public QObject {
    Q_OBJECT
public:

    static const QString get_item_desc(const ITEM_TYPE &type) {
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
        m[AMMO]="Ammo";
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
        m[SIEGEAMMO]="Siege ammo";
        m[BALLISTAARROWHEAD]="Ballista ammo";
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
        return m.value(type, "N/A");
    }
};
#endif // ITEM_H
