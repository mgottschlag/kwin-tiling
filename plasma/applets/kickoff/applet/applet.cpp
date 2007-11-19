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
#include <plasma/layouts/boxlayout.h>
#include <plasma/widgets/icon.h>
#include <plasma/containment.h>

// Local
#include "ui/launcher.h"

LauncherApplet::LauncherApplet(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent,args),
      m_launcher(0)
{
//    setDrawStandardBackground(true);
    Plasma::HBoxLayout *layout = new Plasma::HBoxLayout(this);
    layout->setMargin(0);
    Plasma::Icon *icon = new Plasma::Icon(KIcon("start-here"), QString(), this);
    icon->setFlag(ItemIsMovable, false);
    connect(icon, SIGNAL(pressed(bool, QGraphicsSceneMouseEvent*)), this, SLOT(toggleMenu(bool,QGraphicsSceneMouseEvent*)));
}

LauncherApplet::~LauncherApplet()
{
    delete m_launcher;
}

QSizeF LauncherApplet::contentSizeHint() const
{
    return QSizeF(48,48);
}

Qt::Orientations LauncherApplet::expandingDirections() const
{
    return 0;
}

void LauncherApplet::toggleMenu(bool pressed, QGraphicsSceneMouseEvent *event)
{
    if (!pressed) {
        return;
    }

    //qDebug() << "Launcher button clicked";
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
        if (containment()) {
            containment()->emitLaunchActivated();
        }
    }

    m_launcher->setVisible(!m_launcher->isVisible());
}

#include "applet.moc"
