/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _KHOTKEYS_KDED_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kded.h"

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>
#include <kapplication.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>

#include <settings.h>
#include <input.h>
#include <action_data.h>
#include <gestures.h>

extern "C" 
KDE_EXPORT KDEDModule *create_khotkeys( )
    {
    return new KHotKeys::KHotKeysModule( );
    }

namespace KHotKeys
{

// KhotKeysModule

KHotKeysModule::KHotKeysModule( )
    : KDEDModule( )
    {
    for( int i = 0;
         i < 5;
         ++i )
        {
        if( QDBus::sessionBus().interface()->isServiceRegistered( "khotkeys" ))
            {
            // wait for it to finish
#ifdef __GNUC__
#warning port to DBUS signal quit
#endif
            //kapp->dcopClient()->call( "khotkeys*", "khotkeys", "quit()", data, reply, replyData );
            sleep( 1 );
            }
        }
#ifdef __GNUC__
#warning port to DBUS registerAs
#endif
    //client.registerAs( "khotkeys", false ); // extra dcop connection (like if it was an app)
    init_global_data( true, this ); // grab keys
    // CHECKME triggery a dalsi vytvaret az tady za inicializaci
    actions_root = NULL;
    reread_configuration();
    }

KHotKeysModule::~KHotKeysModule()
    {
    // CHECKME triggery a dalsi rusit uz tady pred cleanupem
    delete actions_root;
    }

void KHotKeysModule::reread_configuration()
    { // TODO
    kDebug( 1217 ) << "reading configuration" << endl;
    delete actions_root;
    khotkeys_set_active( false );
    Settings settings;
    settings.read_settings( false );
    gesture_handler->set_mouse_button( settings.gesture_mouse_button );
    gesture_handler->set_timeout( settings.gesture_timeout );
    gesture_handler->enable( !settings.gestures_disabled_globally );
    gesture_handler->set_exclude( settings.gestures_exclude );
#if 0 // TEST CHECKME
    settings.write_settings();
#endif
    actions_root = settings.actions;
    khotkeys_set_active( true );
    actions_root->update_triggers();
    }

void KHotKeysModule::quit()
    {
    delete this;
    }

} // namespace KHotKeys

#include "kded.moc"
