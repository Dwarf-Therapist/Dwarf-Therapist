#include <QtCore>
#include "memorylayout.h"
#include "gamedatareader.h"
#include "utils.h"
#include "truncatingfilelogger.h"
#include "dfinstance.h"

MemoryLayout::MemoryLayout(const QString &filename)
    : m_filename(filename)
    , m_checksum(QString::null)
    , m_data(0)
    , m_complete(true)
{
    TRACE << "Attempting to contruct MemoryLayout from file " << filename;
    QFileInfo info(m_filename);
    if (info.exists() && info.isReadable()) {
        m_data = new QSettings(m_filename, QSettings::IniFormat);
        load_data();
    } else {
        LOGE << m_filename << "could either not be found or not be opened!";
    }
}

MemoryLayout::MemoryLayout(const QString & filename, QSettings * data):
    m_filename(filename),
    m_checksum(QString::null),
    m_data(NULL),
    m_complete(false)
{
    m_data = new QSettings(m_filename, QSettings::IniFormat);
    foreach(QString key, data->allKeys()) {
        m_data->setValue(key, data->value(key));
    }
}


void MemoryLayout::load_data() {
    if (!is_valid()) {
        LOGE << "Skipping read of invalid memory layout in" << m_filename;
        return;
    }

    // basics (if these are missing, don't read anything else)
    m_data->beginGroup("info");
    m_checksum = m_data->value("checksum", "UNKNOWN").toString().toLower();
    m_game_version = m_data->value("version_name", "UNKNOWN").toString().toLower();
    m_complete = m_data->value("complete", true).toBool();
    m_data->endGroup();

    read_group("addresses", m_addresses);
    read_group("offsets", m_offsets);
    read_group("dwarf_offsets", m_dwarf_offsets);
    read_group("job_details", m_job_details);
    read_group("soul_details", m_soul_details);
    read_group("squad_offsets", m_squad_offsets);

    // flags
    int flag_count = m_data->beginReadArray("valid_flags_1");
    for (int i = 0; i < flag_count; ++i) {
        m_data->setArrayIndex(i);
        m_valid_flags_1.insert(read_hex("value"),
            m_data->value("name", "UNKNOWN VALID FLAG 1").toString());
    }
    m_data->endArray();

    flag_count = m_data->beginReadArray("invalid_flags_1");
    for (int i = 0; i < flag_count; ++i) {
        m_data->setArrayIndex(i);
        m_invalid_flags_1.insert(read_hex("value"),
            m_data->value("name", "UNKNOWN INVALID FLAG 1").toString());
    }
    m_data->endArray();

    flag_count = m_data->beginReadArray("invalid_flags_2");
    for (int i = 0; i < flag_count; ++i) {
        m_data->setArrayIndex(i);
        m_invalid_flags_2.insert(read_hex("value"),
            m_data->value("name", "UNKNOWN INVALID FLAG 2").toString());
    }
    m_data->endArray();
}

uint MemoryLayout::read_hex(QString key) {
    bool ok;
    QString data = m_data->value(key, -1).toString();
    uint val = data.toUInt(&ok, 16);
    return val;
}

bool MemoryLayout::is_valid() {
    return m_data != NULL && m_data->contains("info/checksum")
            && m_data->contains("info/version_name");
}

void MemoryLayout::read_group(const QString &group, AddressHash &map) {
    m_data->beginGroup(group);
    foreach(QString k, m_data->childKeys()) {
        map.insert(k, read_hex(k));
    }
    m_data->endGroup();
}

uint MemoryLayout::string_buffer_offset() {
    return m_offsets.value("string_buffer_offset", DFInstance::STRING_BUFFER_OFFSET);
}

uint MemoryLayout::string_length_offset() {
    return string_buffer_offset() +
            m_offsets.value("string_length_offset", DFInstance::STRING_LENGTH_OFFSET);
}

uint MemoryLayout::string_cap_offset() {
    return string_buffer_offset() +
            m_offsets.value("string_cap_offset", DFInstance::STRING_CAP_OFFSET);
}

void MemoryLayout::set_address(const QString & key, uint value) {
    m_data->setValue(key, hexify(value));
}

void MemoryLayout::set_game_version(const QString & value) {
    m_game_version = value;
    m_data->setValue("info/version_name", m_game_version);
}

void MemoryLayout::set_checksum(const QString & checksum) {
    m_checksum = checksum;
    m_data->setValue("info/checksum", m_checksum);
}

void MemoryLayout::save_data() {
    m_data->sync();
}

void MemoryLayout::set_complete() {
    m_complete = true;
    m_data->setValue("info/complete", "true");
}



