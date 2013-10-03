#ifndef HEALTHCATEGORY_H
#define HEALTHCATEGORY_H

#include "truncatingfilelogger.h"
#include "healthinfo.h"

class MemoryLayout;
class DFInstance;

class HealthCategory{

public:
    HealthCategory(){
        m_individual_types = false;
    }

    HealthCategory(QSettings &s)
    {
        m_id = s.value("id",0).toInt();
        m_color_name = s.value("color","#000000").toString(); //default color for the whole category
        m_name = s.value("name", "Unknown").toString();
        m_multiple = s.value("multiple",false).toBool(); //indicates if a category/group allows multiple descriptions. ie. motor and sensory nerve damage
        m_type_flags = s.value("type",2).toInt(); //default to status
        m_individual_types = false;

        int desc_count = s.beginReadArray("descriptions");
        bool requireSort = false;
        bool hex_ok;

        for (int j = 0; j < desc_count; j++) {
            s.setArrayIndex(j);
            HealthInfo *hi = new HealthInfo();
            hi->set_description(s.value("desc","???").toString());
            QString sym = s.value("symbol","?").toString();
            if(sym.contains(QRegExp("0x[0-9A-F]{4}"))){
                hi->set_symbol(QChar(sym.toUInt(&hex_ok,16)));
            }else{
                hi->set_symbol(sym);
            }
            hi->set_color(s.value("color",m_color_name).toString()); //color override for specific items in the category
            hi->set_category(m_id);
            if(!requireSort && s.contains("severity"))
                requireSort = true;
            hi->set_severity(s.value("severity",j).toInt());
            if(!m_individual_types && s.contains("type"))
                m_individual_types = true;
            hi->set_type(s.value("type",m_type_flags).toInt());
            m_descriptors.insert(j, hi);
        }
        s.endArray();

        if(requireSort)
            std::sort(m_descriptors.begin(), m_descriptors.end(), HealthInfo::less_than_severity());

        m_color = QColor(m_color_name);
    }

    ~HealthCategory(){
        m_descriptors.clear();
    }

    int id() {return m_id;}

    QString name() {return m_name;}
    bool allows_multiple() {return m_multiple;}

    QColor color() {return m_color;}
    QString color_name() {return m_color_name;}

    HealthInfo* description(short idx){
        if(idx >= 0 && idx < m_descriptors.size())
            return m_descriptors.at(idx);
        else
            return 0;
    }

    bool diff_subitem_types() {return m_individual_types;}

    QList<HealthInfo*> descriptions() {return m_descriptors;}

    QStringList get_all_descriptions(bool symbol_only, bool colored){
        QStringList descriptions;
        foreach(HealthInfo *hi, m_descriptors){
            if(symbol_only)
                descriptions.append(hi->symbol(colored));
            else
                descriptions.append(hi->description(colored));
        }
        return descriptions;
    }

    bool is_treatment() {return (has_flag(1,m_type_flags));}
    bool is_status() {return (has_flag(2,m_type_flags));}
    bool is_wound() {return (has_flag(4,m_type_flags));}

private:
    int m_id;
    QString m_name;
    bool m_multiple;
    QString m_color_name;
    QColor m_color;

    quint32 m_type_flags;
    QList<HealthInfo*> m_descriptors;

    bool m_individual_types; //indicates if the individual items within the category have different types
};

#endif // HEALTHCATEGORY_H
