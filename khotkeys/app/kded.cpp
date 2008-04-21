/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "kded.h"

#include "action_data_group.h"
#include "gestures.h"
#include "khotkeysadaptor.h"
#include "settings.h"


#include <kaboutdata.h>
#include <kdebug.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <unistd.h>


K_PLUGIN_FACTORY(KHotKeysModuleFactory,
                 registerPlugin<KHotKeysModule>();
    )
K_EXPORT_PLUGIN(KHotKeysModuleFactory("khotkeys"))


// KhotKeysModule

KHotKeysModule::KHotKeysModule(QObject* parent, const QList<QVariant>&)
    : KDEDModule(parent)
    , actions_root(NULL)
    , dbus_adaptor(NULL)
    {
    setModuleName("khotkeys");
    kError() << "Does not work currently!";
    return;

    dbus_adaptor = new KhotkeysAdaptor(this);

    // Stop the khotkeys executable if it is running
    for( int i = 0;
         i < 5;
         ++i )
        {
        if( QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.khotkeys" ))
            {
            // wait for it to finish
            QDBusConnection::sessionBus().send( QDBusMessage::createMethodCall( "org.kde.khotkeys", "/modules/khotkeys", "", "quit" ));
            sleep( 1 );
            }
        }

    // Initialize the global data, grab keys
    KHotKeys::init_global_data( true, this );

    // Read the configuration from file khotkeysrc
    reread_configuration();
    }


KHotKeysModule::~KHotKeysModule()
    {
    delete dbus_adaptor;
    delete actions_root;
    }


void KHotKeysModule::reread_configuration()
    {
    // Delete a previous configuration
    delete actions_root;

    // Stop listening
    KHotKeys::khotkeys_set_active( false );

    // Load the settings
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
    }

void KHotKeysModule::quit()
    {
    delete this;
    }


#include "kded.moc"
