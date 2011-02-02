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

#include "playercontainer.h"

#include <KDebug>

#include "playercontrol.h"

PlayerContainer::PlayerContainer(Player::Ptr player, QObject* parent)
    : DataContainer(parent)
    , m_player(player)
{
    Q_ASSERT(m_player);

    setObjectName(m_player->name());

    connect(this, SIGNAL(updateRequested(DataContainer*)),
            this, SLOT(updateInfo()));
}

Plasma::Service* PlayerContainer::service(QObject* parent)
{
    kDebug() << "Creating controller";
    Plasma::Service *controller = new PlayerControl(parent, m_player);
    connect(this, SIGNAL(updateRequested(DataContainer*)),
            controller, SLOT(updateEnabledOperations()));
    return controller;
}

void PlayerContainer::updateInfo()
{
    if (!m_player->isRunning()) {
        kDebug() << objectName() << "isn't running";
        return;
    }

    switch(m_player->state()) {
        case Player::Playing:
            setData("State", "playing");
            break;
        case Player::Paused:
            setData("State", "paused");
            break;
        case Player::Stopped:
            setData("State", "stopped");
            break;
    }

    setData("Artist", m_player->artist());
    setData("Album", m_player->album());
    setData("Title", m_player->title());
    setData("Track number", m_player->trackNumber());
    setData("Comment", m_player->comment());
    setData("Genre", m_player->genre());
    setData("Lyrics", m_player->lyrics());
    setData("Length", m_player->length());
    setData("Position", m_player->position());
    setData("Volume", m_player->volume());
    setData("Artwork", m_player->artwork());

    // propagate changes
    checkForUpdate();
}

#include "playercontainer.moc"
