/***************************************************************************
 *   dbussystemtrayprotocol.cpp                                            *
 *                                                                         *
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

#include "dbussystemtraytask.h"
#include "dbussystemtrayprotocol.h"

#include <QDBusConnectionInterface>

#include <KIcon>
#include <KDebug>


namespace SystemTray
{

DBusSystemTrayProtocol::DBusSystemTrayProtocol(QObject *parent)
    : Protocol(parent),
      m_dbus(QDBusConnection::sessionBus()),
      m_notificationItemWatcher(0)
{
}

DBusSystemTrayProtocol::~DBusSystemTrayProtocol()
{
    m_dbus.unregisterService(m_serviceName);
}

void DBusSystemTrayProtocol::init()
{
    if (m_dbus.isConnected()) {
        QDBusConnectionInterface *dbusInterface = m_dbus.interface();

        m_serviceName = "org.kde.NotificationHost-" + QString::number(QCoreApplication::applicationPid());
        m_dbus.registerService(m_serviceName);

        //FIXME: understand why registerWatcher/unregisterWatcher doesn't work
        /*connect(dbusInterface, SIGNAL(serviceRegistered(const QString&)),
            this, SLOT(registerWatcher(const QString&)));
        connect(dbusInterface, SIGNAL(serviceUnregistered(const QString&)),
            this, SLOT(unregisterWatcher(const QString&)));*/
        connect(dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(serviceChange(QString,QString,QString)));

        registerWatcher("org.kde.NotificationItemWatcher");
    }
}

void DBusSystemTrayProtocol::newTask(QString service)
{
    if (m_tasks.contains(service)) {
        kDebug() << "Task " << service << "is already in here.";
        return;
    }

    kDebug() << "Registering task with the manager" << service;
    DBusSystemTrayTask *task = new DBusSystemTrayTask(service, this);

    if (!task->isValid()) {
        // we failed to load our task, *sob*
        delete task;
        return;
    }

    m_tasks[service] = task;
//    connect(task, SIGNAL(taskDeleted(QString)), this, SLOT(cleanupTask(QString)));
    emit taskCreated(task);
}


void DBusSystemTrayProtocol::cleanupTask(QString typeId)
{
    kDebug() << "task with typeId" << typeId << "removed";
    DBusSystemTrayTask *task = m_tasks[typeId];
    if (task) {
        emit task->destroyed(m_tasks[typeId]);
        delete task;
        m_tasks.remove(typeId);
    }
}

void DBusSystemTrayProtocol::initRegisteredServices()
{
    QString interface("org.kde.NotificationItemWatcher");
    org::kde::NotificationItemWatcher notificationItemWatcher(interface, "/NotificationItemWatcher",
                                              QDBusConnection::sessionBus());
    if (notificationItemWatcher.isValid()) {
        foreach (const QString &service, notificationItemWatcher.RegisteredServices().value()) {
            newTask(service);
        }
    } else {
        kDebug()<<"Notification item watcher not reachable";
    }
}

void DBusSystemTrayProtocol::serviceChange(const QString& name,
                                           const QString& oldOwner,
                                           const QString& newOwner)
{
    if (name != "org.kde.NotificationItemWatcher") {
        return;
    }

    kDebug()<<"Service "<<name<<"status change, old owner:"<<oldOwner<<"new:"<<newOwner;

    if (newOwner.isEmpty()) {
        //unregistered
        unregisterWatcher(name);
    } else if (oldOwner.isEmpty()) {
        //registered
        registerWatcher(name);
    }
}

void DBusSystemTrayProtocol::registerWatcher(const QString& service)
{
    kDebug()<<"service appeared"<<service;
    if (service == "org.kde.NotificationItemWatcher") {
        QString interface("org.kde.NotificationItemWatcher");
        if (m_notificationItemWatcher) {
            delete m_notificationItemWatcher;
        }

        m_notificationItemWatcher = new org::kde::NotificationItemWatcher(interface, "/NotificationItemWatcher",
                                                                          QDBusConnection::sessionBus());
        if (m_notificationItemWatcher->isValid() &&
            m_notificationItemWatcher->ProtocolVersion() == s_protocolVersion) {
            connect(m_notificationItemWatcher, SIGNAL(ServiceRegistered(const QString&)), this, SLOT(serviceRegistered(const QString &)));
            connect(m_notificationItemWatcher, SIGNAL(ServiceUnregistered(const QString&)), this, SLOT(serviceUnregistered(const QString&)));

            m_notificationItemWatcher->call(QDBus::NoBlock, "RegisterNotificationHost", m_serviceName);

            foreach (const QString &service, m_notificationItemWatcher->RegisteredServices().value()) {
                newTask(service);
            }
        } else {
            delete m_notificationItemWatcher;
            m_notificationItemWatcher = 0;
            kDebug()<<"System tray daemon not reachable";
        }
    }
}

void DBusSystemTrayProtocol::unregisterWatcher(const QString& service)
{
    if (service == "org.kde.NotificationItemWatcher") {
        kDebug()<<"org.kde.NotificationItemWatcher disappeared";

        disconnect(m_notificationItemWatcher, SIGNAL(ServiceRegistered(const QString&)), this, SLOT(serviceRegistered(const QString &)));
        disconnect(m_notificationItemWatcher, SIGNAL(ServiceUnregistered(const QString&)), this, SLOT(serviceUnregistered(const QString&)));

        foreach (DBusSystemTrayTask *task, m_tasks) {
            emit task->destroyed(task);
        }
        m_tasks.clear();

        delete m_notificationItemWatcher;
        m_notificationItemWatcher = 0;
    }
}

void DBusSystemTrayProtocol::serviceRegistered(const QString &service)
{
    kDebug() << "Registering"<<service;
    newTask(service);
}

void DBusSystemTrayProtocol::serviceUnregistered(const QString &service)
{
    cleanupTask(service);
}

}

#include "dbussystemtrayprotocol.moc"
