/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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
#include "dwarfdetailsdock.h"
#include "dwarfdetailswidget.h"

DwarfDetailsDock::DwarfDetailsDock(QWidget *parent, Qt::WindowFlags flags)
	: QDockWidget(parent, flags)
	, m_skills_layout(new QGridLayout(this))
    , m_widget(new DwarfDetailsWidget(this))
{
    setWindowTitle(tr("Dwarf Details"));
    setObjectName("dwarfdetailsdock");
	setFeatures(QDockWidget::AllDockWidgetFeatures);
	setAllowedAreas(Qt::AllDockWidgetAreas);
    setWidget(m_widget);
}

void DwarfDetailsDock::show_dwarf(Dwarf *d) {
    m_widget->show_dwarf(d);
}

