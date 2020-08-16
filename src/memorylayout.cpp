#include "memorylayout.h"
#include "utils.h"
#include "truncatingfilelogger.h"
#include "dfinstance.h"
#include <QCryptographicHash>

MemoryLayout::MemoryLayout(const QFileInfo &fileinfo)
    : m_fileinfo(fileinfo)
    , m_checksum()
    , m_git_sha()
    , m_valid(false)
    , m_complete(false)
{
    TRACE << "Attempting to contruct MemoryLayout from file " << fileinfo.absoluteFilePath();

    if (!m_fileinfo.exists() || !m_fileinfo.isReadable()) {
        LOGE << m_fileinfo.absoluteFilePath() << "could either not be found or not be opened!";
        return;
    }

    QSettings data(m_fileinfo.absoluteFilePath(), QSettings::IniFormat);

    m_valid = data.contains("info/checksum") && data.contains("info/version_name");
    if (!m_valid) {
        LOGE << "Skipping read of invalid memory layout in" << m_fileinfo.absoluteFilePath();
        return;
    }

    //read the file data, and generate the git sha; the format is blob <contentSize>\0<content>
    QFile file(m_fileinfo.absoluteFilePath());
    if(file.open(QIODevice::ReadOnly)){
        QString file_data = QString(file.readAll()).replace("\r\n","\n"); //line endings need to match the git config
        file_data.prepend(QString("blob %1%2")
                          .arg(QString::number(file_data.size()))
                          .arg(QChar('\0')));
        m_git_sha = QCryptographicHash::hash(file_data.toLocal8Bit(),QCryptographicHash::Sha1).toHex();
    }
    file.close();

    // basics (if these are missing, don't read anything else)
    data.beginGroup("info");
    m_checksum = data.value("checksum", "UNKNOWN").toString().toLower();
    m_game_version = data.value("version_name", "UNKNOWN").toString().toLower();
    m_complete = data.value("complete", true).toBool();
    data.endGroup();

    //load offsets by section
    for(int idx = 0; idx < MEM_COUNT; idx++){
        read_group(static_cast<MEM_SECTION>(idx), data);
    }
    //load flags
    for(int idx = 0; idx < FLAG_TYPE_COUNT; idx++){
        read_flags(static_cast<UNIT_FLAG_TYPE>(idx), data);
    }
}

static unsigned long long read_hex(QSettings &data, QString key) {
    bool ok;
    QString value = data.value(key, -1).toString();
    unsigned long long val = value.toULongLong(&ok, 16);
    if (!ok) {
        LOGE << "Failed to parse hex value for key" << key;
    }
    return val;
}

void MemoryLayout::read_group(const MEM_SECTION &section, QSettings &data) {
    QString ini_name = section_name(section);
    AddressHash map;
    if(m_offsets.contains(section)){
        map = m_offsets.take(section);
    }
    data.beginGroup(ini_name);
    foreach(QString k, data.childKeys()) {
        map.insert(k, read_hex(data, k));
    }
    data.endGroup();
    m_offsets.insert(section,map);
}

void MemoryLayout::read_flags(const UNIT_FLAG_TYPE &flag_type, QSettings &data){
    QString ini_name = flag_type_name(flag_type);
    QHash<uint,QString> map = m_flags[flag_type];
    int flag_count = data.beginReadArray(ini_name);
    for (int idx = 0; idx < flag_count; ++idx) {
        data.setArrayIndex(idx);
        map.insert(read_hex(data, "value"),
            data.value("name", QString("unk_%1.%2").arg(ini_name).arg(idx)).toString());
    }
    data.endArray();
    m_flags.insert(flag_type,map);
}

bool MemoryLayout::is_valid_address(VIRTADDR addr)const{
    return (addr != 0x000);
}

VIRTADDR MemoryLayout::global_address(const DFInstance *df, const QString &key) const { //globals
    auto global = m_offsets.value(MEM_GLOBALS).value(key, -1);
    if (global == static_cast<VIRTADDR>(-1)) {
        LOGE << "Missing global" << key;
        return 0;
    }
    return global + df->df_base_addr();
}

VIRTADDR MemoryLayout::field_address(VIRTADDR object, MEM_SECTION section, const QString &key) const {
    auto offset_value = offset(section, key);
    if (offset_value == static_cast<VIRTADDR>(-1)) {
        LOGE << "Missing offset" << section_name(section) << key;
        return 0;
    }
    return object + offset_value;
}
