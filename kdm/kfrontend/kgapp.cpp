/*

Greeter module for xdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kdm_greet.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kgapp.h"
#include "kgreeter.h"
#ifdef XDMCP
# include "kchooser.h"
#endif

#include <kapplication.h>
#include <kprocess.h>
#include <kcmdlineargs.h>
#include <kcrash.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>

#include <QTimer>
#include <qdesktopwidget.h>
#include <QCursor>
#include <QPalette>
//Added by qt3to4:
#include <QTimerEvent>
#include <QByteArray>

#include <stdlib.h> // free(), exit()
#include <unistd.h> // alarm()

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <QX11Info>

extern "C" {

static void
sigAlarm( int )
{
	exit( EX_RESERVER_DPY );
}

}

GreeterApp::GreeterApp(int argc, char **argv) :
	inherited(argc, argv)
{
	pingInterval = _isLocal ? 0 : _pingInterval;
	if (pingInterval) {
		struct sigaction sa;
		sigemptyset( &sa.sa_mask );
		sa.sa_flags = 0;
		sa.sa_handler = sigAlarm;
		sigaction( SIGALRM, &sa, 0 );
		alarm( pingInterval * 70 ); // sic! give the "proper" pinger enough time
		startTimer( pingInterval * 60000 );
	}
}

void
GreeterApp::timerEvent( QTimerEvent * )
{
	alarm( 0 );
	if (!PingServer( QX11Info::display() ))
		::exit( EX_RESERVER_DPY );
	alarm( pingInterval * 70 ); // sic! give the "proper" pinger enough time
}

bool
GreeterApp::x11EventFilter( XEvent * ev )
{
	KeySym sym;

	switch (ev->type) {
	case FocusIn:
	case FocusOut:
		// Hack to tell dialogs to take focus when the keyboard is grabbed
		ev->xfocus.mode = NotifyNormal;
		break;
	case KeyPress:
		sym = XLookupKeysym( &ev->xkey, 0 );
		if (sym != XK_Return && !IsModifierKey( sym ))
			emit activity();
		break;
	case ButtonPress:
		emit activity();
		/* fall through */
	case ButtonRelease:
		// Hack to let the RMB work as LMB
		if (ev->xbutton.button == 3)
			ev->xbutton.button = 1;
		/* fall through */
	case MotionNotify:
		if (ev->xbutton.state & Button3Mask)
			ev->xbutton.state = (ev->xbutton.state & ~Button3Mask) | Button1Mask;
		break;
	}
	return false;
}

extern "C" {

static int
xIOErr( Display * )
{
	exit( EX_RESERVER_DPY );
}

void
kg_main( const char *argv0 )
{
	static char *argv[] = { (char *)"kdmgreet", 0 };

	//KCrash::setFlags( KCrash::KeepFDs | KCrash::SaferDialog | KCrash::AlwaysDirectly );
	KCrash::setApplicationName( QLatin1String( argv[0] ) );
	KCrash::setCrashHandler( KCrash::defaultCrashHandler );
	XSetIOErrorHandler( xIOErr );
	KInstance inst( argv[0] );
	GreeterApp app( 1, argv );

	Display *dpy = QX11Info::display();

	if (!_GUIStyle.isEmpty())
		app.setStyle( _GUIStyle );

	_colorScheme = KStandardDirs::locate( "data", "kdisplay/color-schemes/" + _colorScheme + ".kcsrc" );
	if (!_colorScheme.isEmpty()) {
		KSimpleConfig config( _colorScheme, true );
		config.setGroup( "Color Scheme" );
		app.setPalette( KApplication::createApplicationPalette( &config, 7 ) );
	}

	app.setFont( _normalFont );

	setup_modifiers( dpy, _numLockStatus );
	SecureDisplay( dpy );
	KProcess *proc = 0;
	if (!_grabServer) {
		if (_useBackground) {
			proc = new KProcess;
			*proc << QByteArray( argv0, strrchr( argv0, '/' ) - argv0 + 2 ) + "krootimage";
			*proc << _backgroundCfg;
			proc->start();
		}
		GSendInt( G_SetupDpy );
		GRecvInt();
	}

	GSendInt( G_Ready );

	setCursor( dpy, app.desktop()->winId(), XC_left_ptr );

	for (;;) {
		int rslt, cmd = GRecvInt();

		if (cmd == G_ConfShutdown) {
			int how = GRecvInt(), uid = GRecvInt();
			char *os = GRecvStr();
			KDMSlimShutdown::externShutdown( how, os, uid );
			if (os)
				free( os );
			GSendInt( G_Ready );
			_autoLoginDelay = 0;
			continue;
		}

		if (cmd == G_ErrorGreet) {
			if (KGVerify::handleFailVerify( qApp->desktop()->screen( _greeterScreen ) ))
				break;
			_autoLoginDelay = 0;
			cmd = G_Greet;
		}

		KProcess *proc2 = 0;
		app.setOverrideCursor( Qt::WaitCursor );
		FDialog *dialog;
#ifdef XDMCP
		if (cmd == G_Choose) {
			dialog = new ChooserDlg;
			GSendInt( G_Ready ); /* tell chooser to go into async mode */
			GRecvInt(); /* ack */
		} else
#endif
		{
			if ((cmd != G_GreetTimed && !_autoLoginAgain) ||
			    _autoLoginUser.isEmpty())
				_autoLoginDelay = 0;
			if (_useTheme && !_theme.isEmpty()) {
				KThemedGreeter *tgrt;
				dialog = tgrt = new KThemedGreeter;
				if (!tgrt->isOK()) {
					delete tgrt;
					dialog = new KStdGreeter;
				}
			} else
				dialog = new KStdGreeter;
			if (*_preloader) {
				proc2 = new KProcess;
				*proc2 << _preloader;
				proc2->start();
			}
		}
		app.restoreOverrideCursor();
		Debug( "entering event loop\n" );
		rslt = dialog->exec();
		Debug( "left event loop\n" );
		delete dialog;
		delete proc2;
#ifdef XDMCP
		switch (rslt) {
		case ex_greet:
			GSendInt( G_DGreet );
			continue;
		case ex_choose:
			GSendInt( G_DChoose );
			continue;
		default:
			break;
		}
#endif
		break;
	}

	KGVerify::done();

	delete proc;
	UnsecureDisplay( dpy );
	restore_modifiers();

	XSetInputFocus( QX11Info::display(), PointerRoot, PointerRoot, CurrentTime );
}

} // extern "C"

#include "kgapp.moc"
