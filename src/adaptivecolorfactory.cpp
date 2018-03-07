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

#include "adaptivecolorfactory.h"

#include <QtMath>

AdaptiveColorFactory::AdaptiveColorFactory(QPalette::ColorRole fg_role, QPalette::ColorRole bg_role, const QPalette &palette)
    : m_fg(palette.color(fg_role))
    , m_bg(palette.color(bg_role))
{
}

QColor AdaptiveColorFactory::gray(qreal value) const
{
    return QColor::fromRgbF(value*m_fg.redF()   + (1.0-value)*m_bg.redF(),
                            value*m_fg.greenF() + (1.0-value)*m_bg.greenF(),
                            value*m_fg.blueF()  + (1.0-value)*m_bg.blueF());
}

QColor AdaptiveColorFactory::color(const QColor &color) const
{
    qreal c = qPow(color.lightnessF(), 0.5); // improve contrast with fg color
    return QColor::fromHslF(color.hueF(),
                            color.saturationF(),
                            (1.0-c/2.0)*m_fg.lightnessF() + (c/2.0)*m_bg.lightnessF());
}
