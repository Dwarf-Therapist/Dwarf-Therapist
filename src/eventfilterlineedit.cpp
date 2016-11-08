#include "eventfilterlineedit.h"

#include <QLineEdit>
#include <QAbstractItemView>
#include <QKeyEvent>

bool EventFilterLineEdit::eventFilter(QObject* watched, QEvent* event) {
    QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched);
    if (event->type() == QEvent::KeyPress){
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
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
