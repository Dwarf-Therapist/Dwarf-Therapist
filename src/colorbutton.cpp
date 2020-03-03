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
#include "colorbutton.h"

#include <QColorDialog>
#include <QHBoxLayout>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionButton>
#include <QToolButton>

ColorButton::ColorButton(const QColor &default_color, QWidget *parent)
    : QWidget(parent)
    , m_default_color(default_color)
    , m_current_color(default_color)
    , m_button(new QToolButton(this))
    , m_change_color(nullptr)
{
    static constexpr int checker_size = 4;
    QPixmap checker(2*checker_size, 2*checker_size);
    {
        QPainter painter(&checker);
        for (int i = 0; i < 4; ++i)
            painter.fillRect(
                    i/2 * checker_size, i%2 * checker_size,
                    checker_size, checker_size,
                    (i/2+i)%2 == 0 ? Qt::lightGray : Qt::darkGray);
    }
    m_checker_brush.setTexture(checker);

    m_button->setDefaultAction(&m_change_color);
    m_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_button->setPopupMode(QToolButton::MenuButtonPopup);
    m_button->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_reset = m_menu.addAction(tr("Reset to default"));
    m_reset->setIconVisibleInMenu(true);
    m_reset->setIcon(makeColorIcon(m_default_color));

    m_button->setMenu(&m_menu);

    update();

    auto layout = new QHBoxLayout;
    layout->setContentsMargins({});
    layout->setSpacing(0);
    layout->addWidget(m_button);
    setLayout(layout);

    connect(this, &ColorButton::colorChanged, this, &ColorButton::update);
    connect(&m_change_color, &QAction::triggered, [this]() {
            auto color = QColorDialog::getColor(m_current_color, nullptr, {}, QColorDialog::ShowAlphaChannel);
            if (color.isValid()) {
                m_current_color = color;
                colorChanged(color);
            }
        });
    connect(m_reset, &QAction::triggered, this, &ColorButton::resetToDefault);
}

const QColor &ColorButton::color() const
{
    return m_current_color;
}

const QColor &ColorButton::defaultColor() const
{
    return m_default_color;
}

void ColorButton::setColor(const QColor &color)
{
    m_current_color = color;
    colorChanged(color);
}

void ColorButton::resetToDefault()
{
    m_current_color = m_default_color;
    colorChanged(m_default_color);
}

void ColorButton::update()
{
    m_change_color.setIcon(makeColorIcon(m_current_color));
    m_change_color.setText((m_current_color == m_default_color
                ? tr("%1 (default)")
                : tr("%1")).arg(m_current_color.name()));

    m_reset->setEnabled(m_current_color != m_default_color);
}

QPixmap ColorButton::makeColorIcon(const QColor &color) const
{
    QStyleOptionButton option;
    option.initFrom(this);
    int icon_size = style()->pixelMetric(QStyle::PM_SmallIconSize, &option);
    QPixmap icon(icon_size, icon_size);
    {
        QRectF rect(0.0, 0.0, icon_size, icon_size);
        QPainter painter(&icon);
        painter.fillRect(rect, m_checker_brush);
        painter.fillRect(rect, QBrush(color));
    }
    return icon;
}
