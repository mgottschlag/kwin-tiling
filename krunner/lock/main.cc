/* This file is part of the KDE project
   Copyright (C) 1999 David Faure
   Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "lockprocess.h"
#include "main.h"
#include "kdesktopsettings.h"

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kglobalsettings.h>

#include <stdlib.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

bool MyApp::x11EventFilter( XEvent *ev )
{
    if (ev->type == XKeyPress || ev->type == ButtonPress)
        emit activity();
    else if (ev->type == MotionNotify) {
        time_t tick = time( 0 );
        if (tick != lastTick) {
            lastTick = tick;
            emit activity();
        }
    }
    return KApplication::x11EventFilter( ev );
}


static KCmdLineOptions options[] =
{
   { "forcelock", I18N_NOOP("Force session locking"), 0 },
   { "dontlock", I18N_NOOP("Only start screensaver"), 0 },
   { "blank", I18N_NOOP("Only use the blank screensaver"), 0 },
   KCmdLineLastOption
};

// -----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    KLocale::setMainCatalogue("kdesktop");

    KCmdLineArgs::init( argc, argv, "kdesktop_lock", I18N_NOOP("KDesktop Locker"), I18N_NOOP("Session Locker for KDesktop"), "2.0" );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    putenv(strdup("SESSION_MANAGER="));

    KApplication::disableAutoDcopRegistration(); // not needed

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
                        "%s: WARNING: unable to set DISPLAY environment variable\n",
                        argv[0]);
                perror("putenv()");
            }
        }
    }

    MyApp app;
    kdDebug() << "app " << kdesktop_screen_number << " " << starting_screen << " " << child << " " << child_sockets.count() << " " << parent_connection << endl;
    app.disableSessionManagement();
    KGlobal::locale()->insertCatalogue("libdmctl");

    // we need to read from the right rc file - possibly taking screen number in account
    KDesktopSettings::instance("kdesktoprc");

    LockProcess process(child, args->isSet( "blank" ));
    if (!child)
        process.setChildren(child_sockets);
    else
        process.setParent(parent_connection);

    bool rt;
    if( !child && args->isSet( "forcelock" ))
        rt = process.lock();
    else if( child || args->isSet( "dontlock" ))
        rt = process.dontLock();
    else
        rt = process.defaultSave();
    if (!rt)
        return 1;

    return app.exec();
}

#include "main.moc"
