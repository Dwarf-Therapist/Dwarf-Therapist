/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert

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
#ifndef CASTE_H
#define CASTE_H

#include <QtGui>
#include "utils.h"
//#include "attribute.h"

class DFInstance;
class MemoryLayout;
class Attribute;
class AttributeLevel;
class FlagArray;

class Caste : public QObject {
    Q_OBJECT
public:
    Caste(DFInstance *df, VIRTADDR address, QString race_name, QObject *parent = 0);
    virtual ~Caste();

    static Caste* get_caste(DFInstance *df, const VIRTADDR &address, QString race_name);

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    QString name() {return m_name;}
    QString description() {return m_description;}
    AttributeLevel get_attribute_level(int id, int value);
    AttributeLevel get_attribute_rating(int attribute);

    int adult_size() {return get_body_size(0);}
    int child_size() {return get_body_size(0) / 2;} //get_body_size(1);}
    int baby_size() {return get_body_size(0) / 4;} //get_body_size(2);}

    void load_data();
    void load_attribute_info();

    FlagArray* flags() {return m_flags;}

    bool is_trainable();
    bool is_milkable();
    bool has_extracts() {return m_has_extracts;}

private:
    VIRTADDR m_address;
    QString m_race_name;
    QString m_tag;
    QString m_name;
    QString m_description;    
    QVector<int> m_body_sizes;

    struct att_range{
        QList<int> raw_bins;
        QList<int> display_bins;        
    };

    QHash<int,att_range> m_ranges;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    FlagArray *m_flags;

    bool m_has_extracts;

    void read_caste();
    int get_body_size(int index);
};

#endif // CASTE_H
