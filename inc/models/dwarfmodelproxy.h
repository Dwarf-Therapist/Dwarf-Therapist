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
#ifndef DWARF_MODEL_PROXY_H
#define DWARF_MODEL_PROXY_H

#include <QSortFilterProxyModel>

#include "global_enums.h"

#ifdef QT_QML_LIB
class QJSEngine;
#elif defined(QT_SCRIPT_LIB)
class QScriptEngine;
#endif
class Dwarf;
class DwarfModel;

class DwarfModelProxy: public QSortFilterProxyModel {
    Q_OBJECT
public:
    //these roles are for the right click sorting of the name column
    typedef enum {
        DSR_NAME_ASC = 0,
        DSR_NAME_DESC,
        DSR_ID_ASC,
        DSR_ID_DESC,
        DSR_AGE_ASC,
        DSR_AGE_DESC,
        DSR_SIZE_ASC,
        DSR_SIZE_DESC,
        DSR_DEFAULT
    } DWARF_SORT_ROLE;

    struct script_info{
        QString script_body;
        FILTER_SCRIPT_TYPE script_type;
    };

    DwarfModelProxy(QObject *parent = 0);
    DwarfModel* get_dwarf_model() const;
    void sort(int column, Qt::SortOrder order);
    Qt::SortOrder m_last_sort_order;
    DWARF_SORT_ROLE m_last_sort_role;

    QList<QString> get_script_names() {return m_scripts.keys();}
    bool active_scripts(){return (!m_test_script.isEmpty() || m_scripts.keys().count() > 0);}
    QString get_script(const QString script_name) {return m_scripts.value(script_name).script_body;}
    void clear_script(const QString script_name = "");
    void clear_script(const FILTER_SCRIPT_TYPE sType, const bool refresh);

    void refresh_script();
    QList<Dwarf*> get_filtered_dwarves();

public slots:
    void redirect_tooltip(const QModelIndex &idx);
    void cell_activated(const QModelIndex &idx);
    void setFilterFixedString(const QString &pattern);
    void sort(int, DwarfModelProxy::DWARF_SORT_ROLE, Qt::SortOrder order);
    void apply_script(const QString &script_name, const QString &script_body, const FILTER_SCRIPT_TYPE &sType = SCR_DEFAULT);
    void test_script(const QString &script_body);
    void clear_test();
    void read_settings();

signals:
    void filter_changed();
    void show_tooltip(QString);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;

private:
    QString m_filter_text;
    QString m_test_script;
#ifdef QT_QML_LIB
    QJSEngine
#elif defined(QT_SCRIPT_LIB)
    QScriptEngine
#endif
        *m_engine;
    QHash<QString,script_info> m_scripts;
    QMultiHash<FILTER_SCRIPT_TYPE,QString> m_scripts_by_type;
    bool m_show_tooltips;
};

#endif
