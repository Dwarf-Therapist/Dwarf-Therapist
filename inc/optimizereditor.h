#ifndef OPTIMIZEREDITOR_H
#define OPTIMIZEREDITOR_H

#include <QDialog>
#include <QKeyEvent>

class DwarfTherapist;
class GameDataReader;
class laborOptimizerPlan;
class PlanDetail;
class LaborOptimizer;
class Dwarf;
class Labor;

namespace Ui {
class optimizereditor;
}

class optimizereditor : public QDialog
{
    Q_OBJECT

public:
    explicit optimizereditor(QWidget *parent);
    ~optimizereditor();
    bool event(QEvent *evt);

    void load_plan(QString name);

public slots:
    void populationChanged();

protected:
    void closeEvent(QCloseEvent *){cancel_pressed();}
    void keyPressEvent(QKeyEvent *e){
        if(e->key()==Qt::Key_Escape)
            cancel_pressed();
    }

private:
    Ui::optimizereditor *ui;
    LaborOptimizer *m_optimizer;
    laborOptimizerPlan *m_original_plan;
    laborOptimizerPlan *m_plan;
    bool m_editing;
    bool m_loading;
    QList<Labor*> m_remaining_labors;

    void insert_row(PlanDetail *d);
    void add_new_detail(int id);

    void save_details(laborOptimizerPlan *p);
    void save(laborOptimizerPlan *p);

    QString find_role(int id);
    QList<Dwarf*> get_dwarfs();
    void find_target_population();

private slots:
    void draw_labor_context_menu(const QPoint &p);
    void add_job();
    void add_remaining_jobs();
    void remove_labor();
    void test_optimize();
    void display_message(QVector<QPair<int, QString> >,bool is_warning = false);
    void display_message(QString msg, bool is_warning = false);
    void clear_log();
    void save_pressed();
    void cancel_pressed();
    void refresh_job_counts();
    void import_details();
    void export_details();

    void role_changed(QString val);
    void ratio_changed(double);
    void priority_changed(double);

    void refresh_actual_counts();
    void max_jobs_changed(int);
    void pop_percent_changed(int);
    void hauler_percent_changed(int);
    void auto_haul_changed(int);
    void filter_option_changed();

    void cleanup();

};

#endif // OPTIMIZEREDITOR_H
