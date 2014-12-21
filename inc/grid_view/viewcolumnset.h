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
#ifndef VIEW_COLUMN_SET_H
#define VIEW_COLUMN_SET_H

#include <QObject>
#include <QColor>
#include <QBrush>

class QStandardItemModel;
class QSettings;
class ViewColumn;
class Dwarf;
class ViewColumnSetColors;

class ViewColumnSet : public QObject {
    Q_OBJECT
public:
    ViewColumnSet(QSettings &s, QObject *parent = 0, int set_num = -1);
    ViewColumnSet(QString name, QObject *parent = 0);
    ViewColumnSet(const ViewColumnSet &copy); //copy ctor
    virtual ~ViewColumnSet();

    void re_parent(QObject *parent);

    QString name() {return m_name;}
    void add_column(ViewColumn *col, int idx=-1);

    void set_bg_color(const QColor &color) {m_bg_color = color;}
    QColor bg_color() {return m_bg_color;}

    ViewColumnSetColors *get_colors() {return m_cell_colors;}

    QList<ViewColumn*> columns() {return m_columns;}
    void remove_column(int offset) {m_columns.removeAt(offset);}
    void remove_column(ViewColumn *vc) {m_columns.removeAll(vc);}
    ViewColumn *column_at(int offset){return m_columns.at(offset);}

    //! order of columns was changed by a view, so reflect those changes internally
    void reorder_columns(const QStandardItemModel &model);

    //! persist this structure to disk
    void write_to_ini(QSettings &s, int start_idx=0);

    public slots:
        void set_name(const QString &name);
        void toggle_for_dwarf(Dwarf *d);
        void toggle_for_dwarf(); // from context menu of single labor
        void toggle_for_dwarf_group(); // from context menu of aggregate
        void read_settings();

private:
    QString m_name;
    QList<ViewColumn*> m_columns;
    QBrush m_bg_brush; // possibly allow textured backgrounds in the long long ago, err future.
    QColor m_bg_color;
    ViewColumnSetColors *m_cell_colors;

    void connect_settings();
};

#endif
