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

#ifndef SOLID_CONTROL_IFACES_REMOTECONTROL_H
#define SOLID_CONTROL_IFACES_REMOTECONTROL_H

#include "../solid_control_export.h"
#include "../remotecontrol.h"
#include <QtCore/QObject>
#include <QtCore/QList>

namespace Solid
{
namespace Control
{
namespace Ifaces
{
    
    class SOLIDCONTROLIFACES_EXPORT RemoteControl
    {
      
    public:
        
        virtual ~RemoteControl();

        virtual QString name() const = 0;

        virtual QList<Solid::Control::RemoteControlButton> buttons() const = 0;


    protected:
    Q_SIGNALS:
        virtual void buttonPressed(const Solid::Control::RemoteControlButton &button) = 0;

    };
} //Ifaces
} //Control
} //Solid

Q_DECLARE_INTERFACE(Solid::Control::Ifaces::RemoteControl, "org.kde.Solid.Control.Ifaces.RemoteControl/0.1")

#endif // SOLID_CONTROL_IFACES_REMOTECONTROL_H
