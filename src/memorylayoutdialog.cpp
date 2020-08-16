/*
Dwarf Therapist
Copyright (c) 2020 Clement Vuchener

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

#include "memorylayoutdialog.h"
#include "ui_memorylayoutdialog.h"
#include "memorylayoutmanager.h"
#include "updater.h"

#include <QDesktopServices>
#include <QDir>
#include <QMenu>

MemoryLayoutDialog::MemoryLayoutDialog(MemoryLayoutManager *memory_layouts,
                                       Updater *updater,
                                       QWidget *parent)
    : QDialog(parent)
    , ui(std::make_unique<Ui::MemoryLayoutDialog>())
    , m_memory_layouts(memory_layouts)
{
    ui->setupUi(this);
    ui->tree_view->setModel(memory_layouts);
    ui->tree_view->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tree_view->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tree_view, &QWidget::customContextMenuRequested,
            this, &MemoryLayoutDialog::show_context_menu);
    connect(ui->check_updates_button, &QAbstractButton::clicked,
            updater, &Updater::check_for_updates);
    connect(ui->reload_button, &QAbstractButton::clicked,
            memory_layouts, &MemoryLayoutManager::reload);
}

MemoryLayoutDialog::~MemoryLayoutDialog()
{
}

void MemoryLayoutDialog::show_context_menu(const QPoint &pos) const
{
    auto index = ui->tree_view->indexAt(pos);
    if (!index.isValid())
        return;
    const auto &fileinfo = m_memory_layouts->fileInfo(index);
    QMenu menu;
    menu.addAction(
            tr("Open directory"),
            [&](){ QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.absoluteDir().absolutePath())); });
    menu.addAction(
            tr("Open memory layout"),
            [&](){ QDesktopServices::openUrl(QUrl::fromLocalFile(fileinfo.absolutePath())); });
    menu.exec(ui->tree_view->mapToGlobal(pos));
}
