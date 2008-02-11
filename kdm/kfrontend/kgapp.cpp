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

#include "kgapp.h"

#include "kdm_greet.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kgreeter.h"
#ifdef XDMCP
# include "kchooser.h"
#endif
#include "themer/kdmthemer.h"

#include <kcrash.h>
#include <kglobalsettings.h>
#include <kcomponentdata.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <QDesktopWidget>
#include <QX11Info>
#include <QFile>

#include <stdlib.h> // free(), exit()
#include <unistd.h> // alarm()
#include <signal.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>

extern "C" {

static void
sigAlarm( int )
{
	exit( EX_RESERVER_DPY );
}

}

GreeterApp::GreeterApp( int argc, char **argv ) :
	inherited( argc, argv ),
	regrabPtr( false ), regrabKbd( false ),
	dragWidget( 0 )
{
	pingInterval = _isLocal ? 0 : _pingInterval;
	if (pingInterval) {
		struct sigaction sa;
		sigemptyset( &sa.sa_mask );
		sa.sa_flags = 0;
		sa.sa_handler = sigAlarm;
		sigaction( SIGALRM, &sa, 0 );
		alarm( pingInterval * 70 ); // sic! give the "proper" pinger enough time
		pingTimerId = startTimer( pingInterval * 60000 );
	} else
		pingTimerId = 0;
}

void
GreeterApp::timerEvent( QTimerEvent *ev )
{
	if (ev->timerId() == pingTimerId) {
		alarm( 0 );
		if (!pingServer( QX11Info::display() ))
			::exit( EX_RESERVER_DPY );
		alarm( pingInterval * 70 ); // sic! give the "proper" pinger enough time
	}
}

bool
GreeterApp::x11EventFilter( XEvent * ev )
{
	KeySym sym;

	switch (ev->type) {
	case FocusIn:
	case FocusOut:
		if (ev->xfocus.mode == NotifyUngrab) {
			if (!regrabKbd) {
				secureKeyboard( QX11Info::display() );
				regrabKbd = true;
			}
		} else
			regrabKbd = false;
		break;
	case EnterNotify:
	case LeaveNotify:
		if (ev->xcrossing.mode == NotifyUngrab) {
			if (!regrabPtr) {
				securePointer( QX11Info::display() );
				regrabPtr = true;
			}
		} else
			regrabPtr = false;
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
		switch (ev->type) {
		case ButtonPress:
			if (((ev->xbutton.state & Mod1Mask) && ev->xbutton.button == 1) ||
			    dragWidget)
			{
				if (!dragWidget &&
				    ev->xbutton.window != QX11Info::appRootWindow( _greeterScreen ) &&
				    (dragWidget = QWidget::find( ev->xbutton.window )))
				{
					dragWidget = dragWidget->topLevelWidget();
					dialogStartPos = dragWidget->geometry().center();
					mouseStartPos = QPoint( ev->xbutton.x_root, ev->xbutton.y_root );
					setOverrideCursor( QCursor( Qt::SizeAllCursor ) );
				}
				return true;
			}
			break;
		case ButtonRelease:
			if (dragWidget) {
				restoreOverrideCursor();
				dragWidget = 0;
				return true;
			}
			break;
		case MotionNotify:
			if (dragWidget) {
				QRect grt( dragWidget->rect() );
				grt.moveCenter( dialogStartPos +
				                QPoint( ev->xbutton.x_root, ev->xbutton.y_root ) -
				                mouseStartPos );
				FDialog::fitInto( qApp->desktop()->screenGeometry( _greeterScreen ), grt );
				dragWidget->setGeometry( grt );
				return true;
			}
			break;
		}
		break;
	}
	return false;
}

extern "C" {

static int
xIOErr( Display * )
{
	exit( EX_RESERVER_DPY );
	// Bogus return value, notreached
	return 0;
}

void
kg_main( const char *argv0 )
{
	static char *argv[] = { (char *)"kdmgreet", 0 };

	KCrash::setFlags( KCrash::KeepFDs | KCrash::SaferDialog | KCrash::AlwaysDirectly );
	KCrash::setApplicationName( QLatin1String( argv[0] ) );
	KCrash::setCrashHandler( KCrash::defaultCrashHandler );
	XSetIOErrorHandler( xIOErr );
	KComponentData inst( argv[0] );
	GreeterApp app( as(argv) - 1, argv );
	foreach (const QString &dir, KGlobal::dirs()->resourceDirs( "qtplugins" ))
		app.addLibraryPath( dir );
	initQAppConfig();
	KGlobalSettings::self();

	Display *dpy = QX11Info::display();

	const QString _configColorScheme = _colorScheme;

	if (_useTheme && !_theme.isEmpty())
		_colorScheme = _theme + "/theme.colors";

	if (!QFile::exists(_colorScheme))
		_colorScheme = locate( "data", "kdisplay/color-schemes/" + _configColorScheme + ".colors" );

	if (!_colorScheme.isEmpty()) {
		KSharedConfigPtr config = KSharedConfig::openConfig( _colorScheme, KConfig::SimpleConfig );
		app.setPalette( KGlobalSettings::createApplicationPalette( config ) );
	}

	KdmThemer *themer;
	if (_useTheme && !_theme.isEmpty()) {
		QMap<QString, bool> showTypes;
		// "config" not implemented
#ifdef XDMCP
		if (_loginMode != LOGIN_LOCAL_ONLY)
			showTypes["chooser"] = true;
#endif
		showTypes["system"] = true;
		if (_allowShutdown != SHUT_NONE) {
			showTypes["halt"] = true;
			showTypes["reboot"] = true;
			// "suspend" not implemented
		}

		themer = new KdmThemer( _theme, showTypes, app.desktop()->screen() );
		if (!themer->isOK()) {
			delete themer;
			themer = 0;
		}
	} else
		themer = 0;

	setupModifiers( dpy, _numLockStatus );
	secureDisplay( dpy );
	KProcess *proc = 0;
	if (!_grabServer) {
		if (_useBackground && !themer) {
			proc = new KProcess;
			*proc << QByteArray( argv0, strrchr( argv0, '/' ) - argv0 + 1 ) + "krootimage";
			*proc << _backgroundCfg;
			proc->start();
		}
		gSendInt( G_SetupDpy );
		gRecvInt();
	}

	gSendInt( G_Ready );

	if (themer) {
		QPixmap pm( app.desktop()->screen()->size() );
		themer->paintBackground( &pm );
		QPalette palette;
		palette.setBrush( app.desktop()->backgroundRole(), QBrush( pm ) );
		app.desktop()->setPalette( palette );
		app.desktop()->setAutoFillBackground( true );
		app.desktop()->show();
		app.desktop()->repaint();
	}

	setCursor( dpy, app.desktop()->winId(), XC_left_ptr );

	int rslt = ex_exit;
	for (;;) {
		int cmd = gRecvInt();

		if (cmd == G_ConfShutdown) {
			gSet( 1 );
			gSendInt( G_QryDpyShutdown );
			int how = gRecvInt(), uid = gRecvInt();
			char *os = gRecvStr();
			gSet( 0 );
			KDMSlimShutdown::externShutdown( how, os, uid );
			if (os)
				free( os );
			gSendInt( G_Ready );
			break;
		}

		if (cmd == G_ErrorGreet) {
			if (KGVerify::handleFailVerify( qApp->desktop()->screen( _greeterScreen ), true ))
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
			gSendInt( G_Ready ); /* tell chooser to go into async mode */
			gRecvInt(); /* ack */
		} else
#endif
		{
			if ((cmd != G_GreetTimed && !_autoLoginAgain) ||
			    _autoLoginUser.isEmpty())
				_autoLoginDelay = 0;
			if (themer)
				dialog = new KThemedGreeter( themer );
			else
				dialog = new KStdGreeter;
			if (*_preloader) {
				proc2 = new KProcess;
				*proc2 << _preloader;
				proc2->start();
			}
		}
		app.restoreOverrideCursor();
		debug( "entering event loop\n" );
		rslt = dialog->exec();
		debug( "left event loop\n" );
		delete dialog;
		delete proc2;
#ifdef XDMCP
		switch (rslt) {
		case ex_greet:
			gSendInt( G_DGreet );
			continue;
		case ex_choose:
			gSendInt( G_DChoose );
			continue;
		default:
			break;
		}
#endif
		break;
	}

	KGVerify::done();

	delete proc;
	delete themer;

	unsecureDisplay( dpy );
	restoreModifiers();

	if (rslt == ex_login) {
		gSendInt( G_Ready );
		KGVerify::handleFailVerify( qApp->desktop()->screen( _greeterScreen ), false );
	}
}

} // extern "C"

#include "kgapp.moc"
