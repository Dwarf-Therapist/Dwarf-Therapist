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
#ifndef NEEDS_WIDGET_H
#define NEEDS_WIDGET_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>

#include <memory>

namespace Ui { class NeedsWidget; }

class NeedsDelegate;

class NeedsWidget: public QWidget
{
    Q_OBJECT
public:
    NeedsWidget(QWidget *parent = nullptr);
    virtual ~NeedsWidget();

public slots:
    void clear();
    void refresh();

signals:
    void focus_selected(QVariantList);
    void need_selected(QVariantList, bool match_all);

private slots:
    void focus_selection_changed();
    void need_selection_changed();

private:
    std::unique_ptr<Ui::NeedsWidget> ui;
    std::unique_ptr<NeedsDelegate> m_focus_delegate;
    std::unique_ptr<NeedsDelegate> m_needs_delegate;
    QStandardItemModel m_focus_model;
    QStandardItemModel m_needs_model;
};

class NeedsDelegate: public QStyledItemDelegate
{
    Q_OBJECT
public:
    NeedsDelegate(const QList<QColor> &colors, QObject *parent = nullptr);
    virtual ~NeedsDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QList<QColor> m_colors;
};

#endif
