/*****************************************************************
ksmserver - the KDE session management server

Copyright (C) 2000 Matthias Ettrich <ettrich@kde.org>
******************************************************************/

#include <config.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <QMessageBox>
#include <QDir>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include "server.h"
#include <QX11Info>


static const char version[] = "0.4";
static const char description[] = I18N_NOOP( "The reliable KDE session manager that talks the standard X11R6 \nsession management protocol (XSMP)." );

static const KCmdLineOptions options[] =
{
   { "r", 0, 0 },
   { "restore", I18N_NOOP("Restores the saved user session if available"), 0},
   { "w", 0, 0 },
   { "windowmanager <wm>", I18N_NOOP("Starts 'wm' in case no other window manager is \nparticipating in the session. Default is 'kwin'"), 0},
   { "nolocal", I18N_NOOP("Also allow remote connections"), 0},
   KCmdLineLastOption
};

extern KSMServer* the_server;

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
}

bool writeTest(Q3CString path)
{
   path += "/XXXXXX";
   int fd = mkstemp(path.data());
   if (fd == -1)
      return false;
   if (write(fd, "Hello World\n", 12) == -1)
   {
      int save_errno = errno;
      close(fd);
      unlink(path.data());
      errno = save_errno;
      return false;
   }
   close(fd);
   unlink(path.data());
   return true;
}

void sanity_check( int argc, char* argv[] )
{
  QByteArray msg;
  QByteArray path = getenv("HOME");
  QByteArray readOnly = getenv("KDE_HOME_READONLY");
  if (path.isEmpty())
  {
     msg = "$HOME not set!";
  }
  if (msg.isEmpty() && access(path.data(), W_OK))
  {
     if (errno == ENOENT)
        msg = "$HOME directory (%s) does not exist.";
     else if (readOnly.isEmpty())
        msg = "No write access to $HOME directory (%s).";
  }
  if (msg.isEmpty() && access(path.data(), R_OK))
  {
     if (errno == ENOENT)
        msg = "$HOME directory (%s) does not exist.";
     else
        msg = "No read access to $HOME directory (%s).";
  }
  if (msg.isEmpty() && readOnly.isEmpty() && !writeTest(path))
  {
     if (errno == ENOSPC)
        msg = "$HOME directory (%s) is out of disk space.";
     else
        msg = QByteArray("Writing to the $HOME directory (%s) failed with\n    "
              "the error '")+QByteArray(strerror(errno))+QByteArray("'");
  }
  if (msg.isEmpty())
  {
     path = getenv("ICEAUTHORITY");
     if (path.isEmpty())
     {
        path = getenv("HOME");
        path += "/.ICEauthority";
     }

     if (access(path.data(), W_OK) && (errno != ENOENT))
        msg = "No write access to '%s'.";
     else if (access(path.data(), R_OK) && (errno != ENOENT))
        msg = "No read access to '%s'.";
  }
  if (msg.isEmpty())
  {
#ifdef __GNUC__
#warning Is something like this needed for D-BUS?
#endif
#if 0
     path = DCOPClient::dcopServerFile();
     if (access(path.data(), R_OK) && (errno == ENOENT))
     {
        // Check iceauth
        if (DCOPClient::iceauthPath().isEmpty())
           msg = "Could not find 'iceauth' in path.";
     }
#endif
  }
  if (msg.isEmpty())
  {
     path = getenv("KDETMP");
     if (path.isEmpty())
        path = "/tmp";
     if (!writeTest(path))
     {
        if (errno == ENOSPC)
           msg = "Temp directory (%s) is out of disk space.";
        else
           msg = "Writing to the temp directory (%s) failed with\n    "
                 "the error '"+QByteArray(strerror(errno))+QByteArray("'");
     }
  }
  if (msg.isEmpty() && (path != "/tmp"))
  {
     path = "/tmp";
     if (!writeTest(path))
     {
        if (errno == ENOSPC)
           msg = "Temp directory (%s) is out of disk space.";
        else
           msg = "Writing to the temp directory (%s) failed with\n    "
                 "the error '"+QByteArray(strerror(errno))+QByteArray("'");
     }
  }
  if (msg.isEmpty())
  {
     path += ".ICE-unix";
     if (access(path.data(), W_OK) && (errno != ENOENT))
        msg = "No write access to '%s'.";
     else if (access(path.data(), R_OK) && (errno != ENOENT))
        msg = "No read access to '%s'.";
  }
  if (!msg.isEmpty())
  {
    const char *msg_pre =
             "The following installation problem was detected\n"
             "while trying to start KDE:"
             "\n\n    ";
    const char *msg_post = "\n\nKDE is unable to start.\n";
    fputs(msg_pre, stderr);
    fprintf(stderr, msg.data(), path.data());
    fputs(msg_post, stderr);

    QApplication a(argc, argv);
    Q3CString qmsg(256+path.length());
    qmsg.sprintf(msg.data(), path.data());
    qmsg = msg_pre+qmsg+msg_post;
    QMessageBox::critical(0, "KDE Installation Problem!",
        QLatin1String(qmsg.data()));
    exit(255);
  }
}

extern "C" KDE_EXPORT int kdemain( int argc, char* argv[] )
{
    sanity_check(argc, argv);

    KAboutData aboutData( "ksmserver", I18N_NOOP("The KDE Session Manager"),
       version, description, KAboutData::License_BSD,
       "(C) 2000, The KDE Developers");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Luboš Luňák", I18N_NOOP( "Maintainer" ), "l.lunak@kde.org" );

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions( options );

    putenv((char*)"SESSION_MANAGER=");
    KApplication a(true); // Disable styles until we need them.
    fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, 1);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if( QDBus::sessionBus().busService()->requestName( "ksmserver", QDBusBusService::DoNotQueueName )
        != QDBusBusService::PrimaryOwnerReply )
    {
       qWarning("Could not register with D-BUS. Aborting.");
       return 1;
    }

    QByteArray wm = args->getOption("windowmanager");
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

    KSMServer *server = new KSMServer( QLatin1String(wm), only_local);

    IceSetIOErrorHandler( IoErrorHandler );

    KConfig *config = KGlobal::config();
    config->setGroup( "General" );

    int realScreenCount = ScreenCount( QX11Info::display() );
    bool screenCountChanged =
         ( config->readEntry( "screenCount", realScreenCount ) != realScreenCount );

    QString loginMode = config->readEntry( "loginMode", "restorePreviousLogout" );

    if ( args->isSet("restore") && ! screenCountChanged )
	server->restoreSession( SESSION_BY_USER );
    else if ( loginMode == "default" || screenCountChanged )
	server->startDefaultSession();
    else if ( loginMode == "restorePreviousLogout" )
	server->restoreSession( SESSION_PREVIOUS_LOGOUT );
    else if ( loginMode == "restoreSavedSession" )
	server->restoreSession( SESSION_BY_USER );
    else
	server->startDefaultSession();
    return a.exec();
}

