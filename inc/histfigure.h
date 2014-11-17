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
#ifndef HISTFIGURE_H
#define HISTFIGURE_H

#include "utils.h"

class DFInstance;
class MemoryLayout;

class HistFigure : public QObject {
    Q_OBJECT

public:
    HistFigure(int id, DFInstance *df, QObject *parent = 0);
    virtual ~HistFigure();

    struct kill_info{
        QString name;
        int year;
        int site;
        int count;
        QString creature;
    };

    void write_nick_name(const QString new_nick);
    bool has_fake_identity();

    VIRTADDR address() const {return m_address;}
    VIRTADDR info_address() const {return m_fig_info_addr;}

    int id(){return m_id;}
    QString fake_name(){return m_fake_name;}
    QString fake_nick_name(){return m_fake_nick;}
    VIRTADDR fake_birth_year_offset(){return m_fake_birth_year;}
    VIRTADDR fake_birth_time_offset(){return m_fake_birth_time;}
    VIRTADDR fake_name_offset(){return m_fake_name_addr;}

    int total_kills();
    int kill_count(bool notable = false);
    QStringList notable_kills();
    QStringList other_kills();
    QString formatted_summary(bool show_no_kills = false, bool space_notable = false);

private:
    DFInstance *m_df;
    MemoryLayout *m_mem;
    VIRTADDR m_address;
    VIRTADDR m_fig_info_addr;
    VIRTADDR m_fake_ident_addr;
    VIRTADDR m_fake_name_addr;
    QList<VIRTADDR> m_nick_addrs;
    int m_id;
    QString m_fake_name;
    QString m_fake_nick;
    VIRTADDR m_fake_birth_year;
    VIRTADDR m_fake_birth_time;

    QList<kill_info> m_notable_kills;
    QList<kill_info> m_other_kills;

    QStringList m_notable_kill_list;
    QStringList m_other_kill_list;

    int m_total_kills_other;

    void read_kills();

    static bool sort_kill_count(const kill_info &k1, const kill_info &k2){
        return k1.count > k2.count;
    }
    static bool sort_kill_date(const kill_info &k1, const kill_info &k2){
        return k1.year > k2.year;
    }

};
#endif // HISTFIGURE_H
