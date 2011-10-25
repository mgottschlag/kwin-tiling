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
#include <KDebug>
#include <KIcon>

#include <Plasma/DataEngineManager>
#include <Plasma/Service>
#include <Plasma/ServiceJob>


static const char engineName[] = "notifications";

DBusNotificationProtocol::DBusNotificationProtocol(Manager *parent)
    : Protocol(parent),
      m_manager(parent),
      m_engine(0)
{
}


DBusNotificationProtocol::~DBusNotificationProtocol()
{
    if (m_engine) {
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
    }
}


void DBusNotificationProtocol::init()
{
    m_engine = Plasma::DataEngineManager::self()->loadEngine(engineName);

    if (!m_engine->isValid()) {
        m_engine = 0;
        Plasma::DataEngineManager::self()->unloadEngine(engineName);
        return;
    }

    connect(m_engine, SIGNAL(sourceAdded(QString)),
            this, SLOT(prepareNotification(QString)));
    connect(m_engine, SIGNAL(sourceRemoved(QString)),
            this, SLOT(hideNotification(QString)));
}


void DBusNotificationProtocol::prepareNotification(const QString &source)
{
    if (m_engine) {
        m_engine->connectSource(source, this);
    }
}


void DBusNotificationProtocol::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    bool isNew = !m_notifications.contains(source);

    if (isNew) {
        DBusNotification * notification = new DBusNotification(source, this);
        connect(notification, SIGNAL(unregisterNotification(QString)),
                this, SLOT(unregisterNotification(QString)));
        connect(notification, SIGNAL(notificationDeleted(QString)),
                this, SLOT(notificationDeleted(QString)));
        connect(notification, SIGNAL(actionTriggered(QString,QString)),
                this, SLOT(relayAction(QString,QString)));
        m_notifications[source] = notification;
    }

    DBusNotification* notification = m_notifications[source];
    notification->setApplicationName(data.value("appName").toString());
    notification->setApplicationIcon(KIcon(data.value("appIcon").toString()));
    notification->setSummary(data.value("summary").toString());
    notification->setMessage(data.value("body").toString());
    notification->setTimeout(data.value("expireTimeout").toInt());
    notification->setUrgency(data.value("urgency").toInt());

    if (data.contains("image")) {
        QImage image = qvariant_cast<QImage>(data.value("image"));
        notification->setImage(image);
    }

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


void DBusNotificationProtocol::relayAction(const QString &source, const QString &actionId)
{
    if (!m_engine) {
        return;
    }

    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("invokeAction");

    if (op.isValid()) {
        op.writeEntry("actionId", actionId);
        KJob *job = service->startOperationCall(op);
        connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
    } else {
        delete service;
        kDebug() << "invalid operation";
    }
}

void DBusNotificationProtocol::unregisterNotification(const QString &source)
{
    if (!m_engine) {
        return;
    }

    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("userClosed");
    KJob *job = service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));
}

void DBusNotificationProtocol::hideNotification(const QString &source)
{
    if (m_notifications.contains(source)) {
        m_notifications.value(source)->hide();
    }
}

void DBusNotificationProtocol::removeNotification(const QString &source)
{
    if (m_notifications.contains(source)) {
        m_notifications.take(source)->destroy();
    }
}

void DBusNotificationProtocol::notificationDeleted(const QString &source)
{
    if (!m_engine) {
        return;
    }

    Plasma::Service *service = m_engine->serviceForSource(source);
    KConfigGroup op = service->operationDescription("userClosed");
    KJob *job = service->startOperationCall(op);
    connect(job, SIGNAL(finished(KJob*)), service, SLOT(deleteLater()));

    m_notifications.remove(source);
}


#include "dbusnotificationprotocol.moc"
