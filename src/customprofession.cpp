#include <QtGui>
#include <QxtGui>
#include "customprofession.h"
#include "gamedatareader.h"
#include "ui_customprofession.h"

CustomProfession::CustomProfession(QObject *parent)
	: QObject(parent)
	, ui(new Ui::CustomProfessionEditor)
	, m_dialog(0)
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

int CustomProfession::show_builder_dialog(QWidget *parent) {
	GameDataReader *gdr = GameDataReader::ptr();

	m_dialog = new QDialog(parent);
	ui->setupUi(m_dialog);
	connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	
	QVector<QStringList> labor_pairs = gdr->read_labor_pairs();
	foreach(QStringList pair, labor_pairs) {
		QxtListWidgetItem *item = new QxtListWidgetItem(pair[1], ui->labor_list);
		item->setData(Qt::UserRole, pair[0].toInt());
		item->setFlag(Qt::ItemIsUserCheckable, true);
		item->setCheckState(Qt::Unchecked);
		ui->labor_list->addItem(item);
	}

	connect(ui->labor_list, 
			SIGNAL(itemCheckStateChanged(QxtListWidgetItem*)),
			this,
			SLOT(item_check_state_changed(QxtListWidgetItem*)));


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