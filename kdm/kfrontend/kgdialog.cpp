/*

Base class for various kdm greeter dialogs

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2004 Oswald Buddenhagen <ossi@kde.org>


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

#include "kgdialog.h"
#include "kconsole.h"
#include "kdmshutdown.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <qaccel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qapplication.h>

#include <stdlib.h>

KGDialog::KGDialog( bool themed ) : inherited( 0, !themed )
{
#ifdef WITH_KDM_XCONSOLE
	consoleView = _showLog ? new KConsole( this ) : 0;
#endif

	optMenu = 0;
}

void
#ifdef XDMCP
KGDialog::completeMenu( int _switchIf, int _switchCode, const QString &_switchMsg, int _switchAccel )
#else
KGDialog::completeMenu()
#endif
{
#ifdef HAVE_VTS
	if (_isLocal) {
		dpyMenu = new QPopupMenu( this );
		inserten( i18n("Sw&itch User"), ALT+Key_I, dpyMenu );
		connect( dpyMenu, SIGNAL(activated( int )),
		         SLOT(slotDisplaySelected( int )) );
		connect( dpyMenu, SIGNAL(aboutToShow()),
		         SLOT(slotPopulateDisplays()) );
	}
#endif

	if (_allowClose)
		inserten( _isLocal ? i18n("R&estart X Server") : i18n("Clos&e Connection"),
		          ALT+Key_E, SLOT(slotExit()) );

#ifdef XDMCP
	if (_isLocal && _loginMode != _switchIf) {
		switchCode = _switchCode;
		inserten( _switchMsg, _switchAccel, SLOT(slotSwitch()) );
	}
#endif

	if (_hasConsole)
		inserten( i18n("Co&nsole Login"), ALT+Key_N, SLOT(slotConsole()) );

	if (_allowShutdown != SHUT_NONE) {
		inserten( i18n("&Shutdown..."), ALT+Key_S, SLOT(slotShutdown()) );
		QAccel *accel = new QAccel( this );
		accel->insertItem( ALT+CTRL+Key_Delete );
		connect( accel, SIGNAL(activated( int )), SLOT(slotShutdown()) );
	}
}

void
KGDialog::ensureMenu()
{
	if (!optMenu) {
		optMenu = new QPopupMenu( this );
		optMenu->setCheckable( false );
		needSep = false;
	} else if (needSep) {
		optMenu->insertSeparator();
		needSep = false;
	}
}

void
KGDialog::inserten( const QString& txt, int accel, const char *member )
{
	ensureMenu();
	optMenu->insertItem( txt, this, member, accel );
}

void
KGDialog::inserten( const QString& txt, int accel, QPopupMenu *cmnu )
{
	ensureMenu();
	int id = optMenu->insertItem( txt, cmnu );
	optMenu->setAccel( accel, id );
	optMenu->connectItem( id, this, SLOT(slotActivateMenu( int )) );
	optMenu->setItemParameter( id, id );
}

void
KGDialog::slotActivateMenu( int id )
{
	QPopupMenu *cmnu = optMenu->findItem( id )->popup();
	QSize sh( cmnu->sizeHint() / 2 );
	cmnu->exec( geometry().center() - QPoint( sh.width(), sh.height() ) );
}

void
KGDialog::slotExit()
{
	::exit( EX_RESERVER_DPY );
}

void
KGDialog::slotSwitch()
{
#ifdef XDMCP
	// workaround for Qt bug
	QTimer::singleShot( 0, this, SLOT(slotReallySwitch()) );
#endif
}

void
KGDialog::slotReallySwitch()
{
#ifdef XDMCP
	done( switchCode );
#endif
}

void
KGDialog::slotConsole()
{
#ifdef HAVE_VTS
	dpySpec *sess = fetchSessions( 0 );
	if (sess) {
		int ret = KDMConfShutdown( -1, sess, SHUT_CONSOLE, 0 ).exec();
		disposeSessions( sess );
		if (!ret)
			return;
	}
#endif
	GSet( 1 );
	GSendInt( G_Console );
	GSet( 0 );
}

void
KGDialog::slotShutdown()
{
	if (_scheduledSd == SHUT_ALWAYS)
		KDMShutdown::scheduleShutdown( this );
	else
		KDMSlimShutdown( this ).exec();
}

void
KGDialog::slotDisplaySelected( int vt )
{
#ifdef HAVE_VTS
	GSet( 1 );
	GSendInt( G_Activate );
	GSendInt( vt );
	GSet( 0 );
#else
	(void)vt;
#endif
}

void
KGDialog::slotPopulateDisplays()
{
#ifdef HAVE_VTS
	dpyMenu->clear();
	dpySpec *sessions = fetchSessions( lstPassive );
	QString user, loc;
	for (dpySpec *sess = sessions; sess; sess = sess->next) {
		decodeSess( sess, user, loc );
		int id = dpyMenu->insertItem(
			i18n("session (location)", "%1 (%2)").arg( user ).arg( loc ),
			sess->vt ? sess->vt : -1 );
		if (!sess->vt)
			dpyMenu->setItemEnabled( id, false );
		if (sess->flags & isSelf)
			dpyMenu->setItemChecked( id, true );
	}
	disposeSessions( sessions );
#endif
}

#include "kgdialog.moc"
