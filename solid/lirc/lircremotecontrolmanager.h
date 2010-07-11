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


#ifndef LIRC_REMOTECONTROLMANAGER
#define LIRC_REMOTECONTROLMANAGER

#include <QObject>
#include <solid/control/ifaces/remotecontrolmanager.h>

#include <QVariantList>
#include <QTimer>
#include <KDirWatch>

class LircRemoteControlManagerPrivate;
class KDE_EXPORT LircRemoteControlManager : public Solid::Control::Ifaces::RemoteControlManager
{
    Q_INTERFACES(Solid::Control::Ifaces::RemoteControlManager)
    Q_OBJECT

public:
    LircRemoteControlManager(QObject * parent, const QVariantList  & args);
    virtual ~LircRemoteControlManager();
    bool connected() const;
    QObject * createRemoteControl(const QString &);
    QStringList remoteNames() const;

private Q_SLOTS:
    void reconnect();
    void connectionClosed();
    void newRemoteList(const QStringList &remoteList);

private:
    LircRemoteControlManagerPrivate * d;
    QStringList m_remotes;
    KDirWatch m_dirWatch;
    
    void readRemotes();
};

#endif
