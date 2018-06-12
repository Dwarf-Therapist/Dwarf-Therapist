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

#ifndef DEFAULT_ROLE_WEIGHT_H
#define DEFAULT_ROLE_WEIGHT_H

#include <QString>

class DefaultRoleWeight
{
public:
    inline float value() const { return m_overwrite ? m_value : m_default_value; }
    inline float default_value() const { return m_default_value; }
    inline bool is_overwritten() const { return m_overwrite; }
    void set(float value);
    void update();

    static DefaultRoleWeight
        attributes,
        skills,
        facets,
        beliefs,
        goals,
        preferences;

    static void update_all();

private:
    DefaultRoleWeight(const char *key, float default_value);

    QString m_value_key;
    QString m_overwrite_key;
    float m_default_value;
    bool m_overwrite;
    float m_value;
};

#endif // DEFAULT_ROLE_WEIGHT_H
