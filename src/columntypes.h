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
#include <QHash>

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
    CT_KILLS,
    CT_PREFERENCE,
    CT_NEED,
    CT_TOTAL_TYPES
} COLUMN_TYPE;

static QHash<QString, COLUMN_TYPE> column_types;

static inline void init_column_types() {
    column_types.insert("SPACER",            CT_SPACER);
    column_types.insert("SKILL",             CT_SKILL);
    column_types.insert("LABOR",             CT_LABOR);
    column_types.insert("HAPPINESS",         CT_HAPPINESS);
    column_types.insert("IDLE",              CT_IDLE);
    column_types.insert("TRAIT",             CT_TRAIT);
    column_types.insert("ATTRIBUTE",         CT_ATTRIBUTE);
    column_types.insert("FLAGS",             CT_FLAGS);
    column_types.insert("ROLE",              CT_ROLE);
    column_types.insert("WEAPON",            CT_WEAPON);
    column_types.insert("PROFESSION",        CT_PROFESSION);
    column_types.insert("MOOD_SKILL",        CT_HIGHEST_MOOD);
    column_types.insert("TRAINED",           CT_TRAINED);
    column_types.insert("HEALTH",            CT_HEALTH);
    column_types.insert("EQUIPMENT",         CT_EQUIPMENT);
    column_types.insert("ITEMTYPE",          CT_ITEMTYPE);
    column_types.insert("SUPER_LABOR",       CT_SUPER_LABOR);
    column_types.insert("CUSTOM_PROFESSION", CT_CUSTOM_PROFESSION);
    column_types.insert("BELIEF",            CT_BELIEF);
    column_types.insert("KILLS",             CT_KILLS);
    column_types.insert("PREFERENCE",        CT_PREFERENCE);
    column_types.insert("NEED",              CT_NEED);
}

static inline COLUMN_TYPE get_column_type(const QString &name) {
    if (column_types.empty()) {
        init_column_types();
    }

    return column_types.value(name, CT_DEFAULT);
}

static inline QString get_column_type(const COLUMN_TYPE &type) {
    if (column_types.empty()) {
        init_column_types();
    }

    return column_types.key(type, "UNKNOWN");
}

#endif
