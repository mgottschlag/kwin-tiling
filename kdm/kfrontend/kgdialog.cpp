    /*

    Base class for various kdm greeter dialogs

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
 
#include "kgdialog.h"
#include "kconsole.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <qaccel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qcursor.h>

#include <stdlib.h>

KGDialog::KGDialog() : inherited( (QWidget *)0, (const char*)0, true )
{
#ifdef WITH_KDM_XCONSOLE
    layout = new QGridLayout (winFrame, 1, 1, 10, 10 );
    if (_showLog) {
	consoleView = new KConsole( winFrame );
	layout->addWidget( consoleView, 1, 0 );
    } else
	consoleView = 0;
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
	QAccel *accel = new QAccel( winFrame );
	accel->insertItem( ALT+CTRL+Key_Delete );
	connect( accel, SIGNAL(activated(int)), SLOT(slotShutdown()) );
    }
}

void
KGDialog::ensureMenu()
{
    if (!optMenu) {
	optMenu = new QPopupMenu( winFrame );
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
    ::exit( EX_TEXTLOGIN );
}

void
KGDialog::slotShutdown()
{
    KDMShutdown k( winFrame );
    k.exec();
}

static void
moveInto( QRect &what, const QRect &where )
{
    int di;

    if ((di = where.right() - what.right()) < 0)
	what.moveBy( di, 0 );
    if ((di = where.left() - what.left()) > 0)
	what.moveBy( di, 0 );
    if ((di = where.bottom() - what.bottom()) < 0)
	what.moveBy( 0, di );
    if ((di = where.top() - what.top()) > 0)
	what.moveBy( 0, di );
}

void
KGDialog::adjustGeometry()
{
    QDesktopWidget *dsk = qApp->desktop();

    QRect scr = dsk->screenGeometry( _greeterScreen );
    setMaximumSize( scr.size() * .9 );
    adjustSize();
    QRect grt( rect() );
    unsigned x = 50, y = 50;
    sscanf(_greeterPos, "%u,%u", &x, &y);
    grt.moveCenter( QPoint( scr.width() * x / 100, scr.height() * y / 100 ) );
    moveInto( grt, scr );
    setGeometry( grt );

    if (dsk->screenNumber( QCursor::pos() ) != _greeterScreen)
	QCursor::setPos( grt.center() );
}

#include "kgdialog.moc"
