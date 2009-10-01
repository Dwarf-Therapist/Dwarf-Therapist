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
#ifndef ROTATED_HEADER_H
#define ROTATED_HEADER_H

#include <QtGui>

class DwarfModel;

class RotatedHeader : public QHeaderView {
	Q_OBJECT
public:
	RotatedHeader(Qt::Orientation orientation, QWidget *parent = 0);
	void paintSection(QPainter *p, const QRect &rect, int idx) const;

    void column_hover(int col);
	
	QSize sizeHint() const;
	public slots:
		void read_settings();
		void resizeSection(int logicalIndex, int size );
		void set_index_as_spacer(int idx);
		void clear_spacers();

protected:
	void leaveEvent(QEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mousePressEvent(QMouseEvent *e);
signals:
	void section_right_clicked(int idx);
private:
	QPoint m_p;
	QList<int> m_spacer_indexes;
	bool m_shade_column_headers;
    int m_hovered_column;
};

#endif
