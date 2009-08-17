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

#include "test.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>

#include <Plasma/Containment>

ContextTest::ContextTest(QObject *parent, const QVariantList &args)
    : Plasma::ContextAction(parent, args)
{
}

void ContextTest::contextEvent(QGraphicsSceneMouseEvent *event)
{
    kDebug() << "test!!!!!!!!!!!!!!!!!!!!!!!" << event->pos();
    kDebug() << event->buttons() << event->modifiers();

    Plasma::Containment *c = qobject_cast<Plasma::Containment*>(parent());
    if (c) {
        kDebug() << c->name();
    } else {
        kDebug() << "fail";
        return;
    }

    KMenu desktopMenu;
    desktopMenu.addAction(c->action("configure"));
    desktopMenu.exec(event->screenPos());

}

void ContextTest::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    kDebug() << "test!!!!!!!!!!!!!11111111!!";
    kDebug() << event->orientation() << event->delta();
    kDebug() << event->buttons() << event->modifiers();
}


#include "test.moc"
