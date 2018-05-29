/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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

#ifndef ROLE_DIALOG_H
#define ROLE_DIALOG_H

#include <QDialog>
#include <QItemEditorFactory>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <memory>

class Dwarf;
class Role;
class RoleModel;
class RolePreferenceModel;

namespace Ui { class RoleDialog; }

class RoleDialog: public QDialog
{
    Q_OBJECT
public:
    RoleDialog(RolePreferenceModel *pref_model, QWidget *parent = nullptr);
    virtual ~RoleDialog();

    void new_role();
    void open_role(const QString &name);
    bool save_role(); // validate and save current role, return false if an error happened

    bool event(QEvent *event) override;

public slots:
    void selection_changed(); // check selected dwarf and update ratings preview
    void done(int r) override;

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void attribute_activated(const QModelIndex &);
    void skill_activated(const QModelIndex &);
    void facet_activated(const QModelIndex &);
    void preference_activated(const QModelIndex &);
    void aspect_tree_context_menu(const QPoint &);
    void copy_role();
    void update_role_preview();

    // autoconnect slots:
    void on_le_role_name_textChanged(const QString &text);

private:
    std::unique_ptr<Ui::RoleDialog> ui;
    std::unique_ptr<Role> m_role;
    Role *m_old_role;
    Dwarf *m_dwarf;
    RolePreferenceModel *m_pref_model;
    QStandardItemModel m_attribute_model, m_skill_model, m_facet_model;
    std::unique_ptr<RoleModel> m_model;
    QStyledItemDelegate m_weight_delegate;
    QItemEditorFactory m_weight_editor_factory;
};

#endif
