#ifndef PLANT_H
#define PLANT_H

#include <QObject>
#include "utils.h"
#include "flagarray.h"

class DFInstance;
class Material;
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

    int id() const {return m_index;}
    Material *get_plant_material(int index);
    QVector<Material*> get_plant_materials();
    int material_count();
    void load_data();

    const QString &name() const {return m_plant_name;}
    const QString &name_plural() const {return m_plant_name_plural;}
    const QString &leaf_plural() const {return m_leaf_name_plural;}
    const QString &seed_plural() const {return m_seed_name_plural;}

    const FlagArray &flags() const {return m_flags;}

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
