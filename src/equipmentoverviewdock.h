/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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
#ifndef EQUIPMENTOVERVIEWDOCK_H
#define EQUIPMENTOVERVIEWDOCK_H

#include "basetreedock.h"

#include <QStyledItemDelegate>

class QLabel;
class QPainter;

class EquipmentOverviewDock : public BaseTreeDock {
    Q_OBJECT
public:
    EquipmentOverviewDock(QWidget *parent = 0, Qt::WindowFlags flags = 0);

protected:
    QLabel *lbl_read;
    void search_tree(QString val);
    void build_tree();

public slots:
    void check_changed(bool);
    void selection_changed();

signals:
    void item_selected(QVariantList);

private:
    bool m_option_state;
    static QString m_option_name;
    QHash<int,QString> m_wear_level_desc;

};

class EquipWarnItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:

    EquipWarnItemDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;

protected:
    QString appendText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const;
    QString prependText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const;
};

#endif // EQUIPMENTOVERVIEWDOCK_H
