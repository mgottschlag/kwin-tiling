/***************************************************************************
 *   plasmoidtask.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
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

#include <plasma/applet.h>


namespace SystemTray
{

class PlasmoidTask::Private
{
public:
    Private(QString name, PlasmoidTask *q)
        : q(q),
          name(name),
          typeId(name),
          applet(0)
    {
        if (!name.isEmpty()) {
            setupApplet();
        }
    }

    void setupApplet();

    PlasmoidTask *q;
    QString name;
    QString typeId;
    QIcon icon;
    Plasma::Applet *applet;
};


PlasmoidTask::PlasmoidTask(QString appletname)
    : d(new Private(appletname, this))
{
}


PlasmoidTask::~PlasmoidTask()
{
    emit taskDeleted(d->typeId);
    delete d;
}


bool PlasmoidTask::isEmbeddable() const
{
    return d->applet != 0;
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


QGraphicsWidget* PlasmoidTask::createWidget(Plasma::Applet *host)
{
    Q_UNUSED(host)
    return static_cast<QGraphicsWidget*>(d->applet);
}


void PlasmoidTask::Private::setupApplet()
{
    applet = Plasma::Applet::load(name);

    if (!applet) {
        kDebug() << "Could not load applet" << name;
        name = QString();
        return;
    }

    applet->setParent(q);
    applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    //connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed(QObject*)));
    applet->init();
    applet->setBackgroundHints(Plasma::Applet::NoBackground);

    // TODO: We'll need the preferred item size here
    // The applet does need a size, otherwise it won't show up correctly.
    applet->setMinimumSize(22, 22);
    kDebug() << applet->name() << " Applet loaded";
}

}

#include "plasmoidtask.moc"
