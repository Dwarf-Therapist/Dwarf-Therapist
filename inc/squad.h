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

class Dwarf;
class DFInstance;

class Squad : public QObject {
    Q_OBJECT
public:
    typedef enum {
        SSL_GROUND,
        SSL_ROOM,
        SSL_BARACKS
    } SQUAD_SLEEP_LOCATION;

    Squad(Dwarf *leader, DFInstance *df, QObject *parent = 0);
    virtual ~Squad();

    QString name() {return m_name;}
    QString generic_name() {return m_generic_name;}
    Dwarf *leader() {return m_leader;}
    QList<Dwarf *> members() {return m_members;}
    bool standing_down() {return m_standing_down;}
    bool carries_water() {return m_carries_water;}
    short carried_rations() {return m_carried_rations;}
    SQUAD_SLEEP_LOCATION sleep_location() {return m_sleep_location;}
    bool chases_opponents() {return m_chases_opponents;}
    bool harasses_animals() {return m_harasses_animals;}

    void add_member(Dwarf *d);

private:
    Dwarf* m_leader;
    QList<Dwarf*> m_members;
    Squad *m_parent_squad;
    QString m_name;
    QString m_generic_name;

    bool m_standing_down;
    short m_carried_rations; //0, 1 or 2
    bool m_carries_water;
    SQUAD_SLEEP_LOCATION m_sleep_location; // room, ground, or barracks
    bool m_chases_opponents;
    bool m_harasses_animals;
};

#endif