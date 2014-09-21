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

#include "unitsummarydock.h"

UnitSummaryDock::UnitSummaryDock(QWidget *parent, Qt::WindowFlags flags)
    : BaseDock(parent, flags)
{
    setObjectName("dock_unit_summary");
    setWindowTitle(tr("Unit Summary"));
    QWidget *main_widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(main_widget);
    main_widget->setLayout(layout);

    QHBoxLayout *l_type = new QHBoxLayout();
    te_info = new QTextEdit(this);
    te_info->setReadOnly(true);
    l_type->addWidget(te_info);
    layout->addLayout(l_type);

    setWidget(main_widget);

    connect(DT,SIGNAL(settings_changed()),this,SLOT(refresh()));
}

void UnitSummaryDock::show_dwarf_info(Dwarf *d) {
    if(d){
        m_dwarf = QPointer<Dwarf>(d);
    }
    refresh();
}

void UnitSummaryDock::show_info(QString info){
    te_info->setText(info);
}

void UnitSummaryDock::refresh(){
//    if(!m_dwarf.isNull()){
//        te_info->setText(m_dwarf->tooltip_text());
//        te_info->moveCursor(QTextCursor::Start);
//        te_info->ensureCursorVisible();
//    }
}

void UnitSummaryDock::clear(){
    te_info->clear();
}
