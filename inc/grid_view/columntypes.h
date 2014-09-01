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
#ifndef COLUMN_TYPES_H
#define COLUMN_TYPES_H

#include <QString>

typedef enum {
    CT_DEFAULT,
    CT_SPACER,
    CT_SKILL,
    CT_LABOR,
    CT_HAPPINESS,
    CT_IDLE,
    CT_TRAIT,
    CT_ATTRIBUTE,    
    CT_FLAGS,
    CT_ROLE,
    CT_WEAPON,
    CT_PROFESSION,
    CT_HIGHEST_MOOD,
    CT_TRAINED,
    CT_HEALTH,
    CT_EQUIPMENT,
    CT_ITEMTYPE,
    CT_SUPER_LABOR,
    CT_CUSTOM_PROFESSION,
    CT_BELIEF,
    CT_TOTAL_TYPES
} COLUMN_TYPE;

static inline COLUMN_TYPE get_column_type(const QString &name) {
    if (name.toLower() == "spacer" || name.toLower() == "space") {
        return CT_SPACER;
    } else if (name.toLower() == "labor") {
        return CT_LABOR;
    } else if (name.toLower() == "skill") {
        return CT_SKILL;
    } else if (name.toLower() == "happiness") {
        return CT_HAPPINESS;
    } else if (name.toLower() == "idle") {
        return CT_IDLE;
    } else if (name.toLower() == "trait") {
        return CT_TRAIT;
    } else if (name.toLower() == "attribute") {
        return CT_ATTRIBUTE;    
    } else if (name.toLower() == "flags") {
        return CT_FLAGS;
    } else if (name.toLower() == "role"){
        return CT_ROLE;
    } else if (name.toLower() == "weapon"){
        return CT_WEAPON;
    } else if (name.toLower() == "profession"){
        return CT_PROFESSION;
    } else if (name.toLower() == "mood_skill"){
        return CT_HIGHEST_MOOD;
    } else if (name.toLower() == "trained"){
        return CT_TRAINED;
    } else if (name.toLower() == "health"){
        return CT_HEALTH;
    }else if (name.toLower() == "equipment"){
        return CT_EQUIPMENT;
    }else if (name.toLower() == "itemtype"){
        return CT_ITEMTYPE;
    }else if (name.toLower() == "super_labor"){
        return CT_SUPER_LABOR;
    }else if (name.toLower() == "custom_profession"){
        return CT_CUSTOM_PROFESSION;
    }else if (name.toLower() == "belief")
        return CT_BELIEF;

    return CT_DEFAULT;
}

static inline QString get_column_type(const COLUMN_TYPE &type) {
    switch (type) {
    case CT_SPACER:                 return "SPACER";
    case CT_SKILL:                  return "SKILL";
    case CT_LABOR:                  return "LABOR";
    case CT_HAPPINESS:              return "HAPPINESS";
    case CT_IDLE:                   return "IDLE";
    case CT_TRAIT:                  return "TRAIT";
    case CT_ATTRIBUTE:              return "ATTRIBUTE";    
    case CT_FLAGS:                  return "FLAGS";
    case CT_ROLE:                   return "ROLE";
    case CT_WEAPON:                 return "WEAPON";
    case CT_PROFESSION:             return "PROFESSION";
    case CT_HIGHEST_MOOD:           return "MOOD_SKILL";
    case CT_TRAINED:                return "TRAINED";
    case CT_HEALTH:                 return "HEALTH";
    case CT_EQUIPMENT:           return "EQUIPMENT";
    case CT_ITEMTYPE:               return "ITEMTYPE";
    case CT_SUPER_LABOR:        return "SUPER_LABOR";
    case CT_CUSTOM_PROFESSION:        return "CUSTOM_PROFESSION";
    case CT_BELIEF:                 return "BELIEF";
    default:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

#endif

