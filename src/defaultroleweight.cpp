/*
Dwarf Therapist
Copyright (c) 2018 Clement Vuchener

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

#include "defaultroleweight.h"

#include "standardpaths.h"

DefaultRoleWeight DefaultRoleWeight::attributes("attributes", 0.25);
DefaultRoleWeight DefaultRoleWeight::skills("skills", 1.0);
DefaultRoleWeight DefaultRoleWeight::facets("traits", 0.20);
DefaultRoleWeight DefaultRoleWeight::beliefs("beliefs", 0.20);
DefaultRoleWeight DefaultRoleWeight::goals("goals", 0.10);
DefaultRoleWeight DefaultRoleWeight::needs("needs", 0.10);
DefaultRoleWeight DefaultRoleWeight::preferences("prefs", 0.15);

DefaultRoleWeight::DefaultRoleWeight(const char *key, float default_value)
    : m_value_key(QString("options/default_%1_weight").arg(key))
    , m_overwrite_key(QString("options/overwrite_default_%1_weight").arg(key))
    , m_default_value(default_value)
    , m_value(default_value)
{
}

void DefaultRoleWeight::update()
{
    auto s = StandardPaths::settings();
    m_overwrite = s->value(m_overwrite_key, s->contains(m_value_key)).toBool();
    m_value = s->value(m_value_key, m_default_value).toFloat();
}

void DefaultRoleWeight::update_all()
{
    attributes.update();
    skills.update();
    facets.update();
    beliefs.update();
    goals.update();
    needs.update();
    preferences.update();
}

