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
#include "kactivityconsumer_p.h"

#include <KDebug>
class KActivityController::Private {
public:
    Private()
    {
        if (s_dbusController) {
            dbusController = s_dbusController;
        } else {
            dbusController = QSharedPointer<KActivityControllerDbus>(new KActivityControllerDbus(manager(), 0));
            s_dbusController = dbusController;
        }
    }

    QSharedPointer<KActivityControllerDbus> dbusController;

    org::kde::ActivityManager *manager()
    {
        return KActivityConsumerPrivate::manager();
    }

    static QWeakPointer<KActivityControllerDbus> s_dbusController;
};

QWeakPointer<KActivityControllerDbus> KActivityController::Private::s_dbusController;

KActivityController::KActivityController(QObject * parent)
    : KActivityConsumer(parent), d(new Private())
{
    connect(d->dbusController.data(), SIGNAL(activityAdded(QString)), this, SIGNAL(activityAdded(QString)));
    connect(d->dbusController.data(), SIGNAL(activityRemoved(QString)), this, SIGNAL(activityRemoved(QString)));
    connect(d->dbusController.data(), SIGNAL(resourceWindowRegistered(QString,uint,QString)), this, SIGNAL(resourceWindowRegistered(QString,uint,QString)));
    connect(d->dbusController.data(), SIGNAL(resourceWindowUnregistered(uint,QString)), this, SIGNAL(resourceWindowUnregistered(uint,QString)));
}

KActivityController::~KActivityController()
{
    delete d;
}

void KActivityController::setActivityName(const QString & id, const QString & name)
{
    d->manager()->SetActivityName(id, name);
}

void KActivityController::setActivityIcon(const QString & id, const QString & icon)
{
    d->manager()->SetActivityIcon(id, icon);
}

bool KActivityController::setCurrentActivity(const QString & id)
{
    return d->manager()->SetCurrentActivity(id);
}

QString KActivityController::addActivity(const QString & name)
{
    return d->manager()->AddActivity(name);
}

void KActivityController::removeActivity(const QString & id)
{
    d->manager()->RemoveActivity(id);
}

