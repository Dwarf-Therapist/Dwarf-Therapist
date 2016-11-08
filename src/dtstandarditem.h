#ifndef DTSTANDARDITEM_H
#define DTSTANDARDITEM_H

#include <QStandardItem>

class DTStandardItem : public QStandardItem
{
public:
    DTStandardItem();

    QVariant data(int role) const;
    void setData(const QVariant &value, int role);

    static void set_show_tooltips(bool val);

private:
    static bool m_show_tooltips;

};

#endif // DTSTANDARDITEM_H
