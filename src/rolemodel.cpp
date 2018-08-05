#include "rolemodel.h"

#include <functional>

#include "defaultroleweight.h"
#include "gamedatareader.h"
#include "item.h"
#include "preference.h"
#include "role.h"
#include "rolepreference.h"
#include "standardpaths.h"

struct RoleModel::AspectInfo
{
    QString category_name;
    Role::weight_info Role::* global_weight;
    float default_global_weight;

    virtual QString get_name(const Role *, int) const = 0;
    virtual QString get_tip(const Role *, int) const = 0;
    virtual Role::aspect_weight &get_weight(Role *, int) const = 0;
    virtual const Role::aspect_weight &get_weight(const Role *, int) const = 0;
    virtual void erase(Role *, int) const = 0;
    virtual std::size_t size(Role *) const = 0;
};

template<typename T>
struct RoleModel::AspectVectorInfo: RoleModel::AspectInfo
{
    std::vector<std::pair<T, Role::aspect_weight>> Role::* vector;

    std::function<QString (const T &)> get_name_fn;
    std::function<QString (const T &)> get_tip_fn;

    Role::aspect_weight &get_weight(Role *r, int index) const override
    {
        return (r->*vector)[index].second;
    }
    const Role::aspect_weight &get_weight(const Role *r, int index) const override
    {
        return (r->*vector)[index].second;
    }
    void erase(Role *r, int index) const override
    {
        (r->*vector).erase((r->*vector).begin() + index);
    }
    std::size_t size(Role *r) const override
    {
        return (r->*vector).size();
    }
    QString get_name(const Role *r, int index) const override
    {
        return get_name_fn((r->*vector)[index].first);
    }
    QString get_tip(const Role *r, int index) const override
    {
        return get_tip_fn((r->*vector)[index].first);
    }
};

static bool is_valid_row(int row)
{
    return row >= 0 && row < RoleModel::RowCount;
}

RoleModel::RoleModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_role(nullptr)
{
    auto make_aspect_info = [] (
            const QString &category_name,
            Role::weight_info Role::* global_weight,
            auto Role:: *vector,
            auto get_name,
            auto get_tip)
        {
            using id_type = typename std::remove_reference<decltype(std::declval<Role>().*vector)>::type::value_type::first_type;
            auto info = std::make_unique<AspectVectorInfo<id_type>>();
            info->category_name = category_name;
            info->global_weight = global_weight;
            info->vector = vector;
            info->get_name_fn = get_name;
            info->get_tip_fn = get_tip;
            return info;
        };

    m_info[Attributes] = make_aspect_info(
            tr("Attributes"),
            &Role::attributes_weight,
            &Role::attributes,
            [] (const QString &id) { return id; },
            [] (const QString &id) { return RoleModel::tr("Attribute: %1").arg(id); });
    m_info[Skills] = make_aspect_info(
            tr("Skills"),
            &Role::skills_weight,
            &Role::skills,
            [] (int id) { return GameDataReader::ptr()->get_skill_name(id); },
            [] (int id) {
                return RoleModel::tr("Skill: %1 (ID %2)")
                        .arg(GameDataReader::ptr()->get_skill_name(id))
                        .arg(id);
            });
    m_info[Facets] = make_aspect_info(
            tr("Facets"),
            &Role::facets_weight,
            &Role::facets,
            [] (int id) { return GameDataReader::ptr()->get_trait_name(id); },
            [] (int id) {
                return RoleModel::tr("Facet: %1 (ID %2)")
                        .arg(GameDataReader::ptr()->get_trait_name(id))
                        .arg(id);
            });
    m_info[Beliefs] = make_aspect_info(
            tr("Beliefs"),
            &Role::beliefs_weight,
            &Role::beliefs,
            [] (int id) { return GameDataReader::ptr()->get_belief_name(id); },
            [] (int id) {
                return RoleModel::tr("Belief: %1 (ID %2)")
                    .arg(GameDataReader::ptr()->get_belief_name(id))
                    .arg(id);
            });
    m_info[Goals] = make_aspect_info(
            tr("Goals"),
            &Role::goals_weight,
            &Role::goals,
            [] (int id) { return GameDataReader::ptr()->get_goal_name(id); },
            [] (int id) {
                return RoleModel::tr("Goal: %1 (ID %2)")
                        .arg(GameDataReader::ptr()->get_goal_name(id))
                        .arg(id);
            });
    m_info[Needs] = make_aspect_info(
            tr("Needs"),
            &Role::needs_weight,
            &Role::needs,
            [] (int id) { return GameDataReader::ptr()->get_need_name(id); },
            [] (int id) {
                return RoleModel::tr("Need: %1 (ID %2)")
                        .arg(GameDataReader::ptr()->get_need_name(id))
                        .arg(id);
            });
    m_info[Preferences] = make_aspect_info(
            tr("Preferences"),
            &Role::prefs_weight,
            &Role::prefs,
            [] (const std::unique_ptr<RolePreference> &pref) { return pref->get_name(); },
            [] (const std::unique_ptr<RolePreference> &pref) {
                if (auto ip = dynamic_cast<const ItemRolePreference *>(pref.get()))
                    return RoleModel::tr("%1 preference: %2 (%3)")
                            .arg(Preference::get_pref_desc(pref->get_pref_category()))
                            .arg(pref->get_name())
                            .arg(Item::get_item_name_plural(ip->get_item_type()));
                else
                    return RoleModel::tr("%1 preference: %2")
                            .arg(Preference::get_pref_desc(pref->get_pref_category()))
                            .arg(pref->get_name());
            });
}

RoleModel::~RoleModel()
{
}

void RoleModel::set_role(Role *role)
{
    beginResetModel();
    m_role = role;
    endResetModel();
}

QModelIndex RoleModel::add_attribute(const QString &attribute)
{
    QModelIndex parent = index(Attributes, 0);
    auto pos = m_role->attributes.size();
    beginInsertRows(parent, pos, pos);
    m_role->attributes.emplace_back(attribute, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Attributes);
}

QModelIndex RoleModel::add_skill(int id)
{
    QModelIndex parent = index(Skills, 0);
    auto pos = m_role->skills.size();
    beginInsertRows(parent, pos, pos);
    m_role->skills.emplace_back(id, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Skills);
}

QModelIndex RoleModel::add_facet(int id)
{
    QModelIndex parent = index(Facets, 0);
    auto pos = m_role->facets.size();
    beginInsertRows(parent, pos, pos);
    m_role->facets.emplace_back(id, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Facets);
}

QModelIndex RoleModel::add_belief(int id)
{
    QModelIndex parent = index(Beliefs, 0);
    auto pos = m_role->beliefs.size();
    beginInsertRows(parent, pos, pos);
    m_role->beliefs.emplace_back(id, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Beliefs);
}

QModelIndex RoleModel::add_goal(int id)
{
    QModelIndex parent = index(Goals, 0);
    auto pos = m_role->goals.size();
    beginInsertRows(parent, pos, pos);
    m_role->goals.emplace_back(id, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Goals);
}

QModelIndex RoleModel::add_need(int id)
{
    QModelIndex parent = index(Needs, 0);
    auto pos = m_role->needs.size();
    beginInsertRows(parent, pos, pos);
    m_role->needs.emplace_back(id, Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(pos, 0, Needs);
}

QModelIndex RoleModel::add_preference(const RolePreference *pref)
{
    QModelIndex parent = index(Preferences, 0);
    beginInsertRows(parent, m_role->prefs.size(), m_role->prefs.size());
    m_role->prefs.emplace_back(pref->copy(), Role::aspect_weight(1.0, false));
    endInsertRows();
    return createIndex(m_role->prefs.size()-1, 0, Preferences);
}

void RoleModel::remove_item(const QModelIndex &index)
{
    Q_ASSERT(index.isValid() && index.internalId() != RowCount); // root items cannot be removed
    if (is_valid_row(index.internalId())) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        m_info[index.internalId()]->erase(m_role, index.row());
        endRemoveRows();
    }
}

void RoleModel::reset_default_weight(const QModelIndex &index)
{
    Q_ASSERT(index.isValid() && index.internalId() == RowCount && is_valid_row(index.row())); // only for root items
    (m_role->*(m_info[index.row()]->global_weight)).reset_to_default();
    auto weight_cell = createIndex(index.row(), ColumnWeight, RowCount);
    emit dataChanged(weight_cell, weight_cell);
}

// QAbstractItemModel implementation
QModelIndex RoleModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
        return createIndex(row, column, parent.row());
    else
        return createIndex(row, column, RowCount);
}

QModelIndex RoleModel::parent(const QModelIndex &index) const
{
    if (index.internalId() == RowCount)
        return QModelIndex();
    else
        return createIndex(index.internalId(), 0, RowCount);
}

int RoleModel::rowCount(const QModelIndex &parent) const
{
    if (!m_role)
        return 0;

    if (!parent.isValid()) // root
        return RowCount;

    if (parent.internalId() != RowCount) // leaf
        return 0;

    if (is_valid_row(parent.row()))
        return m_info[parent.row()]->size(m_role);

    return 0;
}

int RoleModel::columnCount(const QModelIndex &) const
{
    return ColumnCount;
}

QVariant RoleModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_role)
        return QVariant();

    if (index.internalId() == RowCount) { // Root item
        if (!is_valid_row(index.row()))
            return QVariant();
        const auto &info = m_info[index.row()];
        switch (index.column()) {
        case ColumnName:
            if (role == Qt::DisplayRole)
                return info->category_name;
            else
                return QVariant();
        case ColumnWeight: {
            const auto &w = m_role->*(info->global_weight);
            switch (role) {
            case Qt::DisplayRole:
                return w.is_default()
                    ? tr("default (%1)").arg(QString::number(w.weight()))
                    : QString::number(w.weight());
            case Qt::EditRole:
                return w.is_default() ? 0.0 : w.weight();
            case Qt::ToolTipRole:
                return tr("%1 weight").arg(info->category_name);
            case Qt::StatusTipRole:
                return tr("This weight is the importance of %1 relative to "
                          "other categories. Set to 0 to use the default "
                          "value (currently %2). Double-click to change.")
                    .arg(info->category_name)
                    .arg(w.default_weight());
            default:
                return QVariant();
            }
        }
        default:
            return QVariant();
        }
    }
    else if (is_valid_row(index.internalId())) { // leaf item
        const auto &info = m_info[index.internalId()];
        if (index.row() >= (int) info->size(m_role))
            return QVariant();
        switch (index.column()) {
        case ColumnName:
            switch (role) {
            case Qt::DisplayRole:
                return info->get_name(m_role, index.row());
            case Qt::ToolTipRole:
            case Qt::StatusTipRole:
                return info->get_tip(m_role, index.row());
            default:
                return QVariant();
            }
        case ColumnWeight: {
            const auto &w = info->get_weight(m_role, index.row());
            float value = w.is_neg ? -w.weight : w.weight;
            switch (role) {
            case Qt::DisplayRole:
                return QString::number(value);
            case Qt::EditRole:
                return (double) value;
            case Qt::ToolTipRole:
                return tr("%1 weight").arg(info->get_name(m_role, index.row()));
            case Qt::StatusTipRole:
                return tr("This weight is the importance of %1 relative to "
                          "other %2. Double-click to change.")
                    .arg(info->get_name(m_role, index.row()))
                    .arg(info->category_name);
            default:
                return QVariant();
            }
        }
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool RoleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.column() != ColumnWeight || role != Qt::EditRole || !m_role)
        return false;

    bool ok;
    float weight = value.toFloat(&ok);
    if (!ok)
        return false;

    if (index.internalId() == RowCount) { // global weights
        if (is_valid_row(index.row())) {
            auto &w = m_role->*(m_info[index.row()]->global_weight);
            if (weight == 0.0f)
                w.reset_to_default();
            else
                w.set(weight);
            emit dataChanged(index, index);
            return true;
        }
    }
    else { // item weights
        if (is_valid_row(index.internalId())) {
            auto &w = m_info[index.internalId()]->get_weight(m_role, index.row());
            w.weight = fabs(weight);
            w.is_neg = weight < 0.0f;
            emit dataChanged(index, index);
            return true;
        }
    }
    return false;
}

Qt::ItemFlags RoleModel::flags(const QModelIndex &index) const
{
    auto flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;
    if (index.internalId() == RowCount) { // root items
        if (index.column() == ColumnWeight)
            flags |= Qt::ItemIsEditable;
    }
    else { // leaf items
        flags |= Qt::ItemNeverHasChildren;
        if (index.column() == ColumnWeight)
            flags |= Qt::ItemIsEditable;
    }
    return flags;
}

QVariant RoleModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();
    switch (section) {
    case ColumnName:
        return tr("Name");
    case ColumnWeight:
        return tr("Weight");
    default:
        return QVariant();
    }
}

QModelIndex RoleModel::buddy(const QModelIndex &index) const
{
    return sibling(index.row(), ColumnWeight, index);
}
