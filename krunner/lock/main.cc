/* This file is part of the KDE project
   Copyright (C) 1999 David Faure (maintainer)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <stdlib.h>
#include <kglobalsettings.h>

#include "lockprocess.h"

static KCmdLineOptions options[] =
{
   { "forcelock", I18N_NOOP("Force screen locking"), 0 },
   { "dontlock", I18N_NOOP("Only start screensaver"), 0 },
   { 0, 0, 0 }
};

// -----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    int kdesktop_screen_number = 0;
    int starting_screen = 0;

    bool child = false;
    int parent_connection = 0; // socket to the parent saver
    QValueList<int> child_sockets;

    if (KGlobalSettings::isMultiHead())
    {
        Display *dpy = XOpenDisplay(NULL);
        if (! dpy) {
            fprintf(stderr,
                    "%s: FATAL ERROR: couldn't open display '%s'\n",
                    argv[0], XDisplayName(NULL));
            exit(1);
        }

        int number_of_screens = ScreenCount(dpy);
        starting_screen = kdesktop_screen_number = DefaultScreen(dpy);
        int pos;
        QCString display_name = XDisplayString(dpy);
        XCloseDisplay(dpy);
        kdDebug() << "screen " << number_of_screens << " " << kdesktop_screen_number << " " << display_name << " " << starting_screen << endl;
        dpy = 0;

        if ((pos = display_name.findRev('.')) != -1)
            display_name.remove(pos, 10);

        QCString env;
        if (number_of_screens != 1) {
            for (int i = 0; i < number_of_screens; i++) {
                if (i != starting_screen) {
                    int fd[2];
                    if (pipe(fd)) {
                        perror("pipe");
                        break;
                    }
                    if (fork() == 0) {
                        child = true;
                        kdesktop_screen_number = i;
                        parent_connection = fd[0];
                        // break here because we are the child process, we don't
                        // want to fork() anymore
                        break;
                    } else {
                        child_sockets.append(fd[1]);
                    }
                }
            }

            env.sprintf("DISPLAY=%s.%d", display_name.data(),
                        kdesktop_screen_number);
            kdDebug() << "env " << env << endl;

            if (putenv(strdup(env.data()))) {
                fprintf(stderr,
                        "%s: WARNING: unable to set DISPLAY environment vairable\n",
                        argv[0]);
                perror("putenv()");
            }
        }
    }

    QCString appname;
    if (kdesktop_screen_number == 0)
	appname = "kdesktop_lock";
    else
	appname.sprintf("kdesktop_lock-screen-%d", kdesktop_screen_number);

    // TODO i18n ?
    KCmdLineArgs::init( argc, argv, appname.data(), I18N_NOOP("Screen Locker for KDesktop"), "1.0" );
    KCmdLineArgs::addCmdLineOptions( options );

    putenv(strdup("SESSION_MANAGER="));
    // TODO
    KLocale::setMainCatalogue("kdesktop");

    KApplication::disableAutoDcopRegistration(); // not needed
    KApplication app;
    kdDebug() << "app " << kdesktop_screen_number << " " << starting_screen << " " << child << " " << child_sockets.count() << " " << parent_connection << endl;
    app.disableSessionManagement();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    LockProcess process(child);
    if (!child)
        process.setChildren(child_sockets);
    else
        process.setParent(parent_connection);

    if( !child && args->isSet("forcelock"))
	process.lock();
    else if( child || args->isSet( "dontlock" ))
	process.dontLock();
    else
	process.defaultSave();
    return app.exec();
}
