/*
 * Copyright (C) 2007 Menard Alexis <darktears31@gmail.com>
 *
 * This program is free software you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "solidnotifier.h"

#include <KDebug>
#include <KLocale>

#include <plasma/svg.h>
#include <plasma/animator.h>
#include <plasma/corona.h>
#include <plasma/phase.h>


using namespace Plasma;

SolidNotifier::SolidNotifier(QObject *parent, const QStringList &args)
    : Plasma::Applet(parent, args)
{
    setHasConfigurationInterface(false);

    KConfigGroup cg = appletConfig();
    m_pixelSize = cg.readEntry("size", 100);
    m_theme = new Plasma::Svg("widgets/connection-established", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(m_pixelSize, m_pixelSize);
    Plasma::DataEngine* SolidEngine = dataEngine("solidnotifierengine");
    x=450;
    y=300;
	t=new QTimer(this);
	connect(t, SIGNAL(timeout()), this, SLOT(moveMyself()));
    t->start(1000);
    up_down=true;
    constraintsUpdated();
}

QRectF SolidNotifier::boundingRect() const
{
    return m_bounds;
}

void SolidNotifier::constraintsUpdated()
{
    prepareGeometryChange();
    if (formFactor() == Plasma::Planar ||
        formFactor() == Plasma::MediaCenter) {
        QSize s = m_theme->size();
        m_bounds = QRect(x,y, s.width(), s.height());
    }
}

void SolidNotifier::updated(const QString& source, const Plasma::DataEngine::Data &data)
{
    Q_UNUSED(source);
    Q_UNUSED(data);
    update();
}

SolidNotifier::~SolidNotifier()
{
}

void SolidNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);
    QRectF boundRect = boundingRect();
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    m_theme->paint(p, boundRect, "layer1");
}

void SolidNotifier::moveMyself()
{
    if(!up_down)
    {
        Phase::self()->moveItem(this, Phase::SlideIn,QPoint(0,100));
        up_down=true;
    }
    else
    {
        Phase::self()->moveItem(this, Phase::SlideOut,QPoint(0,100));
        up_down=false;
    }
}
#include "solidnotifier.moc"
