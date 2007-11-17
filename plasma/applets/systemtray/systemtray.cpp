/***************************************************************************
 *   systemtray.cpp                                                        *
 *                                                                         *
 *   Copyright (C) 2007 Alexander Rodin <rodin.alexander@gmail.com>        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "systemtray.h"
#include "systemtraywidget.h"

// KDE
#include <KWindowSystem>
#include <QGraphicsView>

SystemTray::SystemTray(QObject *parent, const QVariantList &arguments)
    : Plasma::Applet(parent, arguments),
      m_systemTrayWidget(new SystemTrayWidget(0,
                  Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint))
{
    connect(m_systemTrayWidget, SIGNAL(sizeChanged()), SLOT(updateLayout()));
    m_systemTrayWidget->show();
    KWindowSystem::setState(m_systemTrayWidget->winId(),
            NET::SkipTaskbar | NET::SkipPager | NET::KeepBelow);
}

SystemTray::~SystemTray()
{
    delete m_systemTrayWidget;
}

QSizeF SystemTray::contentSizeHint() const
{
    return QSizeF(m_systemTrayWidget->size());
}

Qt::Orientations SystemTray::expandingDirections() const
{
    // simplify layouting by giving the system tray a fixed
    // size for now
    return 0;
}

QVariant SystemTray::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged) {
        //figure out where this applet really is.
        QPoint realPos(0, 0); //start with a default that's at least visible
        QGraphicsScene *s=scene();
        if (s) {
            QList<QGraphicsView *> viewlist = s->views();
            foreach (QGraphicsView *v, viewlist) {
                QRectF r=v->sceneRect();
                //now find out if this applet is actually on here
                //I consider it "on here" if our pos is within the rect
                //but there may be other valid ways to do this
                if (r.contains(scenePos())) {
                    kDebug() << "using view" << v;
                    QPoint p=v->mapFromScene(scenePos());
                    realPos = v->mapToGlobal(p);
                    break; //no, I don't care if other views show it
                }
            }
        }
        m_systemTrayWidget->move(realPos);
    }
    return Plasma::Applet::itemChange(change, value);
}

#include "systemtray.moc"
