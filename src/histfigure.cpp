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

#include "histfigure.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "races.h"
#include "dwarftherapist.h"
#include "truncatingfilelogger.h"

HistFigure::HistFigure(int id, DFInstance *df, QObject *parent)
    : QObject(parent)
    , m_df(df)
{
    m_id = id;

    m_fig_info_addr=0;
    m_fake_ident_addr=0;
    m_fake_name_addr=0;
    m_fake_name="";
    m_fake_nick="";
    m_fake_birth_year=0;
    m_fake_birth_time=0;
    m_total_kills_other=0;
    m_has_fake_identity=false;

    m_address = m_df->find_historical_figure(id);
    m_mem = m_df->memory_layout();
    if(m_address){
        m_nick_addrs.append(m_mem->word_field(m_mem->hist_figure_field(m_address, "hist_name"), "nickname"));
        m_fig_info_addr = m_df->read_addr(m_mem->hist_figure_field(m_address, "hist_fig_info"));
        m_has_fake_identity = read_fake_identity();
        if(!DT->user_settings()->value("options/highlight_cursed", false).toBool() && m_has_fake_identity){
            return;
        }
        read_kills();
    }
}

HistFigure::~HistFigure(){
    m_mem = 0;
    m_df = 0;
}

void HistFigure::read_kills(){
    VIRTADDR kills_addr = m_df->read_addr(m_mem->hist_figure_field(m_fig_info_addr, "kills"));
    if(kills_addr==0)
        return;
    auto kill_events = m_df->enum_vec<qint32>(kills_addr);
    auto race_ids = m_df->enum_vec<qint16>(m_mem->hist_figure_field(kills_addr, "killed_race_vector"));
    auto undead_kills = m_df->enum_vec<quint16>(m_mem->hist_figure_field(kills_addr, "killed_undead_vector"));
    auto cur_site_kills = m_df->enum_vec<qint32>(m_mem->hist_figure_field(kills_addr, "killed_counts_vector"));
    if(cur_site_kills.count() > 0){
        QHash<int,int> kills; //group by race
        for(int idx=0;idx < race_ids.size();idx++){
            int id = race_ids.at(idx);
            if(undead_kills.at(idx) != 0){
                id = -id;
            }
            int count = cur_site_kills.at(idx);
            int curr_count = kills.value(id,0);
            kills.insert(id,count+curr_count);
        }
        foreach(int race_id, kills.keys()){
            kill_info ki;
            ki.count = kills.value(race_id);
            ki.name = "";
            ki.year = -1;
            Race *r = m_df->get_race(abs(race_id));
            if(r){
                ki.creature = r->name(ki.count).toLower();
                if(race_id < 0){
                    ki.creature.prepend(tr("undead "));
                }
            }
            m_other_kills.append(ki);
            m_total_kills_other += ki.count;
        }
        qSort(m_other_kills.begin(),m_other_kills.end(),&HistFigure::sort_kill_count);
    }
    if(kill_events.count() > 0){
        foreach(quint32 evt_id, kill_events){
            VIRTADDR evt_addr = 0;
            evt_addr = m_df->find_event(evt_id);
            if(evt_addr){
                VIRTADDR vtable_addr = m_df->read_addr(evt_addr);
                int evt_type = m_df->read_int(m_df->read_addr(vtable_addr) + m_df->VM_TYPE_OFFSET());
                LOGD << "found historical event type" << evt_type;
                if(evt_type == 3){ //hist figure died event
                    int hist_id = m_df->read_int(m_mem->hist_event_field(evt_addr, "killed_hist_id"));
                    VIRTADDR h_fig_addr =  m_df->find_historical_figure(hist_id);
                    if(h_fig_addr){
                        VIRTADDR name_addr = m_mem->hist_figure_field(h_fig_addr, "hist_name");
                        kill_info ki;
                        ki.name = capitalizeEach(m_df->read_string(name_addr).append(" ").append(m_df->get_translated_word(name_addr)));
                        ki.count = 1;
                        Race *r = m_df->get_race(m_df->read_short(m_mem->hist_figure_field(h_fig_addr, "hist_race")));
                        if(r){
                            ki.creature = r->name(ki.count).toLower();
                        }
                        ki.year = m_df->read_int(m_mem->hist_event_field(evt_addr, "event_year"));
                        m_notable_kills.append(ki);
                    }
                }
            }
        }
        qSort(m_notable_kills.begin(),m_notable_kills.end(),&HistFigure::sort_kill_date);
    }
}

void HistFigure::write_nick_name(const QString new_nick){
    foreach(VIRTADDR addr, m_nick_addrs){
        if(addr != 0){
            m_df->write_string(addr, new_nick);
        }
    }
}

int HistFigure::total_kills(){
    return m_notable_kills.size() + m_total_kills_other;
}

int HistFigure::kill_count(bool notable){
    if(!notable){
        return m_total_kills_other;
    }else{
        return m_notable_kills.size();
    }
}

QStringList HistFigure::notable_kills(){
    if(m_notable_kill_list.count() <= 0 && kill_count(true) > 0){
        foreach(kill_info ki, m_notable_kills){
            m_notable_kill_list.append(tr("%2 the %1, d.%3").arg(ki.creature).arg(ki.name).arg(ki.year));
        }
    }
    return m_notable_kill_list;
}
QStringList HistFigure::other_kills(){
    if(m_other_kill_list.count() <= 0 && kill_count(false) > 0){
        foreach(kill_info ki, m_other_kills){
            m_other_kill_list.append(tr("%1 %2").arg(ki.count).arg(ki.creature));
        }
    }
    return m_other_kill_list;
}

QString HistFigure::formatted_summary(bool show_no_kills, bool space_notable){
    QString kill_summary = "<p style=\"margin:0px;\">";
    if(total_kills() > 0){
        QStringList kill_lists;
        int count = kill_count(true);
        if(count > 0){
            QString sep = (space_notable ? "<br/>" : ", ");
            kill_lists.append(tr("<b>%1Notable Kill%2:</b> %3")
                              .arg(count > 1 ? QString::number(count)+" " : "").arg(count > 1 ? "s" : "")
                              .arg(notable_kills().join(sep)));
        }
        count = kill_count();
        if(count > 0){
            kill_lists.append(tr("<b>%1Kill%2:</b> %3")
                              .arg(count > 1 ? QString::number(count)+" " : "").arg(count > 1 ? "s" : "")
                              .arg(other_kills().join(", ")));
        }
        kill_summary.append(kill_lists.join("<br/><br/>"));
    }else if (show_no_kills){
        kill_summary.append(tr("No Kills"));
    }
    kill_summary.append("</p>");
    return kill_summary;
}

bool HistFigure::read_fake_identity(){
    VIRTADDR rep_info = m_df->read_addr(m_mem->hist_figure_field(m_fig_info_addr, "reputation"));
    if(!rep_info)
        return false;
    int cur_ident = m_df->read_int(m_mem->hist_figure_field(rep_info, "current_ident"));
    m_fake_ident_addr = m_df->find_identity(cur_ident);
    if (!m_fake_ident_addr)
        return false;
    m_fake_name_addr = m_mem->hist_figure_field(m_fake_ident_addr, "fake_name");
    m_fake_name = capitalize(m_df->read_string(m_mem->word_field(m_fake_name_addr, "first_name")));
    m_nick_addrs.append(m_mem->word_field(m_fake_name_addr, "nickname"));
    m_fake_nick = m_df->read_string(m_nick_addrs.last());
    //vamps also use a fake age
    m_fake_birth_year = m_mem->hist_figure_field(m_fake_ident_addr, "fake_birth_year");
    m_fake_birth_time = m_mem->hist_figure_field(m_fake_ident_addr, "fake_birth_time");
    return true;
}

bool HistFigure::has_fake_identity(){
    return m_has_fake_identity;
}
