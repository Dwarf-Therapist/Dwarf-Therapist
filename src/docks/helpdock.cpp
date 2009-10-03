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
#include "helpdock.h"
#include "defines.h"

HelpDock::HelpDock(QWidget *parent, Qt::WindowFlags flags)
    : QDockWidget(parent, flags)
{
    setObjectName("HelpDock");
    setWindowTitle(tr("Help"));
    setFeatures(QDockWidget::AllDockWidgetFeatures);
    setAllowedAreas(Qt::AllDockWidgetAreas);

    m_engine = new QHelpEngine("doc/dwarftherapist.qhc", this);
    if (!m_engine->setupData()) {
        QString error = m_engine->error();
        LOGW << error;
    }
    QString col = m_engine->collectionFile();

    QSplitter *help_panel = new QSplitter(Qt::Horizontal);
    m_browser = new HelpBrowser(m_engine, this);

    help_panel->insertWidget(0, m_engine->contentWidget());
    help_panel->insertWidget(1, m_browser);
    help_panel->setStretchFactor(1, 1);
    setWidget(help_panel);
    connect(m_engine->contentWidget(), SIGNAL(linkActivated(const QUrl &)), m_browser, SLOT(setSource(const QUrl &)));
}