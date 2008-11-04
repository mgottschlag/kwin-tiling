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
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLAYERACTIONJOB_H
#define PLAYERACTIONJOB_H

#include <Plasma/ServiceJob>
#include <QTimer>
#include "playerinterface/player.h"

#include <kdebug.h>

class PlayerActionJob : public Plasma::ServiceJob
{
    Q_OBJECT

public:
    PlayerActionJob(Player::Ptr player,
                    const QString& operation,
                    QMap<QString,QVariant>& parameters,
                    QObject* parent = 0)
        : ServiceJob(player->name(), operation, parameters, parent),
          m_player(player)
    {
        if (player) {
            setObjectName("PlayerActionJob: " + player->name() + ": " + operation);
        } else {
            setObjectName("PlayerActionJob: null player: " + operation);
        }
    }

    void start();

private:
    Player::Ptr m_player;
};

#endif // PLAYERACTIONJOB_H
