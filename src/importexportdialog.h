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
#ifndef IMPORT_EXPORT_DIALOG
#define IMPORT_EXPORT_DIALOG

#include <QDialog>

namespace Ui {
    class ImportExportDialog;
}
class CustomProfession;
class GridView;
class Role;

class ImportExportDialog : public QDialog {
    Q_OBJECT

public:
    ImportExportDialog(QWidget *parent = 0);
    bool setup_for_profession_export();
    bool setup_for_profession_import();

    bool setup_for_gridview_export();
    bool setup_for_gridview_import();

    bool setup_for_role_export();
    bool setup_for_role_import();

    public slots:
        void accept();

private:
    Ui::ImportExportDialog *ui;
    QList<CustomProfession*> m_profs;
    QList<GridView*> m_views;
    QList<Role*> m_roles;
    QString m_path;

    typedef enum {
        MODE_UNSET,
        MODE_IMPORT_PROFESSIONS,
        MODE_EXPORT_PROFESSIONS,
        MODE_IMPORT_GRIDVIEWS,
        MODE_EXPORT_GRIDVIEWS,
        MODE_IMPORT_ROLES,
        MODE_EXPORT_ROLES
    } DIALOG_MODE;

    DIALOG_MODE m_mode;

    QList<CustomProfession*> get_profs();
    QList<GridView*> get_views();
    QList<Role*> get_roles();

    private slots:
        void select_all();
        void clear_selection();

        void export_selected_professions();
        void import_selected_professions();

        void export_selected_gridviews();
        void import_selected_gridviews();

        void export_selected_roles();
        void import_selected_roles();

};

#endif
