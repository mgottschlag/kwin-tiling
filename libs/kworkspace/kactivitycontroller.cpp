/*
 * Copyright (c) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kactivitycontroller.h"
#include "kactivitycontrollerdbus_p.h"
#include "activitymanager_interface.h"

class KActivityController::Private {
public:
    org::kde::ActivityManager * manager;
};

KActivityController::KActivityController(QObject * parent)
    : KActivityConsumer(parent), d(new Private())
{
    d->manager = new org::kde::ActivityManager(
        "org.kde.ActivityManager",
        "/ActivityManager",
        QDBusConnection::sessionBus()
    );

    new KActivityControllerDbus(this, d->manager);
}

KActivityController::~KActivityController()
{
    delete d;
}

void KActivityController::setActivityName(const QString & id, const QString & name)
{
    d->manager->SetActivityName(id, name);
}

bool KActivityController::setCurrentActivity(const QString & id)
{
    return d->manager->SetCurrentActivity(id);
}

QString KActivityController::addActivity(const QString & name)
{
    return d->manager->AddActivity(name);
}

void KActivityController::removeActivity(const QString & id)
{
    d->manager->RemoveActivity(id);
}

