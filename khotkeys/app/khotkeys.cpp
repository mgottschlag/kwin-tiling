/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _KHOTKEYS_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "khotkeys.h"

#include <settings.h>
#include <input.h>
#include <action_data.h>

namespace KHotKeys
{

// KhotKeysApp

KHotKeysApp::KHotKeysApp()
    :   KUniqueApplication( false, true ), // no styles
        delete_helper( new QObject )
    {
    init_global_data( true, delete_helper ); // grab keys
    // CHECKME triggery a dalsi vytvaret az tady za inicializaci
    actions_root = NULL;
    reread_configuration();
    }

KHotKeysApp::~KHotKeysApp()
    {
    // CHECKME triggery a dalsi rusit uz tady pred cleanupem
    delete actions_root;
// Many global data should be destroyed while the QApplication object still
// exists, and therefore 'this' cannot be the parent, as ~Object
// for 'this' would be called after ~QApplication - use proxy object
    delete delete_helper;
    }

void KHotKeysApp::reread_configuration()
    { // TODO
    kdDebug( 1217 ) << "reading configuration" << endl;
    delete actions_root;
    khotkeys_set_active( false );
    Settings settings;
    settings.read_settings( false );
#if 0 // TEST CHECKME
    settings.write_settings();
#endif
    actions_root = settings.actions;
    khotkeys_set_active( true );
    actions_root->update_triggers();
    }
    
} // namespace KHotKeys

#include "khotkeys.moc"
