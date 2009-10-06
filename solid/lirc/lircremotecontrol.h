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

#ifndef LIRC_REMOTECONTROL
#define LIRC_REMOTECONTROL


#include <solid/control/ifaces/remotecontrol.h>

#include <QtCore/qobject.h>
#include <QVariantMap>


class LircRemoteControlPrivate;

class LircRemoteControl : public QObject, virtual public Solid::Control::Ifaces::RemoteControl
{
    Q_OBJECT
    Q_INTERFACES(Solid::Control::Ifaces::RemoteControl)
public:
    LircRemoteControl(const QString &name);
    virtual ~LircRemoteControl();
    virtual QString name() const;
    virtual QList<Solid::Control::RemoteControlButton> buttons() const;
Q_SIGNALS:
    void buttonPressed(const Solid::Control::RemoteControlButton &button);
    void remoteControlAdded(const QString &name);
    void remoteControlRemoved(const QString &name);
    void _k_destroyed();
private:
    LircRemoteControlPrivate *d;

    Solid::Control::RemoteControlButton::ButtonId lircButtonToSolid(const QString &buttonName) const;
private Q_SLOTS:
    void commandReceived(const QString &remote, const QString &button, int repeatCounter);
};

#endif
