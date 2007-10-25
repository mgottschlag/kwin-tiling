/*  
    Copyright 2007 Robert Knight <robertknight@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

// Own
#include "applet/applet.h"

// Qt
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QtDebug>

// KDE
#include <KIcon>

// Plasma
#include <plasma/widgets/boxlayout.h>
#include <plasma/widgets/icon.h>

// Local
#include "ui/launcher.h"

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      m_launcher(0)
{
//    setDrawStandardBackground(true);
    m_icon = KIcon("start-here");
    Plasma::HBoxLayout *layout = new Plasma::HBoxLayout(this);
    layout->setMargin(0);
    Plasma::Icon *icon = new Plasma::Icon(m_icon, QString(), this);
    icon->setFlag(ItemIsMovable, false);
    icon->installSceneEventFilter(this);
}

LauncherApplet::~LauncherApplet()
{
    delete m_launcher;
}

void LauncherApplet::paintInterface(QPainter *painter, const QStyleOptionGraphicsItem *option, const QRect& contentsRect)
{
    /*m_icon.paint(painter,contentsRect.left(),contentsRect.top(),contentsRect.width(),
                         contentsRect.height());*/
}

QSizeF LauncherApplet::contentSizeHint() const
{
    return QSizeF(48,48);
}

Qt::Orientations LauncherApplet::expandingDirections() const
{
    return 0;
}

bool LauncherApplet::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    kDebug() << event->type();
    if (event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
        mousePressEvent(mouseEvent);
    }

    return false;
}

void LauncherApplet::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "Launcher button clicked";
    if (!m_launcher) {
        m_launcher = new Kickoff::Launcher(0);
        m_launcher->setWindowFlags(m_launcher->windowFlags()|Qt::WindowStaysOnTopHint|Qt::Popup);
        m_launcher->setAutoHide(true);
        m_launcher->adjustSize();
    }

    // try to position the launcher just above the applet with the left
    // edge of the applet and the left edge of the launcher aligned
    if (!m_launcher->isVisible()) {
        QPointF scenePos = mapToScene(boundingRect().topLeft());
        QWidget *viewWidget = event->widget() ? event->widget()->parentWidget() : 0;
        QGraphicsView *view = qobject_cast<QGraphicsView*>(viewWidget);
        if (view) {
            QPoint viewPos = view->mapFromScene(scenePos);
            QPoint globalPos = view->mapToGlobal(viewPos);
            globalPos.ry() -= m_launcher->height(); 
            m_launcher->move(globalPos);
        }
    }

    m_launcher->setVisible(!m_launcher->isVisible());
}

#include "applet.moc"
