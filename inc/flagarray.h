#ifndef FLAGARRAY_H
#define FLAGARRAY_H

#include <QObject>
#include "truncatingfilelogger.h"

class MemoryLayout;
class DFInstance;

class FlagArray : public QObject
{
    Q_OBJECT

private:
    QBitArray *m_flags;
    DFInstance *m_df;

public:
    FlagArray(DFInstance *df, VIRTADDR base_addr){
        m_df = df;
        //get the array from the pointer
        VIRTADDR flags_addr = m_df->read_addr(base_addr);
        //size of the byte array
        quint32 size_in_bytes = m_df->read_addr(base_addr + 0x4);

        m_flags = new QBitArray(size_in_bytes * 8);

        BYTE b;
        int position;
        for(uint i = 0; i < size_in_bytes; i++){
            position = 7;
            b = m_df->read_byte(flags_addr);
            if(b > 0){
                for(int t=128; t>0; t = t/2){
                    if(b & t)
                        m_flags->setBit(i*8 + position, true);
                    position--;
                }
            }
            flags_addr += 0x1;
        }
    }

    virtual ~FlagArray(){
        delete(m_flags);
        m_flags->clear();
        m_flags = 0;
        m_df = 0;
    }


    bool no_flags() {return (m_flags && m_flags->count(true)) <= 0 ? true : false;}

    bool has_flag(int f){
        if((int)f < m_flags->count()){
            return m_flags->at(f);
        }else{
            return false;
        }
    }

    QString output_flag_string(){
        QString val;
        for(int i =0; i < m_flags->size(); i++){
            val.append(QString::number(i)).append(m_flags->at(i) ? "(1)," : "(0),");
        }
        val.chop(1);
        return val;
    }
};
#endif // FLAGARRAY_H
