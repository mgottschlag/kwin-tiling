/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _MAIN_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kcmdlineargs.h>
#include <kconfig.h>
#include <klocale.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "khotkeys.h"

#include <X11/Xlib.h>

using namespace KHotKeys;

// for multihead
static int khotkeys_screen_number = 0;

extern "C"
int kdemain( int argc, char** argv )
    {
        {
	// multiheaded hotkeys
	KInstance inst("khotkeys-multihead");
	KConfig config("kdeglobals", true);
	config.setGroup("X11");
	if (config.readBoolEntry("enableMultihead")) {
	    Display *dpy = XOpenDisplay(NULL);
	    if (! dpy) {
		fprintf(stderr, "%s: FATAL ERROR while trying to open display %s\n",
			argv[0], XDisplayName(NULL));
		exit(1);
	    }

	    int number_of_screens = ScreenCount(dpy);
	    khotkeys_screen_number = DefaultScreen(dpy);
	    int pos;
	    QCString displayname = XDisplayString(dpy);
	    XCloseDisplay(dpy);
	    dpy = 0;

	    if ((pos = displayname.findRev('.')) != -1)
		displayname.remove(pos, 10);

	    QCString env;
	    if (number_of_screens != 1) {
		for (int i = 0; i < number_of_screens; i++) {
		    if (i != khotkeys_screen_number && fork() == 0) {
			khotkeys_screen_number = i;
			// break here because we are the child process, we don't
			// want to fork() anymore
			break;
		    }
		}

		env.sprintf("DISPLAY=%s.%d", displayname.data(), khotkeys_screen_number);
		if (putenv(strdup(env.data()))) {
		    fprintf(stderr,
			    "%s: WARNING: unable to set DISPLAY environment variable\n",
			    argv[0]);
		    perror("putenv()");
		}
	    }
	}
        }

    QCString appname;
    if (khotkeys_screen_number == 0)
	appname = "khotkeys";
    else
	appname.sprintf("khotkeys-screen-%d", khotkeys_screen_number);

                             // no need to i18n these, no GUI
    KCmdLineArgs::init( argc, argv, appname, I18N_NOOP( "KHotKeys" ),
        I18N_NOOP( "KHotKeys daemon" ), KHOTKEYS_VERSION );
    KUniqueApplication::addCmdLineOptions();
    if( !KHotKeysApp::start()) // already running
        return 0;
    KHotKeysApp app;
    app.disableSessionManagement();
    return app.exec();
    }
