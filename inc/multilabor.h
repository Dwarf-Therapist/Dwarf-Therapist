#ifndef  MULTILABOR_H
#define MULTILABOR_H

#include <QtWidgets>
#include "gamedatareader.h"
#include "labor.h"

class Dwarf;

class MultiLabor : public QObject {
    Q_OBJECT
public:
    MultiLabor(QObject *parent = 0);
    virtual ~MultiLabor();

    typedef enum{
        ML_ROLE,
        ML_SKILL,
        ML_SKILL_RATE,
        ML_ACTIVE
    } ML_RATING_TYPE;

    //! Returns a vector of all enabled labor_ids in this template
    virtual QVector<int> get_enabled_labors();
    //! Returns a vector of all enabled labor_ids in this template which have a valid skill (ie. not hauling)
    virtual QVector<int> get_enabled_skilled_labors();
    bool is_active(int labor_id);

    /*!
    Pops up a dialog box asking for a name for this object as well
    as a list of labors that can be enabled/disabled via checkboxes

    \param[in] parent If set, the dialog will launch as a model under parent
    \returns QDialog::exec() result (int)
    */
    virtual int show_builder_dialog(QWidget *parent) = 0;

    QString get_name(){return m_name;}
    QString get_role_name(){return m_role_name;}

    float get_rating(int id, ML_RATING_TYPE type);
    float get_role_rating(int id){return get_rating(id,ML_ROLE);}
    float get_skill_rating(int id){return get_rating(id,ML_SKILL);}
    float get_skill_rate_rating(int id){return get_rating(id,ML_SKILL_RATE);}
    float get_active_rating(int id){return get_rating(id,ML_ACTIVE);}

    QHash<int,QString> get_labor_desc(){return m_labor_desc;}
    QList<QVariant> get_converted_labors(){return m_qvariant_labors;}

    QColor active_labor_color(){return m_active_labor_col;}

public slots:
    void add_labor(int labor_id) {set_labor(labor_id, true);}
    void remove_labor(int labor_id) {set_labor(labor_id, false);}
    void labor_item_check_changed(QListWidgetItem *item);
    virtual void refresh();

    virtual void accept();
    virtual void cancel();
    virtual void set_name(QString name);

    virtual bool is_valid() = 0;
    virtual void delete_from_disk() = 0;
    virtual void save(QSettings &s) = 0;
    virtual void export_to_file(QSettings &s){
        Q_UNUSED(s);
    }

    virtual void read_settings();
signals:
    void selected_count_changed(int);

protected:
    Dwarf *m_dwarf;
    Role *m_role;
    GameDataReader *gdr;
    QString m_name;
    QString m_role_name;
    QHash<int, bool> m_active_labors;
    void set_labor(int labor_id, bool active);
    QDialog *m_dialog;
    int m_selected_count;
    QHash<int,QList<float> > m_ratings;
    QHash<int,QString> m_labor_desc;
    QList<QVariant> m_qvariant_labors;
    QColor m_active_labor_col;

    void load_labors(QListWidget *labor_list);
    void build_role_combo(QComboBox *cb_roles);
    virtual void update_dwarf(){}

    bool m_internal_change_flag;
};

#endif //  MULTILABOR_H
