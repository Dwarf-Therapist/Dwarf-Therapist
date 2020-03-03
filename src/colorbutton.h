/*
Dwarf Therapist
Copyright (c) 2020 Clement Vuchener

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
#ifndef COLOR_BUTTON_H
#define COLOR_BUTTON_H

#include <QAction>
#include <QMenu>
#include <QWidget>

class QToolButton;

class ColorButton: public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    ColorButton(const QColor &default_color = {}, QWidget *parent = nullptr);

    const QColor &color() const;
    const QColor &defaultColor() const;

public slots:
    void setColor(const QColor &color);
    void resetToDefault();

signals:
    void colorChanged(const QColor &color);

private:
    void update();
    QPixmap makeColorIcon(const QColor &color) const;

    QColor m_default_color;
    QColor m_current_color;
    QBrush m_checker_brush;
    QToolButton *m_button;
    QAction m_change_color;
    QMenu m_menu;
    QAction *m_reset;
};

#endif
