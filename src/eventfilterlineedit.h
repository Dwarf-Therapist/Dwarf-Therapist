#ifndef EVENTFILTERLINEEDIT_H
#define EVENTFILTERLINEEDIT_H

#include <QKeyEvent>
#include <QLineEdit>
#include <QAbstractItemView>
#include "truncatingfilelogger.h"

class EventFilterLineEdit : public QObject
{
    Q_OBJECT
public:
    EventFilterLineEdit(QLineEdit* lineEdit, QObject* parent = NULL)
        :QObject(parent)
        ,mLineEdit(lineEdit)
    { }
    virtual ~EventFilterLineEdit()
    { }

    bool eventFilter(QObject* watched, QEvent* event);

signals:
    void enterPressed(QModelIndex);

private:
    QLineEdit* mLineEdit;
};
#endif // EVENTFILTERLINEEDIT_H
