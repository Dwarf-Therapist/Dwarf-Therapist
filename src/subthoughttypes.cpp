#include "subthoughttypes.h"

#include <QSettings>

SubThoughtTypes::SubThoughtTypes(QSettings &s, QObject *parent)
    : QObject(parent)
{
    m_placeholder = s.value("placeholder","").toString();
    int count = s.beginReadArray("subthoughts");
    for (int idx = 0; idx < count; ++idx) {
        s.setArrayIndex(idx);
        m_subthoughts.insert(s.value("id",-1).toInt(),s.value("thought","??").toString());
    }
    s.endArray();
}
