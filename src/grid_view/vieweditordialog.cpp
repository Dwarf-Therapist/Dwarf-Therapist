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
#include "customcolor.h"
#include "viewcolumncolors.h"
#include "viewcolumnsetcolors.h"
#include "viewcolumnset.h"
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

ViewEditorDialog::~ViewEditorDialog(){
    delete m_col_bg;
    qDeleteAll(m_custom_colors);
    m_custom_colors.clear();
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
    int minWidth = 150;
    m_col_bg = new CustomColor(tr("Background"),tr("Background color of the column."),"background", bg_color, this);
    m_col_bg->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    m_col_bg->setMinimumWidth(minWidth);
    ui->v_layout_bg->addWidget(m_col_bg);
    //disabled by default
    ui->background_widget->setEnabled(false);

    int idx = 0;
    foreach(CellColorDef *ccd, cc->colors()){
        CustomColor *c = new CustomColor(ccd->title(),ccd->description(),ccd->key(),defaults->get_color(idx),this);
        c->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
        c->setMinimumWidth(minWidth);
        ui->v_layout_cells->addWidget(c);
        c->set_color(cc->get_color(idx));
        idx++;
    }

//    m_col_active = new CustomColor(tr("Active"),tr("Color when the related action is enabled and active."), "active", defaults->active_color(), this);
//    m_col_active->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//    m_col_active->setMinimumWidth(minWidth);
//    ui->v_layout_cells->addWidget(m_col_active);
//    m_col_active->set_color(cc->active_color());

//    m_col_pending = new CustomColor(tr("Pending"),tr("Color when an action has been flagged to be enabled, but hasn't been yet."),"pending", defaults->pending_color(), this);
//    m_col_pending->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//    m_col_pending->setMinimumWidth(minWidth);
//    ui->v_layout_cells->addWidget(m_col_pending);
//    m_col_pending->set_color(cc->pending_color());

//    m_col_disabled = new CustomColor(tr("Disabled"),tr("Color of the cell when the action cannot be toggled."), "disabled", defaults->disabled_color(), this);
//    m_col_disabled->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
//    m_col_disabled->setMinimumWidth(minWidth);
//    ui->v_layout_cells->addWidget(m_col_disabled);
//    m_col_disabled->set_color(cc->disabled_color());
}

QColor ViewEditorDialog::background_color() const {return m_col_bg->get_color();}
QColor ViewEditorDialog::color(int idx) const{
    return m_custom_colors.at(idx)->get_color();
}

