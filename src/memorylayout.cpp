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

    // addresses
    m_data->beginGroup("addresses");
    foreach(QString k, m_data->childKeys()) {
        m_addresses.insert(k, read_hex(k));
    }
    m_data->endGroup();

    // offsets
    m_data->beginGroup("offsets");
    foreach(QString k, m_data->childKeys()) {
        m_offsets.insert(k, read_hex(k));
    }
    m_data->endGroup();

    // dwarf offsets
    m_data->beginGroup("dwarf_offsets");
    foreach(QString k, m_data->childKeys()) {
        m_dwarf_offsets.insert(k, read_hex(k));
    }
    m_data->endGroup();

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
