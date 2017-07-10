#include "memorylayout.h"
#include "utils.h"
#include "truncatingfilelogger.h"
#include "dfinstance.h"
#include <QCryptographicHash>

MemoryLayout::MemoryLayout(DFInstance *df, const QFileInfo &fileinfo)
    : m_df(df)
    , m_fileinfo(fileinfo)
    , m_checksum(QString::null)
    , m_git_sha(QString::null)
    , m_data(m_fileinfo.absoluteFilePath(), QSettings::IniFormat)
    , m_complete(true)
{
    TRACE << "Attempting to contruct MemoryLayout from file " << fileinfo.absoluteFilePath();

    if (m_fileinfo.exists() && m_fileinfo.isReadable()) {
        load_data();
    } else {
        LOGE << m_fileinfo.absoluteFilePath() << "could either not be found or not be opened!";
    }
}

MemoryLayout::MemoryLayout(DFInstance *df, const QFileInfo &fileinfo, const QSettings &data)
    : m_df(df)
    , m_fileinfo(fileinfo)
    , m_checksum(QString::null)
    , m_git_sha(QString::null)
    , m_data(m_fileinfo.absoluteFilePath(), QSettings::IniFormat)
    , m_complete(true)
{
    foreach(QString key, data.allKeys()) {
        m_data.setValue(key, data.value(key));
    }
}

MemoryLayout::~MemoryLayout(){
    m_df = 0;
    m_offsets.clear();
}

void MemoryLayout::load_data() {
    if (!is_valid()) {
        LOGE << "Skipping read of invalid memory layout in" << m_fileinfo.absoluteFilePath();
        return;
    }

    if(!m_offsets.isEmpty()){
        m_data.sync();
        m_offsets.clear();
        m_flags.clear();
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
    m_data.beginGroup("info");
    m_checksum = m_data.value("checksum", "UNKNOWN").toString().toLower();
    m_game_version = m_data.value("version_name", "UNKNOWN").toString().toLower();
    m_complete = m_data.value("complete", true).toBool();
    m_data.endGroup();

    //load offsets by section
    for(int idx = 0; idx < MEM_COUNT; idx++){
        read_group(static_cast<MEM_SECTION>(idx));
    }
    //load flags
    for(int idx = 0; idx < FLAG_TYPE_COUNT; idx++){
        read_flags(static_cast<UNIT_FLAG_TYPE>(idx));
    }
}

unsigned long long MemoryLayout::read_hex(QString key) {
    bool ok;
    QString data = m_data.value(key, -1).toString();
    unsigned long long val = data.toULongLong(&ok, 16);
    if (!ok) {
        LOGE << "Failed to parse hex value for key" << key;
    }
    return val;
}

bool MemoryLayout::is_valid() {
    return m_data.contains("info/checksum")
           && m_data.contains("info/version_name");
}

void MemoryLayout::read_group(const MEM_SECTION &section) {
    QString ini_name = section_name(section);
    AddressHash map;
    if(m_offsets.contains(section)){
        map = m_offsets.take(section);
    }
    m_data.beginGroup(ini_name);
    foreach(QString k, m_data.childKeys()) {
        map.insert(k, read_hex(k));
    }
    m_data.endGroup();
    m_offsets.insert(section,map);
}

void MemoryLayout::read_flags(const UNIT_FLAG_TYPE &flag_type){
    QString ini_name = flag_type_name(flag_type);
    QHash<uint,QString> map = m_flags[flag_type];
    int flag_count = m_data.beginReadArray(ini_name);
    for (int idx = 0; idx < flag_count; ++idx) {
        m_data.setArrayIndex(idx);
        map.insert(read_hex("value"),
            m_data.value("name", QString("unk_%1.%2").arg(ini_name).arg(idx)).toString());
    }
    m_data.endArray();
    m_flags.insert(flag_type,map);
}

uint MemoryLayout::string_buffer_offset() {
    return m_offsets.value(MEM_LANGUAGE).value("string_buffer_offset", DFInstance::STRING_BUFFER_OFFSET);
}

uint MemoryLayout::string_length_offset() {
    return string_buffer_offset() +
            m_offsets.value(MEM_LANGUAGE).value("string_length_offset", DFInstance::STRING_LENGTH_OFFSET);
}

uint MemoryLayout::string_cap_offset() {
    return string_buffer_offset() +
            m_offsets.value(MEM_LANGUAGE).value("string_cap_offset", DFInstance::STRING_CAP_OFFSET);
}

void MemoryLayout::set_address(const QString & key, uint value) {
    m_data.setValue(key, hexify(value));
}

void MemoryLayout::set_game_version(const QString & value) {
    m_game_version = value;
    m_data.setValue("info/version_name", m_game_version);
}

void MemoryLayout::set_checksum(const QString & checksum) {
    m_checksum = checksum;
    m_data.setValue("info/checksum", m_checksum);
}

void MemoryLayout::save_data() {
    m_data.sync();
}

void MemoryLayout::set_complete() {
    m_complete = true;
    m_data.setValue("info/complete", "true");
}

bool MemoryLayout::is_valid_address(VIRTADDR addr){
    return (addr != 0x000);
}

VIRTADDR MemoryLayout::address(const QString &key, const bool is_global) { //globals
    return m_offsets.value(MEM_GLOBALS).value(key, -1) + (is_global ? m_df->df_base_addr() : 0);
}
