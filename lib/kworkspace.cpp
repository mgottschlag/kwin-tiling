/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include "kworkspace.h"
#include <qapplication.h>
#include <QDataStream>
#include <kapplication.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <klocale.h>
#include <qdatetime.h>
#include <kstandarddirs.h>

#include <stdlib.h> // getenv()

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/SM/SMlib.h>
#include <fixx11h.h>
#endif

#ifdef Q_WS_X11
#define DISPLAY "DISPLAY"
#elif defined(Q_WS_QWS)
#define DISPLAY "QWS_DISPLAY"
#endif

#if defined Q_WS_X11
#include <kipc.h>
#endif

namespace KWorkSpace
{

static SmcConn tmpSmcConnection = 0;

static void cleanup_sm()
{
  if (tmpSmcConnection) {
      SmcCloseConnection( tmpSmcConnection, 0, 0 );
      tmpSmcConnection = 0;
  }
}

bool requestShutDown(
    ShutdownConfirm confirm, ShutdownType sdtype, ShutdownMode sdmode )
{
    QApplication::syncX();
    /*  use ksmserver's dcop interface if necessary  */
    if ( confirm == ShutdownConfirmYes ||
         sdtype != ShutdownTypeDefault ||
         sdmode != ShutdownModeDefault )
    {
        QByteArray data;
        QDataStream arg(&data, QIODevice::WriteOnly);
        arg << (int)confirm << (int)sdtype << (int)sdmode;
	return kapp->dcopClient()->send( "ksmserver", "ksmserver", "logout(int,int,int)", data );
    }

    if (! tmpSmcConnection) {
	char cerror[256];
	char* myId = 0;
	char* prevId = 0;
	SmcCallbacks cb;
	tmpSmcConnection = SmcOpenConnection( 0, 0, 1, 0,
					      0, &cb,
					      prevId,
					      &myId,
					      255,
					      cerror );
	::free( myId ); // it was allocated by C
	if (!tmpSmcConnection )
	    return false;

        qAddPostRoutine(cleanup_sm);
    }

    if ( tmpSmcConnection ) {
        // we already have a connection to the session manager, use it.
        SmcRequestSaveYourself( tmpSmcConnection, SmSaveBoth, True,
				SmInteractStyleAny,
				confirm == ShutdownConfirmNo, True );

	// flush the request
	IceFlush(SmcGetIceConnection(tmpSmcConnection));
        return true;
    }

    // open a temporary connection, if possible

    propagateSessionManager();
    QByteArray smEnv = ::getenv("SESSION_MANAGER");
    if (smEnv.isEmpty())
        return false;

    SmcRequestSaveYourself( tmpSmcConnection, SmSaveBoth, True,
			    SmInteractStyleAny, False, True );

    // flush the request
    IceFlush(SmcGetIceConnection(tmpSmcConnection));
    SmcCloseConnection( tmpSmcConnection, 0, 0 );

    return true;
}

static QTime smModificationTime;
void propagateSessionManager()
{
    QByteArray fName = QFile::encodeName(locateLocal("socket", "KSMserver"));
    QString display = QString::fromLocal8Bit( ::getenv(DISPLAY) );
    // strip the screen number from the display
    display.remove(QRegExp("\\.[0-9]+$"));
    int i;
    while( (i = display.indexOf(':')) >= 0)
       display[i] = '_';

    fName += '_';
    fName += display.toLocal8Bit();
    QByteArray smEnv = ::getenv("SESSION_MANAGER");
    bool check = smEnv.isEmpty();
    if ( !check && smModificationTime.isValid() ) {
         QFileInfo info( fName );
         QTime current = info.lastModified().time();
         check = current > smModificationTime;
    }
    if ( check ) {
        QFile f( fName );
        if ( !f.open( QIODevice::ReadOnly ) )
            return;
        QFileInfo info ( f );
        smModificationTime = QTime( info.lastModified().time() );
        QTextStream t(&f);
        t.setCodec( "ISO 8859-1" );
        QString s = t.readLine();
        f.close();
        ::setenv( "SESSION_MANAGER", s.toLatin1(), true  );
    }
}

} // end namespace

