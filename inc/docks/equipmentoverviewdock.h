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
#include "global_enums.h"
#include "item.h"

#include <QTableWidget>
#include <QLabel>

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

    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
        QStyledItemDelegate::paint(painter, option, index);

        //only draw summary totals for top level items
        if(!index.parent().isValid())
        {
            painter->save();

            QVariantMap counts = index.data(Qt::UserRole+1).toMap();
            QColor default_pen = painter->pen().color();
            QString curr_text = "";

            foreach(QString wear_level, counts.uniqueKeys()){
                int count = counts.value(wear_level).toInt();
                QColor col = Item::get_color(static_cast<Item::ITEM_STATE>(wear_level.toInt()));
                if(curr_text != ""){
                    curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
                }
                curr_text = prependText(painter,option,col,curr_text,QString("%1 ").arg(count==0 ? "-- " : QString("%1").arg(count,2,10,QChar('0'))));
            }
            painter->restore();
        }
    }

protected:
    QString appendText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
        int text_width = painter->fontMetrics().width(curr_text);
        QRect r_display = option.rect.adjusted(text_width,0,0,0);
        painter->setPen(text_color);
        painter->drawText(r_display,Qt::AlignLeft | Qt::AlignVCenter,text);
        return (curr_text + text);
    }
    QString prependText(QPainter *painter, const QStyleOptionViewItem &option, QColor text_color, QString curr_text, QString text) const {
        int text_width = -painter->fontMetrics().width(curr_text);
        QRect r_display = option.rect.adjusted(0,0,text_width,0);
        painter->setPen(text_color);
        painter->drawText(r_display,Qt::AlignRight | Qt::AlignVCenter,text);
        return (curr_text + text);
    }
};

#endif // EQUIPMENTOVERVIEWDOCK_H
