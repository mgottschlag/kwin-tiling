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

#ifndef SOLID_CONTROL_REMOTECONTROLMANAGER
#define SOLID_CONTROL_REMOTECONTROLMANAGER

#include <QtCore/QObject>
#include <QtCore/QVariant>

#include "solid_control_export.h"
#include "remotecontrol.h"

namespace Solid
{
namespace Control
{
    namespace Ifaces
    {
        class RemoteControlManager;
    }
    typedef QList<RemoteControl*> RemoteControlList;

    /**
     * This class allow to query the underlying system to discover the available
     * remote controls installed on the system
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace RemoteControlManager
    {
        /**
         * Get the manager connection state
         */
        SOLIDCONTROL_EXPORT bool connected();

        class SOLIDCONTROL_EXPORT Notifier : public QObject
        {
            Q_OBJECT
        Q_SIGNALS:
            /**
            * This signal is emitted when a new remote control is available.
            *
            * @param name the name of the RemoteControl
            */
            void remoteControlAdded(const QString &name);
            
            /**
            * This signal is emitted when a remote control is not available anymore.
            *
            * @param name the name of the RemoteControl
            */
            void remoteControlRemoved(const QString &name);
            
            /**
             * This signal is emitted when the remote control subsystem becomes available
             */
            void statusChanged(bool connected);

        };

        SOLIDCONTROL_EXPORT Notifier *notifier();
    }

} // Control
} // Solid

#endif
