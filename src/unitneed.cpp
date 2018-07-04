/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "unitneed.h"

#include "adaptivecolorfactory.h"
#include "dfinstance.h"
#include "dwarf.h"
#include "gamedatareader.h"
#include "histfigure.h"
#include "memorylayout.h"

UnitNeed::UnitNeed(VIRTADDR address, DFInstance *df, Dwarf *d)
    : m_dwarf(d)
{
    auto mem = df->memory_layout();

    m_id = df->read_int(mem->need_field(address, "id"));
    m_deity_id = df->read_int(mem->need_field(address, "deity_id"));
    m_focus_level = df->read_int(mem->need_field(address, "focus_level"));
    m_need_level = df->read_int(mem->need_field(address, "need_level"));
    if (m_focus_level <= -100000)
        m_focus_degree = BADLY_DISTRACTED;
    else if (m_focus_level <= -10000)
        m_focus_degree = DISTRACTED;
    else if (m_focus_level <= -1000)
        m_focus_degree = UNFOCUSED;
    else if (m_focus_level < 100)
        m_focus_degree = NOT_DISTRACTED;
    else if (m_focus_level < 200)
        m_focus_degree = UNTROUBLED;
    else if (m_focus_level < 300)
        m_focus_degree = LEVEL_HEADED;
    else // if (m_focus_level < 400)
        m_focus_degree = UNFETTERED;
    if (m_deity_id != -1)
        m_deity_name = HistFigure::get_name(df, m_deity_id, true);
}

QString UnitNeed::adjective() const
{
    static const char * const adjectives[DEGREE_COUNT] = {
        QT_TR_NOOP("badly distracted"),
        QT_TR_NOOP("distracted"),
        QT_TR_NOOP("unfocused"),
        QT_TR_NOOP("not distracted"),
        QT_TR_NOOP("untroubled"),
        QT_TR_NOOP("level-headed"),
        QT_TR_NOOP("unfettered"),
    };
    return tr(adjectives[m_focus_degree]);
}

QString UnitNeed::description() const
{
    auto gdr = GameDataReader::ptr();
    return tr("%1 is %2 after %3.")
            .arg(m_dwarf->get_gender() == Dwarf::SEX_M ? tr("He") : tr("She"))
            .arg(adjective())
            .arg(gdr->get_need_desc(m_id, m_focus_degree <= NOT_DISTRACTED, m_deity_name));
}

QColor UnitNeed::degree_color(int degree, bool tooltip)
{
    AdaptiveColorFactory color(
            tooltip ? QPalette::ToolTipText : QPalette::WindowText,
            tooltip ? QPalette::ToolTipBase : QPalette::Window);
    switch (degree) {
    case BADLY_DISTRACTED:
        return color.color(Qt::red);
    case DISTRACTED:
        return color.color(QColor::fromHsv(30, 255, 255)); // orange instead of yellow
    case UNFOCUSED:
        return color.color(QColor::fromHsv(60, 255, 160)); // instead of dark yellow (brown)
    case NOT_DISTRACTED:
        return color.gray(0.75);
    case UNTROUBLED:
        return color.gray(1.00);
    case LEVEL_HEADED:
        return color.color(QColor::fromHsv(90, 255, 255)); // yellowish green instead of dark green
    case UNFETTERED:
        return color.color(QColor::fromHsv(150, 255, 255)); // blueish green instead of green
    default:
        return QColor();
    }
}

