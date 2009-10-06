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

#include "remotecontrol.h"
#include "remotecontrol_p.h"

#include "soliddefs_p.h"
#include "ifaces/remotecontrol.h"

Solid::Control::RemoteControl::RemoteControl(QObject *backendObject)
    : QObject(), d_ptr(new RemoteControlPrivate(this))
{
    Q_D(RemoteControl);
    d->setBackendObject(backendObject);
}

Solid::Control::RemoteControl::RemoteControl(const RemoteControl &other)
    : QObject(), d_ptr(new RemoteControlPrivate(this))
{
    Q_D(RemoteControl);
    d->setBackendObject(other.d_ptr->backendObject());
}

Solid::Control::RemoteControl::RemoteControl(RemoteControlPrivate &dd, QObject *backendObject)
    : QObject(), d_ptr(&dd)
{
    Q_UNUSED(backendObject);
}

Solid::Control::RemoteControl::RemoteControl(RemoteControlPrivate &dd, const RemoteControl &other)
    : d_ptr(&dd)
{
    Q_UNUSED(other);
}

Solid::Control::RemoteControl::~RemoteControl()
{
    delete d_ptr;
}

QString Solid::Control::RemoteControl::name() const
{
    Q_D(const RemoteControl);
    return_SOLID_CALL(Ifaces::RemoteControl *, d->backendObject(), QString(), name());
}

QList<Solid::Control::RemoteControlButton> Solid::Control::RemoteControl::buttons() const
{
    Q_D(const RemoteControl);
    return_SOLID_CALL(Ifaces::RemoteControl *, d->backendObject(), QList<Solid::Control::RemoteControlButton>(), buttons());
}

void Solid::Control::RemoteControlPrivate::setBackendObject(QObject *object)
{
    FrontendObjectPrivate::setBackendObject(object);

    if (object) {
        QObject::connect(object, SIGNAL(buttonPressed(const Solid::Control::RemoteControlButton &)),
                         parent(), SIGNAL(buttonPressed(const Solid::Control::RemoteControlButton &)));
    }
}

#include "remotecontrol.moc"
