/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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

#include "dwarfdetailswidget.h"
#include "ui_dwarfdetailswidget.h"
#include "dwarftherapist.h"
#include "gamedatareader.h"
#include "dwarf.h"
#include "trait.h"
#include "dwarfstats.h"

DwarfDetailsWidget::DwarfDetailsWidget(QWidget *parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , ui(new Ui::DwarfDetailsWidget)
{
    ui->setupUi(this);
}

void DwarfDetailsWidget::show_dwarf(Dwarf *d) {
    // Draw the name/profession text labels...
    ui->lbl_dwarf_name->setText(d->nice_name());
    ui->lbl_age->setText(QString("Age: %1 years").arg(d->get_age()));
    ui->lbl_translated_name->setText(QString("(%1)").arg(d->translated_name()));
    ui->lbl_profession->setText(d->profession());
    ui->lbl_current_job->setText(QString("%1 %2").arg(d->current_job_id()).arg(d->current_job()));

    GameDataReader *gdr = GameDataReader::ptr();

    /* TODO: bring this back once mapped for 0.31.x
    QMap<QProgressBar*, int> things;
    int str = gdr->get_level_from_xp(d->strength());
    int agi = d->agility();
    int tou = d->toughness();
    things.insert(ui->pb_strength, str);
    things.insert(ui->pb_agility, agi);
    things.insert(ui->pb_toughness, tou);

    foreach(QProgressBar *pb, things.uniqueKeys()) {
        int stat = things.value(pb);
        pb->setMaximum(stat > 5 ? stat : 5);
        pb->setValue(stat);
    }

    int xp_for_current = gdr->get_xp_for_next_attribute_level(str + agi + tou - 1);
    int xp_for_next = gdr->get_xp_for_next_attribute_level(str + agi + tou);
    if (xp_for_current && xp_for_next) {// is 0 when we don't know when the next level is...
        ui->pb_next_attribute_gain->setRange(xp_for_current, xp_for_next);
        ui->pb_next_attribute_gain->setValue(d->total_xp());
        ui->pb_next_attribute_gain->setToolTip(QString("%L1xp / %L2xp").arg(d->total_xp()).arg(xp_for_next));
    } else {
        ui->pb_next_attribute_gain->setValue(-1); //turn it off
        ui->pb_next_attribute_gain->setToolTip(tr("Off the charts! (I have no idea when you'll get the next gain)"));
    }*/


    Dwarf::DWARF_HAPPINESS happiness = d->get_happiness();
    ui->lbl_happiness->setText(QString("<b>%1</b> (%2)").arg(d->happiness_name(happiness)).arg(d->get_raw_happiness()));
    QColor color = DT->user_settings()->value(
        QString("options/colors/happiness/%1").arg(static_cast<int>(happiness))).value<QColor>();
    QString style_sheet_color = QString("#%1%2%3")
        .arg(color.red(), 2, 16, QChar('0'))
        .arg(color.green(), 2, 16, QChar('0'))
        .arg(color.blue(), 2, 16, QChar('0'));
    ui->lbl_happiness->setStyleSheet(QString("background-color: %1;").arg(style_sheet_color));

    //save user changes before cleaning
    if(m_cleanup_list.count() > 0){
        //save splitter settings
        QSplitter* old_splitter = qobject_cast<QSplitter *>(m_cleanup_list[0]);
        m_splitter_sizes = old_splitter->saveState();
        QTableWidget *temp;
        //save skill sorts
        temp = qobject_cast<QTableWidget *>(m_cleanup_list[1]);
        m_skill_sort_col = temp->horizontalHeader()->sortIndicatorSection();
        m_skill_sort_desc = temp->horizontalHeader()->sortIndicatorOrder();
        //save attribute sorts
        temp = qobject_cast<QTableWidget *>(m_cleanup_list[2]);
        m_attribute_sort_col = temp->horizontalHeader()->sortIndicatorSection();
        m_attribute_sort_desc = temp->horizontalHeader()->sortIndicatorOrder();
        //save trait sorts
        temp = qobject_cast<QTableWidget *>(m_cleanup_list[3]);
        m_trait_sort_col = temp->horizontalHeader()->sortIndicatorSection();
        m_trait_sort_desc = temp->horizontalHeader()->sortIndicatorOrder();
        //save role sorts
        temp = qobject_cast<QTableWidget *>(m_cleanup_list[4]);
        m_role_sort_col = temp->horizontalHeader()->sortIndicatorSection();
        m_role_sort_desc = temp->horizontalHeader()->sortIndicatorOrder();
    }else{//defaults
        //splitter
        m_splitter_sizes = DT->user_settings()->value("gui_options/detailPanesSizes").toByteArray();
        //skill sorts
        m_skill_sort_col = 1;
        m_skill_sort_desc = Qt::DescendingOrder;
        //attribute sorts
        m_attribute_sort_col = 0;
        m_attribute_sort_desc = Qt::AscendingOrder;
        //trait sorts
        m_trait_sort_col = 1;
        m_trait_sort_desc = Qt::DescendingOrder;
        //role sorts
        m_role_sort_col = 1;
        m_role_sort_desc = Qt::DescendingOrder;
    }


    foreach(QObject *obj, m_cleanup_list) {
        obj->deleteLater();
    }
    m_cleanup_list.clear();


    QSplitter *details_splitter = new QSplitter(this);
    details_splitter->setOrientation(Qt::Vertical);
    details_splitter->setOpaqueResize(true);
    details_splitter->setObjectName("details_splitter");
    ui->vbox_main->addWidget(details_splitter);
    m_cleanup_list << details_splitter;


    // SKILLS TABLE
    QVector<Skill> *skills = d->get_skills();
    QTableWidget *tw = new QTableWidget(skills->size(), 3, this);
    details_splitter->addWidget(tw);
    m_cleanup_list << tw;
    tw->setEditTriggers(QTableWidget::NoEditTriggers);
    tw->setGridStyle(Qt::NoPen);
    tw->setAlternatingRowColors(true);
    tw->setHorizontalHeaderLabels(QStringList() << "Skill" << "Level" << "Progress");
    tw->verticalHeader()->hide();
    tw->horizontalHeader()->setStretchLastSection(true);
    tw->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    tw->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    tw->setSortingEnabled(false); // no sorting while we're inserting    
    for (int row = 0; row < skills->size(); ++row) {
        tw->setRowHeight(row, 18);
        Skill s = skills->at(row);
        QTableWidgetItem *text = new QTableWidgetItem(s.name());
        QTableWidgetItem *level = new QTableWidgetItem;
        level->setData(0, d->skill_rating(s.id()));


        QProgressBar *pb = new QProgressBar(tw);
        pb->setRange(s.exp_for_current_level(), s.exp_for_next_level());
        pb->setValue(s.actual_exp());
        pb->setToolTip(s.exp_summary());
        pb->setDisabled(true);// this is to keep them from animating and looking all goofy        

        tw->setItem(row, 0, text);
        tw->setItem(row, 1, level);
        tw->setCellWidget(row, 2, pb);
    }
    tw->setSortingEnabled(true); // no sorting while we're inserting    
    tw->sortItems(m_skill_sort_col, static_cast<Qt::SortOrder>(m_skill_sort_desc));


    // ATTRIBUTES TABLE
    QHash<int, short> attributes = d->get_attributes();
    QTableWidget *tw_attributes = new QTableWidget(this);
    details_splitter->addWidget(tw_attributes);
    m_cleanup_list << tw_attributes;
    tw_attributes->setColumnCount(3);
    tw_attributes->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_attributes->setWordWrap(true);
    tw_attributes->setShowGrid(false);
    tw_attributes->setGridStyle(Qt::NoPen);
    tw_attributes->setAlternatingRowColors(true);
    tw_attributes->setHorizontalHeaderLabels(QStringList() << "Attribute" << "Value" << "Message");
    tw_attributes->verticalHeader()->hide();
    tw_attributes->horizontalHeader()->setStretchLastSection(true);
    tw_attributes->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    tw_attributes->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    tw_attributes->setSortingEnabled(false);
    for (int row = 0; row < attributes.size(); ++row) {
        Attribute *r = gdr->get_attribute(row);

        tw_attributes->insertRow(0);
        tw_attributes->setRowHeight(0, 14);
        Attribute::level l = d->get_attribute_rating(row);

        QTableWidgetItem *attribute_name = new QTableWidgetItem(r->name);
        QTableWidgetItem *attribute_rating = new QTableWidgetItem;
        attribute_rating->setData(0,d->attribute(row));

        if (l.rating >= 0) {
            attribute_rating->setBackground(QColor(255, 255, 255, 255));
            attribute_rating->setForeground(QColor(0, 0, 0, 255));
        } else {
            attribute_rating->setBackground(QColor(204, 0, 0, 128));
            attribute_rating->setForeground(QColor(0, 0, 128, 255));
        }

        QString lvl_msg = l.description;
        QTableWidgetItem *attribute_msg = new QTableWidgetItem(lvl_msg);
        attribute_msg->setToolTip(lvl_msg);
        tw_attributes->setItem(0, 0, attribute_name);
        tw_attributes->setItem(0, 1, attribute_rating);
        tw_attributes->setItem(0, 2, attribute_msg);
    }
    tw_attributes->setSortingEnabled(true);
    tw_attributes->sortItems(m_attribute_sort_col, static_cast<Qt::SortOrder>(m_attribute_sort_desc));


    // TRAITS TABLE
    QHash<int, short> traits = d->traits();
    QTableWidget *tw_traits = new QTableWidget(this);
    details_splitter->addWidget(tw_traits);
    m_cleanup_list << tw_traits;
    tw_traits->setColumnCount(3);
    tw_traits->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_traits->setWordWrap(true);
    tw_traits->setShowGrid(false);
    tw_traits->setGridStyle(Qt::NoPen);
    tw_traits->setAlternatingRowColors(true);
    tw_traits->setHorizontalHeaderLabels(QStringList() << "Trait" << "Raw" << "Message");
    tw_traits->verticalHeader()->hide();
    tw_traits->horizontalHeader()->setStretchLastSection(true);
    tw_traits->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    tw_traits->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    tw_traits->setSortingEnabled(false);
    for (int row = 0; row < traits.size(); ++row) {
        short val = traits[row];

        if (d->trait_is_active(row))
        {
            tw_traits->insertRow(0);
            tw_traits->setRowHeight(0, 14);
            Trait *t = gdr->get_trait(row);
            QTableWidgetItem *trait_name = new QTableWidgetItem(t->name);
            QTableWidgetItem *trait_score = new QTableWidgetItem;
            trait_score->setData(0, val);

            int deviation = abs(50 - val);
            if (deviation >= 41) {
                trait_score->setBackground(QColor(0, 0, 128, 255));
                trait_score->setForeground(QColor(255, 255, 255, 255));
            } else if (deviation >= 25) {
                trait_score->setBackground(QColor(220, 220, 255, 255));
                trait_score->setForeground(QColor(0, 0, 128, 255));
            }

            QString lvl_msg = t->level_message(val);
            QTableWidgetItem *trait_msg = new QTableWidgetItem(lvl_msg);
            trait_msg->setToolTip(lvl_msg);
            tw_traits->setItem(0, 0, trait_name);
            tw_traits->setItem(0, 1, trait_score);
            tw_traits->setItem(0, 2, trait_msg);
        }
    }
    tw_traits->setSortingEnabled(true);
    tw_traits->sortItems(m_trait_sort_col, static_cast<Qt::SortOrder>(m_trait_sort_desc));



    // ROLES TABLE
    QList<QPair<QString, float> > roles = d->sorted_role_ratings();
    QTableWidget *tw_roles = new QTableWidget(this);
    details_splitter->addWidget(tw_roles);
    m_cleanup_list << tw_roles;
    tw_roles->setColumnCount(2);
    tw_roles->setEditTriggers(QTableWidget::NoEditTriggers);
    tw_roles->setWordWrap(true);
    tw_roles->setShowGrid(false);
    tw_roles->setGridStyle(Qt::NoPen);
    tw_roles->setAlternatingRowColors(true);
    tw_roles->setHorizontalHeaderLabels(QStringList() << "Role" << "Rating");
    tw_roles->verticalHeader()->hide();
    tw_roles->horizontalHeader()->setStretchLastSection(true);
    tw_roles->horizontalHeader()->setResizeMode(0, QHeaderView::ResizeToContents);
    tw_roles->horizontalHeader()->setResizeMode(1, QHeaderView::ResizeToContents);
    tw_roles->setSortingEnabled(false);
    QString name = "";
    float val = 0.0;
    int max = DT->user_settings()->value("options/role_count_pane",10).toInt();
    if(max > d->sorted_role_ratings().count())
        max = d->sorted_role_ratings().count();
    for(int i = 0; i < max; i++){
        name = roles.at(i).first;
        val = roles.at(i).second;

        tw_roles->insertRow(0);
        tw_roles->setRowHeight(0, 14);

        QTableWidgetItem *role_name = new QTableWidgetItem(name);
        QTableWidgetItem *role_rating = new QTableWidgetItem;
        role_rating->setData(0, static_cast<float>(static_cast<int>(val*100+0.5))/100);


        if (val >= 50) {
            role_rating->setBackground(QColor(255, 255, 255, 255));
            role_rating->setForeground(QColor(0, 0, 0, 255));
        } else {
            role_rating->setBackground(QColor(204, 0, 0, 128));
            role_rating->setForeground(QColor(0, 0, 128, 255));
        }

        tw_roles->setItem(0, 0, role_name);
        tw_roles->setItem(0, 1, role_rating);

        //role_rating->setToolTip(gdr->get_role(name)->role_details);
    }
    tw_roles->setSortingEnabled(true);
    tw_roles->sortItems(m_role_sort_col, static_cast<Qt::SortOrder>(m_role_sort_desc));

    details_splitter->restoreState(m_splitter_sizes);
}

