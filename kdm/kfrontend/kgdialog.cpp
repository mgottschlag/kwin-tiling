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

KGDialog::KGDialog() : inherited( (QWidget *)0, (const char*)0, true )
{

    optMenu = new QPopupMenu( winFrame );
    optMenu->setCheckable( false );
}

void
KGDialog::completeMenu( int _switchIf, int _switchCode, const QString &_switchMsg, int _switchAccel )
{
    Inserten( disLocal ? i18n("R&estart X Server") : i18n("Clos&e Connection"),
	      ALT+Key_E, SLOT(slotExit()) );

    if (disLocal && kdmcfg->_loginMode != _switchIf) {
	switchCode = _switchCode;
	Inserten( _switchMsg, _switchAccel, SLOT(slotSwitch()) );
    }

    if (dhasConsole)
	Inserten( i18n("Co&nsole Login"), ALT+Key_N, SLOT(slotConsole()) );

    if (kdmcfg->_allowShutdown != SHUT_NONE) {
	Inserten( i18n("&Shutdown..."), ALT+Key_S, SLOT(slotShutdown()) );
	QAccel *accel = new QAccel( winFrame );
	accel->insertItem( ALT+CTRL+Key_Delete );
	connect( accel, SIGNAL(activated(int)), SLOT(slotShutdown()) );
    }
}

void
KGDialog::Inserten( const QString& txt, int accel, const char *member )
{
    optMenu->insertItem( txt, this, member, accel );
}

void
KGDialog::Inserten( const QString& txt, int accel, QPopupMenu *cmnu )
{
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
    SessionExit( EX_RESERVER_DPY );
}

void
KGDialog::slotSwitch()
{
    done( switchCode );
}

void
KGDialog::slotConsole()
{
    SessionExit( EX_TEXTLOGIN );
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

    QRect scr = dsk->screenGeometry( kdmcfg->_greeterScreen );
    setMaximumSize( scr.size() * .9 );
    adjustSize();
    QRect grt( rect() );
    if (kdmcfg->_greeterPosX >= 0) {
	grt.moveCenter( QPoint( kdmcfg->_greeterPosX, kdmcfg->_greeterPosY ) );
	moveInto( grt, scr );
    } else
	grt.moveCenter( scr.center() );
    setGeometry( grt );

    if (dsk->screenNumber( QCursor::pos() ) != kdmcfg->_greeterScreen)
	QCursor::setPos( grt.center() );
}

#include "kgdialog.moc"
