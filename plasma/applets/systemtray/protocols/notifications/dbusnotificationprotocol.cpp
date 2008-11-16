/***************************************************************************
 *   fdoprotocol.cpp                                                       *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "dbusnotification.h"
#include "dbusnotificationprotocol.h"

#include <KConfigGroup>
#include <KIcon>

#include <plasma/dataenginemanager.h>
#include <plasma/service.h>


namespace SystemTray
{
namespace DBus
{


class NotificationProtocol::Private
{
public:
    Private()
        : engine(0)
    {
    }

    Plasma::DataEngine *engine;
    QHash<QString, Notification*> notifications;
};


static const char *engineName = "notifications";


NotificationProtocol::NotificationProtocol(QObject *parent)
    : SystemTray::NotificationProtocol(parent),
      d(new NotificationProtocol::Private)
{
}


NotificationProtocol::~NotificationProtocol()
{
    if (d->engine) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
    }

    delete d;
}


void NotificationProtocol::init()
{
    d->engine = Plasma::DataEngineManager::self()->loadEngine(engineName);

    if (!d->engine->isValid()) {
        d->engine = 0;
        return;
    }

    connect(d->engine, SIGNAL(sourceAdded(const QString&)),
            this, SLOT(prepareNotification(const QString&)));
    connect(d->engine, SIGNAL(sourceRemoved(const QString&)),
            this, SLOT(removeNotification(const QString&)));
}


void NotificationProtocol::prepareNotification(const QString &source)
{
    d->engine->connectSource(source, this);
}


void NotificationProtocol::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    bool isNew = !d->notifications.contains(source);

    if (isNew) {
        d->notifications[source] = new Notification(source, this);
        connect(d->notifications[source], SIGNAL(notificationDeleted(const QString&)),
                this, SLOT(removeNotification(const QString&)));
        connect(d->notifications[source], SIGNAL(actionTriggered(const QString&, const QString&)),
                this, SLOT(relayAction(const QString&, const QString&)));
    }

    Notification* notification = d->notifications[source];
    notification->setApplicationName(data.value("appName").toString());
    notification->setApplicationIcon(KIcon(data.value("appIcon").toString()));
    notification->setEventId(data.value("eventId").toString());
    notification->setSummary(data.value("summary").toString());
    notification->setMessage(data.value("body").toString());
    notification->setTimeout(data.value("expireTimeout").toInt());

    QStringList codedActions = data.value("actions").toStringList();
    if (codedActions.count() % 2 != 0) {
        kDebug() << "Invalid actions" << codedActions << "from" << notification->applicationName();
        codedActions.clear();
    }

    QHash<QString, QString> actions;
    QStringList actionOrder;

    while (!codedActions.isEmpty()) {
        QString actionId = codedActions.takeFirst();
        QString actionName = codedActions.takeFirst();
        actions.insert(actionId, actionName);
        actionOrder.append(actionId);
    }

    notification->setActions(actions);
    notification->setActionOrder(actionOrder);

    if (isNew) {
        emit notificationCreated(notification);
    } else {
        emit notification->changed(notification);
    }
}


void NotificationProtocol::relayAction(const QString &source, const QString &actionId)
{
    Plasma::Service *service = d->engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("invokeAction");

    if (op.isValid()) {
        op.writeEntry("actionId", actionId);
        service->startOperationCall(op);
    } else {
        kDebug() << "invalid operation";
    }
}


void NotificationProtocol::removeNotification(const QString &source)
{
    if (d->notifications.contains(source)) {
        d->notifications.take(source)->deleteLater();
    }
}


}
}


#include "dbusnotificationprotocol.moc"
