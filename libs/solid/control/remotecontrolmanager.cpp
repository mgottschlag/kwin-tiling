/*
    Copyright (C) <2009>  Michael Zanetti <michael_zanetti@gmx.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#include "ifaces/remotecontrolmanager.h"
#include "ifaces/remotecontrol.h"

#include "soliddefs_p.h"
#include "remotecontrolmanager_p.h"
#include "remotecontrol.h"
#include "remotecontrol_p.h"

#include "remotecontrolmanager.h"

#include <kglobal.h>

#include <kdebug.h>

K_GLOBAL_STATIC(Solid::Control::RemoteControlManagerPrivate, globalRemoteControlManager)

Solid::Control::RemoteControlManagerPrivate::RemoteControlManagerPrivate() : m_invalidDevice(0)
{
    loadBackend("Remote Control",
                "SolidRemoteControlManager",
                "Solid::Control::Ifaces::RemoteControlManager");

    if (managerBackend()!=0) {
        connect(managerBackend(), SIGNAL(remoteControlAdded(const QString &)),
                this, SLOT(_k_remoteControlAdded(const QString &)));
        connect(managerBackend(), SIGNAL(remoteControlRemoved(const QString &)),
                this, SLOT(_k_remoteControlRemoved(const QString &)));
        connect(managerBackend(), SIGNAL(statusChanged(bool)),
                this, SIGNAL(statusChanged(bool)));
    }
}

Solid::Control::RemoteControlManagerPrivate::~RemoteControlManagerPrivate()
{
    // Delete all the devices, they are now outdated
    typedef QPair<RemoteControl *, QObject *> RemoteControlIfacePair;

    foreach (const RemoteControlIfacePair &pair, m_remoteControlMap) {
        delete pair.first;
        delete pair.second;
    }

    m_remoteControlMap.clear();
}

Solid::Control::RemoteControl::RemoteControl(const QString &name): QObject(), d_ptr(new RemoteControlPrivate(this))
{
    Q_D(RemoteControl);
    
    RemoteControl *other = globalRemoteControlManager->findRemoteControl(name);
    
    if(other){
        d->setBackendObject(other->d_ptr->backendObject());
    }
}

QStringList Solid::Control::RemoteControl::allRemoteNames()
{
    return_SOLID_CALL(Ifaces::RemoteControlManager *, globalRemoteControlManager->managerBackend(), QStringList(), remoteNames());
}

Solid::Control::RemoteControlList Solid::Control::RemoteControl::allRemotes()
{
    return globalRemoteControlManager->allRemotes();
}

Solid::Control::RemoteControlList Solid::Control::RemoteControlManagerPrivate::allRemotes(){
    Ifaces::RemoteControlManager *backend = qobject_cast<Ifaces::RemoteControlManager *>(managerBackend());

    if (backend!= 0)
    {
        return buildDeviceList(backend->remoteNames());
    }
    else
    {
        return RemoteControlList();
    } 
}

Solid::Control::RemoteControlList Solid::Control::RemoteControlManagerPrivate::buildDeviceList(const QStringList &remoteList)
{
    kDebug() << "building device list";
    RemoteControlList list;
    Ifaces::RemoteControlManager *backend = qobject_cast<Ifaces::RemoteControlManager *>(managerBackend());

    if (backend == 0) return list;

    foreach (const QString &remote, remoteList)
    {
        QPair<RemoteControl *, QObject *> pair = findRegisteredRemoteControl(remote);

        if (pair.first!= 0)
        {
            list.append(pair.first);
        }
    }

    return list;
}

bool Solid::Control::RemoteControlManager::connected()
{
    return_SOLID_CALL(Ifaces::RemoteControlManager *, globalRemoteControlManager->managerBackend(), false, connected());
}

Solid::Control::RemoteControl * Solid::Control::RemoteControlManagerPrivate::findRemoteControl(const QString &uni)
{
    Ifaces::RemoteControlManager *backend = qobject_cast<Ifaces::RemoteControlManager *>(managerBackend());

    if (backend == 0) return 0;

    QPair<RemoteControl *, QObject *> pair = findRegisteredRemoteControl(uni);

    if (pair.first != 0)
    {
        return pair.first;
    }
    else
    {
        return 0;
    }
}

Solid::Control::RemoteControlManager::Notifier * Solid::Control::RemoteControlManager::notifier()
{
    return globalRemoteControlManager;
}

void Solid::Control::RemoteControlManagerPrivate::_k_remoteControlAdded(const QString &remote)
{
    QPair<RemoteControl *, QObject*> pair = m_remoteControlMap.take(remote);
    
    if (pair.first!= 0)
    {
        // Oops, I'm not sure it should happen...
        // But well in this case we'd better kill the old device we got, it's probably outdated
        
        delete pair.first;
        delete pair.second;
    }
    
    emit remoteControlAdded(remote);
}

void Solid::Control::RemoteControlManagerPrivate::_k_remoteControlRemoved(const QString &remote)
{
    kDebug() << "Deleting remote" << remote;
    QPair<RemoteControl *, QObject *> pair = m_remoteControlMap.take(remote);
    
    if (pair.first!= 0)
    {
        delete pair.first;
        delete pair.second;
    }
    
    emit remoteControlRemoved(remote);
}

void Solid::Control::RemoteControlManagerPrivate::_k_destroyed(QObject *object)
{
    Ifaces::RemoteControl *remote = qobject_cast<Ifaces::RemoteControl *>(object);
    
    if (remote!=0)
    {
        QString name = remote->name();
        QPair<RemoteControl *, QObject *> pair = m_remoteControlMap.take(name);
        delete pair.first;
    }
}

/***************************************************************************/

QPair<Solid::Control::RemoteControl *, QObject *>
Solid::Control::RemoteControlManagerPrivate::findRegisteredRemoteControl(const QString &remote)
{
    if (m_remoteControlMap.contains(remote)) {
        return m_remoteControlMap[remote];
    } else {
        Ifaces::RemoteControlManager *backend = qobject_cast<Ifaces::RemoteControlManager *>(managerBackend());

        if (backend!=0)
        {
            QObject * iface = backend->createRemoteControl(remote);
            RemoteControl *device = 0;
            if (qobject_cast<Ifaces::RemoteControl *>(iface) != 0) {
                device = new RemoteControl(iface);
            } else {
                kDebug() << "Unknown Remote: " << remote;
            }
            if (device != 0) {
                QPair<RemoteControl *, QObject *> pair(device, iface);
                connect(iface, SIGNAL(destroyed(QObject *)),
                        this, SLOT(_k_destroyed(QObject *)));
                m_remoteControlMap[remote] = pair;
                return pair;
            }
            else
            {
                return QPair<RemoteControl *, QObject *>(0, 0);
            }
        }
        else
        {
            return QPair<RemoteControl *, QObject *>(0, 0);
        }
    }
}

#include "remotecontrolmanager_p.moc"
#include "remotecontrolmanager.moc"
