#ifndef DAEMON_H
#define DAEMON_H
/* Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdemacros.h"

/**
 * @author Michael Jansen <kde@michael-jansen.biz>
 */
namespace KHotKeys { namespace Daemon {

    /**
     * Check if a daemon is running.
     *
     * Checks for the kdedmodule and standalone daemon versions.
     */
    KDE_EXPORT bool isRunning();

    /**
     * Reload configuration.
     *
     * Reloads the configuration in the currently running daemon. If no daemon
     * is running it starts the configured version from
     * GlobalSettings::daemonType
     */
    KDE_EXPORT bool reload();

    /**
     * Start the daemon.
     */
    KDE_EXPORT bool start();

    /**
     * Stop the daemon.
     */
    KDE_EXPORT bool stop();

}} // namespace KHotKeys::Daemon

#endif /* #ifndef DAEMON_H */
