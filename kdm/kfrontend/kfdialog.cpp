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
Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include "kfdialog.h"
#include "kdmconfig.h"

#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qcursor.h>
//Added by qt3to4:
#include <Q3Frame>
#include <QGridLayout>
#include <QResizeEvent>
#include <QDesktopWidget>

#include <stdio.h>

FDialog::FDialog( QWidget *parent, bool framed )
	: inherited( parent, 0, true/*, framed ? 0 : WStyle_NoBorder*/ )
{
	if (framed) {
		winFrame = new Q3Frame( this, 0, Qt::WNoAutoErase );
		winFrame->setFrameStyle( Q3Frame::WinPanel | Q3Frame::Raised );
		winFrame->setLineWidth( 2 );
	} else
		winFrame = 0;
}

void
FDialog::resizeEvent( QResizeEvent *e )
{
	inherited::resizeEvent( e );
	if (winFrame) {
		winFrame->resize( size() );
		winFrame->erase();
	}
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
		int di;
		if ((di = scr.right() - grt.right()) < 0)
			grt.moveBy( di, 0 );
		if ((di = scr.left() - grt.left()) > 0)
			grt.moveBy( di, 0 );
		if ((di = scr.bottom() - grt.bottom()) < 0)
			grt.moveBy( 0, di );
		if ((di = scr.top() - grt.top()) > 0)
			grt.moveBy( 0, di );
		setGeometry( grt );
	}

	if (dsk->screenNumber( QCursor::pos() ) != _greeterScreen)
		QCursor::setPos( grt.center() );
}

struct WinList {
	struct WinList *next;
	QWidget *win;
};

int
FDialog::exec()
{
	static WinList *wins;
	WinList *win;

	win = new WinList;
	win->win = this;
	win->next = wins;
	wins = win;
	show();
	setActiveWindow();
	inherited::exec();
	hide();
	wins = win->next;
	delete win;
	if (wins)
		wins->win->setActiveWindow();
	return result();
}

void
FDialog::box( QWidget *parent, QMessageBox::Icon type, const QString &text )
{
	KFMsgBox dlg( parent, type, text.stripWhiteSpace() );
	dlg.exec();
}

KFMsgBox::KFMsgBox( QWidget *parent, QMessageBox::Icon type, const QString &text )
	: inherited( parent )
{
	QLabel *label1 = new QLabel( this );
	label1->setPixmap( QMessageBox::standardIcon( type ) );
	QLabel *label2 = new QLabel( text, this );
	KPushButton *button = new KPushButton( KStdGuiItem::ok(), this );
	button->setDefault( true );
	button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
	connect( button, SIGNAL(clicked()), SLOT(accept()) );

	QGridLayout *grid = new QGridLayout( this, 2, 2, 10 );
	grid->addWidget( label1, 0, 0, Qt::AlignCenter );
	grid->addWidget( label2, 0, 1, Qt::AlignCenter );
	grid->addMultiCellWidget( button, 1,1, 0,1, Qt::AlignCenter );
}
