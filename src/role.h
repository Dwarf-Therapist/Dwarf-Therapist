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

#include <QObject>
#include <QVector>
#include <QColor>
#include <QHash>

class Preference;
class QSettings;
class RoleAspect;
class Dwarf;

class Role : public QObject {
    Q_OBJECT
public:
    Role();
    Role(QSettings &s, QObject *parent = 0);
    Role(const Role &r);
    virtual ~Role();

    struct weight_info{
        bool is_default;
        bool is_neg;
        float weight;
    };

    struct simple_rating{
        bool is_custom;
        float rating;
        QString name;
    };

    QString name(){return m_name;}
    void name(QString name){m_name = name;}
    QString script(){return m_script;}
    void script(QString script){m_script = script;}
    bool is_custom(){return m_is_custom;}
    void is_custom(bool val){m_is_custom = val;}
    bool updated(){return m_updated;}

    //unfortunately we need to keep all the keys as a string and cast them so we can use the same functions
    //ie can't pass in a hash with <string, aspect> and <int, aspect>
    QHash<QString, RoleAspect*> attributes;
    QHash<QString, RoleAspect*> skills;
    QHash<QString, RoleAspect*> traits;
    QVector<Preference*> prefs;

    //global weights
    weight_info attributes_weight;
    weight_info skills_weight;
    weight_info traits_weight;
    weight_info prefs_weight;

    QString get_role_details(Dwarf *d = 0);

    void set_labors(QList<int> list){m_labors = list;}
    QList<int> get_labors() {return m_labors;}

    void create_role_details(QSettings &s, Dwarf *d=0);

    void write_to_ini(QSettings &s, float default_attributes_weight, float default_traits_weight, float default_skills_weight, float default_prefs_weight);

    Preference* has_preference(QString name);
    static const QColor color_has_prefs() {return QColor(0, 60, 128, 135);}

protected:
    void parseAspect(QSettings &s, QString node, weight_info &g_weight, QHash<QString, RoleAspect *> &list, float default_weight);
    void parsePreferences(QSettings &s, QString node, weight_info &g_weight, float default_weight);
    void validate_pref(Preference *p, int first_flag);
    void write_aspect_group(QSettings &s, QString group_name, weight_info group_weight, float group_default_weight, QHash<QString, RoleAspect *> &list);
    void write_pref_group(QSettings &s, float default_prefs_weight);

    QString get_aspect_details(QString title, weight_info aspect_group_weight, float aspect_default_weight, QHash<QString, RoleAspect *> &list);
    QString get_preference_details(float aspect_default_weight, Dwarf *d=0);
    void refresh_preferences(Dwarf *d);
    void highlight_pref_matches(Dwarf *d, QString &pref_desc);
    QString generate_details(QString title, weight_info aspect_group_weight, float aspect_default_weight, QHash<QString, weight_info> weight_infos);

    QString m_name;
    QString m_script;
    bool m_is_custom;

    QString role_details;
    QList<int> m_labors; //labors associated via the skills in the role
    QString m_pref_desc;
    int m_cur_pref_len;
    bool m_updated;
};
#endif // ROLE_H
