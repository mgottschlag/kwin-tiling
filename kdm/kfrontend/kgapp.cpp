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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include "kdm_greet.h"
#include "kdm_config.h"
#include "kdmconfig.h"
#include "kgapp.h"
#include "kgreeter.h"
#include "kchooser.h"

#include <kprocess.h>
#include <kcmdlineargs.h>
#include <kcrash.h>

#include <qtimer.h>
#include <qcursor.h>

#include <stdlib.h> // free()
#include <unistd.h> // alarm()

#include <X11/Xlib.h>
#include <X11/cursorfont.h>

extern "C" {

static void
sigAlarm( int )
{
    exit( EX_RESERVER_DPY );
}

}

GreeterApp::GreeterApp()
{
    pingInterval = ((GetCfgInt( C_displayType ) & d_location) == dLocal) ?
		      0 : GetCfgInt( C_pingInterval );
    if (pingInterval) {
	struct sigaction sa;
	sigemptyset( &sa.sa_mask );
	sa.sa_flags = 0;
	sa.sa_handler = sigAlarm;
	sigaction( SIGALRM, &sa, 0 );
	alarm( pingInterval * 70 );	// sic! give the "proper" pinger enough time
	startTimer( pingInterval * 60000 );
    }
}

void
GreeterApp::timerEvent( QTimerEvent * )
{
    if (!PingServer( qt_xdisplay() ))
	SessionExit( EX_RESERVER_DPY );
    alarm( pingInterval * 70 );	// sic! give the "proper" pinger enough time
}

bool
GreeterApp::x11EventFilter( XEvent * ev )
{
    switch (ev->type) {
    case FocusIn:
    case FocusOut:
	// Hack to tell dialogs to take focus when the keyboard is grabbed
	ev->xfocus.mode = NotifyNormal;
	break;
    case ButtonPress:
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

extern bool kde_have_kipc;

extern "C" void
kg_main( const char *argv0 )
{
    KProcess *proc = 0;

    static char *argv[] = { (char *)"kdmgreet", 0 };
    KCmdLineArgs::init( 1, argv, *argv, 0, 0, 0, true );

    kde_have_kipc = false;
    KApplication::disableAutoDcopRegistration();
    KCrash::setSafer( true );
    GreeterApp app;

    Display *dpy = qt_xdisplay();

    kdmcfg = new KDMConfig();

    app.setFont( kdmcfg->_normalFont );

    setup_modifiers( dpy, kdmcfg->_numLockStatus );
    SecureDisplay( dpy );
    if (!dgrabServer) {
	if (GetCfgInt( C_UseBackground )) {
	    proc = new KProcess;
	    *proc << QCString( argv0, strrchr( argv0, '/' ) - argv0 + 2 ) + "krootimage";
	    char *conf = GetCfgStr( C_BackgroundCfg );
	    *proc << conf;
	    free( conf );
	    proc->start();
	}
	GSendInt( G_SetupDpy );
	GRecvInt();
    }

    GSendInt( G_Ready );

    setCursor( dpy, app.desktop()->winId(), XC_left_ptr );

  redo:
    int rslt, cmd = GRecvInt();
    
    if (cmd == G_ErrorGreet) {
	if (KGVerify::handleFailVerify( qApp->desktop()->screen( kdmcfg->_greeterScreen ) ))
	    goto done;
	cmd = G_Greet;
    }

    app.setOverrideCursor( Qt::WaitCursor );
    FDialog *dialog;
    if (cmd == G_Greet)
	dialog = new KGreeter;
    else {
	dialog = new ChooserDlg;
	GSendInt( G_Ready );	/* tell chooser to go into async mode */
	GRecvInt();		/* ack */
    }
    app.restoreOverrideCursor();
    Debug( "entering event loop\n" );
    rslt = dialog->exec();
    Debug( "left event loop\n" );
    delete dialog;
    switch (rslt) {
    case ex_greet:
	GSendInt( G_DGreet );
	goto redo;
    case ex_choose:
	GSendInt( G_DChoose );
	goto redo;
    default:
	break;
    }

  done:
    KGVerify::done();

    delete proc;
    UnsecureDisplay( dpy );
    restore_modifiers();

    delete kdmcfg;
}
