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

KlipperTray::KlipperTray( QWidget* parent )
    : Klipper( parent, KGlobal::config())
{
}

// this sucks ... KUniqueApplication registers itself as 'klipper'
// for the unique-app detection calls (and it shouldn't use that name IMHO)
// but in Klipper it's not KUniqueApplication class who handles
// the DCOP calls, but an instance of class Klipper, registered as 'klipper'
// this below avoids a warning when KUniqueApplication wouldn't otherwise
// find newInstance()  (which doesn't do anything in Klipper anyway)
int KlipperTray::newInstance()
{
#ifdef __GNUC__
#warning replacement?
#endif
 //   kapp->dcopClient()->setPriorityCall(false); // Allow other dcop calls

    return 0;
}

// this is used for quitting klipper process, if klipper is being started as an applet
// (AKA ugly hack)
void KlipperTray::quitProcess()
{
    kapp->quit();
}

#include "tray.moc"
