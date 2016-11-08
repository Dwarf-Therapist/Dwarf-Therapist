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
#ifndef VIEWCOLUMNSETCOLORS_H
#define VIEWCOLUMNSETCOLORS_H

#include "cellcolors.h"
#include "dwarftherapist.h"

class ViewColumnSetColors : public CellColors {
    Q_OBJECT
public:
    ViewColumnSetColors(QObject *parent=0);
    ViewColumnSetColors(QSettings &s, QObject *parent=0);
    ~ViewColumnSetColors();

    CellColors *get_default_colors();

public slots:
    void use_defaults();

private:
    CellColors *m_defaults;
};
#endif // VIEWCOLUMNSETCOLORS_H
