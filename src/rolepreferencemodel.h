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

class DFInstance;
class RolePreference;
class GenericRolePreference;
class ExactRolePreference;
class Preference;
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
    QModelIndex sibling(int row, int column, const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Returns nullptr for invalid index or categories.
    const RolePreference *getPreference(const QModelIndex &index) const;

    static constexpr auto SortRole = Qt::UserRole+0;

private:
    DFInstance *m_df = nullptr;

    struct node_t
    {
        const node_t *parent;
        std::size_t row;
        std::unique_ptr<GenericRolePreference> pref;
        std::vector<std::unique_ptr<node_t>> sub_categories;
        std::vector<std::shared_ptr<ExactRolePreference>> exact_prefs;
        std::size_t exact_pref_count = 0; // recursive and unique

        node_t(std::unique_ptr<GenericRolePreference> &&pref, node_t *parent = nullptr);
        ~node_t();

        template<typename T, typename... Args>
        node_t *add_category(Args &&... args);
    };
    std::vector<std::unique_ptr<node_t>> m_prefs;
    bool m_loaded_raws = false;

    template<typename T, typename... Args>
    node_t *add_top_category(Args &&... args);

    void add_material(Material *m);
    static bool add_exact_pref(std::vector<std::unique_ptr<node_t>> &categories,
                               const std::shared_ptr<ExactRolePreference> &role_pref,
                               const Preference &pref);
    static void clear_exact_prefs(std::vector<std::unique_ptr<node_t>> &categories);

    // Function must be callable with either
    //  a const node_t * if the index is a category
    //  and const ExactRolePreference * if the index is an exact preference
    template<typename Ret, typename Function>
    Ret apply_to_item(Function function, const QModelIndex &index) const;
};

#endif
