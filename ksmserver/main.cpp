/*****************************************************************
ksmserver - the KDE session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/
#include <fixx11h.h>
#include <config-workspace.h>
#include <config-ksmserver.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <KMessageBox>
#include <QtDBus/QtDBus>

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kmanagerselection.h>
#include "server.h"
#include <QX11Info>


static const char version[] = "0.4";
static const char description[] = I18N_NOOP( "The reliable KDE session manager that talks the standard X11R6 \nsession management protocol (XSMP)." );

extern KSMServer* the_server;

void IoErrorHandler ( IceConn iceConn)
{
    the_server->ioError( iceConn );
}

bool writeTest(QByteArray path)
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

void sanity_check( int argc, char* argv[], KAboutData* aboutDataPtr )
{
    QString msg;
    QByteArray path = getenv("HOME");
    QByteArray readOnly = getenv("KDE_HOME_READONLY");
    if (path.isEmpty())
    {
        msg = QLatin1String("$HOME not set!");
    }
    if (msg.isEmpty() && access(path.data(), W_OK))
    {
        if (errno == ENOENT)
            msg = QLatin1String("$HOME directory (%1) does not exist.");
        else if (readOnly.isEmpty())
            msg = QLatin1String("No write access to $HOME directory (%1).");
    }
    if (msg.isEmpty() && access(path.data(), R_OK))
    {
        if (errno == ENOENT)
            msg = "$HOME directory (%1) does not exist.";
        else
            msg = "No read access to $HOME directory (%1).";
    }
    if (msg.isEmpty() && readOnly.isEmpty() && !writeTest(path))
    {
        if (errno == ENOSPC)
            msg = "$HOME directory (%1) is out of disk space.";
        else
            msg = QByteArray("Writing to the $HOME directory (%1) failed with\n    "
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
            msg = "No write access to '%1'.";
        else if (access(path.data(), R_OK) && (errno != ENOENT))
            msg = "No read access to '%1'.";
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
            msg = "Temp directory (%1) is out of disk space.";
            else
            msg = "Writing to the temp directory (%1) failed with\n    "
                    "the error '"+QByteArray(strerror(errno))+QByteArray("'");
        }
    }
    if (msg.isEmpty() && (path != "/tmp"))
    {
        path = "/tmp";
        if (!writeTest(path))
        {
            if (errno == ENOSPC)
            msg = "Temp directory (%1) is out of disk space.";
            else
            msg = "Writing to the temp directory (%1) failed with\n    "
                    "the error '"+QByteArray(strerror(errno))+QByteArray("'");
        }
    }
    if (msg.isEmpty())
    {
        path += ".ICE-unix";
        if (access(path.data(), W_OK) && (errno != ENOENT))
            msg = "No write access to '%1'.";
        else if (access(path.data(), R_OK) && (errno != ENOENT))
            msg = "No read access to '%1'.";
    }
    if (!msg.isEmpty())
    {
        const char *msg_pre =
                "The following installation problem was detected\n"
                "while trying to start KDE:"
                "\n\n    ";
        const char *msg_post = "\n\nKDE is unable to start.\n";
        fputs(msg_pre, stderr);
        fprintf(stderr, "%s", qPrintable(msg.arg(QFile::decodeName(path))));
        fputs(msg_post, stderr);

        QApplication a(argc, argv);
        KComponentData i(aboutDataPtr);
        QString qmsg = msg_pre+msg.arg(QFile::decodeName(path))+msg_post;
        KMessageBox::error(0, qmsg, "KDE Installation Problem!");
        exit(255);
    }
}

extern "C" KDE_EXPORT int kdemain( int argc, char* argv[] )
{
    KAboutData aboutData( "ksmserver", 0, ki18n("The KDE Session Manager"),
       version, ki18n(description), KAboutData::License_BSD,
       ki18n("(C) 2000, The KDE Developers"));
    aboutData.addAuthor(ki18n("Matthias Ettrich"),KLocalizedString(), "ettrich@kde.org");
    aboutData.addAuthor(ki18n("Luboš Luňák"), ki18n( "Maintainer" ), "l.lunak@kde.org" );

    sanity_check(argc, argv, &aboutData);

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("r");
    options.add("restore", ki18n("Restores the saved user session if available"));
    options.add("w");
    options.add("windowmanager <wm>", ki18n("Starts 'wm' in case no other window manager is \nparticipating in the session. Default is 'kwin'"));
    options.add("nolocal", ki18n("Also allow remote connections"));
    KCmdLineArgs::addCmdLineOptions( options );

    putenv((char*)"SESSION_MANAGER=");
    KApplication a(true); // Disable styles until we need them.
    fcntl(ConnectionNumber(QX11Info::display()), F_SETFD, 1);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if( !QDBusConnection::sessionBus().interface()->registerService( "org.kde.ksmserver", QDBusConnectionInterface::DontQueueService ) )
    {
        qWarning("Could not register with D-BUS. Aborting.");
        return 1;
    }

    QString wm = args->getOption("windowmanager");
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

    KSMServer *server = new KSMServer( wm, only_local);
    
    // for the KDE-already-running check in startkde
    KSelectionOwner kde_running( "_KDE_RUNNING", 0 );
    kde_running.claim( false );

    IceSetIOErrorHandler( IoErrorHandler );

    KConfigGroup config(KGlobal::config(), "General");

    int realScreenCount = ScreenCount( QX11Info::display() );
    bool screenCountChanged =
         ( config.readEntry( "screenCount", realScreenCount ) != realScreenCount );

    QString loginMode = config.readEntry( "loginMode", "restorePreviousLogout" );

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

