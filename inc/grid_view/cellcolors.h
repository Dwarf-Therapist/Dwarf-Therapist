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
#ifndef CELLCOLORS_H
#define CELLCOLORS_H

#include <QObject>
#include <QSettings>
#include <QColor>

class CellColors : public QObject {
    Q_OBJECT
public:
    CellColors(QObject *parent=0);
    CellColors(const CellColors &cc);
    virtual ~CellColors(){}

    bool overrides_cell_colors() {return m_override_cell_colors;}
    void set_overrides_cell_colors(bool val) {m_override_cell_colors = val;}

    virtual void inherit_colors(const CellColors &cc);

    QColor active_color() const {return get_color(0);}
    void set_active_color(QColor c){set_color(0,c);}

    QColor pending_color() const {return get_color(1);}
    void set_pending_color(QColor c){set_color(1,c);}

    QColor disabled_color() const {return get_color(2);}
    void set_disabled_color(QColor c){set_color(2,c);}

    QColor get_color(int idx) const;
    void set_color(int idx, QColor c);

public slots:
    virtual void read_settings(){}
    virtual void write_to_ini(QSettings &s);
    virtual void use_defaults(){}

protected:
    bool m_override_cell_colors;
    //pairs of colors, and if they're overrides
    QList<QPair<bool,QColor> > m_colors;

    virtual void load_settings(QSettings &s);
    virtual QColor get_default_color(int idx) const;

};
#endif // CELLCOLORS_H
