#include <QtCore>
#include "GameDataReader.h"
#include <QtDebug>

GameDataReader::GameDataReader() {
	QDir working_dir = QDir::current();
	QString filename = working_dir.absoluteFilePath("etc/game_data.ini");
	m_data_settings = new QSettings(filename, QSettings::IniFormat);
}
 
int GameDataReader::get_int_for_key(QString key, short base) {
	if (!m_data_settings->contains(key)) {
		QString error = QString("Couldn't find key '%1' in file '%2'").arg(key).arg(m_data_settings->fileName());
		//qWarning() << error;
		//throw MissingValueException(error.toStdString());
	}
	bool ok;
	QString offset_str = m_data_settings->value(key, QVariant(-1)).toString();
	int val = offset_str.toInt(&ok, base);
	if (!ok) {
		QString error = QString("Key '%1' could not be read as an integer in file '%2'").arg(key).arg(m_data_settings->fileName());
		//qWarning() << error;
		//throw CorruptedValueException(error.toStdString());
	}
	return val;
}

QString GameDataReader::get_string_for_key(QString key) {
	if (!m_data_settings->contains(key)) {
		QString error = QString("Couldn't find key '%1' in file '%2'").arg(key).arg(m_data_settings->fileName());
		//qWarning() << error;
		//throw MissingValueException(error.toStdString());
	}
	return m_data_settings->value(key, QVariant("UNKNOWN")).toString();
}

QString GameDataReader::get_skill_level_name(short level) {
	return get_string_for_key(QString("skill_levels/%1").arg(level));
}

QString GameDataReader::get_skill_name(short skill_id) {
	return get_string_for_key(QString("skill_names/%1").arg(skill_id));
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

GameDataReader *GameDataReader::m_instance = 0;
