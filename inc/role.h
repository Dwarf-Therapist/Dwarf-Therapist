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

#ifndef ROLE_H
#define ROLE_H

#include <QtGui>
#include "math.h"

class Role : public QObject {
    Q_OBJECT
public:
    Role();
    Role(QSettings &s, QObject *parent = 0);
    Role(const Role &r);

    QString name;
    QString script;
    bool is_custom;

    struct aspect{
        bool is_neg;
        float weight;
    };

    struct global_weight{
        bool is_default;
        float weight;
    };

    //unfortunately we need to keep all the keys as a string and cast them so we can use the same functions
    //ie can't pass in a hash with <string, aspect> and <int, aspect>
    QHash<QString, aspect> attributes;
    QHash<QString, aspect> skills;
    QHash<QString, aspect > traits;

    //global weights
    global_weight attributes_weight;
    global_weight skills_weight;
    global_weight traits_weight;

    QString get_role_details();

    void create_role_details(QSettings &s);

    void write_to_ini(QSettings &s, float default_attributes_weight, float default_traits_weight, float default_skills_weight);

protected:
    void parseAspect(QSettings &s, QString node, global_weight &g_weight, QHash<QString,aspect> &list);
    void write_aspect_group(QSettings &s, QString group_name, global_weight group_weight, float group_default_weight, QHash<QString,aspect> &list);
    QString build_aspect_detail(QString title, global_weight aspect_group_weight, float aspect_default_weight, QHash<QString,aspect> &list);
    QString role_details;
};
#endif // ROLE_H
