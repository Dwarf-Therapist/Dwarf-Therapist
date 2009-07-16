#ifndef CUSTOM_PROFESSION_H
#define CUSTOM_PROFESSION_H

#include <QtGui>
class QxtListWidgetItem;

namespace Ui
{
    class CustomProfessionEditor;
}

class CustomProfession : public QObject {
	Q_OBJECT
public:
	CustomProfession(QObject *parent = 0);

	
	bool is_active(int labor_id);

	int show_builder_dialog(QWidget *parent = 0);

	public slots:
		void add_labor(int labor_id) {set_labor(labor_id, true);}
		void remove_labor(int labor_id) {set_labor(labor_id, false);}
		void set_labor(int labor_id, bool active);
		void set_name(QString name) {m_name = name;}
		void accept();
		void cancel() {return;}
		void item_check_state_changed(QxtListWidgetItem*);

private:
	bool is_valid();

	Ui::CustomProfessionEditor *ui;
	QString m_name;
	QMap<int, bool> m_active_labors;
	QDialog *m_dialog;

};
#endif