/***************************************************************************
 *   plasmoidtask.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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

#include "plasmoidtask.h"
#include <fixx11h.h>

#include <KIcon>

#include <plasma/applet.h>
#include <plasma/plasma.h>


namespace SystemTray
{

class PlasmoidTask::Private
{
public:
    Private(QString name, int appletId, PlasmoidTask *q, Plasma::Applet *parentApplet)
        : q(q),
          name(name),
          id(appletId),
          typeId(name),
          applet(0),
          host(parentApplet),
          takenByParent(false)
    {
        if (!name.isEmpty()) {
            setupApplet();
        }
    }

    void setupApplet();

    PlasmoidTask *q;
    QString name;
    int id;
    QString typeId;
    QIcon icon;
    Plasma::Applet *applet;
    Plasma::Applet *host;
    bool takenByParent;
};


PlasmoidTask::PlasmoidTask(QString appletname, int id, QObject *parent, Plasma::Applet *host)
    : Task(parent),
      d(new Private(appletname, id, this, host))
{
}


PlasmoidTask::~PlasmoidTask()
{
    emit taskDeleted(d->typeId);
    delete d;
}


bool PlasmoidTask::isEmbeddable() const
{
    return d->applet != 0 && !d->takenByParent;
}

bool PlasmoidTask::isValid() const
{
    return !d->name.isEmpty();
}

QString PlasmoidTask::name() const
{
    return d->name;
}


QString PlasmoidTask::typeId() const
{
    return d->typeId;
}


QIcon PlasmoidTask::icon() const
{
    return d->icon;
}

Plasma::Applet *PlasmoidTask::host() const
{
    return d->host;
}

QGraphicsWidget* PlasmoidTask::createWidget(Plasma::Applet *host)
{
    if (host != d->host || !d->applet) {
        return 0;
    }

    d->takenByParent = true;
    d->applet->setParent(host);
    d->applet->setParentItem(host);
    d->applet->init();
    d->applet->updateConstraints(Plasma::StartupCompletedConstraint);
    d->applet->flushPendingConstraintsEvents();
    d->applet->updateConstraints(Plasma::AllConstraints);
    d->applet->flushPendingConstraintsEvents();

    connect(d->applet, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(newAppletStatus(Plasma::ItemStatus)));

    return static_cast<QGraphicsWidget*>(d->applet);
}

void PlasmoidTask::forwardConstraintsEvent(Plasma::Constraints constraints)
{
    if (d->applet) {
        d->applet->updateConstraints(constraints);
        d->applet->flushPendingConstraintsEvents();
    }
}

void PlasmoidTask::Private::setupApplet()
{
    applet = Plasma::Applet::load(name, id);

    if (!applet) {
        kDebug() << "Could not load applet" << name;
        name.clear();
        return;
    }

    icon = KIcon(applet->icon());

    //applet->setParent(q);
    applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    connect(applet, SIGNAL(destroyed(QObject*)), q, SLOT(appletDestroyed(QObject*)));
    applet->setBackgroundHints(Plasma::Applet::NoBackground);


    // TODO: We'll need the preferred item size here
    // The applet does need a size, otherwise it won't show up correctly.
    applet->setMinimumSize(22, 22);
    applet->setMaximumSize(22, 22);
    kDebug() << applet->name() << " Applet loaded";
}

void PlasmoidTask::appletDestroyed(QObject *object)
{
    if (object == d->applet) {
        emit taskDeleted(d->typeId);
    }
}

void PlasmoidTask::newAppletStatus(Plasma::ItemStatus status)
{
    switch (status) {
    case Plasma::PassiveStatus:
       setStatus(Task::Passive);
       break;

    case Plasma::ActiveStatus:
       setStatus(Task::Active);
       break;

    case Plasma::NeedsAttentionStatus:
        setStatus(Task::NeedsAttention);
        break;

    default:
    case Plasma::UnknownStatus:
        setStatus(Task::UnknownStatus);
    }

    emit changed(this);
}

}

#include "plasmoidtask.moc"
