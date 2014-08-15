#ifndef PLANT_H
#define PLANT_H

#include <QObject>
#include "utils.h"
#include "material.h"
#include "flagarray.h"

class DFInstance;
class MemoryLayout;

class Plant : public QObject {
    Q_OBJECT
public:
    Plant(QObject *parent = 0);
    Plant(DFInstance *df, VIRTADDR address, int index, QObject *parent = 0);
    virtual ~Plant();

    static Plant* get_plant(DFInstance *df, const VIRTADDR &address, int index);

    //! Return the memory address (in hex) of this Plant in the remote DF process
    VIRTADDR address() {return m_address;}

    int id() {return m_index;}
    Material *get_plant_material(int index);
    QVector<Material*> get_plant_materials();
    int material_count();
    void load_data();

    QString name() {return m_plant_name;}
    QString name_plural() {return m_plant_name_plural;}
    QString leaf_plural() {return m_leaf_name_plural;}
    QString seed_plural() {return m_seed_name_plural;}

    FlagArray flags() {return m_flags;}

private:
    int m_index;
    VIRTADDR m_address;
    DFInstance * m_df;
    MemoryLayout * m_mem;
    QVector<Material*> m_plant_mats;
    FlagArray m_flags;

    QString m_plant_name;
    QString m_plant_name_plural;
    QString m_leaf_name_plural;
    QString m_seed_name_plural;

    void read_plant();
    void load_materials();
};

#endif // PLANT_H
