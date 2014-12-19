#ifndef CELLCOLORDEF_H
#define CELLCOLORDEF_H

#include <QObject>
#include <QColor>

class CellColorDef : public QObject {
    Q_OBJECT
public:
    CellColorDef(QColor color, QString key, QString title, QString desc, QObject *parent=0)
        : QObject(parent)
        , m_overridden(false)
        , m_col(color)
        , m_key(key)
        , m_title(title)
        , m_desc(desc)
    {
    }

    CellColorDef(const CellColorDef &ccd, QObject *parent = 0)
        : QObject(parent)
        , m_overridden(ccd.m_overridden)
        , m_col(ccd.m_col)
        , m_key(ccd.m_key)
        , m_title(ccd.m_title)
        , m_desc(ccd.m_desc)
    {
    }

    bool is_overridden() const {return m_overridden;}
    QColor color() const {return m_col;}
    QString key() const {return m_key;}
    QString title() const {return m_title;}
    QString description() const {return m_desc;}

    void set_overridden(bool val) {m_overridden=val;}
    void set_color(QColor c) {m_col = c;}

private:
    bool m_overridden;
    QColor m_col;
    QString m_key;
    QString m_title;
    QString m_desc;
};
#endif // CELLCOLORDEF_H
