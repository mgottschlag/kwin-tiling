/****************************************************************************

 KHotKeys -  (C) 2000 Lubos Lunak <l.lunak@email.cz>

 main.cpp  -
 
 $Id$

****************************************************************************/

#define __main_CPP

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmdlineargs.h>

#include "khotkeys.h"

int main( int argc, char** argv )
    {                             // no need to i18n these, no GUI
    KCmdLineArgs::init( argc, argv, "khotkeys", "KHotKeys", "1.5" );
    KUniqueApplication::addCmdLineOptions();
    if( !KHotKeysApp::start()) // already running
        return 0;
    KHotKeysApp app;
    app.disableSessionManagement(); // started from startkde now
    return app.exec();
    }
