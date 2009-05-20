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

#include "kcm_module_factory.h"

// local
#include "kcm_hotkeys.h"

// libkhotkeys
#include "daemon/daemon.h"

// KDE
#include <KDE/KDebug>
#include <KDE/KToolInvocation>

K_PLUGIN_FACTORY_DEFINITION(
    KCMModuleFactory,
    registerPlugin<KCMHotkeys>("khotkeys" /*plugin keyword, cf .desktop file*/);
    )
K_EXPORT_PLUGIN(KCMModuleFactory("khotkeys" /*component data name & catalog name*/))
