#ifndef SORTABLETABLEITEMS_H
#define SORTABLETABLEITEMS_H

#include <QTableWidgetItem>

class sortableTableWidgetItem : public QTableWidgetItem
{
public:
    sortableTableWidgetItem(int type = Type) : QTableWidgetItem(type) {}
    ~sortableTableWidgetItem () {}

    bool operator<(const QTableWidgetItem& other) const
    {
        Q_ASSERT(tableWidget());
        Q_ASSERT(tableWidget()->cellWidget(row(), column()));
        Q_ASSERT(tableWidget()->cellWidget(other.row(), other.column()));
        return tableWidget()->cellWidget(row(), column())->property("text").toString() <
            tableWidget()->cellWidget(other.row(), other.column())->property("text").toString();
    }
};

class sortableComboItem : public QTableWidgetItem
{
public:
    sortableComboItem(int type = Type) : QTableWidgetItem(type) {}
    ~sortableComboItem () {}

    bool operator<(const QTableWidgetItem& other) const
    {
        Q_ASSERT(tableWidget());
        Q_ASSERT(tableWidget()->cellWidget(row(), column()));
        Q_ASSERT(tableWidget()->cellWidget(other.row(), other.column()));
        return tableWidget()->cellWidget(row(), column())->property("currentText").toString() <
            tableWidget()->cellWidget(other.row(), other.column())->property("currentText").toString();
    }
};

class sortablePercentTableWidgetItem : public QTableWidgetItem{
public :
    bool operator <(QTableWidgetItem *other){
        return text().toInt() < other->text().toInt();
    }
};

#endif // SORTABLETABLEITEMS_H
