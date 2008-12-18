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
#include <Plasma/DataEngine>
#include <Plasma/Containment>
#include <Plasma/Frame>
#include <KDebug>
#include <QGraphicsLinearLayout>

namespace SM {

QHash< QString, QList<uint> > Applet::s_configIds;

Applet::Applet(QObject *parent, const QVariantList &args)
   : Plasma::Applet(parent, args),
     m_interval(10000),
     m_preferredItemHeight(42),
     m_titleSpacer(false),
     m_header(0),
     m_engine(0),
     m_ratioOrientation(Qt::Vertical),
     m_orientation(Qt::Vertical),
     m_mode(Desktop),
     m_detail(Low),
     m_minimumWidth(DEFAULT_MINIMUM_WIDTH),
     m_mainLayout(0),
     m_configId(0)
{
    if (args.count() > 0) {
        if (args[0].toString() == "SM") {
            m_mode = Monitor;
        }
    }
    QString name = pluginName();

    while (s_configIds[name].contains(m_configId)) {
        ++m_configId;
    }
    s_configIds[name] << m_configId;
}

Applet::~Applet()
{
    s_configIds[pluginName()].removeAll(m_configId);
    deleteMeters();
}

KConfigGroup Applet::persistentConfig() const
{
    KConfigGroup cg = globalConfig();
    return KConfigGroup(cg.config(), QString("General_%1").arg(m_configId));
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
                m_ratioOrientation = m_orientation;
                connectToEngine();
            }
        }
    } else if (constraints & Plasma::SizeConstraint) {
        Detail detail;
        if (size().width() > 250 && size().height() / m_items.count() > 150) {
            detail = High;
        } else {
            detail = Low;
        }
        if (m_detail != detail && m_mode != Monitor) {
            m_detail = detail;
            setDetail(m_detail);
        }
        if (m_keepRatio.count() > 0) {
            foreach (QGraphicsWidget* item, m_keepRatio) {
                QSizeF size = item->size();
                if (size == QSizeF(0, 0)) {
                    continue;
                }
                qreal ratio = item->preferredSize().height() / item->preferredSize().width();
                if (m_ratioOrientation == Qt::Vertical) {
                    size = QSizeF(size.width(), size.width() * ratio);
                } else {
                    size = QSizeF(size.height() * (1.0 / ratio), size.height());
                }
                item->setPreferredSize(size);
                if (m_mode == Panel) {
                    item->setMaximumSize(size);
                    item->setMinimumSize(size);
                }
            }
            for (int i = mainLayout()->count() - 1; i >= 0; --i) {
                QGraphicsLayoutItem* item = mainLayout()->itemAt(i);
                if (item) {
                    QGraphicsLinearLayout* l = dynamic_cast<QGraphicsLinearLayout *>(item);
                    if (l) {
                        l->invalidate();
                    }
                }
            }
        }
    }
}

void Applet::setDetail(Detail detail)
{
    Q_UNUSED(detail);
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
      m_mainLayout->setSpacing(0);
      setLayout(m_mainLayout);
   }
   return m_mainLayout;
}

void Applet::connectToEngine()
{
    deleteMeters();
    disconnectSources();

    mainLayout()->setOrientation(m_orientation);
    if (m_mode != Panel) {
        m_header = new Plasma::Frame(this);
        m_header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        m_header->setText(m_title);
        mainLayout()->addItem(m_header);
    }
    foreach (const QString &item, m_items) {
        if (addMeter(item)) {
            connectSource(item);
        }
    }
    if (m_titleSpacer) {
        mainLayout()->addStretch();
    }
    checkGeometry();
    mainLayout()->activate();
    constraintsEvent(Plasma::SizeConstraint);
    setDetail(m_detail);
}

void Applet::checkGeometry()
{
    if (m_mode != Panel) {
        QSizeF minSize;
        qreal height = 0;

        if (m_header) {
            height = m_header->minimumSize().height();
        }
        m_min.setHeight(height + m_items.count() * m_preferredItemHeight);
        m_min.setWidth(m_minimumWidth);
        if (m_mode != Monitor) {
            m_max = QSizeF();
            m_min += size() - contentsRect().size();
        } else {
            // Reset margins
            setBackgroundHints(NoBackground);
            m_max = QSizeF(QSizeF().width(), minSize.height());
        }
        //kDebug() << minSize << m_preferredItemHeight << height
        //         << m_minimumHeight << metaObject()->className();
        m_pref = m_min;

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
        m_min = QSizeF(16 * x, 16 * y);
        m_max = m_pref = QSizeF(s * x, s * y);
        setAspectRatioMode(Plasma::KeepAspectRatio);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }
    setMinimumSize(m_min);
    setPreferredSize(m_pref);
    setMaximumSize(m_max);
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

void Applet::deleteMeters(QGraphicsLinearLayout* layout)
{
    if (!layout) {
        layout = mainLayout();
        m_meters.clear();
        m_plotters.clear();
        m_keepRatio.clear();
        m_header = 0;
    }
    for (int i = layout->count() - 1; i >= 0; --i) {
        QGraphicsLayoutItem* item = layout->itemAt(i);
        if (item) {
            QGraphicsLinearLayout* l = dynamic_cast<QGraphicsLinearLayout *>(item);
            if (l) {
                deleteMeters(l);
            }
        }
        layout->removeAt(i);
        delete item;
    }
}
/*
QSizeF Applet::sizeHint(Qt::SizeHint which, const QSizeF& constraint) const
{
    QSizeF result;
    Q_UNUSED(constraint)
    switch (which) {
        case Qt::MinimumSize:
            result = m_min;
            break;
        case Qt::MaximumSize:
            result = m_max;
            break;
        case Qt::PreferredSize:
        default:
            result = m_pref;
            break;
    }
    kDebug() << which << result;
    return result;
}
*/
}
