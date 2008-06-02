/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "kded.h"

#include "action_data_group.h"
#include "daemon/stand_alone.h"
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
    dbus_adaptor = new KhotkeysAdaptor(this);

    if (KHotKeys::StandAloneDaemon::isRunning())
        {
        KHotKeys::StandAloneDaemon::stop();
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
    deleteLater();
    }


#include "kded.moc"
