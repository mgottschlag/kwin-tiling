/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <dcopclient.h>
#include <qmessagebox.h>
#include <qdir.h>

#include <kapp.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include "server.h"


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

void sanity_check( int argc, char* argv[] )
{
  const char *msg = 0;
  QCString path = getenv("HOME");
  if (path.isEmpty())
  {
     msg = "$HOME not set!";
  }
  else
  {
     if (access(path.data(), W_OK))
     {
       if (errno == ENOENT)
          msg = "$HOME directory (%s) does not exist.";
       else
          msg = "No write access to $HOME directory (%s).";
     }
     else if (access(path.data(), R_OK))
     {
       if (errno == ENOENT)
          msg = "$HOME directory (%s) does not exist.";
       else
          msg = "No read access to $HOME directory (%s).";
     }
     else
     {
        path += "/.ICEauthority";
        if (access(path.data(), W_OK) && (errno != ENOENT))
           msg = "No write access to '%s'.";
        else if (access(path.data(), R_OK) && (errno != ENOENT))
           msg = "No read access to '%s'.";
        else
        {
           path = "/tmp/.ICE-unix";
           if (access(path.data(), W_OK) && (errno != ENOENT))
              msg = "No write access to '%s'.";
           else if (access(path.data(), R_OK) && (errno != ENOENT))
              msg = "No read access to '%s'.";
        }
     }
  }
  if (msg)
  {
    const char *msg_pre = 
             "The following installation problem was detected\n"
             "while trying to start KDE:"
             "\n\n    ";
    const char *msg_post = "\n\nKDE is unable to start.\n";
    fprintf(stderr, msg_pre);
    fprintf(stderr, msg, path.data());
    fprintf(stderr, msg_post);
 
    QApplication a(argc, argv);
    QCString qmsg(256+path.length());
    qmsg.sprintf(msg, path.data());
    qmsg = msg_pre+qmsg+msg_post; 
    QMessageBox::critical(0, "KDE Installation Problem!",
        QString::fromLatin1(qmsg.data()));
    exit(255);
  }
}

int main( int argc, char* argv[] )
{
    sanity_check(argc, argv);

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

