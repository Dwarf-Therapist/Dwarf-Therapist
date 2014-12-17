#ifndef VIEWEDITORDIALOG_H
#define VIEWEDITORDIALOG_H

#include <QDialog>
#include "ui_vieweditor.h"

class ViewColumn;
class ViewColumnSet;
class CustomColor;
class CellColors;

class ViewEditorDialog : public QDialog {
    Q_OBJECT
public:
    ViewEditorDialog(ViewColumn *vc, QDialog *parent = 0);
    ViewEditorDialog(ViewColumnSet *set, QDialog *parent = 0);
    ~ViewEditorDialog();

    Ui::ViewEditor *ui;
    void configure_ui(QObject *setter);

    QColor background_color() const;
    QColor active_color() const;
    QColor pending_color() const;
    QColor disabled_color() const;

protected:
    CustomColor *m_col_bg;
    CustomColor *m_col_active;
    CustomColor *m_col_pending;
    CustomColor *m_col_disabled;

    void init_cell_colors(CellColors *cc, CellColors *defaults, QColor bg_color);

};
#endif // VIEWEDITORDIALOG_H
