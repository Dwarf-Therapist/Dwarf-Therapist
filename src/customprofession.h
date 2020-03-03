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

#include "multilabor.h"

#include <QString>
#include <QPixmap>

class Dwarf;
class QObject;
class QFont;
class ColorButton;

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
class CustomProfession : public MultiLabor {
    Q_OBJECT
public:
    //! Constructor with blank labor template
    CustomProfession(QObject *parent = 0);
    //! Constructor with labor template based on Dwarf *d
    CustomProfession(Dwarf *d, QObject *parent = 0);
    //! loading from our Dwarf Therapist.ini
    CustomProfession(QString name, QSettings &s, QObject *parent = 0);
    //! importing
    CustomProfession(QSettings &s, QObject *parent = 0);
    //! custom icon
    CustomProfession(int profession_id, QObject *parent = 0);

    ~CustomProfession();

    //! Get the name:: for this custom profession if it's an icon override
    QString get_save_name();
    QPixmap get_pixmap();
    QString get_embedded_pixmap();
    //! Determines whether or not this profession should be applied as a mask
    bool is_mask(){return m_is_mask;}
    int prof_id(){return m_prof_id;}
    bool has_icon();

    //! Shows a small editing dialog for this profession
    int show_builder_dialog(QWidget *parent = 0);
    void save(QSettings &s);
    void delete_from_disk();
    void export_to_file(QSettings &s);

public slots:
    void set_name(QString name);
    void update_dwarf();
    void mask_changed(bool value);
    void build_icon_path(int id);
    void choose_icon();
    void refresh_icon();
    void prefix_changed(QString val);
    void role_changed(int);

private:
    void init(QSettings &s);
    bool is_valid();
    void create_image();
    QFont* get_font();
    Ui::CustomProfessionEditor *ui;
    QString m_icon_path;
    int m_icon_id;
    bool m_is_mask;

    ColorButton *m_font_custom_color;
    ColorButton *m_bg_custom_color;
    QColor m_font_color;
    QColor m_bg_color;

    QString m_txt;
    int m_prof_id;

    QFont *m_fnt;
    QPixmap m_pixmap;
};
#endif
