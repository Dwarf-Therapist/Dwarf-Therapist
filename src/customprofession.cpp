#include <QtGui>
#include <QxtGui>
#include "customprofession.h"
#include "gamedatareader.h"
#include "ui_customprofession.h"
#include "dwarf.h"
#include "defines.h"

CustomProfession::CustomProfession(QObject *parent)
	: QObject(parent)
	, ui(new Ui::CustomProfessionEditor)
	, m_dialog(0)
	, m_dwarf(0)
{}

CustomProfession::CustomProfession(Dwarf *d, QObject *parent)
	: QObject(parent)
	, ui(new Ui::CustomProfessionEditor)
	, m_dialog(0)
	, m_dwarf(d)
{}

void CustomProfession::set_labor(int labor_id, bool active) {
	if (m_active_labors.contains(labor_id) && !active)
		m_active_labors.remove(labor_id);
	if (active)
		m_active_labors.insert(labor_id, true);
}

bool CustomProfession::is_active(int labor_id) {
	return m_active_labors.value(labor_id, false);
}

QVector<int> CustomProfession::get_enabled_labors() {
	QVector<int> labors;
	foreach(int labor, m_active_labors.uniqueKeys()) {
		if (m_active_labors.value(labor)) {
			labors << labor;
		}
	}
	return labors;
}

int CustomProfession::show_builder_dialog(QWidget *parent) {
	GameDataReader *gdr = GameDataReader::ptr();

	m_dialog = new QDialog(parent);
	ui->setupUi(m_dialog);
	ui->name_edit->setText(m_name);
	connect(ui->name_edit, SIGNAL(textChanged(const QString &)), this, SLOT(set_name(QString)));
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	
	QVector<QStringList> labor_pairs = gdr->read_labor_pairs();
	int num_active = 0;
	foreach(QStringList pair, labor_pairs) {
		QxtListWidgetItem *item = new QxtListWidgetItem(pair[1], ui->labor_list);
		int labor_id = pair[0].toInt();
		item->setData(Qt::UserRole, labor_id);
		item->setFlag(Qt::ItemIsUserCheckable, true);
		if (m_dwarf && m_dwarf->is_labor_enabled(labor_id)) {
			item->setCheckState(Qt::Checked);
			add_labor(labor_id);
			num_active++;
		} else if (is_active(labor_id)) {
			item->setCheckState(Qt::Checked);
			num_active++;
		} else {
			item->setCheckState(Qt::Unchecked);
		}
		ui->labor_list->addItem(item);
	}

	connect(ui->labor_list, 
			SIGNAL(itemCheckStateChanged(QxtListWidgetItem*)),
			this,
			SLOT(item_check_state_changed(QxtListWidgetItem*)));


	ui->lbl_skill_count->setNum(num_active);
	return m_dialog->exec();
}

void CustomProfession::accept() {
	if (!is_valid()) {
		return;
	}
	m_dialog->accept();
}

bool CustomProfession::is_valid() {
	if (!m_dialog)
		return true;

	if (ui->name_edit->text().isEmpty()) {
		QMessageBox::warning(m_dialog, tr("Naming Error!"),
			tr("You must enter a name for this custom profession!"));
		return false;
	}
	return true;
	
}

void CustomProfession::item_check_state_changed(QxtListWidgetItem *item) {
	if (item->checkState() == Qt::Checked) {
		add_labor(item->data(Qt::UserRole).toInt());
		ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() + 1);
	} else {
		remove_labor(item->data(Qt::UserRole).toInt());
		ui->lbl_skill_count->setNum(ui->lbl_skill_count->text().toInt() - 1);
	}
}

void CustomProfession::delete_from_disk() {
	QSettings s(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
	s.beginGroup("custom_professions");
	s.remove(m_name);
	s.endGroup();
}