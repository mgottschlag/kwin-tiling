/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <dcopclient.h>
#include "server.h"
#include <stdlib.h>
#include <fcntl.h>

static const char *version = "0.4";
static const char *description = I18N_NOOP( "The reliable KDE session manager that talks the standard X11R6 \nsession management protocol (XSMP)." );

static const KCmdLineOptions options[] =
{
   { "r", 0, 0 },
   { "restore", I18N_NOOP("Restores the previous session if available"), 0},
   { "w", 0, 0 },
   { "windowmanager <wm>", I18N_NOOP("Starts 'wm' in case no other window manager is \nparticipating in the session. Default is 'kwin'"), 0},
   { "nolocal", I18N_NOOP("Also allow remote connections."), 0},
   { 0, 0, 0 }
};

extern KSMServer* the_server;

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
}

int main( int argc, char* argv[] )
{
    KAboutData aboutData( "ksmserver", I18N_NOOP("The KDE Session Manager"),
       version, description, KAboutData::License_BSD,
       "(C) 2000, The KDE Developers");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    putenv((char*)"SESSION_MANAGER=");
    KApplication a(false, true); // Disable styles until we need them.
    fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    kapp->dcopClient()->registerAs("ksmserver", false);
    if (!kapp->dcopClient()->isRegistered())
    {
       qWarning("Could not register with DCOPServer. Aborting.");
       return 1;
    }

    QCString wm = args->getOption("windowmanager");
    if ( wm.isEmpty() )
	wm = "kwin";

    bool only_local = args->isSet("local");
#ifndef HAVE__ICETRANSNOLISTEN
    /* this seems strange, but the default is only_local, so if !only_local
     * the option --nolocal was given, and we warn (the option --nolocal
     * does nothing on this platform, as here the default is reversed)
     */
    if (!only_local) {
        qWarning("--[no]local is not supported on your platform. Sorry.");
    }
    only_local = false;
#endif

    KSMServer *server = new KSMServer( QString::fromLatin1(wm), only_local);
    IceSetIOErrorHandler (IoErrorHandler );

    KConfig *config = KGlobal::config();

    bool screenCountChanged =
         (config->readNumEntry("screenCount") != ScreenCount(qt_xdisplay()));

    if ( args->isSet("restore") && ! screenCountChanged )
	server->restoreSession();
    else
	server->startDefaultSession();

    return a.exec();
}

