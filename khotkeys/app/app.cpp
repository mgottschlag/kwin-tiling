/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "app.h"
#include "daemon/kded_module.h"

#include "action_data_group.h"
#include "gestures.h"
#include "settings.h"

#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>
#include <KDE/KDebug>

#include <unistd.h>

KHotKeysApp::KHotKeysApp()
    : delete_helper(NULL)
    , actions_root(NULL)
{
    QByteArray multiHead = getenv("KDE_MULTIHEAD");
    if (multiHead.toLower() == "true") {
        kError() << "Sorry, KDE_MULTIHEAD is not supported by khotkeys.";
    }

    if (KHotKeys::KdedModuleDaemon::isRunning()) {
        kWarning() << "stopping khotkeys kded module.";
        KHotKeys::KdedModuleDaemon::stop();
    }

    KHotKeys::init_global_data( true, delete_helper );

    reread_configuration();
}


KHotKeysApp::~KHotKeysApp()
    {
    delete actions_root;
// Many global data should be destroyed while the QApplication object still
// exists, and therefore 'this' cannot be the parent, as ~Object
// for 'this' would be called after ~QApplication - use proxy object
    delete delete_helper;
    }


void KHotKeysApp::reread_configuration()
    {
    kDebug( 1217 ) << "reading configuration";
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
    // SOUND: FIXME
    // KHotKeys::voice_handler->set_shortcut( settings.voice_shortcut );
#if 0 // TEST CHECKME
    settings.write_settings();
#endif
    actions_root = settings.takeActions();
    KHotKeys::khotkeys_set_active( true );
    actions_root->update_triggers();
    }


void KHotKeysApp::quit()
    {
    kapp->quit();
    }


extern "C"
int KDE_EXPORT kdemain( int argc, char** argv )
{
    KAboutData aboutData(
        "khotkeys",
        "khotkeys",
        ki18n( "KHotKeys Standalone Daemon" ),
        KHOTKEYS_VERSION,
        ki18n( "KHotKeys standalone daemon catches mouse gesture and global shortcut events." ),
        KAboutData::License_GPL );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KHotKeysApp app;
    app.disableSessionManagement();

    return app.exec();
}

#include "app.moc"
