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
#include "dwarftherapist.h"

ScriptDialog::ScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptDialog)
{
    ui->setupUi(this);


    GameDataReader *gdr = GameDataReader::ptr();
    QString labor_list = "<br><b>Labor Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Labor ID</th><th>Labor</th></tr>";
    foreach(Labor *l, gdr->get_ordered_labors()) {
        if (l->is_weapon)
            continue;
        labor_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(l->labor_id, 2, 10, QChar('0')).arg(l->name));
    }
    labor_list.append("</table>");
    ui->text_help->append(labor_list);

    QString trait_list = "<br><b>Traits Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Trait ID</th><th>Trait</th></tr>";
    foreach(Trait *t, gdr->get_traits()) {
        trait_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(t->trait_id).arg(t->name));
    }
    trait_list.append("</table>");
    ui->text_help->append(trait_list);

    connect(ui->btn_apply, SIGNAL(clicked()), SLOT(apply_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));
}

void ScriptDialog::clear_script() {
    ui->script_edit->clear();
}

void ScriptDialog::apply_pressed() {
    emit apply_script(ui->script_edit->toPlainText());
}

void ScriptDialog::save_pressed() {
    bool ok;
    QString name = QInputDialog::getText(this, tr("Name this Script"), tr("Script Name:"), QLineEdit::Normal, QString(), &ok);
    if (ok) { // TODO: check for name collision
        QSettings *s = DT->user_settings();
        s->setValue(QString("filter_scripts/%1").arg(name), ui->script_edit->toPlainText());
    }
    emit scripts_changed();
}