#ifndef MATERIAL_H
#define MATERIAL_H

#include <QtGui>
#include "utils.h"
#include "global_enums.h"

class DFInstance;
class MemoryLayout;

class Material : public QObject {
    Q_OBJECT
public:
    Material();
    Material(DFInstance *df, VIRTADDR address, int index, QObject *parent = 0);
    virtual ~Material();

    static Material* get_material(DFInstance *df, const VIRTADDR &address, int index);

    //! Return the memory address (in hex) of this Material in the remote DF process
    VIRTADDR address() {return m_address;}

    QString get_material_name(MATERIAL_STATES state);
    int id() {return m_index;}

    void load_data();    

private:
    int m_index;
    VIRTADDR m_address;
    DFInstance * m_df;
    MemoryLayout * m_mem;


    QHash<MATERIAL_STATES, QString> m_state_names;

    void read_material();
};

#endif // MATERIAL_H
