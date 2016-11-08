#include "basedock.h"

void BaseDock::floating_changed(bool floating) {
    //it's currently pretty buggy to do this on linux. no idea why... yet..
#ifdef Q_OS_LINUX
    Q_UNUSED(floating);
#else
    bool vis = this->isVisible();
    if(floating){
        this->setWindowFlags(Qt::Window);
        QPoint pos = this->pos();
        if(pos.x() < 0)
            pos.setX(0);
        if(pos.y() < 0)
            pos.setY(0);
        this->move(pos);

        if(vis)
            this->show();
    }
#endif
}
