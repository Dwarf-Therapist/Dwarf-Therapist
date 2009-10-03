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
#ifndef HELP_DOCK_H
#define HELP_DOCK_H

#include <QtGui>
#include <QtHelp>

class HelpBrowser : public QTextBrowser {
    Q_OBJECT
public:
    HelpBrowser(QHelpEngine *engine, QWidget *parent = 0)
        : QTextBrowser(parent)
        , m_engine(engine)
    {}
    QVariant loadResource(int type, const QUrl &url) {
        if (url.scheme() == "qthelp")
            return QVariant(m_engine->fileData(url));
        else
            return QTextBrowser::loadResource(type, url);
    }
private:
    QHelpEngine *m_engine;
};

class HelpDock : public QDockWidget {
    Q_OBJECT
public:
    HelpDock(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    
private:
    QHelpEngine *m_engine;
    HelpBrowser *m_browser;
};

#endif