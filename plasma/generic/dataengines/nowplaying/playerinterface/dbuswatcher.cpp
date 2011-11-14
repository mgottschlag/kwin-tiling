/*
 * Copyright 2008  Alex Merry <alex.merry@kdemail.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "dbuswatcher.h"

#include "player.h"
#include "playerfactory.h"

#include <QtDBus>
#include <KDebug>

DBusWatcher::DBusWatcher(QObject* parent)
    : QObject(parent),
      m_bus(0)
{
    setObjectName( QLatin1String("DBusWatcher" ));
    QDBusConnection sessionCon = QDBusConnection::sessionBus();
    if (sessionCon.isConnected()) {
        m_bus = sessionCon.interface();
        connect(m_bus, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this,  SLOT(serviceChange(QString,QString,QString)));
    } else {
        kWarning() << "Couldn't connect to session bus";
    }
}

QList<Player::Ptr> DBusWatcher::players()
{
    return m_players.values();
}

void DBusWatcher::addFactory(DBusPlayerFactory* factory)
{
    m_factories.append(factory);

    QDBusReply<QStringList> reply = m_bus->registeredServiceNames();
    if (reply.isValid()) {
        QStringList services = reply.value();
        foreach (const QString &name, services) {
            if (factory->matches(name)) {
                QDBusReply<QString> ownerReply = m_bus->serviceOwner(name);
                if (m_players.contains(name)) {
                    kWarning() << "Two factories tried to claim the same service:" << name;
                } else if (ownerReply.isValid()) {
                    QString owner = ownerReply.value();
                    kDebug() << "Service" << name << "has owner" << owner;
                    if (!m_owners.contains(owner)) {
                        QVariantList args;
                        args << QVariant(name);
                        Player::Ptr player = factory->create(args);
                        if (!player.isNull()) {
                            m_players[name] = player;
                            m_owners << owner;
                            emit(newPlayer(player));
                        } else {
                            kDebug() << "Failed to get player" << name;
                        }
                    }
                }
            }
        }
    } else {
        kWarning() << "Couldn't get service names:" << reply.error().message();
    }
}

void DBusWatcher::serviceChange(const QString& name,
                                const QString& oldOwner,
                                const QString& newOwner)
{
    if (oldOwner.isEmpty() && !newOwner.isEmpty()) {
        kDebug() << "Service" << name << "has owner" << newOwner;
        if (m_owners.contains(newOwner)) {
            kDebug() << "Owner" << newOwner << "is already being dealt with";
            // something is already dealing with this media player
            return;
        }
        // got a new service
        foreach (DBusPlayerFactory* factory, m_factories) {
            if (factory->matches(name)) {
                if (m_players.contains(name)) {
                    kWarning() << "Two factories tried to claim the same service:" << name;
                } else {
                    QVariantList args;
                    args << QVariant(name);
                    Player::Ptr player = factory->create(args);
                    if (!player.isNull()) {
                        m_players[name] = player;
                        m_owners << newOwner;
                        emit(newPlayer(player));
                    } else {
                        kDebug() << "Failed to get player" << name << "; trying other factories";
                    }
                }
            }
        }
    } else if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
        m_owners.removeAll(oldOwner);
        // an old service disappeared
        if (m_players.contains(name)) {
            Player::Ptr player = m_players[name];
            m_players.remove(name);
            emit(playerDisappeared(player));
        }
    } else if (!oldOwner.isEmpty() && !newOwner.isEmpty()) {
        if (m_owners.removeAll(oldOwner) > 0) {
            kDebug() << "Service" << name << "had owner" << oldOwner << "and is now owned by" << newOwner;
            m_owners << newOwner;
        }
    }
}
