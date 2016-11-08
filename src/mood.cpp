#include "mood.h"

Mood::Mood(QSettings &s, QObject *parent)
    : QObject(parent)
{
    m_name = s.value("name","").toString();
    m_desc = capitalize(s.value("description","").toString());
    m_color = QColor(s.value("color","#000000").toString());

    m_name_colored = QString("<font color=%1>%2</font>").arg(m_color.name()).arg(m_name);
    m_desc_colored = QString("<font color=%1>%2</font>").arg(m_color.name()).arg(m_desc);
}
