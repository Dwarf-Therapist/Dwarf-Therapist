#include "rolecalc.h"

rolecalc::rolecalc(Dwarf *d)
{
    m_dwarf = d;
}
void rolecalc::run(){
    m_dwarf->calc_role_ratings();
    emit done();
}
