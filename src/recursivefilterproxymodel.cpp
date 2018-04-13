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
#include "recursivefilterproxymodel.h"

RecursiveFilterProxyModel::RecursiveFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

RecursiveFilterProxyModel::~RecursiveFilterProxyModel()
{
}

bool RecursiveFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent))
        return true;

    QAbstractItemModel *source = sourceModel();
    QModelIndex index = source->index(source_row, 0, source_parent);
    int row_count = source->rowCount(index);
    for (int i = 0; i < row_count; ++i) {
        if (filterAcceptsRow(i, index))
            return true;
    }
    return false;
}
