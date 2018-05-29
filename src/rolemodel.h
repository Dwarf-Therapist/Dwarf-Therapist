#ifndef ROLE_MODEL_H
#define ROLE_MODEL_H

#include <QAbstractItemModel>

class Role;
class RolePreference;

class RoleModel: public QAbstractItemModel
{
    Q_OBJECT
public:
    RoleModel(QObject *parent = nullptr);
    virtual ~RoleModel();

    void set_role(Role *role);

    QModelIndex add_attribute(const QString &attribute);
    QModelIndex add_skill(int id);
    QModelIndex add_facet(int id);
    QModelIndex add_preference(const RolePreference *pref);

    void remove_item(const QModelIndex &index);
    void reset_default_weight(const QModelIndex &index);
    void update_default_weights();

    // QAbstractItemModel implementation
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex buddy(const QModelIndex &index) const override;

    enum Column
    {
        ColumnName = 0,
        ColumnWeight,
        ColumnCategory, // only for preferences
        ColumnItem, // only for item preferences
        ColumnCount,
    };

    // Rows are also used in QModelIndex's internalId for storing the parent row.
    // Root items internalId is RowCount.
    enum Row
    {
        Attributes = 0,
        Skills,
        Facets,
        Preferences,
        RowCount
    };

private:
    Role *m_role;
    float m_default_weights[RowCount];
};


#endif
