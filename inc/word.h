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
#ifndef WORD_H
#define WORD_H

#include <QtGui>
#include "utils.h"

class Dwarf;
class DFInstance;
class MemoryLayout;

class Word : public QObject {
    Q_OBJECT
public:
    Word(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    virtual ~Word();

    static Word* get_word(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this creature in the remote DF process
    VIRTADDR address() {return m_address;}

    QString base() {return m_base;}
    QString noun() {return m_noun;}
    QString plural_noun() {return m_plural_noun;}
    QString adjective() {return m_adjective;}
    QString verb() {return m_verb;}
    QString present_simple_verb() {return m_present_simple_verb;}
    QString past_simple_verb() {return m_past_simple_verb;}
    QString past_participle_verb() {return m_past_participle_verb;}
    QString present_participle_verb() {return m_present_participle_verb;}

    void refresh_data();

private:
    VIRTADDR m_address;
    QString m_base;
    QString m_noun;
    QString m_plural_noun;
    QString m_adjective;
    QString m_verb;
    QString m_present_simple_verb;
    QString m_past_simple_verb;
    QString m_past_participle_verb;
    QString m_present_participle_verb;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_members();
};

#endif
