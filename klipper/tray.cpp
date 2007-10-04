// -*- Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 8; -*-
/* This file is part of the KDE project

   Copyright (C) by Andrew Stanley-Jones
   Copyright (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (C) 2004  Esben Mose Hansen <kde@mosehansen.dk>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tray.h"

#include <kglobal.h>
#include <klocale.h>

#include "klipper.h"

KlipperTray::KlipperTray()
    : KSystemTrayIcon( "klipper" )
{
    klipper = new Klipper( this, KGlobal::config());
    setToolTip( i18n("Klipper - clipboard tool"));
    setContextMenu( NULL );
    show();
    connect( this, SIGNAL( activated( QSystemTrayIcon::ActivationReason )), klipper,
        SLOT( slotPopupMenu()));
}

#include "tray.moc"
