#include "rolemodel.h"

#include "gamedatareader.h"
#include "item.h"
#include "preference.h"
#include "role.h"
#include "rolepreference.h"
#include "standardpaths.h"

RoleModel::RoleModel(QObject *parent)
    : QAbstractItemModel(parent)
    , m_role(nullptr)
{
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
    // TODO: improve position search
    auto it = m_role->attributes.lower_bound(attribute);
    auto pos = std::distance(m_role->attributes.begin(), it);
    beginInsertRows(parent, pos, pos);
    m_role->attributes.emplace_hint(it, attribute, RoleAspect { 1.0, false });
    endInsertRows();
    return createIndex(pos, 0, Attributes);
}

QModelIndex RoleModel::add_skill(int id)
{
    QModelIndex parent = index(Skills, 0);
    // TODO: improve position search
    auto it = m_role->skills.lower_bound(QString::number(id));
    auto pos = std::distance(m_role->skills.begin(), it);
    beginInsertRows(parent, pos, pos);
    m_role->skills.emplace_hint(it, QString::number(id), RoleAspect { 1.0, false });
    endInsertRows();
    return createIndex(pos, 0, Skills);
}

QModelIndex RoleModel::add_facet(int id)
{
    QModelIndex parent = index(Facets, 0);
    // TODO: improve position search
    auto it = m_role->traits.lower_bound(QString::number(id));
    auto pos = std::distance(m_role->traits.begin(), it);
    beginInsertRows(parent, pos, pos);
    m_role->traits.emplace_hint(it, QString::number(id), RoleAspect { 1.0, false });
    endInsertRows();
    return createIndex(pos, 0, Facets);
}

QModelIndex RoleModel::add_preference(const RolePreference *pref)
{
    QModelIndex parent = index(Preferences, 0);
    beginInsertRows(parent, m_role->prefs.size(), m_role->prefs.size());
    m_role->prefs.emplace_back(pref->copy());
    endInsertRows();
    return createIndex(m_role->prefs.size()-1, 0, Preferences);
}

template<typename Key, typename T>
static void map_erase_pos(std::map<Key, T> &map, int pos)
{
    auto it = map.begin();
    std::advance(it, pos);
    map.erase(it);
}

void RoleModel::remove_item(const QModelIndex &index)
{
    Q_ASSERT(index.isValid() && index.internalId() != RowCount); // root items cannot be removed
    beginRemoveRows(index.parent(), index.row(), index.row());
    switch (index.internalId()) {
    case Attributes:
        map_erase_pos(m_role->attributes, index.row());
        break;
    case Skills:
        map_erase_pos(m_role->skills, index.row());
        break;
    case Facets:
        map_erase_pos(m_role->traits, index.row());
        break;
    case Preferences:
        m_role->prefs.erase(m_role->prefs.begin() + index.row());
        break;
    }
    endRemoveRows();
}

void RoleModel::reset_default_weight(const QModelIndex &index)
{
    Q_ASSERT(index.isValid() && index.internalId() == RowCount); // only for root items
    auto s = StandardPaths::settings();
    switch (index.row()) {
    case Attributes:
    case Skills:
    case Facets:
    case Preferences:
        m_role->attributes_weight.is_default = true;
        m_role->attributes_weight.is_neg = false;
        m_role->attributes_weight.weight = m_default_weights[index.row()];
        break;
    default:
        return;
    }
    auto weight_cell = createIndex(index.row(), ColumnWeight, RowCount);
    emit dataChanged(weight_cell, weight_cell);
}

void RoleModel::update_default_weights()
{
    auto s = StandardPaths::settings();
    m_default_weights[Attributes] = s->value("options/default_attributes_weight",1.0).toFloat();
    m_default_weights[Skills] = s->value("options/default_skills_weight",1.0).toFloat();
    m_default_weights[Facets] = s->value("options/default_traits_weight",1.0).toFloat();
    m_default_weights[Preferences] = s->value("options/default_prefs_weight",1.0).toFloat();
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

    switch (parent.row()) {
    case Attributes:
        return m_role->attributes.size();
    case Skills:
        return m_role->skills.size();
    case Facets:
        return m_role->traits.size();
    case Preferences:
        return m_role->prefs.size();
    default:
        return 0;
    }
}

int RoleModel::columnCount(const QModelIndex &parent) const
{
    return ColumnCount;
}

static QVariant aspect_data(const RoleAspect &aspect, int role)
{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return (double) (aspect.is_neg ? -1 : +1) * aspect.weight;
    default:
        return QVariant();
    }
}

QVariant RoleModel::data(const QModelIndex &index, int role) const
{
    auto gdr = GameDataReader::ptr();

    if (!index.isValid() || !m_role)
        return QVariant();

    if (index.internalId() == RowCount) { // Root item
        switch (index.column()) {
        case ColumnName:
            if (role == Qt::DisplayRole) {
                switch (index.row()) {
                case Attributes:
                    return tr("Attributes");
                case Skills:
                    return tr("Skills");
                case Facets:
                    return tr("Facets");
                case Preferences:
                    return tr("Preferences");
                default:
                    return QVariant();
                }
            }
            else
                return QVariant();
        case ColumnWeight: {
            const Role::weight_info *w;
            switch (index.row()) {
            case Attributes:
                w = &m_role->attributes_weight;
                break;
            case Skills:
                w = &m_role->skills_weight;
                break;
            case Facets:
                w = &m_role->traits_weight;
                break;
            case Preferences:
                w = &m_role->prefs_weight;
                break;
            default:
                return QVariant();
            }
            switch (role) {
            case Qt::DisplayRole:
                return w->is_default
                    ? tr("default (%1)").arg(QString::number(m_default_weights[index.row()]))
                    : QString::number((w->is_neg ? -1 : +1) * w->weight);
            case Qt::EditRole:
                return (double) (w->is_default
                    ? m_default_weights[index.row()]
                    : (w->is_neg ? -1 : +1) * w->weight);
            default:
                return QVariant();
            }
        }
        default:
            return QVariant();
        }
    }
    else { // leaf item
        switch (index.internalId()) {
        case Attributes: {
            // TODO: improve element access
            auto it = m_role->attributes.begin();
            std::advance(it, index.row());
            switch (index.column()) {
            case ColumnName:
                if (role == Qt::DisplayRole)
                    return it->first;
                else
                    return QVariant();
            case ColumnWeight:
                return aspect_data(it->second, role);
            default:
                return QVariant();
            }
        }
        case Skills: {
            // TODO: improve element access
            auto it = m_role->skills.begin();
            std::advance(it, index.row());
            switch (index.column()) {
            case ColumnName:
                if (role == Qt::DisplayRole)
                    return gdr->get_skill_name(it->first.toInt());
                else
                    return QVariant();
            case ColumnWeight:
                return aspect_data(it->second, role);
            default:
                return QVariant();
            }
        }
        case Facets: {
            // TODO: improve element access
            auto it = m_role->traits.begin();
            std::advance(it, index.row());
            switch (index.column()) {
            case ColumnName:
                if (role == Qt::DisplayRole)
                    return gdr->get_trait_name(it->first.toInt());
                else
                    return QVariant();
            case ColumnWeight:
                return aspect_data(it->second, role);
            default:
                return QVariant();
            }
        }
        case Preferences: {
            const auto &pref = m_role->prefs[index.row()];
            switch (index.column()) {
            case ColumnName:
                if (role == Qt::DisplayRole)
                    return pref->get_name();
                else
                    return QVariant();
            case ColumnWeight:
                return aspect_data(pref->aspect, role);
            case ColumnCategory:
                if (role == Qt::DisplayRole)
                    return Preference::get_pref_desc(pref->get_pref_category());
                else
                    return QVariant();
            case ColumnItem:
                if (role == Qt::DisplayRole) {
                    if (auto ip = dynamic_cast<const ItemRolePreference *>(pref.get()))
                        return Item::get_item_name_plural(ip->get_item_type());
                    else
                        return QVariant();
                }
                else
                    return QVariant();
            default:
                return QVariant();
            }
        }
        default:
            return QVariant();
        }
    }
}

bool RoleModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool ok;

    if (!index.isValid() || index.column() != ColumnWeight || role != Qt::EditRole || !m_role)
        return false;

    if (index.internalId() == RowCount) { // global weights
        Role::weight_info *w;
        switch (index.row()) {
        case Attributes:
            w = &m_role->attributes_weight;
            break;
        case Skills:
            w = &m_role->skills_weight;
            break;
        case Facets:
            w = &m_role->traits_weight;
            break;
        case Preferences:
            w = &m_role->prefs_weight;
            break;
        default:
            return false;
        }
        float weight = value.toFloat(&ok);
        if (!ok)
            return false;
        if (weight == 0.0f) {
            w->is_default = true;
            w->is_neg = false;
            w->weight = m_default_weights[index.row()];
        }
        else {
            w->is_default = false;
            w->is_neg = weight < 0.0f;
            w->weight = fabs(weight);
        }
        emit dataChanged(index, index);
        return true;
    }
    else { // item weights
        RoleAspect *a = nullptr;
        // TODO: element access
        switch (index.internalId()) {
        case Attributes: {
            auto it = m_role->attributes.begin();
            std::advance(it, index.row());
            a = &it->second;
            break;
        }
        case Skills: {
            auto it = m_role->skills.begin();
            std::advance(it, index.row());
            a = &it->second;
            break;
        }
        case Facets: {
            auto it = m_role->traits.begin();
            std::advance(it, index.row());
            a = &it->second;
            break;
        }
        case Preferences:
            a = &m_role->prefs[index.row()]->aspect;
            break;
        }
        if (!a)
            return false;
        float weight = value.toFloat(&ok);
        if (!ok)
            return false;
        a->is_neg = weight < 0.0f;
        a->weight = fabs(weight);
        emit dataChanged(index, index);
        return true;
    }
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
    case ColumnCategory:
        return tr("Category");
    case ColumnItem:
        return tr("Item");
    default:
        return QVariant();
    }
}

QModelIndex RoleModel::buddy(const QModelIndex &index) const
{
    return sibling(index.row(), ColumnWeight, index);
}
