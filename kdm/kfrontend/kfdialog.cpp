/*

Dialog class that handles input focus in absence of a wm

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

#include "kfdialog.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <KGuiItem>
#include <KPushButton>

#include <QApplication>
#include <QDesktopWidget>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QX11Info>

#include <X11/Xlib.h>

#include <stdio.h>

FDialog::FDialog( QWidget *parent, bool framed )
	: inherited( parent/*, framed ? 0 : WStyle_NoBorder*/ )
{
	setModal( true );
	if (framed) {
		winFrame = new QFrame( this );
		winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
		winFrame->setLineWidth( 2 );
	} else
		winFrame = 0;
}

void
FDialog::resizeEvent( QResizeEvent *e )
{
	inherited::resizeEvent( e );
	if (winFrame)
		winFrame->resize( size() );
}

void
FDialog::fitInto( const QRect &scr, QRect &grt )
{
	int di;
	if ((di = scr.right() - grt.right()) < 0)
		grt.translate( di, 0 );
	if ((di = scr.left() - grt.left()) > 0)
		grt.translate( di, 0 );
	if ((di = scr.bottom() - grt.bottom()) < 0)
		grt.translate( 0, di );
	if ((di = scr.top() - grt.top()) > 0)
		grt.translate( 0, di );
}

void
FDialog::adjustGeometry()
{
	QDesktopWidget *dsk = qApp->desktop();

	if (_greeterScreen < 0)
		_greeterScreen = _greeterScreen == -2 ?
			dsk->screenNumber( QPoint( dsk->width() - 1, 0 ) ) :
			dsk->screenNumber( QPoint( 0, 0 ) );

	QRect scr = dsk->screenGeometry( _greeterScreen );
	if (!winFrame)
		setFixedSize( scr.size() );
	else {
		setMaximumSize( scr.size() * .9 );
		setMinimumSize( 0, 0 );
		adjustSize();
	}

	if (parentWidget())
		return;

	QRect grt( rect() );
	if (winFrame) {
		unsigned x = 50, y = 50;
		sscanf( _greeterPos, "%u,%u", &x, &y );
		grt.moveCenter( QPoint( scr.x() + scr.width() * x / 100,
		                        scr.y() + scr.height() * y / 100 ) );
		fitInto( scr, grt );
		setGeometry( grt );
	}

	if (dsk->screenNumber( QCursor::pos() ) != _greeterScreen)
		QCursor::setPos( grt.center() );
}

static void
fakeFocusIn( WId window )
{
	// We have keyboard grab, so this application will
	// get keyboard events even without having focus.
	// Fake FocusIn to make Qt realize it has the active
	// window, so that it will correctly show cursor in the dialog.
	XEvent ev;
	memset( &ev, 0, sizeof(ev) );
	ev.xfocus.display = QX11Info::display();
	ev.xfocus.type = FocusIn;
	ev.xfocus.window = window;
	ev.xfocus.mode = NotifyNormal;
	ev.xfocus.detail = NotifyAncestor;
	XSendEvent( QX11Info::display(), window, False, NoEventMask, &ev );
}

int
FDialog::exec()
{
	static QWidget *current;

	adjustGeometry();
	if (!current)
		secureInputs( QX11Info::display() );
	show();
	qApp->processEvents();
	fakeFocusIn( winId() );
	QWidget *previous = current;
	current = this;
	inherited::exec();
	current = previous;
	if (current)
		fakeFocusIn( current->winId() );
	else
		unsecureInputs( QX11Info::display() );
	return result();
}

KFMsgBox::KFMsgBox( QWidget *parent, QMessageBox::Icon type, const QString &text )
	: inherited( parent )
{
	QLabel *label1 = new QLabel( this );
	label1->setPixmap( QMessageBox::standardIcon( type ) );
	QLabel *label2 = new QLabel( text, this );
	KPushButton *button = new KPushButton( KStandardGuiItem::ok(), this );
	button->setDefault( true );
	button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
	connect( button, SIGNAL(clicked()), SLOT(accept()) );

	QGridLayout *grid = new QGridLayout( this );
	grid->addWidget( label1, 0, 0, Qt::AlignCenter );
	grid->addWidget( label2, 0, 1, Qt::AlignCenter );
	grid->addWidget( button, 1, 0, 1, 2, Qt::AlignCenter );
}

void
KFMsgBox::box( QWidget *parent, QMessageBox::Icon type, const QString &text )
{
	KFMsgBox dlg( parent, type, text.trimmed() );
	dlg.exec();
}
