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
                if (m_players.contains(name)) {
                    kWarning() << "Already got a player called" << name;
                } else {
                    QVariantList args;
                    args << QVariant(name);
                    Player::Ptr player = factory->create(args);
                    if (!player.isNull()) {
                        m_players[name] = player;
                        emit(newPlayer(player));
                    } else {
                        kWarning() << "Failed to get player" << name;
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
        // got a new service
        foreach (DBusPlayerFactory* factory, m_factories) {
            if (factory->matches(name)) {
                if (m_players.contains(name)) {
                    kWarning() << "Already got a player at" << name;
                } else {
                    QVariantList args;
                    args << QVariant(name);
                    Player::Ptr player = factory->create(args);
                    if (!player.isNull()) {
                        m_players[name] = player;
                        emit(newPlayer(player));
                    } else {
                        kWarning() << "Failed to get player" << name;
                    }
                }
            }
        }
    } else if (!oldOwner.isEmpty() && newOwner.isEmpty()) {
        // an old service disappeared
        if (m_players.contains(name)) {
            Player::Ptr player = m_players[name];
            m_players.remove(name);
            emit(playerDisappeared(player));
        }
    }
}
