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

using namespace Plasma;

SolidNotifier::SolidNotifier(QObject *parent, const QStringList &args)
    : Plasma::Applet(parent, args)
{
    setHasConfigurationInterface(false);

    KConfigGroup cg = config();
    m_pixelSize = cg.readEntry("size", 200);
    m_theme = new Plasma::Svg("widgets/connection-established", this);
    m_theme->setContentType(Plasma::Svg::SingleImage);
    m_theme->resize(m_pixelSize, m_pixelSize);
    SolidEngine = dataEngine("solidnotifierengine");
    x=450;
    y=300;
    t=new QTimer(this);
    m_udi="";
    icon = false;
    //connect the timer to MoveDown Animation
    connect(t, SIGNAL(timeout()), this, SLOT(moveDown()));

    //connect to engine when a device is plug
    connect(SolidEngine, SIGNAL(newSource(const QString&)),
            this, SLOT(SourceAdded(const QString&)));
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

SolidNotifier::~SolidNotifier()
{
}

void SolidNotifier::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);
    QRectF boundRect = boundingRect();
    p->setRenderHint(QPainter::SmoothPixmapTransform);
    if(icon)
    {
        k_icon->paint(p,450,300,70,70);
        p->drawText(525,325,device_name);
    }
    //clean the icon from the screen, see it later
}

void SolidNotifier::SourceAdded(const QString& source)
{
    //kDebug()<<data_source.size()<<endl;
    m_udi = source;
    SolidEngine->connectSource(source, this);
}

void SolidNotifier::updated(const QString &source, Plasma::DataEngine::Data data)
{
    kDebug()<<"SolidNotifier:: "<<data[source].toString()<<endl;
    QStringList desktop_files=data["desktoplist"].toStringList();
    //kDebug()<<data["icon"].toString()<<endl;
    QString icon_temp = data["icon"].toString();
    k_icon = new KIcon(icon_temp);
    icon = true;
    device_name=data["text"].toString();
    update();
    moveUp();
}
void SolidNotifier::moveUp()
{
    t->start(5000);
    disconnect(Phase::self(),SIGNAL(movementComplete(QGraphicsItem *)),
               this, SLOT(hideNotifier(QGraphicsItem *)));
    show();
    Phase::self()->moveItem(this, Phase::SlideIn,QPoint(0,50));
}
void SolidNotifier::moveDown()
{
    t->stop();
    Phase::self()->moveItem(this, Phase::SlideOut,QPoint(0,50));
    connect(Phase::self(),SIGNAL(movementComplete(QGraphicsItem *)),
            this, SLOT(hideNotifier(QGraphicsItem *)));
}

void SolidNotifier::hideNotifier(QGraphicsItem * item)
{
    item->hide();
}

#include "solidnotifier.moc"
