/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _UPDATE_CPP_

#include <config-workspace.h>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <settings.h>

using namespace KHotKeys;

int main( int argc, char* argv[] )
    {
    KCmdLineArgs::init( argc, argv, "khotkeys_update", 0, ki18n("KHotKeys Update"), "1.0" ,
	ki18n("KHotKeys update utility"));

    KCmdLineOptions options;
    options.add("id <id>", ki18n("Id of the script to add to khotkeysrc."));
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app( true ); // X11 connection is necessary for KKey* stuff :-/
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    QString id = args->getOption( "id" );
    QString file = KStandardDirs::locate( "data", "khotkeys/" + id + ".khotkeys" );
    if( file.isEmpty())
        {
        kWarning() << "File " << id << " not found!" ;
        return 1;
        }
    init_global_data( false, &app );
    Settings settings;
    settings.read_settings( true );
    KConfig cfg(  file );
    if( !settings.import( cfg, false ))
        {
        kWarning() << "Import of " << id << " failed!" ;
        return 2;
        }
    settings.write_settings();
#ifdef __GNUC__
#warning port to DBUS reread_configuration
#endif
    //kapp->dcopClient()->send( "khotkeys*", "khotkeys", "reread_configuration()", data );
    return 0;
    }
