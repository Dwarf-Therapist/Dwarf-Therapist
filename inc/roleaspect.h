#ifndef ROLEASPECT_H
#define ROLEASPECT_H

#include <QObject>

class RoleAspect : QObject {
    Q_OBJECT
public:

    RoleAspect(QObject *parent = 0)
        :QObject(parent)
    {
        weight = 0;
        is_neg = false;
    }

    float weight;
    bool is_neg;
};

#endif // ROLEASPECT_H
