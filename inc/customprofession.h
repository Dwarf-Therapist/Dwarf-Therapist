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
#ifndef CUSTOM_PROFESSION_H
#define CUSTOM_PROFESSION_H

#include <QtWidgets>
#include "customcolor.h"
class Dwarf;

namespace Ui
{
	class CustomProfessionEditor;
}

//! Manages custom professions independent of a fortress
/*!
CustomProfession objects hold all data needed to map a set of labors onto a
dwarf. A dwarf can either have a default-profession (determined by the game
based on the dwarf's highest skill) or a custom-profession. In DF, custom
professions don't actually have any real meaning other than semantic.

Using Dwarf Therapist, you can associate any number of labors with a custom 
profession and save this association outside of the game. You can then
apply a custom profession to a dwarf with this tool, and it will set the 
appropriate labors on that dwarf.

Example:
*/
class CustomProfession : public QObject {
	Q_OBJECT
public:
	//! Constructor with blank labor template
	CustomProfession(QObject *parent = 0);
	//! Constructor with labor template based on Dwarf *d
	CustomProfession(Dwarf *d, QObject *parent = 0);

	//! Writes to disk for later use
	void save();

	//! Completely kills this profession
	void delete_from_disk();

	//! Get the game-visible name of this profession
	QString get_name() {return m_name;}
    //! Get the name:: for this custom profession if it's an icon override
    QString get_save_name();

    //! Get the icon resource name for this profession
    QString get_icon_path() {return m_path;}
    int get_icon_id() {return m_icon_id;}

    //! Get the icon's text color for this profession
    QColor get_font_color() {return m_font_color;}

    //! Get the background color (exclusively for icon overrides)
    QColor get_bg_color() {return m_bg_color;}

    QString get_text() {return m_txt;}

    QPixmap get_pixmap();
    QString get_embedded_pixmap();

	//! Check if our template has a particular labor enabled
	bool is_active(int labor_id);

	//! Shows a small editing dialog for this profession
	int show_builder_dialog(QWidget *parent = 0);

	//! Returns a vector of all enabled labor_ids in this template
	QVector<int> get_enabled_labors();

    //! Determines whether or not this profession should be applied as a mask
    bool is_mask(){return m_is_mask;}
    void set_mask(bool value){m_is_mask = value;}

    int prof_id(){return m_id;}

	public slots:
		void add_labor(int labor_id) {set_labor(labor_id, true);}
		void remove_labor(int labor_id) {set_labor(labor_id, false);}
		void set_labor(int labor_id, bool active);
        void set_name(QString name);
		void accept();
		void cancel() {return;}
		void item_check_state_changed(QListWidgetItem*);
        void mask_changed(bool value);
        void set_path(int id) {
            m_icon_id = id;
            m_path = ":/profession/img/profession icons/prof_" + QString::number(id) + ".png";}
        void choose_icon();
        void refresh_icon();
        void set_font_color(QColor c){m_font_color = c;}
        void set_bg_color(QColor c){m_bg_color = c;}
        void set_text(QString s){m_txt = s;}
        void set_prof_id(int val){m_id = val;}
        void color_selected(QString key,QColor col);
        void prefix_changed(QString val);

private:
	bool is_valid();
    void create_image();
    QFont* get_font();
    Ui::CustomProfessionEditor *ui;
	Dwarf *m_dwarf;
    QString m_name;
    QString m_path;
    int m_icon_id;
	QMap<int, bool> m_active_labors;
	QDialog *m_dialog;
    bool m_is_mask;

    CustomColor *m_font_custom_color;
    CustomColor *m_bg_custom_color;
    QColor m_font_color;
    QColor m_bg_color;

    QString m_txt;
    int m_id;

    QFont *m_fnt;
    QPixmap m_pixmap;

};
#endif
