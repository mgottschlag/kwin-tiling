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

#ifndef PLAYERCONTAINER_H
#define PLAYERCONTAINER_H

#include <Plasma/DataContainer>

#include "playerinterface/player.h"

class PlayerControl;

class PlayerContainer : public Plasma::DataContainer
{
    Q_OBJECT

public:
    explicit PlayerContainer(Player::Ptr player, QObject* parent = 0);

    Plasma::Service* service(QObject* parent = 0);

public slots:
    void updateInfo();

private:
    Player::Ptr m_player;
};

#endif // PLAYERCONTAINER_H
