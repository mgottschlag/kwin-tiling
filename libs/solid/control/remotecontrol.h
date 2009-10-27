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

#ifndef SOLID_CONTROL_REMOTECONTROL_H
#define SOLID_CONTROL_REMOTECONTROL_H

#include "remotecontrolbutton.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>

#include "solid_control_export.h"

namespace Solid
{
namespace Control
{
    class RemoteControlPrivate;

    class SOLIDCONTROL_EXPORT RemoteControl : public QObject
    {
    Q_OBJECT
    Q_DECLARE_PRIVATE(RemoteControl)

    public:

        RemoteControl(const QString &name);
        
        /**
        * Get the Names of the available remotes in the system
        */
        static QStringList allRemoteNames();
        
        /**
        * Get all RemoteControl's available in the system
        */
        static QList<RemoteControl *> allRemotes();
        
        /**
         * Creates a new RemoteControl object.
         *
         * @param backendObject the RemoteControl object provided by the backend
         */
        explicit RemoteControl(QObject *backendObject);

        /**
         * Constructs a copy of a remote control.
         *
         * @param remoteControl the remote control to copy
         */
        RemoteControl(const RemoteControl &remoteControl);

        /**
         * Destroys a RemoteControl object.
         */
        virtual ~RemoteControl();


        /**
         * Retrieves the name of the remote.
         *
         * @returns Returns the name of the remote control
         */
        QString name() const;

        /**
         * Retrieves the buttons of the remote.
         *
         * @returns Returns the buttons of the remote control
         */
        QList<Solid::Control::RemoteControlButton> buttons() const;
	
    Q_SIGNALS:
        /**
         * This signal is emitted when button on the remote is pressed
         *
         * @param button The RemoteControlButton pressed
         */
        void buttonPressed(const Solid::Control::RemoteControlButton &button);

    protected:
        /**
         * @internal
         */
        RemoteControl(RemoteControlPrivate &dd, QObject *backendObject);

        /**
         * @internal
         */
        RemoteControl(RemoteControlPrivate &dd, const RemoteControl &name);

        RemoteControlPrivate *d_ptr;
        friend class RemoteControlManagerPrivate;

    private:
        Q_PRIVATE_SLOT(d_ptr, void _k_destroyed(QObject *))
    };
    typedef QList<RemoteControl *> RemoteControlList;


} //Control
} //Solid

//Q_DECLARE_OPERATORS_FOR_FLAGS(Solid::Control::RemoteControl::Capabilities)

#endif //SOLID_CONTROL_REMOTECONTROL_H
