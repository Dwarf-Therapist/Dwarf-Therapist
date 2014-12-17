#ifndef VIEWEDITORDIALOG_H
#define VIEWEDITORDIALOG_H

#include <QDialog>
#include "ui_vieweditor.h"
#include "viewcolumn.h"
#include "viewcolumnset.h"
#include "customcolor.h"

class ViewEditorDialog : public QDialog {
    Q_OBJECT
public:
    ViewEditorDialog(ViewColumn *vc, QDialog *parent = 0);
    ViewEditorDialog(ViewColumnSet *set, QDialog *parent = 0);

    ~ViewEditorDialog(){
        delete m_col_bg;
        delete m_col_active;
        delete m_col_pending;
        delete m_col_disabled;
        delete ui;
    }

    Ui::ViewEditor *ui;
    void configure_ui(QObject *setter);

    QColor background_color() {return m_col_bg->get_color();}
    QColor active_color() {return m_col_active->get_color();}
    QColor pending_color() {return m_col_pending->get_color();}
    QColor disabled_color() {return m_col_disabled->get_color();}

public slots:
    //void color_selected(QString,QColor);

protected:
    CustomColor *m_col_bg;
    CustomColor *m_col_active;
    CustomColor *m_col_pending;
    CustomColor *m_col_disabled;

    void init_cell_colors(CellColors *cc, CellColors *defaults, QColor bg_color);

};
#endif // VIEWEDITORDIALOG_H
