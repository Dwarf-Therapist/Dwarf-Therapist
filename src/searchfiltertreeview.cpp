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
#include "searchfiltertreeview.h"
#include "ui_searchfiltertreeview.h"

SearchFilterTreeView::SearchFilterTreeView(QWidget *parent)
    : QWidget(parent)
    , ui(std::make_unique<Ui::SearchFilterTreeView>())
    , m_expand_collapse_hidden(false)
{
    ui->setupUi(this);

    ui->tree_view->setModel(&m_filter_proxy);
    ui->tree_view->setSortingEnabled(true);
    ui->tree_view->collapseAll();

    if (!m_expand_collapse_hidden) {
        ui->tree_view->addAction(ui->action_collapse_all);
        ui->tree_view->addAction(ui->action_expand_all);
    }
    ui->collapse_all_button->setHidden(m_expand_collapse_hidden);
    ui->expand_all_button->setHidden(m_expand_collapse_hidden);

    connect(ui->search_edit, &QLineEdit::textChanged,
            this, &SearchFilterTreeView::search_text);
    connect(ui->clear_button, &QAbstractButton::pressed,
            this, &SearchFilterTreeView::clear_search);

    connect(ui->tree_view->selectionModel(), &QItemSelectionModel::currentChanged,
            [this] (const QModelIndex &current, const QModelIndex &) {
                emit item_selected(m_filter_proxy.mapToSource(current));
            });
    connect(ui->tree_view, &QAbstractItemView::activated,
            [this] (const QModelIndex &index) {
                emit item_activated(m_filter_proxy.mapToSource(index));
            });
}

SearchFilterTreeView::~SearchFilterTreeView()
{
}

void SearchFilterTreeView::set_model(QAbstractItemModel *model)
{
    m_filter_proxy.setSourceModel(model);
}

QAbstractItemView *SearchFilterTreeView::view()
{
    return ui->tree_view;
}

QModelIndex SearchFilterTreeView::get_selected_item() const
{
    QModelIndex current = ui->tree_view->selectionModel()->currentIndex();
    return m_filter_proxy.mapToSource(current);
}

bool SearchFilterTreeView::is_expand_collapse_hidden() const
{
    return m_expand_collapse_hidden;
}

void SearchFilterTreeView::set_expand_collapse_hidden(bool hidden)
{
    ui->collapse_all_button->setHidden(hidden);
    ui->expand_all_button->setHidden(hidden);
    if (m_expand_collapse_hidden && !hidden) {
        ui->tree_view->addAction(ui->action_collapse_all);
        ui->tree_view->addAction(ui->action_expand_all);
    }
    else if (!m_expand_collapse_hidden && hidden) {
        ui->tree_view->removeAction(ui->action_collapse_all);
        ui->tree_view->removeAction(ui->action_expand_all);
    }
    m_expand_collapse_hidden = hidden;
}

const QAbstractItemView *SearchFilterTreeView::view() const
{
    return ui->tree_view;
}

void SearchFilterTreeView::search_text(const QString &text)
{
    QString val = text;
    QRegExp filter("(" + val.replace(" ", "|") + ")", Qt::CaseInsensitive);
    m_filter_proxy.setFilterRegExp(filter);
    m_filter_proxy.setFilterKeyColumn(0);
}

void SearchFilterTreeView::clear_search()
{
    ui->search_edit->setText("");
    m_filter_proxy.setFilterRegExp(QRegExp());
    ui->tree_view->collapseAll();
}
