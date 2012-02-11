/* This file is part of the KDE project
   Copyright (C) 1999 David Faure
   Copyright 2003 Oswald Buddenhagen <ossi@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "lockprocess.h"
#include "main.h"
#include "kscreensaversettings.h"

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <QtDBus/QtDBus>
#include "kscreensaver_interface.h"

#include <QList>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

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


// -----------------------------------------------------------------------------

int main( int argc, char **argv )
{
    KCmdLineArgs::init(argc, argv, "kscreenlocker", "krunner", ki18n("KDE Screen Locker"),
                       "2.0" , ki18n("Session Locker for KDE Workspace"));

    KCmdLineOptions options;
    options.add("forcelock", ki18n("Force session locking"));
    options.add("dontlock", ki18n("Only start screen saver"));
    options.add("showunlock", ki18n("Immediately show the unlock dialog"));
    options.add("blank", ki18n("Only use the blank screen saver"));
    options.add("plasmasetup", ki18n("start with plasma unlocked for configuring"));
    options.add("daemon", ki18n("Fork into the background after starting up"));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    bool daemonize = false;
    int daemonPipe[2];
    char daemonBuf;
    if (args->isSet("daemon")) {
        daemonize = true;
        if (pipe(daemonPipe))
            kFatal() << "pipe() failed";
        switch (fork()) {
        case -1:
            kFatal() << "fork() failed";
        case 0:
            break;
        default:
            if (read(daemonPipe[0], &daemonBuf, 1) != 1)
                _exit(1);
            _exit(0);
        }
    }

    putenv(strdup("SESSION_MANAGER="));

    //KApplication::disableAutoDcopRegistration();

    int kdesktop_screen_number = 0;
    int starting_screen = 0;

    bool child = false;
    int parent_connection = 0; // socket to the parent saver
    QList<int> child_sockets;

    if (KGlobalSettings::isMultiHead())
    {
        Display *dpy = XOpenDisplay(NULL);
        if (! dpy) {
            fprintf(stderr,
                    "%s: FATAL ERROR: could not open display '%s'\n",
                    argv[0], XDisplayName(NULL));
            exit(1);
        }

        int number_of_screens = ScreenCount(dpy);
        starting_screen = kdesktop_screen_number = DefaultScreen(dpy);
        int pos;
        QByteArray display_name = XDisplayString(dpy);
        XCloseDisplay(dpy);
        kDebug() << "screen " << number_of_screens << " " << kdesktop_screen_number << " " << display_name << " " << starting_screen;
        dpy = 0;

        if ((pos = display_name.lastIndexOf('.')) != -1)
            display_name.remove(pos, 10);

        QString env;
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
            kDebug() << "env " << env;

            if (putenv(strdup(env.toLatin1().data()))) {
                fprintf(stderr,
                        "%s: WARNING: unable to set DISPLAY environment variable\n",
                        argv[0]);
                perror("putenv()");
            }
        }
    }

    MyApp app;
    kDebug() << "app " << kdesktop_screen_number << " " << starting_screen << " " << child << " " << child_sockets.count() << " " << parent_connection;
    app.disableSessionManagement();
    app.setQuitOnLastWindowClosed( false );
    KGlobal::locale()->insertCatalog(QLatin1String( "libkworkspace" ));

    LockProcess process(child, args->isSet("blank"));
    if (!child)
        process.setChildren(child_sockets);
    else
        process.setParent(parent_connection);

    bool rt;
    bool sig = false;
    if (!child && (args->isSet("forcelock"))) {
        rt = process.lock(args->isSet("showunlock"));
        sig = true;
    }
    else if( child || args->isSet( "dontlock" ))
        rt = process.dontLock();
    else if (args->isSet("plasmasetup")) {
        rt = process.startSetup();
    }
    else
        rt = process.defaultSave();
    if (!rt)
        return 1;

    if( sig )
    {
        org::kde::screensaver kscreensaver(QLatin1String( "org.kde.screensaver" ), QLatin1String( "/ScreenSaver" ), QDBusConnection::sessionBus());
        kscreensaver.saverLockReady();
    }
    args->clear();
    if (daemonize) {
        daemonBuf = 0;
        write(daemonPipe[1], &daemonBuf, 1);
    }
    return app.exec();
}

#include "main.moc"

#define KDM_NO_SHUTDOWN
#include <kworkspace/kdisplaymanager.cpp>
