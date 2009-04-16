/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

#include "panelspacer.h"

#include <QPainter>

#include <Plasma/PaintUtils>
#include <Plasma/Containment>
#include <Plasma/Theme>

K_EXPORT_PLASMA_APPLET(panelspacer_internal, PanelSpacer)

PanelSpacer::PanelSpacer(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_configurationMode(false)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setHasConfigurationInterface(false);
}

PanelSpacer::~PanelSpacer()
{
}

void PanelSpacer::init()
{
    if (containment()) {
        connect(containment(), SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
    }
}

void PanelSpacer::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    Q_UNUSED(option)

    if (!m_configurationMode) {
        return;
    }
    painter->setRenderHint(QPainter::Antialiasing);
    QPainterPath p = Plasma::PaintUtils::roundedRectangle(contentsRect.adjusted(1, 1, -2, -2), 4);
    QColor c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.3);

    painter->fillPath(p, c);
}

void PanelSpacer::updateConfigurationMode(bool config)
{
    m_configurationMode = config;
    update();
}


#include "panelspacer.moc"

