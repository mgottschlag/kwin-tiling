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
    {                             // CHECKME no need to i18n these
    KCmdLineArgs::init( argc, argv, "KHotKeys", "KHotKeys", "1.5" );
    KUniqueApplication::addCmdLineOptions();
    if( !KHotKeysApp::start()) // already running
        return 0;
    KHotKeysApp app;
    return app.exec();
    }
