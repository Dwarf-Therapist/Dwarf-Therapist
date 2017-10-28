#include "flagarray.h"
#include "dfinstance.h"
#include "truncatingfilelogger.h"

FlagArray::FlagArray(){
    m_df = 0;
    m_flags = QBitArray(0);
}

FlagArray::FlagArray(DFInstance *df, VIRTADDR base_addr)
{
    m_df = df;
    //get the array from the pointer
    VIRTADDR flags_addr = m_df->read_addr(base_addr);
    //size of the byte array
    uint32_t size_in_bytes = m_df->read_mem<uint32_t>(base_addr + m_df->pointer_size());

    m_flags = QBitArray(size_in_bytes * 8);
    if(size_in_bytes > 1000){
        LOGW << "aborting reading flags, size too large" << size_in_bytes;
        return;
    }
    BYTE b;
    int position;
    for(uint i = 0; i < size_in_bytes; i++){
        position = 7;
        b = m_df->read_byte(flags_addr);
        if(b > 0){
            for(int t=128; t>0; t = t/2){
                if(b & t)
                    m_flags.setBit(i*8 + position, true);
                position--;
            }
        }
        flags_addr += 0x1;
    }
}

FlagArray::FlagArray(const FlagArray &f)
    : m_flags(f.m_flags)
    , m_flags_custom(f.m_flags_custom)
    , m_df(f.m_df)
{
}

FlagArray::~FlagArray(){
    m_df = 0;
}

bool FlagArray::has_flag(const int f) const {
    if(f < m_flags.count()){
        return m_flags.at(f);
    }else if(m_flags_custom.contains(f)){
        return m_flags_custom.value(f);
    }else{
        return false;
    }
}

void FlagArray::set_flag(int f,bool state){
    if(f >= m_flags.size()){
        m_flags_custom.insert(f,state);
    }else{
        m_flags.setBit(f,state);
    }
}

int FlagArray::count() const {
    int count = 0;
    foreach(bool val, m_flags_custom.values()){
        if(val)
            count++;
    }
    count += m_flags.count(true);
    return count;
}

QString FlagArray::output_flag_string(bool active){
    QStringList ret;
    for(int i=0; i < m_flags.size(); i++){
        ret.append(output_flag(i,m_flags.at(i),active));
    }
    foreach(int f, m_flags_custom.uniqueKeys()){
        ret.append(output_flag(f,m_flags_custom.value(f),active));
    }
    ret.removeAll("");
    return ret.join(",");
}

QString FlagArray::output_flag(int f, bool val, bool active){
    if(active){
        if(val)
            return QString::number(f);
        else
            return "";
    }else{
        return QString::number(f).append(val ? "(1)" : "(0)");
    }
}

QList<int> FlagArray::active_flags() const {
    QList<int> active;
    for(int idx=0; idx < m_flags.size(); idx++){
        if(m_flags.at(idx))
            active << idx;
    }
    foreach(int f, m_flags_custom.uniqueKeys()){
        if(m_flags_custom.value(f))
            active << f;
    }
    return active;
}
