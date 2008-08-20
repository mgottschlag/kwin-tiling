/*
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
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

#include "color.h"

#include <QPainter>
#include <KDebug>

Color::Color(QObject *parent, const QVariantList &args)
    : Plasma::Wallpaper(parent, args), m_color(Qt::gray)
{
}

void Color::paint(QPainter *painter, const QRectF& exposedRect)
{
    painter->fillRect(exposedRect, QBrush(m_color));
}

void Color::init(const KConfigGroup &config, const QString &mode)
{
    Q_UNUSED(mode)
    m_color = config.readEntry("wallpapercolor", QColor(Qt::gray));
}

QWidget* Color::createConfigurationInterface(QWidget* parent)
{
    QWidget *widget = new QWidget(parent);
    m_ui.setupUi(widget);

    m_ui.m_color->setColor(m_color);
    connect(m_ui.m_color, SIGNAL(changed(const QColor&)), this, SLOT(setColor(const QColor&)));
    return widget;
}

void Color::setColor(const QColor& color)
{
    m_color = color;
    emit update(boundingRect());
}

void Color::save(KConfigGroup config)
{
    config.writeEntry("wallpapercolor", m_color);
}

#include "color.moc"
