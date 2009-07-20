#ifndef CUSTOM_COLOR_H
#define CUSTOM_COLOR_H

#include <QtGui>
#include <qtcolorpicker.h>

class CustomColor : public QWidget {
	Q_OBJECT
public:
	CustomColor(QString setting_name, QString tooltip, QString config_key, QColor default_color, QWidget *parent = 0);

	void set_color(QColor new_color) {m_picker->setCurrentColor(new_color);}
	QColor get_color() {return m_picker->currentColor();}
	QColor get_default() {return m_default;}
	bool is_dirty() {return m_dirty;}
	void reset_to_default() {m_picker->setCurrentColor(m_default);}
	void reset_to_last() {m_picker->setCurrentColor(m_last_color);}
	QString get_config_key() {return m_config_key;}
	

private:
	QString m_name;
	QString m_tooltip;
	QString m_config_key;
	QtColorPicker *m_picker;
	QLabel *m_label;
	QColor m_default;
	QColor m_last_color;

	bool m_dirty;

	private slots:
		void color_changed(const QColor &);

signals:
	void color_changed(QString config_key, const QColor &new_color);
};

#endif;