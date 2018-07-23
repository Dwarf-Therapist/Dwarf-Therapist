/*
Dwarf Therapist
Copyright (c) 2018 Clément Vuchener

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
#ifndef SORT_FILTER_PROXY_MODEL_H
#define SORT_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

/**
 * Extend QSortFilterProxyModel with several filter modes
 */
class SortFilterProxyModel: public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(SortFilterProxyModel::Mode mode READ mode WRITE set_mode)
public:
    SortFilterProxyModel(QObject *parent = nullptr);
    virtual ~SortFilterProxyModel();

    enum Mode {
        StandardMode, // behave like QSortFilterProxyModel
        RecursiveMode, // similar to Qt 5.10 recursive filtering, accept parent if any child is accepted
        TopLevelMode, // only filter top-level items, children are always shown
    };

    Mode mode() const { return m_mode; };
    void set_mode(Mode mode);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    Mode m_mode;
};

#endif
