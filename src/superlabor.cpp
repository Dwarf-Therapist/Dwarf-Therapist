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

#include <QWidget>
#include <QMessageBox>
#include "superlabor.h"
#include "ui_superlabor.h"
#include "gamedatareader.h"
#include "labor.h"
#include "dwarf.h"
#include "defines.h"

//new blank superlabor
SuperLabor::SuperLabor(QObject *parent)
    :MultiLabor(parent)
    , ui(new Ui::SuperLaborEditor)
{
}

SuperLabor::SuperLabor(QSettings &s, QObject *parent)
    :MultiLabor(parent)
    , ui(new Ui::SuperLaborEditor)
{
    m_name = s.value("id","").toString();
    m_role_name = s.value("role_name","").toString();

    int labors = s.beginReadArray("labors");
    for (int i = 0; i < labors; ++i) {
        s.setArrayIndex(i);
        int labor_id = s.value("id", -1).toInt();
        if (labor_id != -1)
            add_labor(labor_id);
    }
    s.endArray();
}

SuperLabor::SuperLabor(Dwarf *d, QObject *parent)
    : MultiLabor(parent)
    , ui(new Ui::SuperLaborEditor)
{
    m_dwarf = d;
    if(m_dwarf){
        m_name = m_dwarf->profession();
        set_labors();
    }
}

SuperLabor::~SuperLabor() {
    delete ui;
}

void SuperLabor::load_cp_labors(CustomProfession *cp){
    m_active_labors.clear();
    m_selected_count = 0;
    if(cp){
        QList<Labor*> labors = gdr->get_ordered_labors();
        foreach(Labor *l, labors) {
            if (cp && cp->get_enabled_labors().contains(l->labor_id))
                add_labor(l->labor_id);
        }
    }
}

int SuperLabor::show_builder_dialog(QWidget *parent) {
    m_dialog = new QDialog(parent);

    ui->setupUi(m_dialog);

    ui->name_edit->setText(m_name);
    connect(ui->name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(QString)));
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(this,SIGNAL(selected_count_changed(int)),ui->lbl_count,SLOT(setNum(int)));

    build_role_combo(ui->cb_roles);
    connect(ui->cb_roles,SIGNAL(currentIndexChanged(int)),this,SLOT(role_changed(int)),Qt::UniqueConnection);
    load_labors(ui->labor_list);

    int code = m_dialog->exec();
    m_dialog->deleteLater();
    return code;
}

void SuperLabor::role_changed(int index){
    m_role_name = ui->cb_roles->itemData(index,Qt::UserRole).toString();
}

bool SuperLabor::is_valid() {
    if (!m_dialog)
        return true;

    QString proposed_name = ui->name_edit->text().trimmed();
    if (proposed_name.isEmpty()) {
        QMessageBox::warning(m_dialog, tr("Naming Error!"),
                             tr("You must enter a name for this Super Labor!"));
        return false;
    }
    foreach(SuperLabor *sl, DT->get_super_labors()){
        if(sl != this && sl->get_name() == proposed_name){
            QMessageBox::warning(m_dialog, tr("Duplicate Name!"),
                                 tr("A Super Labor with this name already exists!"));
            return false;
        }
    }
    foreach(CustomProfession *cp, DT->get_custom_professions()){
        if(cp->get_name() == proposed_name){
            QMessageBox::warning(m_dialog, tr("Duplicate Name!"),
                                 tr("A Custom Profession with this name already exists!"));
            return false;
        }
    }
    return true;
}

void SuperLabor::delete_from_disk() {
    QSettings s(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    int size = s.beginReadArray("super_labors");
    for(int idx=0;idx<size;idx++){
        s.setArrayIndex(idx);
        if(s.value("id") == m_name)
            s.remove(m_name);
    }
    s.endArray();
}

void SuperLabor::save(QSettings &s){
    s.setValue("id",m_name);
    s.setValue("role_name",m_role_name);

    s.beginWriteArray("labors");
    int i = 0;
    foreach(int labor_id, get_enabled_labors()) {
        s.setArrayIndex(i++);
        s.setValue("id",QString::number(labor_id));
    }
    s.endArray();
}
