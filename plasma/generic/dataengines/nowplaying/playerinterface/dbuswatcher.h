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

#ifndef DBUSWATCHER_H
#define DBUSWATCHER_H

#include <QObject>
#include <QHash>
#include <QString>

#include "player.h"

class QDBusConnectionInterface;
class DBusPlayerFactory;

class DBusWatcher : public QObject
{
    Q_OBJECT

public:
    DBusWatcher(QObject* parent = 0);

    QList<Player::Ptr> players();

    /**
     * Adds a service to watch for.
     *
     * @param factory the factory for the player
     */
    void addFactory(DBusPlayerFactory* factory);

Q_SIGNALS:
    /**
     * A new service appeared on D-Bus
     */
    void newPlayer(Player::Ptr player);
    /**
     * A player disappeared from D-Bus
     *
     * @param player the now-invalid player
     */
    void playerDisappeared(Player::Ptr player);

private Q_SLOTS:
    void serviceChange(const QString& name,
                       const QString& oldOwner,
                       const QString& newOwner);

private:
    QStringList m_owners;
    QList<DBusPlayerFactory*> m_factories;
    QHash<QString,Player::Ptr> m_players;
    QDBusConnectionInterface* m_bus;
};

#endif // DBUSWATCHER_H
