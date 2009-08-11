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
#include <QtGui>
#include "GameDataReader.h"
#include "labor.h"
#include <QtDebug>

GameDataReader::GameDataReader(QObject *parent) :
	QObject(parent)
 {
	QDir working_dir = QDir::current();
	QString filename = working_dir.absoluteFilePath("etc/game_data.ini");
	m_data_settings = new QSettings(filename, QSettings::IniFormat);

	m_data_settings->beginGroup("labors");
	foreach(QString k, m_data_settings->childGroups()) {
		m_data_settings->beginGroup(k);
		Labor *l = new Labor(get_string_for_key("name"), get_int_for_key("id", 10), 
							 get_int_for_key("skill", 10), k.toInt(), this);
		m_labors[l->labor_id] = l;
		m_ordered_labors[l->list_order] = l;
		m_data_settings->endGroup();
	}
	m_data_settings->endGroup();

	m_data_settings->beginGroup("skill_names");
	foreach(QString k, m_data_settings->childKeys()) {
		int skill_id = k.toInt();
		m_skills.insert(skill_id, m_data_settings->value(k, "UNKNOWN").toString());
	}
	m_data_settings->endGroup();

	m_data_settings->beginGroup("skill_levels");
	foreach(QString k, m_data_settings->childKeys()) {
		int rating = k.toInt();
		m_skill_levels.insert(rating, m_data_settings->value(k, "UNKNOWN").toString());
	}
	m_data_settings->endGroup();
}
 
int GameDataReader::get_int_for_key(QString key, short base) {
	if (!m_data_settings->contains(key)) {
		QString error = QString("Couldn't find key '%1' in file '%2'").arg(key).arg(m_data_settings->fileName());
		qWarning() << error;
		//throw MissingValueException(error.toStdString());
	}
	bool ok;
	QString offset_str = m_data_settings->value(key, QVariant(-1)).toString();
	int val = offset_str.toInt(&ok, base);
	if (!ok) {
		QString error = QString("Key '%1' could not be read as an integer in file '%2'").arg(key).arg(m_data_settings->fileName());
		qWarning() << error;
		//throw CorruptedValueException(error.toStdString());
	}
	return val;
}

QString GameDataReader::get_string_for_key(QString key) {
	if (!m_data_settings->contains(key)) {
		QString error = QString("Couldn't find key '%1' in file '%2'").arg(key).arg(m_data_settings->fileName());
		qWarning() << error;
		//throw MissingValueException(error.toStdString());
	}
	return m_data_settings->value(key, QVariant("UNKNOWN")).toString();
}

QColor GameDataReader::get_color(QString key) {
	QString hex_code = get_string_for_key(key);
	bool ok;
	QColor c(hex_code.toInt(&ok, 16));
	if (!ok || !c.isValid())
		c = Qt::white;
	return c;
}

QString GameDataReader::get_skill_level_name(short level) {
	return m_skill_levels.value(level, "UNKNOWN");
	//return get_string_for_key(QString("skill_levels/%1").arg(level));
}

QString GameDataReader::get_skill_name(short skill_id) {
	return m_skills.value(skill_id, "UNKNOWN");
	//return get_string_for_key(QString("skill_names/%1").arg(skill_id));
}

QString GameDataReader::get_profession_name(int profession_id) {
	return get_string_for_key(QString("profession_names/%1").arg(profession_id));
}

QStringList GameDataReader::get_child_groups(QString section) {
	m_data_settings->beginGroup(section);
	QStringList groups = m_data_settings->childGroups();
	m_data_settings->endGroup();
	return groups;
}

QStringList GameDataReader::get_keys(QString section) {
	m_data_settings->beginGroup(section);
	QStringList keys = m_data_settings->childKeys();
	m_data_settings->endGroup();
	return keys;
}

Labor *GameDataReader::get_labor(int labor_id) {
	return m_labors.value(labor_id, new Labor("UNKNOWN", -1, -1, 1000, this));
}

GameDataReader *GameDataReader::m_instance = 0;
