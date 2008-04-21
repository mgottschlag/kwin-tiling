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

#include "daemon.h"

#include "stand_alone.h"
#include "kded_module.h"

namespace KHotKeys { namespace Daemon {

bool isRunning()
    {
    return StandAloneDaemon::isRunning()
        or KdedModuleDaemon::isRunning();
    }


bool reload()
    {
    if (StandAloneDaemon::isRunning())
        {
        return StandAloneDaemon::reload();
        }
    else if (KdedModuleDaemon::isRunning())
        {
        return KdedModuleDaemon::reload();
        }
    return false;
    }


bool start()
    {
    // TODO: Allow configuration. Currently KdedModuleDaemon doesn't work. See
    // kcm_hotkeys/kcm_hotkeys.cpp
    return StandAloneDaemon::start();
    }


bool stop()
    {
    return StandAloneDaemon::isRunning()
        ? StandAloneDaemon::stop()
        : KdedModuleDaemon::stop();
    }


}} // namespace KHotKeys::Daemon
