#ifndef DFINSTANCENIX_H
#define DFINSTANCENIX_H

#include "dfinstance.h"
#include "utils.h"

class DFInstanceNix : public DFInstance
{
    Q_OBJECT
public:
    DFInstanceNix(QObject *parent);

    QString read_string(const VIRTADDR addr);
    USIZE write_string(const VIRTADDR addr, const QString &str);

    bool df_running();

protected:
    pid_t m_pid;
    QString calculate_checksum();

    VIRTADDR get_string(const QString &str);
    virtual VIRTADDR alloc_chunk(USIZE size) = 0;

    QString m_loc_of_dfexe;

private:
    QHash<QString, VIRTADDR> m_string_cache;
};

#endif // DFINSTANCENIX_H
