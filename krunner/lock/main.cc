/* This file is part of the KDE project
   Copyright (C) 1999 David Faure (maintainer)

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdlib.h>

#include "lockprocess.h"

static KCmdLineOptions options[] =
{
// TODO
#undef I18N_NOOP
#define I18N_NOOP(x) ""
   { "forcelock", I18N_NOOP("Force screen locking"), 0 },
   { "dontlock", I18N_NOOP("Only start screensaver"), 0 },
   { 0, 0, 0 }
#undef I18N_NOOP
};

// -----------------------------------------------------------------------------

int main( int argc, char **argv )
{
// TODO i18n ?
    KCmdLineArgs::init( argc, argv, "kdesktop_lock", "Helper for KDesktop", "1.0" );
    KCmdLineArgs::addCmdLineOptions( options );

    putenv(strdup("SESSION_MANAGER="));
    // TODO
    KLocale::setMainCatalogue("kdesktop");

    KApplication::disableAutoDcopRegistration(); // not needed
    KApplication app;
    app.disableSessionManagement();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    LockProcess process;
    if( args->isSet("forcelock"))
	process.lock();
    else if( args->isSet( "dontlock" ))
	process.dontLock();
    else
	process.defaultSave();
    return app.exec();
}
