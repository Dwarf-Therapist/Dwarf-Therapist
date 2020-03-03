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
#include "cellcolordef.h"
#include "cellcolors.h"
#include "colorbutton.h"
#include "spacercolumn.h"
#include "ui_vieweditor.h"
#include "viewcolumncolors.h"
#include "viewcolumnset.h"
#include "viewcolumnsetcolors.h"

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

ViewEditorDialog::~ViewEditorDialog(){
    delete ui;
}


void ViewEditorDialog::configure_ui(QObject *setter){
    QColor bg_color = QColor(Qt::white);
    QString name = "";
    QString title = "";
    QString window_title = "";
    int width = -1;
    bool allow_cell_overrides = true;
    bool overrides_cells = false;

    ViewColumn *vc = qobject_cast<ViewColumn*>(setter);
    if(vc){
        ViewColumnSet *set = static_cast<ViewColumnSet*>(vc->parent());
        overrides_cells = vc->get_colors()->overrides_cell_colors();
        //use the set's cell colors if we're not overriding
        if (vc->override_color()){
            bg_color = vc->bg_color();
        }else{
            bg_color = set->bg_color();
        }
        init_cell_colors(vc->get_colors(),set->get_colors(),bg_color);

        title = tr("Column Title");
        window_title = tr("Edit Column");
        name = vc->title();

        connect(ui->cb_override, SIGNAL(toggled(bool)), ui->background_widget, SLOT(setEnabled(bool)));
        ui->cb_override->setChecked(vc->override_color());

        //only show the column width for spacers
        if(vc->type() == CT_SPACER) {
            SpacerColumn *c = static_cast<SpacerColumn*>(vc);
            width = c->width();
            allow_cell_overrides = false;
        }
    }else{
        ViewColumnSet *vcs = qobject_cast<ViewColumnSet*>(setter);
        if(vcs){
            overrides_cells = vcs->get_colors()->overrides_cell_colors();
            bg_color = vcs->bg_color();
            init_cell_colors(vcs->get_colors(),vcs->get_colors()->get_default_colors(),bg_color);

            title = tr("Set Name");
            window_title = tr("Edit Set");
            name = vcs->name();

            //always allow setting the sets bg color
            ui->cb_override->hide();
            ui->background_widget->setEnabled(true);
        }
    }

    connect(ui->cb_override_cell_colors,SIGNAL(toggled(bool)),ui->colors_widget,SLOT(setEnabled(bool)));
    ui->cb_override_cell_colors->setChecked(overrides_cells);

    if(!allow_cell_overrides){
        ui->colors_widget->hide();
        ui->cb_override_cell_colors->hide();
    }

    ui->lbl_title->setText(title);
    ui->le_title->setText(name);
    ui->cb_override_cell_colors->setText(tr("Override default cell colors"));

    if(width >= 0){
        ui->sb_width->setValue(width);
    }else{// don't show the width form for non-spacer columns
        ui->size_widget->hide();
    }

    this->setWindowTitle(window_title);
    this->adjustSize();
}

void ViewEditorDialog::init_cell_colors(CellColors *cc, CellColors *defaults, QColor bg_color){
    m_col_bg = new ColorButton(bg_color);
    m_col_bg->setStyleSheet("text-align: left; padding: 4px");
    ui->f_layout_bg->addRow(tr("Background"), m_col_bg);
    //disabled by default
    ui->background_widget->setEnabled(false);

    int idx = 0;
    foreach(QSharedPointer<CellColorDef> ccd, cc->get_color_defs()){
        auto *c = new ColorButton(defaults->get_color(idx));
        c->setStyleSheet("text-align: left; padding: 4px");
        ui->f_layout_cells->addRow(ccd->title(), c);
        c->setColor(cc->get_color(idx));
        m_custom_colors << c;
        idx++;
    }
}

QColor ViewEditorDialog::background_color() const {return m_col_bg->color();}
QColor ViewEditorDialog::color(int idx) const{
    return m_custom_colors.at(idx)->color();
}

