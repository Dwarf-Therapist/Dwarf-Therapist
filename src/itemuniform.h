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
#ifndef ITEMUNIFORM
#define ITEMUNIFORM

#include "item.h"

class ItemSubtype;
class ItemDefUniform;

class ItemUniform : public Item {
        Q_OBJECT
public:

    ItemUniform(const Item &baseItem);

    ItemUniform(DFInstance *df, VIRTADDR item_addr);

    ItemUniform(DFInstance *df, ItemDefUniform *u, QObject *parent);

    virtual ~ItemUniform();

    short item_subtype() const;
    ItemSubtype * get_subType();

private:
    ItemSubtype *m_item_def;
    ItemDefUniform *m_uniform_def;

    void read_def();
};
#endif // ITEMUNIFORM

