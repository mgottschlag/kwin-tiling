/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <dcopclient.h>
#include "server.h"
#include <fcntl.h>

static const char *version = "0.4";
static const char *description = I18N_NOOP( "The reliable KDE session manager that talks the standard X11R6 \nsession management protocol (XSMP)." );

static const KCmdLineOptions options[] =
{
   { "r", 0, 0 },
   { "restore", I18N_NOOP("Restores the previous session if available"), 0},
   { "w", 0, 0 },
   { "windowmanager <wm>", I18N_NOOP("Starts 'wm' in case no other window manager is \nparticipating in the session. Default is 'kwin'"), 0},
   { 0, 0, 0 }
};

extern KSMServer* the_server;
#if defined(X_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#include <setjmp.h>
#undef _POSIX_C_SOURCE
#elif defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#include <setjmp.h>
#else
#define _POSIX_SOURCE
#include <setjmp.h>
#undef _POSIX_SOURCE
#endif
jmp_buf JumpHere;

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
    longjmp (JumpHere, 1);
}

int main( int argc, char* argv[] )
{
    KAboutData aboutData( "ksmserver", I18N_NOOP("The KDE Session Manager"),
       version, description, KAboutData::License_BSD,
       "(C) 2000, The KDE Developers");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication a(false, false);
    fcntl(ConnectionNumber(qt_xdisplay()), F_SETFD, 1);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    QCString wm = args->getOption("windowmanager");
    if ( wm.isEmpty() )
	wm = "kwin";

    KSMServer server ( QString::fromLatin1(wm) );
    IceSetIOErrorHandler (IoErrorHandler );

    if ( args->isSet("restore") )
	server.restoreSession();
    else
	server.startDefaultSession();

    setjmp (JumpHere);
    return a.exec();
}

