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
#ifndef THOUGHTSDOCK_H
#define THOUGHTSDOCK_H

#include "basedock.h"
#include "global_enums.h"
#include <QTreeWidget>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QFontMetrics>
#include <QDebug>

class ThoughtsDock : public BaseDock {
    Q_OBJECT
public:
    ThoughtsDock(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    void filter();

protected:
    QTreeWidget *tw_thoughts;
    QLineEdit* le_search;
    QPushButton *btn_toggle_tree;

    QIcon arr_in;
    QIcon arr_out;

    void search_tree(QString val);
    bool m_collapsed;

    void closeEvent(QCloseEvent *event);

public slots:
    void refresh();
    void clear();

protected slots:
    void search_changed(QString val);
    void clear_search();
    void toggle_tree();
    void selection_changed();
    void clear_filter();

signals:
    void item_selected(QVariantList);

};

class SortableTreeItem : public QTreeWidgetItem {
public:
    SortableTreeItem(QTreeWidgetItem* parent = 0)
        : QTreeWidgetItem(parent)
    {}
private:
    bool operator<(const QTreeWidgetItem &other)const {
        int col = treeWidget()->sortColumn();
        Qt::ItemDataRole idr = (Qt::ItemDataRole)((int)Qt::UserRole + 50 + col);
        //qDebug() << "sorting on column" << col << "values" << data(col,idr).toString() << " | " << other.data(col,idr).toString();
        if(col == 1){
            return data(col,idr).toInt() < other.data(col,idr).toInt();
        }else{
            return data(col,idr).toString() < other.data(col,idr).toString();
        }
    }
};

class ThoughtsItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const {
        QStyledItemDelegate::paint(painter, option, index);

        //only draw summary totals for top level items
        if(!index.parent().isValid())
        {
            painter->save();

            QVariantList counts = index.data(Qt::UserRole+1).toList();
            QColor default_pen = painter->pen().color();
            //QString curr_text = index.data().toString();
            QString curr_text = "";
            //draw the eustress count
            curr_text = prependText(painter,option,QColor(Qt::darkGreen),curr_text,QString("%1 ").arg(counts.at(2).toInt(),2,10,QChar('0')));
            //spacer
            curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
            //draw the neutral count
            curr_text = prependText(painter,option,QColor(Qt::darkGray),curr_text,QString(" %1").arg(counts.at(1).toInt(),2,10,QChar('0')));
            //spacer
            curr_text = prependText(painter,option,default_pen,curr_text,QString(" / "));
            //draw the stress count
            curr_text = prependText(painter,option,QColor(Qt::darkRed),curr_text,QString("  %1").arg(counts.at(0).toInt(),2,10,QChar('0')));

//            //draw the stress count
//            curr_text = appendText(painter,option,QColor(Qt::darkRed),curr_text,QString("  %1").arg(counts.at(0).toString()));
//            //spacer
//            curr_text = appendText(painter,option,default_pen,curr_text,QString(" / "));
//            //draw the neutral count
//            curr_text = appendText(painter,option,QColor(Qt::darkGray),curr_text,QString(" %1").arg(counts.at(1).toString()));
//            //spacer
//            curr_text = appendText(painter,option,default_pen,curr_text,QString(" / "));
//            //draw the eustress count
//            curr_text = appendText(painter,option,QColor(Qt::darkGreen),curr_text,QString(" %1").arg(counts.at(2).toString()));

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

#endif // THOUGHTSDOCK_H
