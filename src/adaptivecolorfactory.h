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
#ifndef ADAPTIVE_COLOR_FACTORY_H
#define ADAPTIVE_COLOR_FACTORY_H

#include <QPalette>

//! Compute adaptive colors with regard to the given palette and foreground
//  and background colors.
class AdaptiveColorFactory {
public:
    AdaptiveColorFactory(QPalette::ColorRole fg_role = QPalette::WindowText, QPalette::ColorRole bg_role = QPalette::Window, const QPalette &palette = QPalette());

    /*! Linear interpolation between background and text colors.
     *  Low value is closer to background, high value is closer to text color.
     */
    QColor gray(qreal value) const;

    //! Change color lightness to be closer to the foreground color.
    QColor color(const QColor &color) const;

private:
    QColor m_fg, m_bg;
};

#endif
