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

#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

#include <QtCore>
#include "global_enums.h"
#include "qcolor.h"

class Attribute {
public:    
    Attribute();
    Attribute(ATTRIBUTES_TYPE id, int value, int display_value, int max, int cost_to_improve = 500, int desc_index = 0, QString desc = "");

    int id(){return m_id;}
    ATTRIBUTES_TYPE att_type(){return static_cast<ATTRIBUTES_TYPE>(m_id);}
    QString get_value_display();
    QString get_syndrome_desc();
    QString get_name();
    QString get_descriptor(){return m_descriptor;}
    int get_descriptor_rank(){return m_descriptor_index;}
    int get_value() {return m_value;}
    float get_potential_value();
    float get_balanced_value();
    void calculate_balanced_value();
    int display_value(){return m_display_value;}
    float rating(bool potential = false);
    QStringList syndrome_names(){return m_syn_names;}
    float max() {return m_max;}
    float cti() {return m_cti;}

    void set_rating(float rating, bool potential=false);
    void set_syn_names(QStringList names);

    static void load_attribute_descriptors(QSettings &s);
    static QString find_descriptor(ATTRIBUTES_TYPE, int index = -1);

    static ATTRIBUTES_TYPE get_attribute_type(QString name);
    static const QColor color_affected_by_syns() {return QColor(0, 60, 128, 135);}

private:
    ATTRIBUTES_TYPE m_id;
    int m_value; //raw value including permanent syndrome effects
    float m_value_potential;
    float m_value_balanced;
    int m_display_value; //raw value including permanent and temporary syndrome effects
    int m_max;
    float m_rating_potential;
    float m_rating;
    int m_cti; //cost to improve (caste specific)
    QString m_descriptor; //caste specific depending on the bins
    int m_descriptor_index;
    QStringList m_syn_names;    

    static QHash<int, QVector<QString> > m_display_descriptions;

};

#endif // ATTRIBUTE_H
