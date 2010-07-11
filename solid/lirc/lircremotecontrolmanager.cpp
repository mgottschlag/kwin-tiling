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

#include "lircremotecontrolmanager.h"
#include "lircremotecontrol.h"
#include "lircclient.h"

#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>

#include <kdebug.h>

#include <QStringList>
#include <QLocalSocket>

class LircRemoteControlManagerPrivate
{
public:
    LircRemoteControlManagerPrivate();
    bool connected;
    bool cachedState;
    LircClient *m_client;
    
    QHash<QString, LircRemoteControl *> remotes;

    bool recacheState();

};

LircRemoteControlManagerPrivate::LircRemoteControlManagerPrivate()
        : connected(false), cachedState(false)
{
    m_client = LircClient::self();
}

bool LircRemoteControlManagerPrivate::recacheState()
{
    connected = m_client->isConnected();
    if(!connected){
        connected = m_client->connectToLirc();
    }
    
    if(cachedState != connected){
      cachedState = connected;
      return true;
    } else {
      return false;
    }
}

LircRemoteControlManager::LircRemoteControlManager(QObject * parent, const QVariantList  & /*args */)
        : RemoteControlManager(parent), d(new LircRemoteControlManagerPrivate())
{

    m_dirWatch.addFile("/var/run/lirc/lircd");
    m_dirWatch.addFile("/dev/lircd");
    m_dirWatch.addFile("/tmp/.lircd");
    connect(&m_dirWatch, SIGNAL(created(QString)), this, SLOT(reconnect()));
    
    if(d->recacheState()){
      readRemotes();
    }

    connect(d->m_client, SIGNAL(connectionClosed()), this, SLOT(connectionClosed()));
}

LircRemoteControlManager::~LircRemoteControlManager()
{
    delete d;
}

void LircRemoteControlManager::reconnect()
{
    
    if(!d->connected){
        if(d->recacheState()){
            readRemotes();
            foreach(const QString &remote, m_remotes){
                emit remoteControlAdded(remote);
            }
            emit statusChanged(true);
        }
    }
}

void LircRemoteControlManager::connectionClosed(){
    d->connected = false;
    d->cachedState = false;
    kDebug() << "Lirc now disconnected";
    foreach(const QString &remote, m_remotes){
        emit remoteControlRemoved(remote);
    }
    readRemotes();
    emit statusChanged(false);    
}

void LircRemoteControlManager::newRemoteList(const QStringList &remoteList){
    foreach(const QString &remote, m_remotes){
        emit remoteControlRemoved(remote);
    }
    m_remotes = remoteList;
    foreach(const QString &remote, m_remotes){
        emit remoteControlAdded(remote);
    }
    
}

bool LircRemoteControlManager::connected() const
{
    return d->connected;
}

void LircRemoteControlManager::readRemotes() {
    m_remotes = d->m_client->remotes();
}

QStringList LircRemoteControlManager::remoteNames() const
{
    // Connect to lirc and read the available remotes
    if(!d->connected){
      kDebug() << "not connected... connecting to lircd";
      if(!d->recacheState()){
        kDebug() << "error: lirc not running";
        return QStringList(); 
      }
    }
    
    return m_remotes;
    
}

QObject * LircRemoteControlManager::createRemoteControl(const QString  &name)
{
    kDebug(1441) << name;

    // Check lirc if the requested remote is avaialble and create the interface
    if (!remoteNames().contains(name)) {
        kDebug() << "Remote Control not present in the available list, returning 0";
        return 0;
    }

    LircRemoteControl * rcInterface = 0;
    QHash<QString, LircRemoteControl *>::Iterator it = d->remotes.find(name);
    if (it == d->remotes.end()) {
        kDebug() << "unknown interface:" << name << "creating it";
    } else {
        kDebug() << "Interface already created";
        return it.value();
    }

    rcInterface = new LircRemoteControl(name);
    
    return rcInterface;
}

#include "lircremotecontrolmanager.moc"
