/****************************************************************************

 KHotKeys
 
 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _UPDATE_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kdebug.h>

#include <settings.h>
//Added by qt3to4:
#include <Q3CString>

using namespace KHotKeys;

static const KCmdLineOptions options[] =
    {
    // no need for I18N_NOOP(), this is not supposed to be used directly
        { "id <id>", "Id of the script to add to khotkeysrc.", 0 },
        KCmdLineLastOption
    };

int main( int argc, char* argv[] )
    {
    KCmdLineArgs::init( argc, argv, "khotkeys_update", "KHotKeys Update",
	"KHotKeys update utility", "1.0" );
    KCmdLineArgs::addCmdLineOptions( options );
    KApplication::disableAutoDcopRegistration();
    KApplication app( true ); // X11 connection is necessary for KKey* stuff :-/
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    QByteArray id = args->getOption( "id" );
    QString file = locate( "data", "khotkeys/" + id + ".khotkeys" );
    if( file.isEmpty())
        {
        kdWarning() << "File " << id << " not found!" << endl;
        return 1;
        }
    init_global_data( false, &app );
    Settings settings;
    settings.read_settings( true );
    KConfig cfg( file, true );
    if( !settings.import( cfg, false ))
        {
        kdWarning() << "Import of " << id << " failed!" << endl;
        return 2;
        }
    settings.write_settings();
    return 0;
    }
