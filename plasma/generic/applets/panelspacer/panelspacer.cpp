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
#include <QAction>

#include <Plasma/PaintUtils>
#include <Plasma/Containment>
#include <Plasma/Theme>

K_EXPORT_PLASMA_APPLET(panelspacer_internal, PanelSpacer)

PanelSpacer::PanelSpacer(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_configurationMode(false),
      m_fixedSize(false)
{
    setAspectRatioMode(Plasma::IgnoreAspectRatio);
    setHasConfigurationInterface(false);
    QAction *toggleFixed = new QAction(i18n("Set Flexible Size"), this);
    toggleFixed->setCheckable(true);
    m_actions.append(toggleFixed);
    addAction(QLatin1String("toggle fixed"), toggleFixed);
    connect(toggleFixed, SIGNAL(toggled(bool)), this, SLOT(toggleFixed(bool)));
    setCacheMode(DeviceCoordinateCache);
}

PanelSpacer::~PanelSpacer()
{
}

QList<QAction *> PanelSpacer::contextualActions()
{
    return m_actions;
}

void PanelSpacer::toggleFixed(bool flexible)
{
    m_fixedSize = !flexible;

    if (!m_fixedSize) {
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        setMinimumSize(0,0);
    } else if (formFactor() == Plasma::Horizontal) {
        setMaximumWidth(size().width());
        setMinimumWidth(size().width());
    } else if (formFactor() == Plasma::Vertical) {
        setMaximumHeight(size().height());
        setMinimumHeight(size().height());
    }

    config().writeEntry("FixedSize", m_fixedSize);
    emit configNeedsSaving();
}

void PanelSpacer::init()
{
    if (containment()) {
        connect(containment(), SIGNAL(toolBoxVisibilityChanged(bool)), this, SLOT(updateConfigurationMode(bool)));
    }

    configChanged();
}

void PanelSpacer::configChanged()
{
    m_fixedSize = config().readEntry("FixedSize", false);
    toggleFixed(!m_fixedSize);
    QAction *fixedAction = action(QLatin1String("toggle fixed"));
    if (fixedAction) {
        fixedAction->setChecked(!m_fixedSize);
    }
}

void PanelSpacer::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Horizontal) {
            setMinimumWidth(minimumHeight());
            setMaximumWidth(maximumHeight());
            setMaximumHeight(QWIDGETSIZE_MAX);
            setMinimumHeight(0);
        } else if (formFactor() == Plasma::Vertical) {
            setMinimumHeight(minimumWidth());
            setMaximumHeight(maximumWidth());
            setMaximumWidth(QWIDGETSIZE_MAX);
            setMinimumWidth(0);
        }
    }

    if (constraints & Plasma::StartupCompletedConstraint) {
        if (!m_fixedSize) {
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            setMinimumSize(0,0);
        } else if (formFactor() == Plasma::Horizontal) {
            setMaximumWidth(size().width());
            setMinimumWidth(size().width());
        } else if (formFactor() == Plasma::Vertical) {
            setMaximumHeight(size().height());
            setMinimumHeight(size().height());
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_fixedSize = ((formFactor() == Plasma::Horizontal && maximumWidth() == minimumWidth()) || (formFactor() == Plasma::Vertical && maximumHeight() == minimumHeight()));
        config().writeEntry("FixedSize", m_fixedSize);

        QAction *fixedAction = action(QLatin1String("toggle fixed"));
        if (fixedAction) {
            fixedAction->setChecked(!m_fixedSize);
        }
    }
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
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

    painter->setRenderHint(QPainter::Antialiasing);
    c = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
    c.setAlphaF(0.7);

    const int margin = 8;

    QPainterPath path;
    if (formFactor() == Plasma::Horizontal) {
        path = Plasma::PaintUtils::roundedRectangle(contentsRect.adjusted(1, 1, -contentsRect.width()+margin-1, -2), 4);
        painter->fillPath(path, c);
        path = Plasma::PaintUtils::roundedRectangle(contentsRect.adjusted(contentsRect.width()-margin, 1, -2, -2), 4);
    } else if (formFactor() == Plasma::Vertical) {
        path = Plasma::PaintUtils::roundedRectangle(contentsRect.adjusted(1, 1, -2, -contentsRect.height()+margin-1), 4);
        painter->fillPath(path, c);
        path = Plasma::PaintUtils::roundedRectangle(contentsRect.adjusted(1, contentsRect.height()-margin, -2, -2), 4);
    }
    painter->fillPath(path, c);
}

void PanelSpacer::updateConfigurationMode(bool config)
{
    if (containment() && containment()->immutability() != Plasma::Mutable) {
        config = false;
    }

    if (config != m_configurationMode) {
        m_configurationMode = config;
        update();
    }
}


#include "panelspacer.moc"

