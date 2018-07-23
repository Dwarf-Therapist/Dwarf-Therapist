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

#include <QProgressDialog>
#include <QPushButton>
#include <QtConcurrent>

#include "truncatingfilelogger.h"
#include "rolepreference.h"
#include "rolepreferencemodel.h"

PreferencePickerDialog::PreferencePickerDialog(RolePreferenceModel *model, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::PreferencePickerDialog>())
    , m_model(model)
{
    ui->setupUi(this);

    ui->preference_view->set_filter_mode(SortFilterProxyModel::RecursiveMode);
    ui->preference_view->set_model(model);

    // Wait for a valid selection for enabling Ok button
    ui->buttons->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(ui->preference_view, &SearchFilterTreeView::item_selected,
            this, &PreferencePickerDialog::item_selected);
    connect(ui->preference_view, &SearchFilterTreeView::item_activated,
            this, &PreferencePickerDialog::item_activated);
}

PreferencePickerDialog::~PreferencePickerDialog()
{
}

const RolePreference *PreferencePickerDialog::get_selected_preference() const
{
    return m_model->getPreference(ui->preference_view->get_selected_item());
}

void PreferencePickerDialog::showEvent(QShowEvent *e)
{
    QDialog::showEvent(e);

    m_model->load_pref_from_raws(this);
}

void PreferencePickerDialog::item_selected(const QModelIndex &index)
{
    auto pref = m_model->getPreference(index);
    ui->buttons->button(QDialogButtonBox::Ok)->setEnabled(pref != nullptr);
}

void PreferencePickerDialog::item_activated(const QModelIndex &index)
{
    auto pref = m_model->getPreference(index);
    if (pref)
        accept();
}
