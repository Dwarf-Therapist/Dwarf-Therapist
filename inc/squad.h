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
#ifndef SQUAD_H
#define SQUAD_H

#include <QtGui>
#include "utils.h"

class Dwarf;
class DFInstance;
class MemoryLayout;

class Squad : public QObject {
    Q_OBJECT
public:
    Squad(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Squad();

    static Squad* get_squad(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}
    int id() {return m_id;}
    QString name() {return m_name;}
    QVector<Dwarf *> members() {return m_members;}
    void refresh_data();

private:
    VIRTADDR m_address;
    int m_id;
    QString m_name;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    QVector<Dwarf *> m_members;

    void read_id();
    void read_name();
    void read_members();

    QString word_chunk(uint word, bool use_generic=true);
    QString read_chunked_name(const VIRTADDR &addr, bool use_generic=true);
};

#endif
