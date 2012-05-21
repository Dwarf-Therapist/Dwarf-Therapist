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
#ifndef RACES_H
#define RACES_H

#include <QtGui>
#include "utils.h"

class DFInstance;
class MemoryLayout;
class Caste;

class Race : public QObject {
    Q_OBJECT
public:
    Race(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Race();

    static Race* get_race(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this race in the remote DF process
    VIRTADDR address() {return m_address;}

    int race_id() {return m_id;}
    QString name() {return m_name;}
    QString plural_name() {return m_name_plural;}
    QString adjective() {return m_adjective;}
    QString description() {return m_description;}
    QString baby_name() {return m_baby_name;}
    QString baby_name_plural() {return m_baby_name_plural;}
    QString child_name() {return m_child_name;}
    QString child_name_plural() {return m_child_name_plural;}
    VIRTADDR pref_string_vector() {return m_pref_string_vector;}
    VIRTADDR pop_ratio_vector() {return m_pop_ratio_vector;}
    VIRTADDR castes_vector() {return m_castes_vector;}
    Caste *get_caste_by_id(int id) const {return m_castes.value(id, 0);}

    void load_data();

private:
    VIRTADDR m_address;
    VIRTADDR m_pref_string_vector;
    VIRTADDR m_pop_ratio_vector;
    VIRTADDR m_castes_vector;
    int m_id;
    QString m_name;
    QString m_name_plural;
    QString m_adjective;
    QString m_baby_name;
    QString m_baby_name_plural;
    QString m_child_name;
    QString m_child_name_plural;
    QString m_description;
    QMap<int, Caste*> m_castes;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_race();
};

#endif // RACES_H
