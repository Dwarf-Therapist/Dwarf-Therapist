#ifndef OPTIMIZEREDITOR_H
#define OPTIMIZEREDITOR_H

#include <QWidget>
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "laboroptimizerplan.h"
#include "laboroptimizer.h"

namespace Ui {
class optimizereditor;
}

class optimizereditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit optimizereditor(QString name, QWidget *parent = 0);
    ~optimizereditor();
    bool event(QEvent *evt);

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
    bool is_editing;
    bool loading;
    //QList<Dwarf*> m_population;
//    int m_total_jobs;
//    int m_assigned_jobs;
//    int m_excluded; //counts nobles and military and injured
//    int m_total_population; //total selected dwarves - excluded dwarves
//    float m_target_population; //m_total_population * % population to use
//    float m_total_coverage;
//    int m_pop_percent;
//    int m_jobs_per_dwarf;
    QList<Labor*> m_remaining_labors;

    void insert_row(laborOptimizerPlan::detail *d);
    void add_new_detail(int id);
    QString find_role(int id);
    //bool job_exists(int id);
    void save_details(laborOptimizerPlan *p);
    void save(laborOptimizerPlan *p);

    QList<Dwarf*> get_dwarfs();
    void find_target_population();    

private slots:
    void draw_labor_context_menu(const QPoint &p);
    void add_job();
    void add_remaining_jobs();
    void remove_labor();
    void test_optimize();
    void display_message(QVector<QPair<int, QString> >);
    void display_message(QString msg);
    void clear_log();
    void save_pressed();
    void cancel_pressed();
    //void refresh_all_counts();
    void refresh_job_counts();
    void import_details();
    void export_details();

    void ratio_changed(double);
    void priority_changed(double);

    void refresh_actual_counts();
    void max_jobs_changed(int);
    void pop_percent_changed(int);
    void hauler_percent_changed(int);
    void auto_haul_changed(int);
    void filter_option_changed();

};

class sortableTableWidgetItem : public QTableWidgetItem
{
public:
    sortableTableWidgetItem(int type = Type) : QTableWidgetItem(type) {}
    ~sortableTableWidgetItem () {}

    bool operator<(const QTableWidgetItem& other) const
    {
        Q_ASSERT(tableWidget());
        Q_ASSERT(tableWidget()->cellWidget(row(), column()));
        Q_ASSERT(tableWidget()->cellWidget(other.row(), other.column()));
        return tableWidget()->cellWidget(row(), column())->property("text").toString() <
            tableWidget()->cellWidget(other.row(), other.column())->property("text").toString();
    }
};

class sortableComboItem : public QTableWidgetItem
{
public:
    sortableComboItem(int type = Type) : QTableWidgetItem(type) {}
    ~sortableComboItem () {}

    bool operator<(const QTableWidgetItem& other) const
    {
        Q_ASSERT(tableWidget());
        Q_ASSERT(tableWidget()->cellWidget(row(), column()));
        Q_ASSERT(tableWidget()->cellWidget(other.row(), other.column()));
        return tableWidget()->cellWidget(row(), column())->property("currentText").toString() <
            tableWidget()->cellWidget(other.row(), other.column())->property("currentText").toString();
    }
};

#endif // OPTIMIZEREDITOR_H
