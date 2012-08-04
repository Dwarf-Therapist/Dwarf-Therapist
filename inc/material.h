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

    static const QString get_pref_desc(const PREF_TYPES &type) {
        QMap<PREF_TYPES, QString> desc;
        desc[LIKE_MATERIAL] = "Materials";
        desc[LIKE_CREATURE] = "Creatures";
        desc[LIKE_FOOD] = "Food & Drink";
        desc[HATE_CREATURE] = "Dislikes";
        desc[LIKE_ITEM] = "Items";
        desc[LIKE_PLANT] = "Plants";
        desc[LIKE_TREE] = "Trees";
        desc[LIKE_COLOR] = "Colors";
        desc[LIKE_SHAPE] = "Shapes";
        return desc.value(type, "N/A");
    }

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
