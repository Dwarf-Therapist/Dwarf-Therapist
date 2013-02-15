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

#include "scriptdialog.h"
#include "ui_scriptdialog.h"
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include "skill.h"
#include "dwarftherapist.h"
#include "attribute.h"

ScriptDialog::ScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptDialog)
{
    ui->setupUi(this);


    GameDataReader *gdr = GameDataReader::ptr();
    QString labor_list = "<br><b>Labor Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Labor ID</th><th>Labor</th></tr>";
    foreach(Labor *l, gdr->get_ordered_labors()) {
        labor_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(l->labor_id, 2, 10, QChar('0')).arg(l->name));
    }
    labor_list.append("</table>");
    ui->text_help->append(labor_list);

    QString trait_list = "<br><b>Traits Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Trait ID</th><th>Trait</th></tr>";
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, gdr->get_ordered_traits()) {
        trait_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(trait_pair.second->trait_id).arg(trait_pair.second->name));
    }
    trait_list.append("</table>");
    ui->text_help->append(trait_list);

    QString skill_list = "<br><b>Skills Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Skill ID</th><th>Skill</th></tr>";
    QPair<int, QString> skill_pair;
    foreach(skill_pair, gdr->get_ordered_skills()) {
        skill_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(skill_pair.first).arg(skill_pair.second));
    }
    skill_list.append("</table>");
    ui->text_help->append(skill_list);

    QString attribute_list = "<br><b>Attribute Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Attribute ID</th><th>Attribute</th></tr>";
    QPair<int, QString> att_pair;
    foreach(att_pair, gdr->get_ordered_attribute_names()) {
        attribute_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(att_pair.first).arg(att_pair.second));
    }
    attribute_list.append("</table>");
    ui->text_help->append(attribute_list);

    connect(ui->btn_apply, SIGNAL(clicked()), SLOT(apply_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));
}

void ScriptDialog::clear_script() {
    ui->script_edit->clear();
    ui->txt_script_name->clear();
    ui->lbl_save_status->clear();
    m_name = "";
}

void ScriptDialog::load_script(QString name, QString script){
    ui->script_edit->setText(script);    
    m_name = name;
    ui->txt_script_name->setText(m_name);
    ui->lbl_save_status->clear();
}

void ScriptDialog::apply_pressed() {
    emit apply_script(ui->script_edit->toPlainText());
    ui->lbl_save_status->setText(tr("Script has been applied but hasn't been saved."));
}

void ScriptDialog::save_pressed() {        
    QString m_old_name = m_name;
    m_name = ui->txt_script_name->text();

    QSettings *s = DT->user_settings();
    int answer = QMessageBox::Yes;

    if(m_old_name != m_name){
        s->beginGroup("filter_scripts");
        foreach(QString script_name, s->childKeys()){
            if(m_name==script_name){
                answer = QMessageBox::question(0,"Confirm Replace",
                                               tr("A script with this name already exists and will be overwritten. Continue?"),
                                               QMessageBox::Yes,QMessageBox::No);
                break;
            }
        }
        s->endGroup();
    }

    if(answer == QMessageBox::No){
        ui->lbl_save_status->setText(tr("Save cancelled."));
        return;
    }

    if(m_old_name != m_name && m_old_name != "")
        s->remove(QString("filter_scripts/%1").arg(m_old_name));

    s->setValue(QString("filter_scripts/%1").arg(m_name), ui->script_edit->toPlainText());
    emit scripts_changed();

    ui->lbl_save_status->setText(tr("Script saved successfully!"));
}
