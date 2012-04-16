#ifndef ROLEDIALOG_H
#define ROLEDIALOG_H

#include <QDialog>
#include "defines.h"
#include "role.h"

namespace Ui {
class roleDialog;
}

class roleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit roleDialog(QWidget *parent = 0, QString name = "");
    ~roleDialog();
    bool event(QEvent *evt);
    
private:
    Ui::roleDialog *ui;
    Role *m_role;
    QColor color_override;
    QColor color_default;

    void load_role_data();
    void load_aspects_data(QTableWidget &table, QHash<QString,Role::aspect> aspects);
    void save_aspects(QTableWidget &table, QHash<QString,Role::aspect> &list);
    void insert_row(QTableWidget &table, Role::aspect a, QString key);

    void add_aspect(QString id, QTableWidget &table, QHash<QString,Role::aspect> &list);

    void clear_table(QTableWidget &t);

    bool m_override;
private slots:
    void close_pressed();
    void save_pressed();
    void copy_pressed();
    void name_changed(QString text);    

    //attributes
    void draw_attribute_context_menu(const QPoint &);
    void add_attribute();
    void remove_attribute();

    //traits
    void draw_trait_context_menu(const QPoint &);
    void add_trait();
    void remove_trait();

    //skills
    void draw_skill_context_menu(const QPoint &);
    void add_skill();
    void remove_skill();

signals:
    void role_changed();
};

#endif // ROLEDIALOG_H
