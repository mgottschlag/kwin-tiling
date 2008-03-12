/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _KHOTKEYS_KDED_CPP_

#include <config-workspace.h>

#include "kded.h"

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <klocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include <settings.h>
#include <input.h>
#include <action_data.h>
#include <gestures.h>
#include <voices.h>

#include "khotkeysadaptor.h"

K_PLUGIN_FACTORY(KHotKeysModuleFactory,
                 registerPlugin<KHotKeysModule>();
    )
K_EXPORT_PLUGIN(KHotKeysModuleFactory("khotkeys"))


// KhotKeysModule

KHotKeysModule::KHotKeysModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    {
    new KhotkeysAdaptor(this);
    for( int i = 0;
         i < 5;
         ++i )
        {
        if( QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.khotkeys" ))
            {
            // wait for it to finish
            QDBusConnection::sessionBus().send( QDBusMessage::createMethodCall( "org.kde.khotkeys", "/KHotKeys", "", "quit" ));
            sleep( 1 );
            }
        }
    QDBusConnection::sessionBus().registerObject("/KHotKeys", this);
    KHotKeys::init_global_data( true, this ); // grab keys
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
    kDebug( 1217 ) << "reading configuration";
    delete actions_root;
    KHotKeys::khotkeys_set_active( false );
    KHotKeys::Settings settings;
    settings.read_settings( false );
    KHotKeys::gesture_handler->set_mouse_button( settings.gestureMouseButton() );
    KHotKeys::gesture_handler->set_timeout( settings.gestureTimeOut() );
    KHotKeys::gesture_handler->enable( !settings.areGesturesDisabled() );
    KHotKeys::gesture_handler->set_exclude( settings.gesturesExclude() );
    // FIXME: SOUND
    // KHotKeys::voice_handler->set_shortcut( settings.voice_shortcut );
#if 0 // TEST CHECKME
    settings.write_settings();
#endif
    actions_root = settings.takeActions();
    KHotKeys::khotkeys_set_active( true );
    actions_root->update_triggers();
    }

void KHotKeysModule::quit()
    {
    delete this;
    }


#include "kded.moc"
