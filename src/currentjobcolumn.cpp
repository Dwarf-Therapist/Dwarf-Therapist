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

#include "currentjobcolumn.h"
#include "columntypes.h"
#include "dwarfmodel.h"
#include "dwarf.h"
#include "dwarftherapist.h"
#include "dwarfjob.h"
#include "gamedatareader.h"
#include "reaction.h"

#include <QFile>

CurrentJobColumn::CurrentJobColumn(QSettings &s, ViewColumnSet *set, QObject *parent)
    : ViewColumn(s,set,parent)
{
}

CurrentJobColumn::CurrentJobColumn(const QString &title, ViewColumnSet *set, QObject *parent)
    : ViewColumn(title, CT_IDLE, set, parent)
{
}

CurrentJobColumn::CurrentJobColumn(const CurrentJobColumn &to_copy)
    : ViewColumn(to_copy)
{
}

QStandardItem *CurrentJobColumn::build_cell(Dwarf *d) {
    QStandardItem *item = init_cell(d);
    short job_id = d->current_job_id();
    QString pixmap_name(":img/question-frame.png");
    DwarfJob *job = GameDataReader::ptr()->get_job(job_id);
    if (job) {
        int prof_id = -1;
        if(!job->reactionClass().isEmpty() && !d->current_sub_job_id().isEmpty()) {
            Reaction* reaction = d->get_reaction();
            if(reaction!=0) {
                prof_id = GameDataReader::ptr()->get_mood_skill_prof(reaction->skill_id());
            }
        }

        if(prof_id != -1){
            pixmap_name = ":/profession/prof_" + QString::number(prof_id+1) + ".png";  //offset for the image name
            item->setData(QColor(50,50,50), DwarfModel::DR_DEFAULT_BG_COLOR); //shade the background
        }else if(!job->img_path().isEmpty()){
            pixmap_name = QString(":/activities/%1.png").arg(job->img_path());
        }
        TRACE << "Unit:" << d->nice_name() << " jobID:" << job_id << "(" << job->name() << ")";
    }

    if(!QFile::exists(pixmap_name)){
        LOGW << "job icon not found:" << pixmap_name;
    }

    item->setData(QIcon(pixmap_name), Qt::DecorationRole);
    item->setData(CT_IDLE, DwarfModel::DR_COL_TYPE);
    item->setData(d->current_job_id(), DwarfModel::DR_SORT_VALUE);

    QString tooltip = QString("<center><h3>%1</h3>%2 (%3)%4</center>")
            .arg(m_title)
            .arg(d->current_job())
            .arg(d->current_job_id())
            .arg(tooltip_name_footer(d));
    item->setToolTip(tooltip);
    return item;
}

QStandardItem *CurrentJobColumn::build_aggregate(const QString &group_name,const QVector<Dwarf*> &dwarves) {
    Q_UNUSED(dwarves);
    QStandardItem *item = init_aggregate(group_name);
    return item;
}
