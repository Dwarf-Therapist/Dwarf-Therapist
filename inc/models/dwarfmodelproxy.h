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

#include <QtGui>

class DwarfModel;

class DwarfModelProxy: public QSortFilterProxyModel {
	Q_OBJECT
public:
	DwarfModelProxy(QObject *parent = 0);
	DwarfModel* get_dwarf_model() const;

	void sort(int column, Qt::SortOrder order = Qt::DescendingOrder);
	public slots:
		void cell_activated(const QModelIndex &idx);
		void setFilterFixedString(const QString &pattern);

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;
	bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const;
private:
	QString m_filter_text;
};

#endif
