/*
Dwarf Therapist
Copyright (c) 2011 Justin Ehlert (Dwarf Engineer)

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
#ifndef RAWNODE_H
#define RAWNODE_H

#include <QObject>
#include <QVector>
#include <QSharedPointer>

class RawNode;
typedef QSharedPointer<RawNode> RawNodePtr;

class RawNode : public QObject {
    Q_OBJECT
    friend class RawReader;
public:
    RawNode(QObject * parent = 0) : QObject(parent) {
    }

    RawNode(QString name, QObject * parent) :
            QObject(parent), name(name) {
    }

    RawNode(QString name, const QVector<QString> & values, QObject * parent) :
            QObject(parent), name(name), values(values) {
    }

    QString get_name() const {
        return name;
    }

    QString get_value(int idx, QString default_value = "") const {
        if(values.size() > idx)
            return values[idx];
        return default_value;
    }

    const QVector<RawNodePtr> & get_children() const {
        return children;
    }

    QVector<RawNodePtr> get_children(QString name) const {
        QVector<RawNodePtr> result;

        foreach(RawNodePtr n, children) {
            if(n->name == name)
                result.append(n);
        }

        return result;
    }

    const QString get_value(QString name, QString default_value = "") const {
        QString result = default_value;
        foreach(RawNodePtr n, children) {
            if(n->name == name && !n->values.isEmpty()) {
                result = n->values[0];
                break;
            }
        }
        return result;
    }

protected:
    QString name;
    QVector<QString> values;
    QVector<RawNodePtr> children;
};



#endif // RAWNODE_H
