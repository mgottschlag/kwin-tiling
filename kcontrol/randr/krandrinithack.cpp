/* 
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "krandrmodule.h"

#ifdef XRANDR_STARTUP_HACK

#include <kcmdlineargs.h>
#include <kapplication.h>

extern "C"
KDE_EXPORT int kdemain( int argc, char* argv[] )
{
	KCmdLineArgs::init( argc, argv, "krandrinithack", "RANDR hack", "RANDR hack", "0.1" );
	    { // avoid creating KApplication in case it won't be needed
	    KInstance inst( "krandrinithack" );
	    KConfig config( "kcmrandrrc", true );
	    if( !RandRDisplay::applyOnStartup( config ))
		return 0; // exit
	    }
	KApplication app( false, true );
	KRandRModule::performApplyOnStartup();
	return 0;
}
#endif
