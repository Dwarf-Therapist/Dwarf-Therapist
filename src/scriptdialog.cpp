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

#include <QMessageBox>
#include "scriptdialog.h"
#include "ui_scriptdialog.h"
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include "dwarftherapist.h"
#include "unithealth.h"
#include "healthcategory.h"
#include "healthinfo.h"
#include "item.h"
#include "dwarf.h"

#include <QJSEngine>

ScriptDialog::ScriptDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ScriptDialog)
{
    ui->setupUi(this);

    //TODO: convert to tables/trees and add in a search (and sort?) function/options

    //LABORS
    GameDataReader *gdr = GameDataReader::ptr();
    QString labor_list = "<b>Labor Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Labor ID</th><th>Labor</th></tr>";
    foreach(Labor *l, gdr->get_ordered_labors()) {
        labor_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(l->labor_id, 2, 10, QChar('0')).arg(l->name));
    }
    labor_list.append("</table>");
    ui->text_labors->append(labor_list);

    //SKILLS
    QString skill_list = "<b>Skills Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Skill ID</th><th>Skill</th></tr>";
    QPair<int, QPair<QString,QString> > skill_pair;
    foreach(skill_pair, gdr->get_ordered_skills()) {
        skill_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(skill_pair.first).arg(skill_pair.second.first));
    }
    skill_list.append("</table>");
    ui->text_skills->append(skill_list);

    //ATTRIBUTES
    QString attribute_list = "<b>Attribute Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Attribute ID</th><th>Attribute</th></tr>";
    QPair<ATTRIBUTES_TYPE, QString> att_pair;
    foreach(att_pair, gdr->get_ordered_attribute_names()) {
        attribute_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(att_pair.first).arg(att_pair.second));
    }
    attribute_list.append("</table>");
    ui->text_attributes->append(attribute_list);

    //PERSONALITY
    QString trait_list = "<b>Trait Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Trait ID</th><th>Trait</th></tr>";
    QPair<int, Trait*> trait_pair;
    foreach(trait_pair, gdr->get_ordered_traits()) {
        trait_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(trait_pair.second->id()).arg(trait_pair.second->get_name()));
    }
    trait_list.append("</table>");
    ui->text_personality->append(trait_list);

    QString belief_list = "<b>Belief Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Belief ID</th><th>Belief</th></tr>";
    QPair<int, QString> belief_pair;
    foreach(belief_pair, gdr->get_ordered_beliefs()) {
        belief_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(belief_pair.first).arg(belief_pair.second));
    }
    belief_list.append("</table>");
    ui->text_personality->append(belief_list);

    QString goal_list = "<b>Goal Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Goal ID</th><th>Goal</th></tr>";
    QPair<int, QString> goal_pair;
    foreach(goal_pair, gdr->get_ordered_goals()) {
        goal_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(goal_pair.first).arg(goal_pair.second));
    }
    goal_list.append("</table>");
    ui->text_personality->append(goal_list);

    //JOBS/PROFESSIONS
    QString job_list = "<b>Job Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Job ID</th><th>Job</th></tr>";
    QPair<int, QString> job_pair;
    foreach(job_pair, gdr->get_ordered_jobs()) {
        job_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(job_pair.first).arg(job_pair.second));
    }
    job_list.append("</table>");
    ui->text_jobs->append(job_list);

    //HEALTH
    QString health_list = "<b>Health Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Category ID</th><th>Title</th><th>Descriptors</th></tr>";

    QPair<eHealth::H_INFO,QString> cat_pair;
    foreach(cat_pair, UnitHealth::ordered_category_names()) {
        HealthCategory *hc = UnitHealth::get_display_categories().value(cat_pair.first);
        health_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td>").arg(hc->id()).arg(hc->name()));
        health_list.append("<td><table border=0 cellpadding=1 cellspacing=1 width=100%>");
        short idx = 0;
        foreach(HealthInfo *hi, hc->descriptions()){
            health_list.append(QString("<tr><td width=5%>%1</td><td>%2</td></tr>").arg(idx).arg(hi->description(false)));
            idx++;
        }
        health_list.append("</table></td></tr>");
    }
    health_list.append("</table>");
    ui->text_health->append(health_list);

    //EQUIPMENT/ITEMS
    QString item_list = "<b>Item Reference</b><table border=1 cellpadding=3 cellspacing=0 width=100%>"
        "<tr><th width=24%>Item Type ID</th><th>Name</th></tr>";
    QMap<QString,int> item_types;
    for(int i=0; i < NUM_OF_ITEM_TYPES; i++){
        item_types.insert(Item::get_item_name_plural(static_cast<ITEM_TYPE>(i)),i);
    }
    foreach(QString name, item_types.uniqueKeys()){
        item_list.append(QString("<tr><td><font color=blue>%1</font></td><td><b>%2</b></td></tr>").arg(QString::number(item_types.value(name))).arg(name));
    }
    item_list.append("</table>");
    ui->text_items->append(item_list);

    connect(ui->btn_apply, SIGNAL(clicked()), SLOT(apply_pressed()));
    connect(ui->btn_save, SIGNAL(clicked()), SLOT(save_pressed()));

    //reposition widgets
    reposition_horiz_splitters();
    reposition_cursors();
    //adjust script splitter
    ui->splitter_script->setStretchFactor(0,8);
    ui->splitter_script->setStretchFactor(1,1);
}

void ScriptDialog::reposition_horiz_splitters(){
    foreach(QSplitter *s, ui->tabInfo->findChildren<QSplitter*>()){
        if(s->orientation() != Qt::Horizontal)
            continue;
        s->setStretchFactor(0,1);
        s->setStretchFactor(1,3);
    }
}

void ScriptDialog::reposition_cursors(){
    foreach(QTextEdit *te, ui->tabInfo->findChildren<QTextEdit*>()){
        te->moveCursor(QTextCursor::Start);
        te->ensureCursorVisible();
    }
}

ScriptDialog::~ScriptDialog(){
    delete ui;
}

void ScriptDialog::clear_script() {
    ui->script_edit->clear();
    ui->txt_script_name->clear();
    ui->lbl_save_status->clear();
    m_name = "";
}

void ScriptDialog::load_script(QString name, QString script){
    ui->script_edit->setPlainText(script);
    m_name = name;
    ui->txt_script_name->setText(m_name);
    ui->lbl_save_status->clear();
}

void ScriptDialog::apply_pressed() {
    ui->lbl_save_status->clear();
    if(script_is_valid()){
        emit test_script(ui->script_edit->toPlainText());
        ui->lbl_save_status->setText(tr("Script has been applied but hasn't been saved."));
    }
}

bool ScriptDialog::script_is_valid(){
    QString script = ui->script_edit->toPlainText().trimmed();
    Dwarf *m_dwarf = 0;
    if(DT->get_dwarves().count()>0){
        m_dwarf = DT->get_dwarves().at(0);
    }
    if(!script.isEmpty() && m_dwarf){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(m_dwarf);
        m_engine.globalObject().setProperty("d", d_obj);
        QJSValue ret = m_engine.evaluate(script);
        if(!ret.isBool()){
            QString err_msg;
            if(ret.isError()) {
                err_msg = tr("<font color=red>%1: %2<br/>%3</font>")
                                 .arg(ret.property("name").toString())
                                 .arg(ret.property("message").toString())
                                 .arg(ret.property("stack").toString().replace("\n", "<br/>"));
            }else{
                m_engine.globalObject().setProperty("__internal_script_return_value_check", ret);
                err_msg = tr("<font color=red>Script returned %1 instead of boolean</font>")
                                 .arg(m_engine.evaluate(QString("typeof __internal_scripte_return_value_check")).toString());
                m_engine.globalObject().deleteProperty("__internal_script_return_value_check");
            }
            ui->script_edit->setStatusTip(err_msg);
            ui->txt_status_tip->setText(err_msg);
            return false;
        }else{
            ui->script_edit->setStatusTip(ui->script_edit->whatsThis());
        }
    }else{
        ui->script_edit->setStatusTip(ui->script_edit->whatsThis());
    }
    return true;
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

void ScriptDialog::close_pressed(){
    this->reject();
}

bool ScriptDialog::event(QEvent *evt) {
    if (evt->type() == QEvent::StatusTip) {
        ui->txt_status_tip->setHtml(static_cast<QStatusTipEvent*>(evt)->tip());
        return true; // we've handled it, don't pass it
    }
    return QWidget::event(evt); // pass the event along the chain
}
