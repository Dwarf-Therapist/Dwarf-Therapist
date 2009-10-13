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

#include "gridviewdialog.h"
#include "ui_gridviewdialog.h"
#include "viewmanager.h"
#include "gridview.h"
#include "viewcolumnset.h"
#include "viewcolumn.h"
#include "spacercolumn.h"
#include "happinesscolumn.h"
#include "laborcolumn.h"
#include "skillcolumn.h"
#include "idlecolumn.h"
#include "traitcolumn.h"
#include "defines.h"
#include "statetableview.h"
#include "gamedatareader.h"
#include "labor.h"
#include "utils.h"
#include "trait.h"
#include "ui_columneditdialog.h"

GridViewDialog::GridViewDialog(ViewManager *mgr, GridView *view, QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::GridViewDialog)
	, m_view(view)
	, m_pending_view(new GridView((const GridView)*view))
	, m_manager(mgr)
	, m_is_editing(false)
	, m_set_model(new QStandardItemModel)
	, m_col_model(new QStandardItemModel)
{
	ui->setupUi(this);
	ui->list_sets->setModel(m_set_model);
	ui->list_columns->setModel(m_col_model);

	connect(ui->list_sets->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
		SLOT(set_selection_changed(const QItemSelection&, const QItemSelection&)));

	if (!m_pending_view->name().isEmpty()) { // looks like this is an edit...
		ui->le_name->setText(m_pending_view->name());
		draw_sets();
		ui->buttonBox->setEnabled(!m_pending_view->name().isEmpty());
		m_is_editing = true;
		m_original_name = m_pending_view->name();
	}
	ui->list_sets->installEventFilter(this);
	ui->list_columns->installEventFilter(this);

	/*/ TODO: show a STV with a preview using this gridview...
	StateTableView *stv = new StateTableView(this);
	QStandardItemModel *m = new QStandardItemModel(this);
	stv->setModel(m);
	ui->vbox->addWidget(stv, 10);
	*/
	
	connect(ui->list_sets, SIGNAL(activated(const QModelIndex &)), SLOT(edit_set(const QModelIndex &)));
	connect(ui->list_sets, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_set_context_menu(const QPoint &)));
	connect(ui->list_columns, SIGNAL(activated(const QModelIndex &)), SLOT(edit_column(const QModelIndex &)));
	connect(ui->list_columns, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(draw_column_context_menu(const QPoint &)));
	connect(ui->le_name, SIGNAL(textChanged(const QString &)), SLOT(check_name(const QString &)));
}

QString GridViewDialog::name() {
	 return ui->le_name->text();
}

bool GridViewDialog::eventFilter(QObject *sender, QEvent *evt) {
	if (evt->type() == QEvent::ChildRemoved) {
		if (sender == ui->list_columns)
			column_order_changed();
		else if (sender == ui->list_sets)
			set_order_changed();
	}
	return false; // don't actually interrupt anything
}

void GridViewDialog::set_order_changed() {
	m_pending_view->reorder_sets(*m_set_model);
}

void GridViewDialog::column_order_changed() {
	m_active_set->reorder_columns(*m_col_model);
}

void GridViewDialog::draw_sets() {
	m_set_model->clear();
	m_col_model->clear();
	foreach(ViewColumnSet *set, m_pending_view->sets()) {
		QStandardItem *set_item = new QStandardItem(set->name());
		set_item->setBackground(set->bg_color());
		set_item->setForeground(compliment(set->bg_color()));
		set_item->setDropEnabled(false);
		set_item->setData(set->name(), GPDT_TITLE);
		set_item->setData(set->bg_color(), GPDT_BG_COLOR);
		m_set_model->appendRow(set_item);
	}
	ui->list_sets->selectionModel()->select(m_set_model->index(0,0), QItemSelectionModel::SelectCurrent);
}

void GridViewDialog::set_selection_changed(const QItemSelection &selected, const QItemSelection &) {
	if (selected.indexes().size() != 1)
		return;
	QStandardItem *set_item = m_set_model->itemFromIndex(selected.indexes().at(0));
	foreach(ViewColumnSet *set, m_pending_view->sets()) {
		if (set->name() == set_item->data(GPDT_TITLE).toString()) {
			m_active_set = set;
			draw_columns_for_set(m_active_set);
		}
	}
}

void GridViewDialog::draw_columns_for_set(ViewColumnSet *set) {
	m_col_model->clear();
	foreach(ViewColumn *vc, set->columns()) {
		QStandardItem *item = new QStandardItem(vc->title());
		item->setData(vc->title(), GPDT_TITLE);
		item->setData(vc->type(), GPDT_COLUMN_TYPE);
		item->setBackground(vc->override_color() ? vc->bg_color() : set->bg_color());
		item->setForeground(compliment(vc->override_color() ? vc->bg_color() : set->bg_color()));
		item->setDropEnabled(false);
		m_col_model->appendRow(item);
	}
}

void GridViewDialog::check_name(const QString &name) {
	ui->buttonBox->setDisabled(name.isEmpty());
}

void GridViewDialog::add_set() {
	QRegExp rx = QRegExp("^New Set (\\d+)$");
	int highest_new_set_number = 0;
	foreach(ViewColumnSet *set, m_pending_view->sets()) {
		if (rx.indexIn(set->name()) != -1) {
			if (rx.cap(1).toInt() > highest_new_set_number)
				highest_new_set_number = rx.cap(1).toInt();
		}
	}

	ViewColumnSet *set = new ViewColumnSet(tr("New Set %1").arg(highest_new_set_number + 1), m_manager);
	m_pending_view->add_set(set);
	draw_sets();
}

void GridViewDialog::edit_set() {
	if (m_temp_set < 0)
		return;
	edit_set(m_set_model->index(m_temp_set, 0));
}

void GridViewDialog::edit_set(const QModelIndex &idx) {
	QStandardItem *item = m_set_model->itemFromIndex(idx);

	QDialog *d = new QDialog(this);
	QVBoxLayout *vbox = new QVBoxLayout(d);
	d->setLayout(vbox);

	QFormLayout *form = new QFormLayout(d);
	QLineEdit *le_name = new QLineEdit(item->text(), d);
	QtColorPicker *cp = new QtColorPicker(d);
	cp->setCurrentColor(item->background().color());
	cp->setStandardColors();

	form->addRow(tr("Name of set"), le_name);
	form->addRow(tr("Background color"), cp);

	QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, d);
	vbox->addLayout(form, 10);
	vbox->addWidget(buttons);

	connect(buttons, SIGNAL(accepted()), d, SLOT(accept()));
	connect(buttons, SIGNAL(rejected()), d, SLOT(reject()));

	if (d->exec()) {
		ViewColumnSet *set = m_pending_view->get_set(item->data(GPDT_TITLE).toString());
		set->set_name(le_name->text());
		set->set_bg_color(cp->currentColor());
		draw_sets();
		draw_columns_for_set(set);
	}
	d->deleteLater();
}

void GridViewDialog::remove_set() {
	QModelIndexList selected = ui->list_sets->selectionModel()->selectedIndexes();
	QList<ViewColumnSet*> sets_to_remove;
	foreach(QModelIndex idx, selected) {
		sets_to_remove << m_pending_view->get_set(idx.row());
	}
	foreach(ViewColumnSet *set, sets_to_remove) {
		m_pending_view->remove_set(set);
	}
	draw_sets();
}

void GridViewDialog::draw_set_context_menu(const QPoint &p) {
	QMenu m(this);
	QModelIndex idx = ui->list_sets->indexAt(p);
	if (idx.isValid()) {
		m.addAction(QIcon(":img/application_edit.png"), tr("Edit..."), this, SLOT(edit_set()));
		m.addAction(QIcon(":img/delete.png"), tr("Remove"), this, SLOT(remove_set()));
		m_temp_set = idx.row();
	} else {
		m.addAction(tr("Add New Set"), this, SLOT(add_set()));
	}
	m.exec(ui->list_sets->viewport()->mapToGlobal(p));
}


void GridViewDialog::edit_column() {
	if (m_temp_col < 0)
		return;
	edit_column(m_col_model->index(m_temp_col, 0));
}

void GridViewDialog::edit_column(const QModelIndex &idx) {
	ViewColumn *vc = m_active_set->column_at(idx.row());
	
	// build the column dialog
	QDialog *d = new QDialog(this);
	Ui::ColumnEditDialog *dui = new Ui::ColumnEditDialog;
	dui->setupUi(d);
	d->setModal(true);

	connect(dui->cb_override, SIGNAL(toggled(bool)), dui->cp_bg_color, SLOT(setEnabled(bool)));

	if (vc->override_color())
		dui->cp_bg_color->setCurrentColor(vc->bg_color());
	else
		dui->cp_bg_color->setCurrentColor(m_active_set->bg_color());
	dui->cp_bg_color->setStandardColors();
	dui->le_title->setText(vc->title());
	dui->cb_override->setChecked(vc->override_color());
	if (vc->type() == CT_SPACER) {
		SpacerColumn *c = static_cast<SpacerColumn*>(vc);
		dui->sb_width->setValue(c->width());
	} else { // don't show the width form for non-spacer columns
		dui->lbl_col_width->hide();
		dui->sb_width->hide();
		dui->verticalLayout->removeItem(dui->hbox_width);
	}

	if (d->exec()) { //accepted
		vc->set_title(dui->le_title->text());
		vc->set_override_color(dui->cb_override->isChecked());
		if (dui->cb_override->isChecked()) {
			vc->set_bg_color(dui->cp_bg_color->currentColor());
		}
		if (vc->type() == CT_SPACER) {
			SpacerColumn *c = static_cast<SpacerColumn*>(vc);
			int w = dui->sb_width->value();
			if (w < 1)
				w = DEFAULT_SPACER_WIDTH;
			c->set_width(w);
		}
		draw_columns_for_set(m_active_set);
	}
	delete dui;
}

void GridViewDialog::remove_column() {
	QModelIndexList selected = ui->list_columns->selectionModel()->selectedIndexes();
	QList<ViewColumn*> cols_to_remove;
	foreach(QModelIndex idx, selected) {
		cols_to_remove << m_active_set->column_at(idx.row());
	}
	foreach(ViewColumn *vc, cols_to_remove) {
		m_active_set->remove_column(vc);
	}
	draw_columns_for_set(m_active_set);
}

void GridViewDialog::draw_column_context_menu(const QPoint &p) {
	QMenu *m = new QMenu(this);
	QModelIndex idx = ui->list_columns->indexAt(p);
	if (idx.isValid()) { // context on a column
		m->addAction(QIcon(":img/application_edit.png"), tr("Edit..."), this, SLOT(edit_column()));
		m->addAction(QIcon(":img/delete.png"), tr("Remove"), this, SLOT(remove_column()));
		m_temp_col = idx.row();
	} else { // in whitespace
		GameDataReader *gdr = GameDataReader::ptr();
		QAction *a = m->addAction("Add Spacer", this, SLOT(add_spacer_column()));
		a->setToolTip(tr("Adds a non-selectable spacer to this set. You can set a custom width and color on spacer columns."));

		QMenu *m_labor = m->addMenu(tr("Add Labor Column"));
		m_labor->setToolTip(tr("Labor columns function as toggle switches for individual labors on a dwarf."));
		QMenu *labor_a_l = m_labor->addMenu(tr("A-I"));
		QMenu *labor_j_r = m_labor->addMenu(tr("J-R"));
		QMenu *labor_s_z = m_labor->addMenu(tr("S-Z"));
		foreach(Labor *l, gdr->get_ordered_labors()) {
			QMenu *menu_to_use = labor_a_l;
			if (l->name.at(0).toLower() > 'i')
				menu_to_use = labor_j_r;
			if (l->name.at(0).toLower() > 'r')
				menu_to_use = labor_s_z;
			QAction *a = menu_to_use->addAction(l->name, this, SLOT(add_labor_column()));
			a->setData(l->labor_id);
			a->setToolTip(tr("Add a column for labor %1 (ID%2)").arg(l->name).arg(l->labor_id));
		}

		QMenu *m_skill = m->addMenu(tr("Add Skill Column"));
		m_skill->setToolTip(tr("Skill columns function as a read-only display of a dwarf's skill in a particular area."
			" Note that you can add skill columns for labors but they won't work as toggles."));
		QMenu *skill_a_l = m_skill->addMenu(tr("A-I"));
		QMenu *skill_j_r = m_skill->addMenu(tr("J-R"));
		QMenu *skill_m_z = m_skill->addMenu(tr("S-Z"));
		QPair<int, QString> skill_pair;
		foreach(skill_pair, gdr->get_ordered_skills()) {
			QMenu *menu_to_use = skill_a_l;
			if (skill_pair.second.at(0).toLower() > 'i')
				menu_to_use = skill_j_r;
			if (skill_pair.second.at(0).toLower() > 'r')
				menu_to_use = skill_m_z;
			QAction *a = menu_to_use->addAction(skill_pair.second, this, SLOT(add_skill_column()));
			a->setData(skill_pair.first);
			a->setToolTip(tr("Add a column for skill %1 (ID%2)").arg(skill_pair.second).arg(skill_pair.first));
		}

        QMenu *m_trait = m->addMenu(tr("Add Trait Column"));
        m_trait->setToolTip(tr("Trait columns show a read-only display of a dwarf's score in a particular trait."));
        QMenu *trait_a_l = m_trait->addMenu(tr("A-I"));
        QMenu *trait_j_r = m_trait->addMenu(tr("J-R"));
        QMenu *trait_m_z = m_trait->addMenu(tr("S-Z"));
        QHash<int, Trait*> traits = gdr->get_traits();
        foreach(int trait_id, traits.uniqueKeys()) {
            Trait *t = traits.value(trait_id);
            QMenu *menu_to_use = trait_a_l;
            if (t->name.at(0).toLower() > 'i')
                menu_to_use = trait_j_r;
            if (t->name.at(0).toLower() > 'r')
                menu_to_use = trait_m_z;
            QAction *a = menu_to_use->addAction(t->name, this, SLOT(add_trait_column()));
            a->setData(trait_id);
            a->setToolTip(tr("Add a column for trait %1 (ID%2)").arg(t->name).arg(trait_id));
        }

		a = m->addAction(tr("Add Happiness"), this, SLOT(add_happiness_column()));
		a->setToolTip(tr("Adds a single column that shows a color-coded happiness indicator for "
			"each dwarf. You can customize the colors used in the options menu."));
        
        a = m->addAction(tr("Add Idle/Current Job"), this, SLOT(add_idle_column()));
        a->setToolTip(tr("Adds a single column that shows a the current idle state for a dwarf."));
	}
	m->exec(ui->list_columns->viewport()->mapToGlobal(p));
}

void GridViewDialog::add_spacer_column() {
	if (!m_active_set)
		return;
	
	new SpacerColumn("SPACER", m_active_set, m_active_set);
	draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_happiness_column() {
	if (!m_active_set)
		return;
	new HappinessColumn("Happiness", m_active_set, m_active_set);
	draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_idle_column() {
    if (!m_active_set)
        return;
    new IdleColumn("Idle", m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_labor_column() {
	if (!m_active_set)
		return;
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int labor_id = a->data().toInt();
	Labor *l = GameDataReader::ptr()->get_labor(labor_id);
	new LaborColumn(l->name, l->labor_id, l->skill_id, m_active_set, m_active_set);
	draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_skill_column() {
	if (!m_active_set)
		return;
	QAction *a = qobject_cast<QAction*>(QObject::sender());
	int skill_id = a->data().toInt();
	new SkillColumn(GameDataReader::ptr()->get_skill_name(skill_id), skill_id, m_active_set, m_active_set);
	draw_columns_for_set(m_active_set);
}

void GridViewDialog::add_trait_column() {
    if (!m_active_set)
        return;
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    int trait_id = a->data().toInt();
    new TraitColumn(GameDataReader::ptr()->get_trait(trait_id)->name, trait_id, m_active_set, m_active_set);
    draw_columns_for_set(m_active_set);
}

void GridViewDialog::accept() {
	if (ui->le_name->text().isEmpty()) {
		QMessageBox::warning(this, tr("Empty Name"), tr("Cannot save a view with no name!"));
		return;
	} else if (m_manager->get_view(ui->le_name->text())) {
		// this name exists
		if (!m_is_editing || (m_is_editing && m_original_name != ui->le_name->text())) {
			QMessageBox m(QMessageBox::Question, tr("Overwrite View?"),
				tr("There is already a view named <b>%1</b><h3>Do you want to overwrite it?</h3>").arg(ui->le_name->text()),
				QMessageBox::Yes | QMessageBox::No, 0);
			if (m.exec() == QMessageBox::Yes) {
				return QDialog::accept();
			} else {
				return;
			}
		}
	}
	m_pending_view->set_name(ui->le_name->text());
	return QDialog::accept();
}