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
#ifndef MATERIAL_H
#define MATERIAL_H

#include <QtWidgets>
#include "utils.h"
#include "global_enums.h"
#include "flagarray.h"

class DFInstance;
class MemoryLayout;

class Material : public QObject {
    Q_OBJECT
public:
    Material(QObject *parent = 0);
    Material(DFInstance *df, VIRTADDR address, int index, bool inorganic = false, QObject *parent = 0);
    virtual ~Material();

    static Material* get_material(DFInstance *df, const VIRTADDR &address, int index, bool inorganic = false, QObject *parent = 0);

    //! Return the memory address (in hex) of this Material in the remote DF process
    VIRTADDR address() {return m_address;}

    QString get_material_name(MATERIAL_STATES state);
    int id() {return m_index;}

    void load_data();        
    bool is_inorganic() {return m_inorganic;}

    FlagArray flags() {return m_flags;}

    static const QString get_material_flag_desc(const MATERIAL_FLAGS &flag) {
        QMap<MATERIAL_FLAGS, QString> m;
        m[BONE]=tr("Bone");
        m[TOOTH]=tr("Tooth");
        m[HORN]=tr("Horn");
        m[PEARL]=tr("Pearl");
        m[SHELL]=tr("Shell");
        m[LEATHER]=tr("Leather");
        m[SILK]=tr("Silk");
        m[IS_GEM]=tr("Gems");
        m[IS_GLASS]=tr("Glass");
        m[IS_WOOD]=tr("Wood");
        m[IS_STONE]=tr("Stone");
        m[IS_METAL]=tr("Metal");
        m[ALCOHOL_PLANT]=tr("Plants (Alcohol)");
        m[THREAD_PLANT]=tr("Cloth");
        m[YARN]=tr("Yarn");
        return m.value(flag, "Missing Description");
    }

    static const QString get_mat_class_desc(const int mat_class){
        QMap<int, QString> m;
        m[1]=tr("Leather");
        m[2]=tr("Cloth");
        m[3]=tr("Wooden");
        m[5]=tr("Stone");
        m[14]=tr("Metal");
        m[16]=tr("Metal");
        m[17]=tr("Gem");
        m[18]=tr("Bone");
        m[19]=tr("Shell");
        m[20]=tr("Pearl");
        m[21]=tr("Tooth");
        m[22]=tr("Horn");
        m[27]=tr("Plant Fiber");
        m[28]=tr("Silk");
        m[29]=tr("Yarn");
        return m.value(mat_class, "???");
    }

private:
    int m_index;
    VIRTADDR m_address;
    VIRTADDR m_flag_address;
    DFInstance * m_df;
    MemoryLayout * m_mem;    
    FlagArray m_flags;
    bool m_inorganic;
    QHash<MATERIAL_STATES, QString> m_state_names;

    void read_material();
};

#endif // MATERIAL_H
