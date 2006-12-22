/* This file is part of the KDE project
   Copyright 1999-2006 David Faure <faure@kde.org>

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

#include <config.h>

#include "desktop.h"
#include "lockeng.h"
#include "init.h"
#include "krootwm.h"
#include "kdesktopsettings.h"
#include <ksmserver_interface.h>

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kconfig.h>
#include <kmanagerselection.h>
#include <kglobal.h>
#include <kauthorized.h>
#include <QtDBus/QtDBus>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

static const char description[] =
        I18N_NOOP("The KDE desktop");

static const char version[] = VERSION;

static KCmdLineOptions options[] =
{
   { "x-root", I18N_NOOP("Use this if the desktop window appears as a real window"), 0 },
   { "noautostart", I18N_NOOP("Obsolete"), 0 },
   { "waitforkded", I18N_NOOP("Wait for kded to finish building database"), 0 },
   KCmdLineLastOption
};

// -----------------------------------------------------------------------------

int kdesktop_screen_number = 0;

static void crashHandler(int sigId)
{
    // Unregister from DBus
    QDBusConnection::sessionBus().unregisterObject( "/MainApplication", QDBusConnection::UnregisterNode );
    sleep( 1 );
    system("kdesktop &"); // try to restart
    fprintf(stderr, "*** kdesktop (%ld) got signal %d\n", (long) getpid(), sigId);
    ::exit(1);
}

static void signalHandler(int sigId)
{
    fprintf(stderr, "*** kdesktop got signal %d (Exiting)\n", sigId);
    KCrash::setEmergencySaveFunction(0); // No restarts any more
    // try to cleanup all windows
    signal(SIGTERM, SIG_DFL); // next one kills
    signal(SIGHUP,  SIG_DFL); // next one kills
    if (kapp)
        kapp->quit(); // turn catchable signals into clean shutdown
}

void KDesktop::slotUpAndRunning()
{
    // Activate crash recovery
    if (getenv("KDE_DEBUG") == NULL)
        KCrash::setEmergencySaveFunction(crashHandler); // Try to restart on crash
}

extern "C" KDE_EXPORT int kdemain( int argc, char **argv )
{
    //setup signal handling
    signal(SIGTERM, signalHandler);
    signal(SIGHUP,  signalHandler);

    {
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
	    kdesktop_screen_number = DefaultScreen(dpy);
	    int pos;
	    QByteArray display_name = XDisplayString(dpy);
	    XCloseDisplay(dpy);
	    dpy = 0;

	    if ((pos = display_name.lastIndexOf('.')) != -1)
		display_name.remove(pos, 10);

            QByteArray env;
	    if (number_of_screens != 1) {
		for (int i = 0; i < number_of_screens; i++) {
		    if (i != kdesktop_screen_number && fork() == 0) {
			kdesktop_screen_number = i;
			// break here because we are the child process, we don't
			// want to fork() anymore
			break;
		    }
		}

            	env = "DISPLAY=" + display_name + '.' + QByteArray::number( kdesktop_screen_number );

		if (putenv(strdup(env.data()))) {
		    fprintf(stderr,
			    "%s: WARNING: unable to set DISPLAY environment variable\n",
			    argv[0]);
		    perror("putenv()");
		}
	    }
	}
    }

    QByteArray appname;
    if (kdesktop_screen_number == 0)
	appname = "kdesktop";
    else
	appname = "kdesktop-screen-" + QByteArray::number( kdesktop_screen_number );

    QDBusConnection::sessionBus().interface()->registerService( "org.kde." + appname, QDBusConnectionInterface::DontQueueService );

    KAboutData aboutData( appname.data(), I18N_NOOP("KDesktop"),
			  version, description, KAboutData::License_GPL,
			  "(c) 1998-2000, The KDesktop Authors");
    aboutData.addAuthor("David Faure", I18N_NOOP("Current maintainer"), "faure@kde.org");
    aboutData.addAuthor("Martin Koller", 0, "m.koller@surfeu.at");
    aboutData.addAuthor("Waldo Bastian", 0, "bastian@kde.org");
    aboutData.addAuthor("Luboš Luňák", 0, "l.lunak@kde.org");
    aboutData.addAuthor("Joseph Wenninger", 0, "kde@jowenn.at");
    aboutData.addAuthor("Tim Jansen", 0, "tim@tjansen.de");
    aboutData.addAuthor("Benoit Walter", 0, "b.walter@free.fr");
    aboutData.addAuthor("Torben Weis", 0, "weis@kde.org");
    aboutData.addAuthor("Matthias Ettrich", 0, "ettrich@kde.org");

    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineArgs::addCmdLineOptions( options );

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "kdesktop is already running!\n");
        exit(0);
    }
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());
    ksmserver.suspendStartup(QString( "kdesktop" ));

    KUniqueApplication app;
    app.disableSessionManagement(); // Do SM, but don't restart.

    KDesktopSettings::instance(appname + "rc");
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    bool x_root_hack = args->isSet("x-root");
    bool wait_for_kded = args->isSet("waitforkded");

    // This MUST be created before any widgets are created
    SaverEngine saver;

    // Do this before forking so that if a dialog box appears it won't
    // be covered by other apps.
    // And before creating KDesktop too, of course.
    testLocalInstallation();

    // Mark kdeskop as immutable if all of its config modules have been disabled
    if (!KGlobal::config()->isImmutable() &&
        KAuthorized::authorizeControlModules(KRootWm::configModules()).isEmpty())
    {
		KGlobal::config()->setReadOnly(true);
       	KGlobal::config()->reparseConfiguration();
    }

    // for the KDE-already-running check in startkde
    KSelectionOwner kde_running( "_KDE_RUNNING", 0 );
    kde_running.claim( false );

    KDesktop desktop( x_root_hack, wait_for_kded );

    args->clear();

    return app.exec();
}

