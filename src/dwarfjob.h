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
#ifndef DWARF_JOB_H
#define DWARF_JOB_H

#include "truncatingfilelogger.h"
#include "gamedatareader.h"

#include <QString>
#include <QObject>
#include <QSettings>

class DwarfJob : public QObject {
    Q_OBJECT
public:

    DwarfJob(QObject *parent = 0)
        : QObject(parent)
        , m_id(JOB_UNKNOWN)
        , m_name("Unknown")
        , m_group_name("Unknown")
        , m_reaction_class("")
        , m_img("")
        , m_military(false)
        , m_has_placeholder(false)
    {
    }

    DwarfJob(QSettings &s, int offset, QString group_name, QString img_path, bool military = false, QObject *parent = 0)
        : QObject(parent)
    {
        m_id = s.value("id",0).toInt() + offset;
        m_name = s.value("name", group_name).toString();
        if(m_name.isEmpty()){
            m_name = group_name;
        }
        m_reaction_class = s.value("reaction_class").toString();

        m_group_name = group_name;
        if(m_group_name.isEmpty()){
            m_group_name = m_name;
        }
        m_img = s.value("img",img_path).toString();
        m_military = s.value("is_military",military).toBool();
        m_has_placeholder = m_name.contains("[");
    }

    static const int ACTIVITY_OFFSET = 5000;
    static const int ORDER_OFFSET = 7000;

    //negative jobs are custom
    typedef enum {
        JOB_UNKNOWN = -999,
        JOB_MEETING = -5,
        JOB_CAGED = -4,
        JOB_IDLE = -3,
        JOB_ON_BREAK = -2,
        JOB_SOLDIER = -1,
        JOB_SLEEP = 21,
        JOB_DRINK_BLOOD = 223
    } UNIT_JOB_TYPE;

    static QString get_job_mat_category_name(quint32 flags){
        switch(flags){
        case 0:
            return tr("plant");
        case 2:
            return tr("wood");
        case 4:
            return tr("cloth");
        case 8:
            return tr("silk");
        case 16:
            return tr("leather");
        case 32:
            return tr("bone");
        case 64:
            return tr("shell");
        case 128:
            return tr("wood mat");
        case 256:
            return tr("soap");
        case 512:
            return tr("ivory/tooth");
        case 1024:
            return tr("horn/hoof");
        case 2048:
            return tr("pearl");
        case 4096:
            return tr("yarn/wool/fur");
        }

        return "unknown";
    }

    int id() {return m_id;}

    QString name(QString replacement = ""){
        if(replacement.isEmpty() || !m_has_placeholder){
            return m_name;
        }else{
            QString name = m_name;
            QRegExp re(".*(\\[.*\\]).*");
            int idx = re.indexIn(m_name);
            if(idx > -1){
                name.replace(re.cap(1),replacement);
            }
            return name;
        }
    }

    QString reactionClass() {return m_reaction_class;}
    QString group_name() {return m_group_name;}
    QString img_path() {return m_img;}
    bool is_military() {return m_military;}
    bool has_placeholder() {return m_has_placeholder;}
    bool is_mood() {return (m_id >= 55 && m_id <= 67);}

private:
    int m_id;
    QString m_name;
    QString m_group_name;
    QString m_reaction_class;
    QString m_img;
    bool m_military;
    bool m_has_placeholder;
};

#endif
