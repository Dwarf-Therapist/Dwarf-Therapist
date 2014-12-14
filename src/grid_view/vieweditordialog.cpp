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

#include "vieweditordialog.h"
#include "cellcolors.h"
#include "spacercolumn.h"

ViewEditorDialog::ViewEditorDialog(ViewColumn *vc, QDialog *parent)
    : QDialog(parent)
    , ui(new Ui::ViewEditor)
{
    ui->setupUi(this);
    configure_ui(vc);
}

ViewEditorDialog::ViewEditorDialog(ViewColumnSet *set, QDialog *parent)
    : QDialog(parent)
    , ui(new Ui::ViewEditor)
{
    ui->setupUi(this);
    configure_ui(set);
}

void ViewEditorDialog::configure_ui(QObject *setter){
    CellColors *cc = 0;
    QColor bg_color = QColor(Qt::white);
    QString name = "";
    QString title = "";
    int width = -1;
    bool allow_cell_overrides = true;

    ViewColumn *vc = qobject_cast<ViewColumn*>(setter);
    if(vc){
        cc = vc->get_colors();
        bg_color = vc->bg_color();
        title = tr("Column Title");
        name = vc->title();

        if (vc->override_color()){
            ui->cp_bg_color->setCurrentColor(vc->bg_color());
        }else{
            ui->cp_bg_color->setCurrentColor(static_cast<ViewColumnSet*>(vc->parent())->bg_color());
        }

        connect(ui->cb_override, SIGNAL(toggled(bool)), ui->cp_bg_color, SLOT(setEnabled(bool)));
        ui->cb_override->setChecked(vc->override_color());

        if(vc->type() == CT_SPACER) {
            SpacerColumn *c = static_cast<SpacerColumn*>(vc);
            width = c->width();
            allow_cell_overrides = false;
        }

    }else{
        ViewColumnSet *vcs = qobject_cast<ViewColumnSet*>(setter);
        if(vcs){
            cc = vcs->get_colors();
            bg_color = vcs->bg_color();

            ui->cp_bg_color->setEnabled(true);
            ui->cb_override->hide();
            title = tr("Set Name");
            name = vcs->name();
        }
    }

    ui->cp_bg_color->setStandardColors();
    ui->cp_bg_color->setCurrentColor(bg_color);

    connect(ui->cb_override_cell_colors,SIGNAL(toggled(bool)),ui->colors_widget,SLOT(setEnabled(bool)));
    ui->cb_override_cell_colors->setChecked(cc->overrides_cell_colors());

    if(allow_cell_overrides){
        ui->cp_bg_color->setStandardColors();
        ui->cp_active_color->setStandardColors();
        ui->cp_disabled_color->setStandardColors();
        ui->cp_active_color->setCurrentColor(cc->active_color());
        ui->cp_disabled_color->setCurrentColor(cc->disabled_color());
        ui->cp_pending_color->setCurrentColor(cc->pending_color());
    }else{
        ui->colors_widget->hide();
    }

    ui->lbl_title->setText(title);
    ui->le_title->setText(name);
    ui->lbl_bg_color->setText(tr("Background Color"));
    ui->cb_override_cell_colors->setText(tr("Override default cell colors"));

    if(width >= 0){
        ui->sb_width->setValue(width);
    }else{// don't show the width form for non-spacer columns
        ui->lbl_col_width->hide();
        ui->sb_width->hide();
        ui->verticalLayout->removeItem(ui->hbox_width);
    }

    this->adjustSize();
}


/*
if (vc->override_color()){
    col_editor->cp_bg_color->setCurrentColor(vc->bg_color());
}else{
    col_editor->cp_bg_color->setCurrentColor(m_active_set->bg_color());
}
col_editor->cp_bg_color->setStandardColors();
col_editor->cp_active_color->setStandardColors();
col_editor->cp_disabled_color->setStandardColors();

col_editor->le_title->setText(vc->title());

connect(col_editor->cb_override, SIGNAL(toggled(bool)), col_editor->cp_bg_color, SLOT(setEnabled(bool)));
connect(col_editor->cb_override_cell_colors,SIGNAL(toggled(bool)),col_editor->colors_widget,SLOT(setEnabled(bool)));

col_editor->cb_override->setChecked(vc->override_color());
col_editor->cb_override_cell_colors->setChecked(vc->get_colors()->overrides_cell_colors());

if (vc->type() == CT_SPACER) {
    SpacerColumn *c = static_cast<SpacerColumn*>(vc);
    col_editor->sb_width->setValue(c->width());
} else { // don't show the width form for non-spacer columns
    col_editor->lbl_col_width->hide();
    col_editor->sb_width->hide();
    col_editor->verticalLayout->removeItem(col_editor->hbox_width);

    col_editor->cp_active_color->setCurrentColor(vc->get_colors()->active_color());
    col_editor->cp_disabled_color->setCurrentColor(vc->get_colors()->disabled_color());
    col_editor->cp_pending_color->setCurrentColor(vc->get_colors()->pending_color());
}
d->adjustSize();
*/
