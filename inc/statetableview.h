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
#ifndef STATE_TABLE_VIEW_H
#define STATE_TABLE_VIEW_H

#include <QtGui>

class UberDelegate;
class RotatedHeader;
class Dwarf;
class DwarfModel;
class DwarfModelProxy;

class StateTableView : public QTreeView
{
	Q_OBJECT

public:
	StateTableView(QWidget *parent = 0);
	~StateTableView();

	void set_model(DwarfModel *model, DwarfModelProxy *proxy);
	UberDelegate *get_delegate() {return m_delegate;}
	RotatedHeader *get_header() {return m_header;}

	public slots:
		void read_settings();
		void filter_dwarves(QString text);
        void set_single_click_labor_changes(bool enabled) {m_single_click_labor_changes = enabled;}
		void jump_to_dwarf(QTreeWidgetItem* current, QTreeWidgetItem* previous);
		void jump_to_profession(QListWidgetItem* current, QListWidgetItem* previous);
        void select_dwarf(Dwarf *d);

		// expand/collapse persistence
		void expandAll();
		void collapseAll();
		void index_expanded(const QModelIndex &idx);
		void index_collapsed(const QModelIndex &idx);
		void restore_expanded_items();
		void currentChanged(const QModelIndex &, const QModelIndex &);
        void clicked(const QModelIndex &index);

protected:
	virtual void contextMenuEvent(QContextMenuEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
	DwarfModel *m_model;
	DwarfModelProxy *m_proxy;
	UberDelegate *m_delegate;
	RotatedHeader *m_header;
	int m_grid_size;
	QList<int> m_expanded_rows;
	bool m_auto_expand_groups;
    bool m_single_click_labor_changes;
    //! we have to store this ourselves since the click(), accept() etc... don't send which button caused them
    Qt::MouseButton m_last_button; 

	private slots:
		void set_nickname();
		void new_custom_profession();
		void apply_custom_profession();
		void custom_profession_from_dwarf();
		void reset_custom_profession();

signals:
	void new_custom_profession(Dwarf *d);
	void dwarf_focus_changed(Dwarf *d);

};
#endif // STATETABLEVIEW_H
