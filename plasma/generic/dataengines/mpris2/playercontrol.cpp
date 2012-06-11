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

#include "playercontrol.h"

#include "playeractionjob.h"
#include "playercontainer.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <KDebug>

#define MPRIS2_PATH "/org/mpris/MediaPlayer2"
#define MPRIS2_IFACE "org.mpris.MediaPlayer2"
#define MPRIS2_PLAYER_IFACE "org.mpris.MediaPlayer2.Player"

PlayerControl::PlayerControl(PlayerContainer* container, QObject* parent)
    : Plasma::Service(parent)
    , m_container(container)
{
    setObjectName(container->objectName() + QLatin1String(" controller"));
    setName("mpris2");
    setDestination(container->objectName());

    m_rootIface = new QDBusInterface(container->dbusAddress(),
            MPRIS2_PATH, MPRIS2_IFACE,
            QDBusConnection::sessionBus(), this);
    m_playerIface = new QDBusInterface(container->dbusAddress(),
            MPRIS2_PATH, MPRIS2_PLAYER_IFACE,
            QDBusConnection::sessionBus(), this);

    connect(container, SIGNAL(dataUpdated(QString,Plasma::DataEngine::Data)),
            this,      SLOT(updateEnabledOperations()));
    connect(container, SIGNAL(destroyed(QObject*)),
            this,      SLOT(containerDestroyed()));
    updateEnabledOperations();
}

void PlayerControl::updateEnabledOperations()
{
    PlayerContainer::Caps caps = PlayerContainer::NoCaps;
    if (m_container)
        caps = m_container->capabilities();

    setOperationEnabled("Quit", caps & PlayerContainer::CanQuit);
    setOperationEnabled("Raise", caps & PlayerContainer::CanRaise);
    setOperationEnabled("SetFullscreen", caps & PlayerContainer::CanSetFullscreen);

    setOperationEnabled("Play", caps & PlayerContainer::CanPlay);
    setOperationEnabled("Pause", caps & PlayerContainer::CanPause);
    setOperationEnabled("PlayPause", caps & PlayerContainer::CanPause);
    setOperationEnabled("Stop", caps & PlayerContainer::CanStop);
    setOperationEnabled("Next", caps & PlayerContainer::CanGoNext);
    setOperationEnabled("Previous", caps & PlayerContainer::CanGoPrevious);
    setOperationEnabled("Seek", caps & PlayerContainer::CanSeek);
    setOperationEnabled("SetPosition", caps & PlayerContainer::CanSeek);
    setOperationEnabled("OpenUri", caps & PlayerContainer::CanControl);
    setOperationEnabled("SetVolume", caps & PlayerContainer::CanControl);
    setOperationEnabled("SetLoopStatus", caps & PlayerContainer::CanControl);
    setOperationEnabled("SetRate", caps & PlayerContainer::CanControl);
    setOperationEnabled("SetShuffle", caps & PlayerContainer::CanControl);

    emit enabledOperationsChanged();
}

QDBusInterface* PlayerControl::propertiesInterface() const
{
    return m_container->propertiesInterface();
}

QVariant PlayerControl::trackId() const
{
    return m_container->data().value("Metadata").toMap().value("mpris:trackid");
}

void PlayerControl::containerDestroyed()
{
    m_container = 0;
}

Plasma::ServiceJob* PlayerControl::createJob(const QString& operation,
                                             QMap<QString,QVariant>& parameters)
{
    if (!m_container)
        return 0;
    return new PlayerActionJob(operation, parameters, this);
}

#include "playercontrol.moc"

// vim: sw=4 sts=4 et tw=100
