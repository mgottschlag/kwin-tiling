/*
 *   Copyright 2009 Alain Boyer <alainboyer@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "checkbox.h"

// Qt
#include <QPainter>

// Plasma
#include <Plasma/Theme>

CheckBox::CheckBox(QWidget *parent)
    : QCheckBox(parent),
    m_styleOptionButton(),
    m_initialized(false)
{
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateStyle()));
}

void CheckBox::updateStyle()
{
    initStyleOption(&m_styleOptionButton);
    m_styleOptionButton.palette.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
}

void CheckBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    if (!m_initialized) {
        updateStyle();
    }
    style()->drawControl(QStyle::CE_CheckBox, &m_styleOptionButton, &painter, this);
}

#include "checkbox.moc"
