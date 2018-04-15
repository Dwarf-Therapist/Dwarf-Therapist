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
#include "preferencepickerdialog.h"
#include "ui_preferencepickerdialog.h"

#include <QtConcurrent>
#include <QProgressDialog>

#include "truncatingfilelogger.h"
#include "rolepreference.h"
#include "rolepreferencemodel.h"
#include "recursivefilterproxymodel.h"

PreferencePickerDialog::PreferencePickerDialog(RolePreferenceModel *model, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::PreferencePickerDialog>())
    , m_model(model)
    , m_filter_proxy(new RecursiveFilterProxyModel(this))
{
    ui->setupUi(this);

    m_filter_proxy->setSourceModel(model);
    m_filter_proxy->sort(0, Qt::AscendingOrder);
    ui->preference_view->setModel(m_filter_proxy);
    ui->preference_view->setSortingEnabled(true);
    ui->preference_view->collapseAll();

    // Wait for a valid selection for enabling Ok button
    ui->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->search_edit, &QLineEdit::textChanged, this, &PreferencePickerDialog::search_text);
    connect(ui->clear_button, &QAbstractButton::pressed, this, &PreferencePickerDialog::clear_search);
    connect(ui->preference_view->selectionModel(), &QItemSelectionModel::currentChanged, this, &PreferencePickerDialog::selection_changed);
    connect(ui->preference_view, &QAbstractItemView::activated, this, &PreferencePickerDialog::item_activated);
}

PreferencePickerDialog::~PreferencePickerDialog()
{
}

const RolePreference *PreferencePickerDialog::get_selected_preference() const
{
    QModelIndex current = ui->preference_view->selectionModel()->currentIndex();
    return m_model->getPreference(m_filter_proxy->mapToSource(current));
}

void PreferencePickerDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

    m_model->load_pref_from_raws(this);
}

void PreferencePickerDialog::search_text(const QString &text)
{
    QString val = text;
    QRegExp filter("(" + val.replace(" ", "|") + ")", Qt::CaseInsensitive);
    m_filter_proxy->setFilterRegExp(filter);
    m_filter_proxy->setFilterKeyColumn(0);
}

void PreferencePickerDialog::clear_search()
{
    ui->search_edit->setText("");
    m_filter_proxy->setFilterRegExp(QRegExp());
    ui->preference_view->collapseAll();
}

void PreferencePickerDialog::selection_changed(const QModelIndex &current, const QModelIndex &)
{
    auto pref = m_model->getPreference(m_filter_proxy->mapToSource(current));
    ui->buttons->button(QDialogButtonBox::Ok)->setEnabled(pref != nullptr);
}

void PreferencePickerDialog::item_activated(const QModelIndex &index)
{
    auto pref = m_model->getPreference(m_filter_proxy->mapToSource(index));
    if (pref)
        accept();
}
