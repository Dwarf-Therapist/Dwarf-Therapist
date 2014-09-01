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
#include "unitbelief.h"
#include "gamedatareader.h"
#include "trait.h"

UnitBelief::UnitBelief()
    : m_belief_id(-1)
    , m_belief_value(-999)
    , m_is_personal(true)
    , m_is_active(false)
{
}

UnitBelief::UnitBelief(short id, int value, bool is_personal)
    : m_belief_id(id)
    , m_belief_value(value)
    , m_is_personal(is_personal)
{
    check_active(id,value);
}

void UnitBelief::check_active(short id, int val){
    Belief *b = GameDataReader::ptr()->get_belief(id);
    if(b){
        m_is_active = b->is_active(val);
    }
}
