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

#include "zoom.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>

#include <Plasma/Containment>
#include <Plasma/Corona>

Q_DECLARE_METATYPE(QPointer<Plasma::Containment>)

Zoom::Zoom(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

void Zoom::contextEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            contextEvent(static_cast<QGraphicsSceneMouseEvent*>(event));
            break;
        case QEvent::GraphicsSceneWheel:
            wheelEvent(static_cast<QGraphicsSceneWheelEvent*>(event));
            break;
        default:
            break;
    }
}

void Zoom::contextEvent(QGraphicsSceneMouseEvent *event)
{
    Plasma::Containment *myCtmt = containment();
    if (!myCtmt) {
        return;
    }
    Plasma::Corona *c = myCtmt->corona();
    if (!c) {
        return;
    }

    KMenu desktopMenu;

    desktopMenu.addTitle(i18n("Activities"));

    QList<Plasma::Containment*> containments = c->containments();
    foreach (Plasma::Containment *ctmt, containments) {
        if (ctmt->containmentType() == Plasma::Containment::PanelContainment ||
                ctmt->containmentType() == Plasma::Containment::CustomPanelContainment ||
                c->offscreenWidgets().contains(ctmt)) {
            continue;
        }

        QString name = ctmt->activity();
        if (name.isEmpty()) {
            name = ctmt->name();
        }
        QAction *action = desktopMenu.addAction(name);
        action->setData(QVariant::fromValue<QPointer<Plasma::Containment> >(QPointer<Plasma::Containment>(ctmt)));

        //WARNING this assumes the plugin will only ever be set on activities, not panels!
        if (ctmt==myCtmt) {
            action->setEnabled(false);
        }
    }

    connect(&desktopMenu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));
    desktopMenu.exec(event->screenPos());
}

void Zoom::switchTo(QAction *action)
{
    QPointer<Plasma::Containment> ctmt = action->data().value<QPointer<Plasma::Containment> >();
    if (!ctmt) {
        return;
    }
    Plasma::Containment *myCtmt = containment();
    if (!myCtmt) {
        return;
    }

    ctmt->setScreen(myCtmt->screen(), myCtmt->desktop());
}

void Zoom::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    Plasma::Containment *myCtmt = containment();
    if (!myCtmt) {
        return;
    }
    Plasma::Corona *c = myCtmt->corona();
    if (!c) {
        return;
    }

    QList<Plasma::Containment*> containments = c->containments();
    int start = containments.indexOf(myCtmt);
    int step = (event->delta() < 0) ? 1 : -1;
    int i = (start + step + containments.size()) % containments.size();

    //FIXME we *really* need a proper way to cycle through activities
    while (i != start) {
        Plasma::Containment *ctmt = containments.at(i);
        if (ctmt->containmentType() == Plasma::Containment::PanelContainment ||
                ctmt->containmentType() == Plasma::Containment::CustomPanelContainment ||
                c->offscreenWidgets().contains(ctmt)) {
            //keep looking
            i = (i + step + containments.size()) % containments.size();
        } else {
            break;
        }
    }

    Plasma::Containment *ctmt = containments.at(i);
    if (ctmt && ctmt != myCtmt) {
        ctmt->setScreen(myCtmt->screen(), myCtmt->desktop());
    }
}


#include "zoom.moc"
