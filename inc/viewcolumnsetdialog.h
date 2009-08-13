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

#ifndef VIEW_COLUMN_SET_DIALOG_H
#define VIEW_COLUMN_SET_DIALOG_H

#include <QtGui>

class ViewManager;
class ViewColumn;
class ViewColumnSet;

namespace Ui {
	class ViewColumnSetDialog;
}

class ViewColumnSetDialog : public QDialog {
	Q_OBJECT
public:
	ViewColumnSetDialog(ViewManager *mgr, ViewColumnSet *view, QWidget *parent = 0);
	QString name();
	QColor bg_color();
	QList<ViewColumn*> columns() {return m_pending_columns;}
	
	//! used to hack into the list of columns, since they don't seem to send a proper re-order signal
	bool eventFilter(QObject *, QEvent *);

	public slots:
		void accept();
	
private:
	Ui::ViewColumnSetDialog *ui;
	ViewColumnSet *m_set;
	ViewManager *m_manager;
	//! holds the color for this set's background if the dialog is accepted
	QColor m_pending_bg_color;
	//! holds the list of columns that will become the new columns for this set, if the dialog is accepted
	QList<ViewColumn*> m_pending_columns;
	bool m_is_editing;
	QString m_original_name;

	private slots:

		//! for redrawing sets in the edit dialog
		void draw_columns();

		//! called from event filter on the list widget to get updates about re-ordering columns
		void order_changed();
		
		//! called when the colorpicker is changed by the user
		void update_color(const QColor &new_color);

		//! makes sure the name for this view is ok
		void check_name(const QString &);

		//! when the user chooses a column type category, populate the second combo box
		void type_chosen(const QString &type_name);

		//! the add button was clicked, so figure out which column to append to the set
		void add_column_from_gui();

		//! overridden context menu for the column list
		void draw_column_context_menu(const QPoint &);

		//! show a dialog for editing individual columns
		void show_edit_column_dialog(ViewColumn *vc);
		
		//! from context menu
		void edit_column(); 
		
		//! from double click
		void edit_column(QListWidgetItem*); 
		
		//! from context menu
		void remove_column(); 

signals:
	void column_changed(ViewColumn *);

};

#endif;