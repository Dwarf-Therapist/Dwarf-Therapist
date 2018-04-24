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
#ifndef ITEMTOOLSUBTYPE_H
#define ITEMTOOLSUBTYPE_H

#include "itemsubtype.h"

class ItemToolSubtype : public ItemSubtype {
    Q_OBJECT
public:
    enum Flags {
        HARD_MAT = 0,
        METAL_MAT,
        HAS_EDGE_ATTACK,
        METAL_WEAPON_MAT,
        UNIMPROVABLE,
        SOFT_MAT,
        WOOD_MAT,
        INVERTED_TILE,
        FURNITURE,
        LEATHER_MAT,
        SILK_MAT,
        THREAD_PLANT_MAT,
        GLASS_MAT,
        CERAMIC_MAT,
        STONE_MAT,
        SHELL_MAT,
        BONE_MAT,
        NO_DEFAULT_JOB,
        INCOMPLETE_ITEM,
        SHEET_MAT,
    };

    ItemToolSubtype(DFInstance *df, VIRTADDR address, QObject *parent = 0);
};

#endif // ITEMTOOLSUBTYPE_H
