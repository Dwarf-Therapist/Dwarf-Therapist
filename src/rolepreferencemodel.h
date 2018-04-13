/*
Dwarf Therapist
Copyright (c) 2018 Clément Vuchener

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
#ifndef ROLE_PREFERENCE_MODEL_H
#define ROLE_PREFERENCE_MODEL_H

#include <QAbstractItemModel>
#include <memory>
#include <vector>
#include <array>

class DFInstance;
class RolePreference;
class Material;

class RolePreferenceModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    RolePreferenceModel(QObject *parent = nullptr);
    virtual ~RolePreferenceModel();

    void set_df_instance(DFInstance *df = nullptr);

    // Load preference from raws while displaying a progress dialog child of parent
    void load_pref_from_raws(QWidget *parent);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Returns nullptr for invalid index or categories.
    const RolePreference *getPreference(const QModelIndex &index) const;

private:
    DFInstance *m_df = nullptr;

    enum GenericCategory {
        ITEM,
        EQUIPMENT,
        MATERIAL,
        CREATURE,
        TRADE_GOOD,
        PLANT_TREE,
        OTHER,
        GENERAL_CATEGORY_COUNT
    };
    static QString get_category_name(GenericCategory category);

    enum ExactCategory {
        GEMS,
        GLASS,
        METALS,
        STONE,
        WOOD,
        GLAZES_WARES,
        FABRICS,
        PAPERS,
        LEATHERS,
        PARCHMENTS,
        OTHER_MATERIALS,
        PLANTS,
        PLANTS_ALCOHOL,
        PLANTS_CROPS,
        PLANTS_CROPS_PLANTABLE,
        PLANTS_MILL,
        PLANTS_EXTRACT,
        TREES,
        CREATURES,
        CREATURES_HATEABLE,
        CREATURES_TRAINABLE,
        CREATURES_MILKABLE,
        CREATURES_EXTRACTS,
        CREATURES_EXTRACTS_FISH,
        CREATURES_FISHABLE,
        CREATURES_SHEARABLE,
        CREATURES_BUTCHER,
        CREATURES_DOMESTIC,
        WEAPONS_MELEE,
        WEAPONS_RANGED,
        EXACT_CATEGORY_COUNT
    };
    static QString get_category_name(ExactCategory category);

    std::array<std::vector<std::unique_ptr<RolePreference>>, GENERAL_CATEGORY_COUNT> m_general_prefs;
    std::array<std::vector<std::shared_ptr<RolePreference>>, EXACT_CATEGORY_COUNT> m_raw_prefs;
    std::vector<std::pair<QString, std::vector<std::shared_ptr<RolePreference>>>> m_item_prefs;
    bool m_loaded_raws = false;

    void load_material_prefs(QVector<Material*> mats);

    template<typename Ret, typename Function>
    Ret select_category(std::size_t index, Function function) const
    {
        if (index < m_general_prefs.size())
            return function(m_general_prefs[index]);
        else if ((index -= m_general_prefs.size()) < m_raw_prefs.size())
            return function(m_raw_prefs[index]);
        else if ((index -= m_raw_prefs.size()) < m_item_prefs.size())
            return function(m_item_prefs[index].second);
        else
            return Ret();
    }

    QString get_category_name(std::size_t index) const;
};

#endif
