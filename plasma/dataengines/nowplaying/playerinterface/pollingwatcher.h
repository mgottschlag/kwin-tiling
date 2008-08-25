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
#ifndef POLLINGWATCHER_H
#define POLLINGWATCHER_H


#include <QObject>
#include <QSet>
#include <QString>

#include "player.h"

class QTimer;

class PollingPlayerFactory;

class PollingWatcher : public QObject
{
    Q_OBJECT

public:
    PollingWatcher(QObject* parent = 0);

    QList<Player::Ptr> players();

    /**
     * Adds a service to watch for.
     *
     * @param factory the factory for the the player
     */
    void addFactory(PollingPlayerFactory* factory);

Q_SIGNALS:
    /**
     * A new player is available
     */
    void newPlayer(Player::Ptr player);
    /**
     * A previously existing player is no longer available
     *
     * @param player the now-invalid player
     */
    void playerDisappeared(Player::Ptr player);

private Q_SLOTS:
    void checkPlayers();

private:
    // the factories we are waiting for player to appear on
    QSet<PollingPlayerFactory*> m_polledFactories;
    // the factories we have a player for
    QSet<PollingPlayerFactory*> m_usedFactories;
    QSet<Player::Ptr> m_players;
    QTimer* m_timer;
};

#endif // POLLINGWATCHER_H
