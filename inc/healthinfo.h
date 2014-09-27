#ifndef HEALTHINFO_H
#define HEALTHINFO_H

#include "utils.h"
#include "global_enums.h"
#include <QObject>
#include <QString>
#include <QColor>

//class HealthInfo : QObject  {
//    Q_OBJECT
class HealthInfo{

public:
    HealthInfo()
    {
        m_description = QObject::tr("Missing");
         m_symbol = "???";
         m_color_name = "#FF0000";
         m_severity = 99;
         m_cat_id = eHealth::HI_UNK;
    }

    void set_description(QString desc) {m_description = desc;}
    void set_symbol(QString sym) {m_symbol = sym;}
    void set_color(QString c) {m_color_name = c;}
    void set_severity(int s) {m_severity = s;}
    void set_category(eHealth::H_INFO id) {m_cat_id = id;}
    void set_type(quint32 val) {m_type_flags = val;}

    QString formatted_value(bool colored, bool symbol){
        if(symbol){
            return this->symbol(colored);
        }else{
            return this->description(colored);
        }
    }

    QString description(bool colored = true){
        if(colored && m_color_name != "#000000")
            return QString("<font color=%1>%2</font>").arg(m_color_name).arg(m_description);
        else
            return m_description;
    }
    QString symbol(bool colored = true){
        if(colored && m_color_name != "#000000")
            return QString("<font color=%1>%2</font>").arg(m_color_name).arg(m_symbol);
        else
            return m_symbol;
    }

    QColor color(){
        return QColor(m_color_name);
    }

    int severity(){
        return m_severity;
    }

    eHealth::H_INFO h_category(){
        return m_cat_id;
    }

//    int type(){
//        return m_type_flags;
//    }

    bool is_treatment() {return (has_flag(1,m_type_flags));}
    bool is_status() {return (has_flag(2,m_type_flags));}
    bool is_wound() {return (has_flag(4,m_type_flags));}

    bool operator==(const HealthInfo &other) const {
        if(this == &other)
            return true;
        return (this->m_description == other.m_description && this->m_type_flags == other.m_type_flags);
    }

    static bool less_than_severity(const HealthInfo *h1, const HealthInfo *h2)
    {
        return h1->m_severity < h2->m_severity;
    }

private:
    QString m_description;
    QString m_symbol;
    QString m_color_name;
    int m_severity; //lower is more severe
    quint32 m_type_flags;
    eHealth::H_INFO m_cat_id;
};

#endif // HEALTHINFO_H
