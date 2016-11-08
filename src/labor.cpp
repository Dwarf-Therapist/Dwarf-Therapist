#include "labor.h"

#include <QSettings>

Labor::Labor(QSettings &s, QObject *parent)
    : QObject(parent)
    , name(s.value("name", "UNKNOWN LABOR").toString())
    , labor_id(s.value("id", -1).toInt())
    , skill_id(s.value("skill", -1).toInt())
    , requires_equipment(s.value("requires_equipment", false).toBool())
    , is_hauling(s.value("hauling", false).toBool())
{
    int excludes = s.beginReadArray("excludes");
    for (int i = 0; i < excludes; ++i) {
        s.setArrayIndex(i);
        int labor = s.value("labor_id", -1).toInt();
        if (labor != -1)
            m_excluded_labors << labor;
    }
    s.endArray();
    is_skilled = (skill_id > -1);
}
