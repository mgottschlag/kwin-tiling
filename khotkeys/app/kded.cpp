/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "kded.h"

#include "action_data_group.h"
#include "gestures.h"
#include "settings.h"
#include "khotkeysadaptor.h"


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
    {
    setModuleName("khotkeys");

    // Initialize the global data, grab keys
    KHotKeys::init_global_data( true, this );

    // Read the configuration from file khotkeysrc
    reread_configuration();

    new KhotkeysAdaptor(this);
    }


KHotKeysModule::~KHotKeysModule()
    {
    delete actions_root;
    }


void KHotKeysModule::reread_configuration()
    {
    kDebug() << "Reloading the khotkeys configuration";
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


QString KHotKeysModule::get_menuentry_shortcut(const QString &storageId) const
    {
    kDebug() << storageId;
    return "ALT+X";
    }

QString KHotKeysModule::register_menuentry_shortcut(
        const QString &storageId,
        const QString &sequence)
    {
    kDebug() << storageId << "," << sequence;
    return sequence;
    }


void KHotKeysModule::quit()
    {
    deleteLater();
    }


#include "kded.moc"
