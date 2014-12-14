#ifndef VIEWEDITORDIALOG_H
#define VIEWEDITORDIALOG_H

#include <QDialog>
#include "ui_vieweditor.h"
#include "viewcolumn.h"
#include "viewcolumnset.h"

class ViewEditorDialog : public QDialog {
    Q_OBJECT
public:
    ViewEditorDialog(ViewColumn *vc, QDialog *parent = 0);
    ViewEditorDialog(ViewColumnSet *set, QDialog *parent = 0);

    ~ViewEditorDialog(){
        delete ui;
    }

    Ui::ViewEditor *ui;
    void configure_ui(QObject *setter);


};
#endif // VIEWEDITORDIALOG_H
