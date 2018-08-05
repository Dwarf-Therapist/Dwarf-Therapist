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
#include <memory>

class DefaultRoleWeight;
class RolePreference;
class QSettings;
class Dwarf;

class Role: public QObject {
    Q_OBJECT
public:
    Role(QObject *parent = nullptr);
    Role(QSettings &s, QObject *parent = nullptr);
    Role(const Role &r, QObject *parent = nullptr);

    virtual ~Role();

    class weight_info {
        const DefaultRoleWeight &m_default_value;
        bool m_is_default;
        float m_weight;

    public:
        float weight() const;
        float default_weight() const;
        bool is_default() const { return m_is_default; }

        weight_info(const DefaultRoleWeight &default_value);

        void reset_to_default();
        void set(float weight);
    };

    struct aspect_weight {
        float weight;
        bool is_neg;

        aspect_weight(float weight = 1.0f, bool is_neg = false)
            : weight(weight)
            , is_neg(is_neg)
        {
        }
    };

    struct simple_rating{
        bool is_custom;
        float rating;
        QString name;
    };

    const QString &name() const {return m_name;}
    void name(const QString &name){m_name = name;}
    const QString &script() const {return m_script;}
    void script(const QString &script){m_script = script;}
    bool is_custom() const {return m_is_custom;}
    void is_custom(bool val){m_is_custom = val;}
    bool updated() const {return m_updated;}

    std::vector<std::pair<QString, aspect_weight>> attributes;
    std::vector<std::pair<int, aspect_weight>> skills;
    std::vector<std::pair<int, aspect_weight>> facets;
    std::vector<std::pair<int, aspect_weight>> beliefs;
    std::vector<std::pair<int, aspect_weight>> goals;
    std::vector<std::pair<int, aspect_weight>> needs;
    std::vector<std::pair<std::unique_ptr<RolePreference>, aspect_weight>> prefs;

    bool has_aspects() const {
        return !attributes.empty() || !skills.empty() || !facets.empty() ||
                !beliefs.empty() || !goals.empty() || !needs.empty() || !prefs.empty();
    }

    //global weights
    weight_info attributes_weight;
    weight_info skills_weight;
    weight_info facets_weight;
    weight_info beliefs_weight;
    weight_info goals_weight;
    weight_info needs_weight;
    weight_info prefs_weight;

    QString get_role_details(Dwarf *d = 0);

    void set_labors(QList<int> list){m_labors = list;}
    QList<int> get_labors() const {return m_labors;}

    void create_role_details(Dwarf *d=0);

    void write_to_ini(QSettings &s) const;

    RolePreference* has_preference(QString name);
    static const QColor color_has_prefs() {return QColor(0, 60, 128, 135);}

private:
    template<typename T>
    void parseAspect(QSettings &s, QString node, weight_info &g_weight, std::vector<std::pair<T, aspect_weight>> &list);
    void validate_pref(RolePreference *p, int first_flag);
    template<typename T>
    void write_aspect_group(QSettings &s, const QString &group_name,
                            const weight_info &group_weight,
                            const std::vector<std::pair<T, aspect_weight>> &list) const;

    template<typename T, typename F>
    QString get_aspect_details(const QString &title,
                               const weight_info &aspect_group_weight,
                               const std::vector<std::pair<T, aspect_weight>> &list,
                               F id_to_name);
    QString get_preference_details(Dwarf *d=0);
    void refresh_preferences(Dwarf *d);
    void highlight_pref_matches(Dwarf *d, QString &pref_desc);
    QString generate_details(const QString &title,
                             const weight_info &aspect_group_weight,
                             const std::vector<std::pair<QString, aspect_weight>> &aspects);

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
