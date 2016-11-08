#ifndef SELECTPARENTLAYOUTDIALOG_H
#define SELECTPARENTLAYOUTDIALOG_H

#include <QDialog>

namespace Ui {
    class SelectParentLayoutDialog;
}
class DFInstance;
class MemoryLayout;

class SelectParentLayoutDialog : public QDialog
{
    Q_OBJECT

public:
    SelectParentLayoutDialog(DFInstance *df, QWidget *parent = 0);
    ~SelectParentLayoutDialog();

    MemoryLayout * get_layout();
    QString get_file_name();
    QString get_version_name();

private:
    Ui::SelectParentLayoutDialog *ui;
    DFInstance *m_df;
    MemoryLayout * m_layout;

    private slots:
        void selection_changed(int);
};

#endif // SELECTPARENTLAYOUTDIALOG_H
