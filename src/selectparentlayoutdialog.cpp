#include "dfinstance.h"
#include "memorylayout.h"
#include "selectparentlayoutdialog.h"
#include "ui_selectparentlayoutdialog.h"

SelectParentLayoutDialog::SelectParentLayoutDialog(DFInstance *df, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectParentLayoutDialog),
    m_df(df),
    m_layout(NULL)
{
    ui->setupUi(this);

    for (const auto &p: m_df->get_layouts()) {
        auto layout = p.second.get();
        ui->cmb_select_layout->addItem(layout->game_version(), QVariant::fromValue(layout));
    }

    ui->cmb_select_layout->model()->sort(0);
}

SelectParentLayoutDialog::~SelectParentLayoutDialog()
{
    delete ui;
}

MemoryLayout * SelectParentLayoutDialog::get_layout()
{
    return m_layout;
}

QString SelectParentLayoutDialog::get_file_name()
{
    return ui->le_file_name->text();
}

QString SelectParentLayoutDialog::get_version_name()
{
    return ui->le_version_name->text();
}

void SelectParentLayoutDialog::selection_changed(int idx)
{
    QVariant var = ui->cmb_select_layout->itemData(idx);
    m_layout = var.value<MemoryLayout *>();

    ui->le_file_name->setText(m_layout->filename());
    ui->le_version_name->setText(m_layout->game_version());
}
