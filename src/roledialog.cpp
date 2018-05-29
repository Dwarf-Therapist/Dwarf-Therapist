/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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

#include "roledialog.h"

#include <QDoubleSpinBox>
#include <QJSEngine>
#include <QMenu>
#include <QMessageBox>

#include "adaptivecolorfactory.h"
#include "dwarf.h"
#include "gamedatareader.h"
#include "mainwindow.h"
#include "role.h"
#include "rolemodel.h"
#include "rolepreferencemodel.h"
#include "standardpaths.h"
#include "trait.h"
#include "viewmanager.h"

#include "ui_roledialog.h"

class WeightEditorCreator: public QItemEditorCreatorBase
{
public:
    QWidget *createWidget(QWidget *parent) const override
    {
        auto spinbox = new QDoubleSpinBox(parent);
        spinbox->setMinimum(-100.0);
        spinbox->setMaximum(+100.0);
        spinbox->setSingleStep(0.25);
        return spinbox;
    }

    QByteArray valuePropertyName() const override
    {
        return "value";
    }
};

RoleDialog::RoleDialog(RolePreferenceModel *pref_model, QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::RoleDialog>())
    , m_dwarf(nullptr)
    , m_pref_model(pref_model)
    , m_model (std::make_unique<RoleModel>())
{
    ui->setupUi(this);
    update_role_preview();

    // Setup aspects tree view
    m_weight_editor_factory.registerEditor(QVariant::Double, new WeightEditorCreator);
    m_weight_delegate.setItemEditorFactory(&m_weight_editor_factory);
    ui->tree_aspects->setItemDelegateForColumn(RoleModel::ColumnWeight, &m_weight_delegate);
    ui->tree_aspects->setModel(m_model.get());

    connect(ui->tree_aspects, &QWidget::customContextMenuRequested,
            this, &RoleDialog::aspect_tree_context_menu);
    ui->tree_aspects->setContextMenuPolicy(Qt::CustomContextMenu);

    // Build models
    GameDataReader *gdr = GameDataReader::ptr();
    m_attribute_model.setHorizontalHeaderLabels({"Attributes"});
    for (const auto &p: gdr->get_ordered_attribute_names()) {
        auto item = new QStandardItem();
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setText(p.second);
        item->setToolTip(tr("Include %1 as an aspect for this role.").arg(p.second));
        item->setData(p.second);
        m_attribute_model.appendRow(item);
    }
    m_skill_model.setHorizontalHeaderLabels({"Skills"});
    for (auto skill: gdr->get_ordered_skills()) {
        auto item = new QStandardItem();
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setText(skill->noun);
        item->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.")
                .arg(skill->noun)
                .arg(skill->id));
        item->setData(skill->id);
        m_skill_model.appendRow(item);
    }
    m_facet_model.setHorizontalHeaderLabels({"Facets"});
    for (const auto &p: gdr->get_ordered_traits()) {
        auto item = new QStandardItem();
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        item->setText(p.second->get_name());
        item->setToolTip(tr("Include %1 (ID %2) as an aspect for this role.")
                .arg(p.second->get_name())
                .arg(p.first));
        item->setData(p.first);
        m_facet_model.appendRow(item);
    }

    ui->attribute_list->set_model(&m_attribute_model);
    ui->skill_list->set_model(&m_skill_model);
    ui->facet_list->set_model(&m_facet_model);
    ui->preference_list->set_model(pref_model);

    connect(ui->attribute_list, &SearchFilterTreeView::item_activated,
            this, &RoleDialog::attribute_activated);
    connect(ui->skill_list, &SearchFilterTreeView::item_activated,
            this, &RoleDialog::skill_activated);
    connect(ui->facet_list, &SearchFilterTreeView::item_activated,
            this, &RoleDialog::facet_activated);
    connect(ui->preference_list, &SearchFilterTreeView::item_activated,
            this, &RoleDialog::preference_activated);

    // connect buttons
    connect(ui->btn_copy, &QAbstractButton::pressed,
            this, &RoleDialog::copy_role);
    connect(ui->btn_refresh_ratings, &QAbstractButton::pressed,
            this, &RoleDialog::update_role_preview);
}

RoleDialog::~RoleDialog()
{
}

void RoleDialog::new_role()
{
    setWindowTitle(tr("Creating new custom role"));
    m_role = std::make_unique<Role>();
    m_old_role = nullptr;
    ui->le_role_name->clear();
    m_model->set_role(m_role.get());
    ui->te_script->clear();
}

void RoleDialog::open_role(const QString &name)
{
    GameDataReader *gdr = GameDataReader::ptr();
    auto src_role = gdr->get_roles().value(name, nullptr);
    if (!src_role) {
        LOGE << "Role" << name << "not found for editing";
        return;
    }

    setWindowTitle(tr("Editing %1 custom role").arg(name));
    m_role = std::make_unique<Role>(*src_role);
    m_old_role = src_role;
    ui->le_role_name->setText(m_role->name());
    m_model->set_role(m_role.get());
    ui->te_script->setPlainText(m_role->script());
}

bool RoleDialog::save_role()
{
    QString new_name = ui->le_role_name->text().trimmed();
    if (new_name.isEmpty()) {
        QMessageBox::critical(this, tr("Invalid Role Name"), tr("Role names cannot be blank."));
        return false;
    }
    m_role->name(new_name);
    m_role->script(ui->te_script->toPlainText());
    m_role->is_custom(true);
    m_role->create_role_details(*StandardPaths::settings().get());

    ui->le_role_name->clear();
    m_model->set_role(nullptr);
    ui->te_script->clear();

    auto gdr = GameDataReader::ptr();
    if (m_old_role) {
        gdr->get_roles().remove(m_old_role->name());
        m_old_role = nullptr;
    }
    gdr->get_roles().remove(new_name);
    gdr->get_roles().insert(new_name, m_role.release());
    return true;
}

bool RoleDialog::event(QEvent *event)
{
    if (event->type() == QEvent::StatusTip) {
        ui->txt_status_tip->setHtml(reinterpret_cast<QStatusTipEvent*>(event)->tip());
        return true; // we've handled it, don't pass it
    }
    return QDialog::event(event); // pass the event along the chain
}

void RoleDialog::selection_changed()
{
    QList<Dwarf*> dwarfs = DT->get_main_window()->get_view_manager()->get_selected_dwarfs();
    if(dwarfs.count() > 0)
        m_dwarf = dwarfs.at(0);
    else
        m_dwarf = nullptr;
    update_role_preview();
}

void RoleDialog::done(int r)
{
    if (r == QDialog::Accepted) {
        if (save_role())
            QDialog::done(r);
    }
    else { // rejected
        QDialog::done(r);
    }
}

void RoleDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);

    m_model->update_default_weights();
    m_pref_model->load_pref_from_raws(this);

    // Fill copy combobox with roles
    ui->cmb_copy->clear();
    auto gdr = GameDataReader::ptr();
    for (const auto &p: gdr->get_ordered_roles()) {
        ui->cmb_copy->addItem(p.first);
    };
}

void RoleDialog::attribute_activated(const QModelIndex &index)
{
    auto item = m_attribute_model.itemFromIndex(index);
    auto attr = item->data().toString();
    ui->tree_aspects->setCurrentIndex(m_model->add_attribute(attr));
}

void RoleDialog::skill_activated(const QModelIndex &index)
{
    auto item = m_skill_model.itemFromIndex(index);
    auto id = item->data().toInt();
    ui->tree_aspects->setCurrentIndex(m_model->add_skill(id));
}

void RoleDialog::facet_activated(const QModelIndex &index)
{
    auto item = m_facet_model.itemFromIndex(index);
    auto id = item->data().toInt();
    ui->tree_aspects->setCurrentIndex(m_model->add_facet(id));
}

void RoleDialog::preference_activated(const QModelIndex &index)
{
    auto pref = m_pref_model->getPreference(index);
    ui->tree_aspects->setCurrentIndex(m_model->add_preference(pref));
}

void RoleDialog::aspect_tree_context_menu(const QPoint &pos)
{
    auto index = ui->tree_aspects->indexAt(pos);

    QMenu menu (ui->tree_aspects);
    if (index.isValid()) {
        if (index.parent().isValid()) {
            menu.addAction(ui->remove_action);
        }
        else {
            menu.addAction(ui->reset_default_weight_action);
        }
    }
    if (!menu.actions().isEmpty())
        menu.addSeparator();
    menu.addAction(ui->collapse_tree_aspects_action);
    menu.addAction(ui->expand_tree_aspects_action);

    auto result = menu.exec(ui->tree_aspects->viewport()->mapToGlobal(pos));
    if (result == ui->remove_action) {
        m_model->remove_item(index);
    }
    else if (result == ui->reset_default_weight_action) {
        m_model->reset_default_weight(index);
    }
}

void RoleDialog::copy_role()
{
    auto answer = QMessageBox::Yes;
    if(!m_role->traits.empty() || !m_role->skills.empty() || !m_role->traits.empty() || !m_role->prefs.empty()) {
        answer = QMessageBox::question(
                    0, tr("Confirm Copy"),
                    tr("Copying this role will replace all current aspects with the selected role's aspects. Continue?"),
                    QMessageBox::Yes | QMessageBox::No);
    }
    if (answer != QMessageBox::Yes)
        return;

    auto role_name = ui->cmb_copy->currentText();
    auto gdr = GameDataReader::ptr();
    m_model->set_role(nullptr);
    m_role = std::make_unique<Role>(*gdr->get_role(role_name));
    m_model->set_role(m_role.get());
    ui->te_script->setPlainText(m_role->script());
    if (ui->le_role_name->text().trimmed().isEmpty())
        ui->le_role_name->setText(role_name);
}

void RoleDialog::update_role_preview()
{
    if (!m_dwarf) {
        ui->lbl_name->setText("Select a dwarf to view ratings.");
        ui->lbl_current->clear();
        ui->lbl_new->clear();
        return;
    }

    // Check script syntax
    QString script = ui->te_script->toPlainText().trimmed();
    m_role->script(script);
    if(!script.isEmpty()){
        QJSEngine m_engine;
        QJSValue d_obj = m_engine.newQObject(m_dwarf);
        m_engine.globalObject().setProperty("d", d_obj);
        QJSValue ret = m_engine.evaluate(script);
        if(!ret.isNumber()){
            QString err_msg;
            if(ret.isError()) {
                err_msg = tr("<font color=red>%1: %2<br/>%3</font>")
                        .arg(ret.property("name").toString())
                        .arg(ret.property("message").toString())
                        .arg(ret.property("stack").toString().replace("\n", "<br/>"));
            }else{
                m_engine.globalObject().setProperty("__internal_role_return_value_check", ret);
                err_msg = tr("<font color=red>Script returned %1 instead of number</font>")
                        .arg(m_engine.evaluate(QString("typeof __internal_role_return_value_check")).toString());
                m_engine.globalObject().deleteProperty("__internal_role_return_value_check");
            }
            ui->te_script->setStatusTip(err_msg);
            ui->txt_status_tip->setText(err_msg);
            return;
        }else{
            ui->te_script->setStatusTip(ui->te_script->whatsThis());
        }
    }else{
        ui->te_script->setStatusTip(ui->te_script->whatsThis());
    }

    ui->lbl_name->setText(m_dwarf->nice_name());
    ui->lbl_current->setText(tr("Current Raw Rating: %1%")
            .arg(QString::number(m_old_role ? m_dwarf->calc_role_rating(m_old_role) : 0,'g',4)));
    ui->lbl_new->setText(tr("New Raw Rating: %1%")
            .arg(QString::number(m_role ? m_dwarf->calc_role_rating(m_role.get()) : 0,'g',4)));
}

void RoleDialog::on_le_role_name_textChanged(const QString &text)
{
    if(GameDataReader::ptr()->get_default_roles().contains(text.trimmed())){
        ui->le_role_name->setStatusTip("This role has the same name as a default role and will override it.");

        AdaptiveColorFactory adaptive(QPalette::Base, QPalette::Text);
        QPalette pal;
        pal.setColor(QPalette::Base, adaptive.color(QColor::fromRgb(230,161,92,255)));
        ui->le_role_name->setPalette(pal);
    }else{
        ui->le_role_name->setStatusTip("Name of the role.");
        ui->le_role_name->setPalette(QPalette());
    }
    emit event(new QStatusTipEvent(ui->le_role_name->statusTip()));
}
