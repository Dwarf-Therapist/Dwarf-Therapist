#ifndef EVENTFILTERLINEEDIT_H
#define EVENTFILTERLINEEDIT_H

#include <QObject>
#include <QString>

class QEvent;
class QLineEdit;
class QModelIndex;

class EventFilterLineEdit : public QObject
{
    Q_OBJECT
public:
    EventFilterLineEdit(QLineEdit* lineEdit, QObject* parent = nullptr)
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
