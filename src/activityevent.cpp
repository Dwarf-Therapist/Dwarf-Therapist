/*
Dwarf Therapist
Copyright (c) 2015 Josh Butgereit

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
#include "activityevent.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "gamedatareader.h"
#include "dwarfjob.h"
#include "histfigure.h"

ActivityEvent::ActivityEvent(DFInstance *df, VIRTADDR addr, QHash<int, QPair<int,QString> > *histfig_actions, QObject *parent)
    :QObject(parent)
    , m_df(df)
    , m_address(addr)
    , m_histfig_actions(histfig_actions)
    , m_id(-1)
    , m_type(ACT_UNKNOWN)
{
    read_data();
}

void ActivityEvent::read_data(){
    if(m_address && m_df){
        MemoryLayout *mem = m_df->memory_layout();

        short raw_type = m_df->read_short(m_df->read_addr(m_df->read_addr(m_address))+m_df->VM_TYPE_OFFSET());
        if(raw_type < 0){
            raw_type = 0;
        }
        m_id = m_df->read_int(m_address+m_df->pointer_size());
        m_type = static_cast<ACT_EVT_TYPE>(raw_type);

        QList<ACT_EVT_TYPE> ignore;
        ignore << HARASS << TALK << CONFLICT << REUNION;
        if(ignore.contains(m_type)){
            LOGD << "   ignoring activity event due to type";
            return;
        }

        LOGD << "   activity event type:" << raw_type << " id:" << m_id;

        ACT_EVT_TYPE event_type = m_type;

        if(event_type == SERVICE_ORDER || event_type == COPY_WRITTEN){ //only a single participant
            int id = m_df->read_int(mem->activity_field(m_address, "participants"));
            if(!m_histfig_actions->contains(id)){
                add_action(id,event_type);
            }
            //vectors after the participants has id numbers that correspond to either the artifact being copied, or the drink/food being served
        }else{
            GameDataReader *gdr = GameDataReader::ptr();
            USIZE participant_offset = mem->activity_offset("participants");
            VIRTADDR participant_addr = m_address + participant_offset;
            QSet<qint32> participants = m_df->enum_vec<qint32>(participant_addr).toList().toSet();

            foreach(qint32 histfig_id,participants){
                event_type = m_type;
                //single participants
                if(m_histfig_actions->contains(histfig_id)){
                    continue;
                }
                //squad lead participants, check and change type or cancel if necessary
                if(event_type == SQ_SKILL_DEMO){
                    if(m_df->read_int(mem->activity_field(m_address, "sq_train_rounds")) <= 0){
                        continue;
                    }
                    if(m_df->read_int(mem->activity_field(m_address, "sq_lead")) == histfig_id){ //some squad activities have leaders, check that first
                        event_type = SQ_LEAD_DEMO;
                    }else{
                        event_type = SQ_WATCH_DEMO;
                    }
                    //squad skill demo leaders and watchers
                    QString skill_name = gdr->get_skill_name(m_df->read_int(mem->activity_field(m_address, "sq_skill")), false);
                    QString job_name = gdr->get_job((int)DwarfJob::ACTIVITY_OFFSET + (int)event_type)->name(skill_name);
                    add_action(histfig_id,event_type, job_name);
                    continue;
                }

                if(event_type == PRAY){
                    short sphere_id = m_df->read_short(mem->activity_field(m_address, "pray_sphere"));
                    if(sphere_id == -1){ //no sphere indicates prayer
                        int deity_id = m_df->read_short(mem->activity_field(m_address, "pray_deity"));
                        QString deity_name = HistFigure::get_name(m_df, deity_id, true);
                        if(!deity_name.isEmpty()){
                            add_action(histfig_id,event_type,
                                       tr("Pray to %1").arg(deity_name));
                            continue;
                        }
                    }else{
                        add_action(histfig_id,MEDITATION,tr("Meditate on %1").arg(GameDataReader::ptr()->get_sphere_name(sphere_id)));
                        continue;
                    }
                }
                if(event_type == PONDER){
                    int field = m_df->read_int(mem->activity_field(m_address, "knowledge_category"));
                    int topic = m_df->read_int(mem->activity_field(m_address, "knowledge_flag"));
                    topic = log2((double)topic);
                    add_action(histfig_id,event_type,gdr->get_knowledge_desc(field,topic));
                    continue;
                }
                if(event_type == PERFORM){
                    ACT_PERF_TYPE p_type = static_cast<ACT_PERF_TYPE>(m_df->read_int(mem->activity_field(m_address, "perf_type")));
                    QVector<VIRTADDR> p_participants = m_df->enumerate_vector(mem->activity_field(m_address, "perf_participants"));
                    foreach(VIRTADDR p_addr, p_participants){
                        int id = m_df->read_int(mem->activity_field(p_addr, "perf_histfig"));
                        if(m_histfig_actions->contains(id)){
                            continue;
                        }
                        ACT_PERF_TYPE par_type = static_cast<ACT_PERF_TYPE>(m_df->read_int(p_addr));
                        if(par_type == PERF_IGNORE){
                            continue; //they're either not listening, or ignoring
                        }

                        QString desc = "";
                        event_type = ACT_UNKNOWN;
                        if(par_type == PERF_AUDIENCE){
                            desc = get_audience_desc(p_type);
                            if(p_type == PERF_DANCE){
                                event_type = PF_AUDIENCE;
                            }else{
                                event_type = PF_LISTEN;
                            }
                        }else if((par_type == PERF_STORY || par_type == PERF_POETRY) && par_type == (int)p_type){
                            desc = get_perf_desc(p_type);
                            event_type = static_cast<ACT_EVT_TYPE>((int)PF_STORY + (int)p_type);
                        }else if(par_type == PERF_MUSIC){
                            int p_sub_type = m_df->read_int(p_addr + 0x4);
                            if(p_sub_type == 0){
                                event_type = PF_SING;
                            }else if(p_sub_type > 0){
                                event_type = PF_INSTRUMENT;
                                //instrument name can be found by musical_form->instrument sub form->index (p_sub_type)
                                //the first integer is an instrument sub-type id
                            }
                        }else if(par_type == PERF_DANCE){
                            desc = get_perf_desc(par_type);
                            event_type = PF_DANCE;
                        }

                        if(event_type != ACT_UNKNOWN){
                            add_action(id,event_type,desc);
                        }
                    }
                    continue;
                }
                //default
                add_action(histfig_id,event_type);
            }
        }
    }
}

void ActivityEvent::add_action(int histfig_id, ACT_EVT_TYPE evt_type, QString desc){
    int job_id = (int)evt_type + DwarfJob::ACTIVITY_OFFSET;
    if(desc.isEmpty()){
        desc = GameDataReader::ptr()->get_job(job_id)->name();
    }
    if(desc.toLower() == "unknown" || desc.toLower().trimmed().isEmpty()){
        LOGW << "unknown activity event:" << evt_type;
    }
    m_histfig_actions->insert(histfig_id,qMakePair(job_id,desc));
}

