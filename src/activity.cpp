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
#include "activity.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "gamedatareader.h"
#include "activityevent.h"
#include "truncatingfilelogger.h"
#include "dwarfjob.h"

Activity::Activity(DFInstance *df, VIRTADDR addr, QObject *parent)
    :QObject(parent)
    , m_df(df)
    , m_address(addr)
    , m_id(-1)
    , m_type(ACT_NONE)
{
    read_data();
}

Activity::~Activity(){
    m_df = 0;
    qDeleteAll(m_events);
    m_events.clear();
}

void Activity::read_data(){
    if(m_address && m_df){
        MemoryLayout *mem = m_df->memory_layout();
        m_id = m_df->read_int(m_address);
        m_type = static_cast<ACT_CATEGORY>(m_df->read_short(m_address + mem->activity_offset("activity_type")));
        LOGD << "reading activity of type" << m_type;

        if(m_type == ACT_UNK_3 || m_type == ACT_UNK_4 || m_type == ACT_UNK_6){
            LOGD << "uknown activity category" << m_type;
        }else if(m_type == ACT_CONFLICT || m_type == ACT_CONVERSE){
            return; //ignore these activity types as they don't impact current jobs/actions
        }

        //go through the events backwards, the vector contains events in order from parent to child
        //ie. training->combat training->skill demonstration so the last items are the most specific
        QVector<VIRTADDR> events = m_df->enumerate_vector(m_address + mem->activity_offset("events"));
        for(int idx=events.count()-1;idx>=0;idx--){
            ActivityEvent *ae = new ActivityEvent(m_df,events.at(idx),&m_histfig_actions,this);
            if(ae){
                m_events.insert(ae->id(),ae);
            }
        }
    }
}

QPair<int, QString> Activity::find_activity(int histfig_id){
    return m_histfig_actions.value(histfig_id,qMakePair<int,QString>(DwarfJob::JOB_UNKNOWN,"unknown"));
}
