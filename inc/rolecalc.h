#ifndef ROLECALC_H
#define ROLECALC_H

#include "QRunnable"
#include "dwarf.h"
#include "QObject"

class rolecalc : public QObject, public QRunnable
{
    Q_OBJECT

public:
    rolecalc(Dwarf *d);

private:
    Dwarf *m_dwarf;

protected:
    void run();

signals:
    void done();

};

#endif // ROLECALC_H
