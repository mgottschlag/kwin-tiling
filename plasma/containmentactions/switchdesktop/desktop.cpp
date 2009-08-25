/*
 *   Copyright 2009 by Chani Armitage <chani@kde.org>
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

#include "desktop.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>
#include <KWindowSystem>

SwitchDesktop::SwitchDesktop(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

void SwitchDesktop::contextEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            contextEvent(dynamic_cast<QGraphicsSceneMouseEvent*>(event));
            break;
        case QEvent::GraphicsSceneWheel:
            wheelEvent(dynamic_cast<QGraphicsSceneWheelEvent*>(event));
            break;
        default:
            break;
    }
}

void SwitchDesktop::contextEvent(QGraphicsSceneMouseEvent *event)
{
    int numDesktops = KWindowSystem::numberOfDesktops();
    int currentDesktop = KWindowSystem::currentDesktop();
    KMenu desktopMenu;

    desktopMenu.addTitle(i18n("Virtual Desktops"));

    for (int i=1; i<=numDesktops; ++i) {
        QString name = KWindowSystem::desktopName(i);
        QAction *action = desktopMenu.addAction(QString("%1: %2").arg(i).arg(name));
        action->setData(i);
        if (i==currentDesktop) {
            action->setEnabled(false);
        }
    }

    connect(&desktopMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));
    desktopMenu.exec(event->screenPos());
}

void SwitchDesktop::switchTo(QAction *action)
{
    int desktop = action->data().toInt();
    KWindowSystem::setCurrentDesktop(desktop);
}

void SwitchDesktop::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    kDebug() << event->orientation() << event->delta();
    int numDesktops = KWindowSystem::numberOfDesktops();
    int currentDesktop = KWindowSystem::currentDesktop();

    if (event->delta() < 0) {
        KWindowSystem::setCurrentDesktop(currentDesktop % numDesktops + 1);
    } else {
        KWindowSystem::setCurrentDesktop((numDesktops + currentDesktop - 2) % numDesktops + 1);
    }
}


#include "desktop.moc"
