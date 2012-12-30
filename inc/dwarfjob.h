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
#ifndef DWARF_JOB_H
#define DWARF_JOB_H

#include <QtCore>
#include "truncatingfilelogger.h"

class DwarfJob : public QObject {
    Q_OBJECT
public:
    typedef enum {
        DJT_DEFAULT,
        DJT_IDLE,
        DJT_DIG,
        DJT_CUT,
        DJT_REST,
        DJT_DRINK,
        DJT_FOOD,
        DJT_BUILD,
        DJT_HAUL,
        DJT_ADMIN,
        DJT_FIGHT,
        DJT_MOOD,
        DJT_FORGE,
        DJT_MEDICAL,
        DJT_FURNACE,
        DJT_WAX_WORKING,
        DJT_BEE_KEEPING,
        DJT_PRESSING,
        DJT_SPINNING,
        DJT_POTTERY,
        DJT_STAIRS,
        DJT_FORTIFICATION,
        DJT_ENGRAVE,
        DJT_LEAF,
        DJT_BUILD_REMOVE,
        DJT_BAG_ADD,
        DJT_MONEY,
        DJT_RETURN,
        DJT_PARTY,
        DJT_SOAP,
        DJT_SEEK,
        DJT_GEM_CUT,
        DJT_GEM_ENCRUST,
        DJT_SEEDS,
        DJT_LEAF_ARROW,
        DJT_WATER_ARROW,
        DJT_TOMBSTONE,
        DJT_ANIMAL,
        DJT_BOOK_OPEN,
        DJT_HANDSHAKE,
        DJT_CONSTRUCT,
        DJT_ABACUS,
        DJT_REPORT,
        DJT_JUSTICE,
        DJT_SHIELD,
        DJT_DEPOT,
        DJT_BROOM,
        DJT_SWITCH,
        DJT_CHAIN,
        DJT_UNCHAIN,
        DJT_FILL_WATER,
        DJT_MARKET,
        DJT_KNIFE,
        DJT_BOW,
        DJT_MILK,
        DJT_CHEESE,
        DJT_GLOVE,
        DJT_BOOT,
        DJT_ARMOR,
        DJT_HELM,
        DJT_FISH,
        DJT_SLEEP,
        DJT_COOKING,
        DJT_BUCKET_POUR,
        DJT_GIVE_LOVE,
        DJT_DYE,
        DJT_WEAPON,
        DJT_SWITCH_CONNECT,
        DJT_ZONE_ADD,
        DJT_CRAFTS,
        DJT_GEAR,
        DJT_TROUBLE,
        DJT_STORAGE,
        DJT_BREW,
        DJT_RAW_FISH
    } DWARF_JOB_TYPE;

    static DWARF_JOB_TYPE get_type(const QString &type) {
        QMap<QString, DWARF_JOB_TYPE> m;
        m["idle"] = DJT_IDLE;
        m["dig"] = DJT_DIG;
        m["cut"] = DJT_CUT;
        m["sleep"] = DJT_SLEEP;
        m["drink"] = DJT_DRINK;
        m["food"] = DJT_FOOD;
        m["build"] = DJT_BUILD;
        m["haul"] = DJT_HAUL;
        m["admin"] = DJT_ADMIN;
        m["fight"] = DJT_FIGHT;
        m["mood"] = DJT_MOOD;
        m["forge"] = DJT_FORGE;
        m["medical"] = DJT_MEDICAL;
        m["furnace"] = DJT_FURNACE;
        m["wax_working"] = DJT_WAX_WORKING;
        m["bee_keeping"] = DJT_BEE_KEEPING;
        m["pressing"] = DJT_PRESSING;
        m["spinning"] = DJT_SPINNING;
        m["pottery"] = DJT_POTTERY;
        m["glazing"] = DJT_POTTERY;
        m["stair"] = DJT_STAIRS;
        m["fortification"] = DJT_FORTIFICATION;
        m["engrave"] = DJT_ENGRAVE;
        m["leaf"] = DJT_LEAF;
        m["build_remove"] = DJT_BUILD_REMOVE;
        m["bag_add"] = DJT_BAG_ADD;
        m["money"] = DJT_MONEY;
        m["return"] = DJT_RETURN;
        m["party"] = DJT_PARTY;
        m["soap"] = DJT_SOAP;
        m["seek"] = DJT_SEEK;
        m["gem_cut"] = DJT_GEM_CUT;
        m["gem_encrust"] = DJT_GEM_ENCRUST;
        m["seeds"] = DJT_SEEDS;
        m["harvest"] = DJT_LEAF_ARROW;
        m["give_water"] = DJT_WATER_ARROW;
        m["tombstone"] = DJT_TOMBSTONE;
        m["animal"] = DJT_ANIMAL;
        m["book"] = DJT_BOOK_OPEN;
        m["construct"] = DJT_CONSTRUCT;
        m["handshake"] = DJT_HANDSHAKE;
        m["abacus"] = DJT_ABACUS;
        m["report"] = DJT_REPORT;
        m["justice"] = DJT_JUSTICE;
        m["shield"] = DJT_SHIELD;
        m["depot"] = DJT_DEPOT;
        m["broom"] = DJT_BROOM;
        m["switch"] = DJT_SWITCH;
        m["chain"] = DJT_CHAIN;
        m["unchain"] = DJT_UNCHAIN;
        m["fill_water"] = DJT_FILL_WATER;
        m["market"] = DJT_MARKET;
        m["knife"] = DJT_KNIFE;
        m["bow"] = DJT_BOW;
        m["milk"] = DJT_MILK;
        m["cheese"] = DJT_CHEESE;
        m["glove"] = DJT_GLOVE;
        m["boot"] = DJT_BOOT;
        m["armor"] = DJT_ARMOR;
        m["helm"] = DJT_HELM;
        m["fish"] = DJT_FISH;
        m["rawfish"] = DJT_RAW_FISH;
        m["rest"] = DJT_REST;
        m["cooking"] = DJT_COOKING;
        m["bucket_pour"] = DJT_BUCKET_POUR;
        m["give_love"] = DJT_GIVE_LOVE;
        m["weapon"] = DJT_WEAPON;
        m["dye"] = DJT_DYE;
        m["switch_connect"] = DJT_SWITCH_CONNECT;
        m["zone_add"] = DJT_ZONE_ADD;
        m["crafts"] = DJT_CRAFTS;
        m["gear"] = DJT_GEAR;
        m["trouble"] = DJT_TROUBLE;
        m["storage"] = DJT_STORAGE;
        m["brew"] = DJT_BREW;
        return m.value(type.toLower(), DJT_DEFAULT);
    }

    DwarfJob(short id, QString description, DWARF_JOB_TYPE type, QString reactionClass, QObject *parent = 0)
        : QObject(parent)
        , id(id)
        , description(description)
        , type(type)
        , reactionClass(reactionClass)
    {}

    short id;
    QString description;
    DWARF_JOB_TYPE type;
    QString reactionClass;

};

#endif
