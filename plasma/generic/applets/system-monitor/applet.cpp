/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "applet.h"
#include <math.h>
#include <Plasma/DataEngine>
#include <Plasma/Containment>
#include <Plasma/Frame>
#include <Plasma/IconWidget>
#include <Plasma/SignalPlotter>
#include <Plasma/ToolTipManager>
#include <KIcon>
#include <KDebug>
#include <QGraphicsLinearLayout>
#include "plotter.h"

namespace SM {

Applet::Applet(QObject *parent, const QVariantList &args)
   : Plasma::Applet(parent, args),
     m_interval(10000),
     m_preferredItemHeight(42),
     m_titleSpacer(false),
     m_header(0),
     m_engine(0),
     m_orientation(Qt::Vertical),
     m_noSourcesIcon(0),
     m_mode(Desktop),
     m_mainLayout(0),
     m_configSource(0)
{
    if (args.count() > 0 && args[0].toString() == "SM") {
        m_mode = Monitor;
    }

    Plasma::ToolTipManager::self()->registerWidget(this);
}

Applet::~Applet()
{
    deleteMeters();
}

void Applet::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (m_mode == Monitor) {
            setBackgroundHints(NoBackground);
            m_orientation = Qt::Vertical;
        } else {
            SM::Applet::Mode mode = m_mode;
            switch (formFactor()) {
                case Plasma::Planar:
                case Plasma::MediaCenter:
                    mode = Desktop;
                    m_orientation = Qt::Vertical;
                    break;
                case Plasma::Horizontal:
                    mode = Panel;
                    m_orientation = Qt::Horizontal;
                    break;
                case Plasma::Vertical:
                    mode = Panel;
                    m_orientation = Qt::Vertical;
                    break;
            }
            if (mode != m_mode) {
                m_mode = mode;
                connectToEngine();
            }
        }
    } else if (constraints & Plasma::SizeConstraint) {
        checkGeometry();
    }
}

void Applet::setTitle(const QString& title, bool spacer)
{
    m_title = title;
    m_titleSpacer = spacer;
    if (m_header) {
        m_header->setText(m_title);
    }
}

QGraphicsLinearLayout* Applet::mainLayout()
{
   if (!m_mainLayout) {
      m_mainLayout = new QGraphicsLinearLayout(m_orientation);
      m_mainLayout->setContentsMargins(0, 0, 0, 0);
      m_mainLayout->setSpacing(5);
      setLayout(m_mainLayout);
   }
   return m_mainLayout;
}

void Applet::connectToEngine()
{
    deleteMeters();
    // We delete the layout since it seems to be only way to remove stretch set for some applets.
    setLayout(0);
    m_mainLayout = 0;
    disconnectSources();

    mainLayout()->setOrientation(m_orientation);
    if (m_mode != Panel) {
        m_header = new Plasma::Frame(this);
        m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_header->setText(m_title);
        mainLayout()->addItem(m_header);
    }

    if (m_items.isEmpty()){
        displayNoAvailableSources();
        return;
    }

    foreach (const QString &item, m_items) {
        if (addMeter(item)) {
            connectSource(item);
        }
    }

    if (m_titleSpacer) {
        mainLayout()->addStretch();
    }
    mainLayout()->activate();
    constraintsEvent(Plasma::SizeConstraint);
}

void Applet::checkGeometry()
{
    QSizeF min;
    QSizeF pref;
    QSizeF max;

    if (m_mode != Panel) {
        qreal height = 0;
        qreal width = MINIMUM;

        if (m_header) {
            height = m_header->minimumSize().height();
            width = m_header->minimumSize().width();
        }
        min.setHeight(qMax(height + m_items.count() * MINIMUM,
                             mainLayout()->minimumSize().height()));
        min.setWidth(width + MINIMUM);
        pref.setHeight(height + m_items.count() * m_preferredItemHeight);
        pref.setWidth(PREFERRED);
        max = QSizeF();
        if (m_mode != Monitor) {
            min += size() - contentsRect().size();
            pref += size() - contentsRect().size();
        } else {
            // Reset margins
            setBackgroundHints(NoBackground);
        }
        //kDebug() << minSize << m_preferredItemHeight << height
        //         << m_minimumHeight << metaObject()->className();

        setAspectRatioMode(Plasma::IgnoreAspectRatio);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        update();
    } else {
        int x = 1;
        int y = 1;
        QSizeF size = containment()->size();
        qreal s;

        if (m_orientation == Qt::Horizontal) {
            x = m_items.count();
            s = size.height();
        } else {
            y = m_items.count();
            s = size.width();
        }
        min = QSizeF(16 * x, 16 * y);
        max = pref = QSizeF(s * x, s * y);
        setAspectRatioMode(Plasma::KeepAspectRatio);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
    setMinimumSize(min);
    setPreferredSize(pref);
    setMaximumSize(max);
    //kDebug() << m_min << m_pref << m_max << metaObject()->className();
    emit geometryChecked();
}

void Applet::connectSource(const QString& source)
{
   if (m_engine) {
      m_engine->connectSource(source, this, m_interval);
      m_connectedSources << source;
   }
}

void Applet::disconnectSources()
{
   Plasma::DataEngine *engine = dataEngine("soliddevice");
   if (engine) {
      foreach (const QString &source, m_connectedSources) {
         engine->disconnectSource(source, this);
      }
   }
   m_connectedSources.clear();
}

void Applet::deleteMeters()
{
    if (!m_mainLayout) {
        return;
    }
    foreach (QString item, m_plotters.keys()) {
        if (m_plotters.contains(item)) {
            m_plotters.value(item)->deleteLater();
        }
    }
    m_plotters.clear();
    m_toolTips.clear();
    m_header = 0;
}

void Applet::displayNoAvailableSources()
{
    KIcon appletIcon(icon());
    m_noSourcesIcon = new Plasma::IconWidget(appletIcon, "", this);
    mainLayout()->addItem(m_noSourcesIcon);
}

KConfigGroup Applet::config()
{
    if (m_configSource) {
        return m_configSource->config();
    }

    return Plasma::Applet::config();
}

void Applet::save(KConfigGroup &config) const
{
    // work around for saveState being protected
    if (m_mode != Monitor) {
        Plasma::Applet::save(config);
    }
}

void Applet::saveConfig(KConfigGroup &config)
{
    // work around for saveState being protected
    saveState(config);
}

QVariant Applet::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (m_mode == Monitor && change == ItemParentHasChanged) {
        QGraphicsWidget *parent = parentWidget();
        Plasma::Applet *container = 0;
        while (parent) {
            container = qobject_cast<Plasma::Applet *>(parent);

            if (container) {
                break;
            }

            parent = parent->parentWidget();
        }

        if (container && container != containment()) {
            m_configSource = container;
        }
    }

    // We must be able to change position when in monitor even if not mutable
    if (m_mode == Monitor && change == ItemPositionChange) {
        return QGraphicsWidget::itemChange(change, value);
    } else {
        return Plasma::Applet::itemChange(change, value);
    }
}

void Applet::toolTipAboutToShow()
{
    if (mode() == SM::Applet::Panel && !m_toolTips.isEmpty()) {
        QString html = "<table>";
        foreach (const QString& s, m_toolTips.values()) {
            if (!s.isEmpty()) {
                html += s;
            }
        }
        html += "</table>";
        Plasma::ToolTipContent data(title(), html);
        Plasma::ToolTipManager::self()->setContent(this, data);
    }
}

void Applet::appendPlotter(const QString& source, SM::Plotter* plotter)
{
    m_plotters[source] = plotter;
    mainLayout()->addItem(plotter);
}

QHash<QString, SM::Plotter*> Applet::plotters()
{
    return m_plotters;
}

uint Applet::interval()
{
    return m_interval;
}

void Applet::setInterval(uint interval)
{
    m_interval = interval;
}

QString Applet::title()
{
    return m_title;
}

SM::Applet::Mode Applet::mode()
{
    return m_mode;
}

void Applet::setToolTip(const QString &source, const QString &tipContent)
{
    m_toolTips.insert(source, tipContent);
    if (Plasma::ToolTipManager::self()->isVisible(this)) {
        toolTipAboutToShow();
    }
}

void Applet::setEngine(Plasma::DataEngine* engine)
{
    m_engine = engine;
}

Plasma::DataEngine* Applet::engine()
{
    return m_engine;
}

bool Applet::addMeter(const QString&)
{
    return false;
}

QStringList Applet::connectedSources()
{
    return m_connectedSources;
}

}
