#include <QtCore>
#include "memorylayout.h"
#include "gamedatareader.h"
#include "utils.h"
#include "truncatingfilelogger.h"

MemoryLayout::MemoryLayout(const QString &filename)
    : m_filename(filename)
    , m_checksum(0)
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
