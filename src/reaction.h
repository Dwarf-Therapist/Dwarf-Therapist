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
#ifndef REACTION_H
#define REACTION_H

#include <QObject>
#include "utils.h"

class DFInstance;
class MemoryLayout;

class Reaction : public QObject {
    Q_OBJECT
public:
    Reaction(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Reaction();

    static Reaction* get_reaction(DFInstance *df, const VIRTADDR &address);

    VIRTADDR address() {return m_address;}

    QString name() {return m_name;}
    short skill_id() {return m_skill_id;}
    QString tag() {return m_tag;}

    void load_data();

private:
    VIRTADDR m_address;
    QString m_tag;
    QString m_name;
    short m_skill_id;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_reaction();
};

#endif // REACTION_H
