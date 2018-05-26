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
#ifndef SEARCH_FILTER_TREE_VIEW_H
#define SEARCH_FILTER_TREE_VIEW_H

#include <QWidget>
#include <memory>

namespace Ui { class SearchFilterTreeView; }
class QAbstractItemModel;
class QAbstractItemView;
class RecursiveFilterProxyModel;

class SearchFilterTreeView: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool expand_collapse_hidden
               READ is_expand_collapse_hidden
               WRITE set_expand_collapse_hidden)
public:
    SearchFilterTreeView(QWidget *parent = nullptr);
    ~SearchFilterTreeView();

    void set_model(QAbstractItemModel *model);

    QAbstractItemView *view();
    const QAbstractItemView *view() const;

    QModelIndex get_selected_item() const;

    bool is_expand_collapse_hidden() const;
    void set_expand_collapse_hidden(bool hidden);

signals:
    void item_selected(const QModelIndex &index);
    void item_activated(const QModelIndex &index);

public slots:
    void search_text(const QString &text);
    void clear_search();

private:
    std::unique_ptr<Ui::SearchFilterTreeView> ui;
    bool m_expand_collapse_hidden;
    RecursiveFilterProxyModel *m_filter_proxy;
};

#endif
