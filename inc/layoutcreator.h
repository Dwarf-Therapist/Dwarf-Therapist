#ifndef LAYOUT_CREATOR_H
#define LAYOUT_CREATOR_H

#include <QObject>
#include "dfinstance.h"

class MemoryLayout;

class LayoutCreator : public QObject {
    Q_OBJECT
public:
    LayoutCreator(DFInstance *m_df, MemoryLayout * parent, QString file_name, QString version_name);
    virtual ~LayoutCreator();

    bool write_file();

protected:
    DFInstance *m_df;
    MemoryLayout * m_parent;
    QString m_file_name;
    QString m_version_name;

    quint32 m_dwarf_race_index;
    quint32 m_translation_vector;
    quint32 m_language_vector;
    quint32 m_creature_vector;
    quint32 m_squad_vector;

private slots:
    void report_address(const QString&, const quint32&);
};

#endif // LAYOUT_CREATOR_H
