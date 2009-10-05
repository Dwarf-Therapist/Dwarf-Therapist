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
#include "gamedatareader.h"
#include "labor.h"
#include "trait.h"
#include <QtDebug>

GameDataReader::GameDataReader(QObject *parent)
	: QObject(parent)
{
	QDir working_dir = QDir::current();
	QString filename = working_dir.absoluteFilePath("etc/game_data.ini");
	m_data_settings = new QSettings(filename, QSettings::IniFormat);

	m_data_settings->beginGroup("labors");
	foreach(QString k, m_data_settings->childGroups()) {
		m_data_settings->beginGroup(k);
		Labor *l = new Labor(get_string_for_key("name"), get_int_for_key("id", 10), 
							 get_int_for_key("skill", 10), k.toInt(), this);
        int count = m_data_settings->beginReadArray("excludes");
        for (int i = 0; i < count; ++i) {
            m_data_settings->setArrayIndex(i);
            l->add_exclusive_labor(m_data_settings->value("labor_id").toInt());
        }
        m_data_settings->endArray();
		m_labors[l->labor_id] = l;
		m_data_settings->endGroup();
	}
	m_data_settings->endGroup();

	QStringList labor_names;
	foreach(Labor *l, m_labors) {
		labor_names << l->name;
	}
	qSort(labor_names);
	foreach(QString name, labor_names) {
		foreach(Labor *l, m_labors) {
			if (name == l->name) {
				m_ordered_labors << l;
				break;
			}
		}
	}

	m_data_settings->beginGroup("skill_names");
	foreach(QString k, m_data_settings->childKeys()) {
		int skill_id = k.toInt();
		m_skills.insert(skill_id, m_data_settings->value(k, "UNKNOWN").toString());
	}
	m_data_settings->endGroup();

	QStringList skill_names;
	foreach(QString name, m_skills) {
		skill_names << name;
	}
	qSort(skill_names);
	foreach(QString name, skill_names) {
		foreach(int skill_id, m_skills.uniqueKeys()) {
			if (name == m_skills.value(skill_id)) {
				m_ordered_skills << QPair<int, QString>(skill_id, name);
				break;
			}
		}
	}

	m_data_settings->beginGroup("skill_levels");
	foreach(QString k, m_data_settings->childKeys()) {
		int rating = k.toInt();
		m_skill_levels.insert(rating, m_data_settings->value(k, "UNKNOWN").toString());
	}
	m_data_settings->endGroup();

	m_data_settings->beginGroup("non_labor_professions");
	foreach(QString k, m_data_settings->childKeys()) {
		int profession_id = k.toInt();
		m_non_labor_professions.insert(profession_id, m_data_settings->value(k, "UNKNOWN").toString());
	}
	m_data_settings->endGroup();

	m_data_settings->beginGroup("attribute_levels");
	foreach(QString k, m_data_settings->childKeys()) {
		int num_attributes = k.toInt();
		m_attribute_levels.insert(num_attributes, m_data_settings->value(k).toInt());
	}
	m_data_settings->endGroup();

	int traits = m_data_settings->beginReadArray("traits");
	for(int i = 0; i < traits; ++i) {
		m_data_settings->setArrayIndex(i);
		Trait *t = new Trait(*m_data_settings);
		m_traits.insert(i, t);
	}
	m_data_settings->endArray();
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

Trait *GameDataReader::get_trait(int trait_id) {
	return m_traits.value(trait_id, 0);
}

bool GameDataReader::profession_can_have_labors(const int &profession_id) {
	return !m_non_labor_professions.contains(profession_id);
}

int GameDataReader::get_xp_for_next_attribute_level(int current_number_of_attributes) {
	return m_attribute_levels.value(current_number_of_attributes + 1, 0); // return 0 if we don't know
}

GameDataReader *GameDataReader::m_instance = 0;
