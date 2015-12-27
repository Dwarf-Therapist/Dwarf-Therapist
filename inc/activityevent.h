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
#ifndef ACTIVITYEVENT
#define ACTIVITYEVENT

#include <QObject>
#include "utils.h"

class DFInstance;
class MemoryLayout;

class ActivityEvent : public QObject {
    Q_OBJECT
public:
    ActivityEvent(DFInstance *df, VIRTADDR addr, QHash<int, QPair<int, QString> > *histfig_actions, QObject *parent = 0);

    typedef enum {
        ACT_UNKNOWN = -1,
        SQ_TRAIN = 0,
        SQ_COMBAT_TRAIN = 1,
        SQ_SKILL_DEMO = 2,
        SQ_DRILL = 3,
        SQ_SPAR = 4,
        SQ_RANGED = 5,
        HARASS = 6,
        TALK = 7,
        CONFLICT = 8,
        GUARD = 9,
        REUNION = 10,
        PRAY = 11,
        SOCIALIZE = 12,
        WORSHIP = 13,
        PERFORM = 14,
        RESEARCH = 15,
        PONDER = 16,
        DISCUSS = 17,
        READ = 18,
        SERVICE_ORDER = 19,
        WRITE = 20,
        COPY_WRITTEN = 21,
        TEACH = 22,
        PLAY = 23,
        PLAY_BELIEVE = 24,
        PLAY_TOY = 25,
        //custom performance types
        PF_STORY = 100,
        PF_POETRY = 101,
        PF_MUSIC = 102,
        PF_DANCE = 103,
        PF_AUDIENCE = 104,
        PF_INSTRUMENT = 105,
        PF_SING = 106,
        PF_LISTEN = 107,
        //custom squad types
        SQ_LEAD_DEMO = 200,
        SQ_WATCH_DEMO = 201,
        //custom other types
        MEDITATION = 300
    } ACT_EVT_TYPE;

    typedef enum {
        PERF_UNKNOWN = -1,
        PERF_STORY = 0,
        PERF_POETRY = 1,
        PERF_MUSIC = 2,
        PERF_DANCE = 3,
        PERF_AUDIENCE = 4,
        PERF_IGNORE = 5
    } ACT_PERF_TYPE;

    static QString get_perf_desc(const ACT_PERF_TYPE &type) {
        QMap<ACT_PERF_TYPE,QString> m;
        m[PERF_STORY] = tr("Tell Story");
        m[PERF_POETRY] = tr("Recite Poetry");
        m[PERF_MUSIC] = tr("Play Music/Sing");
        m[PERF_DANCE] = tr("Dance");
        return m.value(type,"");
    }
    static QString get_audience_desc(const ACT_PERF_TYPE &type) {
        QMap<ACT_PERF_TYPE,QString> m;
        m[PERF_STORY] = tr("Listen to Story");
        m[PERF_POETRY] = tr("Listen to Poetry");
        m[PERF_MUSIC] = tr("Listen to Music");
        m[PERF_DANCE] = tr("Watch Dance");
        return m.value(type,"");
    }

    short id(){return m_id;}

private:
    DFInstance *m_df;
    VIRTADDR m_address;
    QHash<int,QPair<int, QString> > *m_histfig_actions;

    short m_id;
    ACT_EVT_TYPE m_type;

    void read_data();
    void add_action(int histfig_id, ACT_EVT_TYPE evt_type, QString desc = "");
};

#endif // ACTIVITYEVENT
