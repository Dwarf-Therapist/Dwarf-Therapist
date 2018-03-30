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

#ifndef PREFERENCECOLUMN_H
#define PREFERENCECOLUMN_H

#include "viewcolumn.h"

#include "rolepreference.h"

class PreferenceColumn : public ViewColumn {
    Q_OBJECT
public:
    PreferenceColumn(QSettings &s, ViewColumnSet *set, QObject *parent = nullptr);
    PreferenceColumn(const QString &title, std::unique_ptr<RolePreference> &&pref, ViewColumnSet *set = nullptr, QObject *parent = nullptr);
    PreferenceColumn(const PreferenceColumn &to_copy); // copy ctor
    PreferenceColumn* clone() override {return new PreferenceColumn(*this);}
    QStandardItem *build_cell(Dwarf *d) override;
    QStandardItem *build_aggregate(const QString &, const QVector<Dwarf*> &) override;
    void write_to_ini(QSettings &s) override;

private:
    std::unique_ptr<RolePreference> m_pref;
};

#endif // PREFERENCECOLUMN_H
