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

    QString name;
    QString script;

    struct aspect{
        bool is_neg;
        float weight;
    };

    //unfortunately we need to keep all the keys as a string and cast them so we can use the same functions
    //ie can't pass in a hash with <string, aspect> and <int, aspect>
    QHash<QString, aspect> attributes;
    QHash<QString, aspect> skills;
    QHash<QString, aspect > traits;

    //global weights
    float attribute_weight;
    float skills_weight;
    float trait_weight;

private:
    void parseAspect(QString raw, float &weight, QHash<QString,aspect> &list);



};
#endif // ROLE_H
