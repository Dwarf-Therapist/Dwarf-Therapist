#include <QtCore>
#include "GameDataReader.h"

GameDataReader::GameDataReader() {
	QDir working_dir = QDir::current();
	QString filename = working_dir.absoluteFilePath("etc/game_data.ini");
	m_data_settings = new QSettings(filename, QSettings::IniFormat);
	/*
	foreach(QString k, m_data_settings->allKeys()) {
		qDebug() << "KEY" << k;
	}
	*/
}
 
int GameDataReader::get_int_for_key(QString key) {
	if (!m_data_settings->contains(key)) {
		QString error = QString("Couldn't find key '%1' in file '%2'").arg(key).arg(m_data_settings->fileName());
		throw MissingValueException(error.toStdString());
	}
	bool ok;
	QString offset_str = m_data_settings->value(key, QVariant(0)).toString();
	int val = offset_str.toInt(&ok, 16);
	if (!ok) {
		QString error = QString("Key '%1' could not be read as an integer in file '%2'").arg(key).arg(m_data_settings->fileName());
		throw CorruptedValueException(error.toStdString());
	}
	return val;
}

QString GameDataReader::get_skill_level_name(short level) {
	return m_data_settings->value(QString("skill_levels/%1").arg(level), QVariant("UNKNOWN")).toString();
}

QString GameDataReader::get_skill_name(short skill_id) {
	return m_data_settings->value(QString("skill_names/%1").arg(skill_id), QVariant("UNKNOWN")).toString();
}

QString GameDataReader::get_profession_name(int profession_id) {
	return m_data_settings->value(QString("profession_names/%1").arg(profession_id), QVariant("UNKNOWN")).toString();
}


GameDataReader *GameDataReader::m_instance = 0;
