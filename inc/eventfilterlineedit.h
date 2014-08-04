#ifndef EVENTFILTERLINEEDIT_H
#define EVENTFILTERLINEEDIT_H

#include <QtWidgets>
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

    bool eventFilter(QObject* watched, QEvent* event)
    {
        QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched);
        //LOGI << event->type();
        //event->type() == QEvent::HideToParent || event->type() == QEvent::Hide || event->type() == QEvent::Leave ||
//        if(event->type() == QEvent::Leave){
//            mLineEdit->clear();
//            view->hide();
//            return true;
//        }
        if (event->type() == QEvent::KeyPress){
            QKeyEvent* keyEvent = dynamic_cast<QKeyEvent*>(event);
            if (keyEvent->key() == Qt::Key_Return ||
                    keyEvent->key() == Qt::Key_Enter)
            {
                mLineEdit->clear();
                view->hide();
                emit enterPressed(view->currentIndex());
                return true;
            }
        }
        return false;
    }

signals:
    void enterPressed(QModelIndex);

private:
    QLineEdit* mLineEdit;
};
#endif // EVENTFILTERLINEEDIT_H
