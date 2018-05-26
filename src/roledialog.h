#ifndef ROLEDIALOG_H
#define ROLEDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <memory>
#include "global_enums.h"

class Dwarf;
class Material;
class Plant;
class RolePreference;
class RolePreferenceModel;
class RecursiveFilterProxyModel;
class QSplitter;
class QTableWidget;
class Role;
struct RoleAspect;
namespace Ui { class roleDialog; }

class roleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit roleDialog(RolePreferenceModel *pref_model, QWidget *parent = 0);
    ~roleDialog();
    bool event(QEvent *evt);

    void load_role(QString role_name);

public slots:
    void selection_changed();

private:
    std::unique_ptr<Ui::roleDialog> ui;
    Role *m_role;
    Dwarf *m_dwarf;
    RolePreferenceModel *m_pref_model;
    RecursiveFilterProxyModel *m_proxy_model;

    void load_role_data();
    void decorate_splitter(QSplitter *s);
    void load_aspects_data(QTableWidget &table, const std::map<QString, RoleAspect> &aspects);
    void save_aspects(QTableWidget &table, std::map<QString, RoleAspect> &list);
    void save_prefs(Role *r);
    void insert_row(QTableWidget &table, const RoleAspect &a, QString key);
    void insert_pref_row(const RolePreference *p);

    void add_aspect(QString id, QTableWidget &table, std::map<QString, RoleAspect> &list);

    void clear_table(QTableWidget &t);
    bool m_override;

protected:
    void showEvent(QShowEvent *);
    void closeEvent(QCloseEvent *){close_pressed();}
    void keyPressEvent(QKeyEvent *e){
        if(e->key()==Qt::Key_Escape)
            close_pressed();
    }

private slots:
    void close_pressed();
    void save_pressed();
    void save_role(Role *);
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

    //void tree_selection_changed(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void preference_activated(const QModelIndex &index);
    void draw_prefs_context_menu(const QPoint &);
    void remove_pref();

    //update new role ratings
    void calc_new_role();

signals:
    void role_changed();
};

//Q_DECLARE_METATYPE(Preference*)
#endif // ROLEDIALOG_H
