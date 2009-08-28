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

#include "switch.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>

#include <KDebug>
#include <KMenu>

#include <Plasma/Containment>
#include <Plasma/Corona>

Q_DECLARE_METATYPE(QPointer<Plasma::Containment>)

SwitchActivity::SwitchActivity(QObject *parent, const QVariantList &args)
    : Plasma::ContainmentActions(parent, args)
{
}

void SwitchActivity::contextEvent(QEvent *event)
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

void SwitchActivity::makeMenu(QMenu *menu)
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
        QAction *action = menu->addAction(name);
        action->setData(QVariant::fromValue<QPointer<Plasma::Containment> >(QPointer<Plasma::Containment>(ctmt)));

        //WARNING this assumes the plugin will only ever be set on activities, not panels!
        if (ctmt==myCtmt) {
            action->setEnabled(false);
        }
    }
    connect(menu, SIGNAL(triggered(QAction*)), this, SLOT(switchTo(QAction*)));
}

void SwitchActivity::contextEvent(QGraphicsSceneMouseEvent *event)
{
    KMenu desktopMenu;

    desktopMenu.addTitle(i18n("Activities"));
    makeMenu(&desktopMenu);

    desktopMenu.exec(event->screenPos());
}

QList<QAction*> SwitchActivity::contextualActions()
{
    QList<QAction*> list;
    QMenu *menu = new QMenu();

    makeMenu(menu);
    QAction *action = new QAction(this); //FIXME I hope this doesn't leak
    action->setMenu(menu);
    menu->setTitle(i18n("Activities"));

    list << action;
    return list;
}

void SwitchActivity::switchTo(QAction *action)
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

void SwitchActivity::wheelEvent(QGraphicsSceneWheelEvent *event)
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


#include "switch.moc"
