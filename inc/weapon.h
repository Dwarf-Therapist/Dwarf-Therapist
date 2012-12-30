#ifndef WEAPON_H
#define WEAPON_H

#include <QtGui>
#include "utils.h"

class DFInstance;
class MemoryLayout;

class Weapon : public QObject {
    Q_OBJECT
public:
    Weapon(DFInstance *df, VIRTADDR address, QObject *parent = 0);
    Weapon(const Weapon &w);
    virtual ~Weapon();

    static Weapon* get_weapon(DFInstance *df, const VIRTADDR &address);

    //! Return the memory address (in hex) of this weapon in the remote DF process
    VIRTADDR address() {return m_address;}

    QString name_plural() const {return m_name_plural;}
    void name_plural(const QString& new_name) {m_name_plural = new_name;}

    bool is_ranged() {return m_ammo.isEmpty() ? false : true;}

    int single_grasp() {return m_single_grasp_size;}
    int multi_grasp() {return  m_multi_grasp_size;}
//    int melee_skill() {return m_melee_skill_id;}
//    int ranged_skill() {return m_ranged_skill_id;}

    void set_name(QString new_name){m_name_plural = new_name;}

    void load_data();

    QString group_name;

private:
    VIRTADDR m_address;

    QString m_name_plural;
    int m_single_grasp_size;
    int m_multi_grasp_size;
    QString m_ammo;
//    int m_melee_skill_id;
//    int m_ranged_skill_id;

    DFInstance * m_df;
    MemoryLayout * m_mem;

    void read_weapon();
};

#endif // WEAPON_H
