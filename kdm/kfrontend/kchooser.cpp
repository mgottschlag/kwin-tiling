/*

chooser widget for KDM

Copyright (C) 2002-2003 Oswald Buddenhagen <ossi@kde.org>
based on the chooser (C) 1999 by Harald Hoyer <Harald.Hoyer@RedHat.de>

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

#ifdef XDMCP

#include "kchooser.h"
#include "kconsole.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qsocketnotifier.h>
#include <qlistview.h>
#include <qlineedit.h>

#include <stdlib.h> // for free()

class ChooserListViewItem : public QListViewItem {
  public:
	ChooserListViewItem( QListView* parent, int _id, const QString& nam, const QString& sts )
		: QListViewItem( parent, nam, sts ) { id = _id; };

	int id;
};


ChooserDlg::ChooserDlg()
	: inherited()
{
	completeMenu( LOGIN_REMOTE_ONLY, ex_greet, i18n("&Local Login"), ALT+Key_L );

	QBoxLayout *vbox = new QVBoxLayout( this, 10, 10 );

	QLabel *title = new QLabel( i18n("XDMCP Host Menu"), this );
	title->setAlignment( AlignCenter );
	vbox->addWidget( title );

	host_view = new QListView( this, "hosts" );
	host_view->addColumn( i18n("Hostname") );
	host_view->setColumnWidth( 0, fontMetrics().width( "login.crap.net" ) );
	host_view->addColumn( i18n("Status") );
	host_view->setMinimumWidth( fontMetrics().width( "login.crap.com Display not authorized to connect this server" ) );
	host_view->setResizeMode( QListView::LastColumn );
	host_view->setAllColumnsShowFocus( true );
	vbox->addWidget( host_view );

	iline = new QLineEdit( this );
	iline->setEnabled( TRUE );
	QLabel *itxt = new QLabel( iline, i18n("Hos&t:"), this );
	QPushButton *addButton = new QPushButton( i18n("A&dd"), this );
	connect( addButton, SIGNAL(clicked()), SLOT(addHostname()) );
	QBoxLayout *hibox = new QHBoxLayout( vbox, 10 );
	hibox->addWidget( itxt );
	hibox->addWidget( iline );
	hibox->addWidget( addButton );

	// Buttons
	QPushButton *acceptButton = new QPushButton( i18n("&Accept"), this );
	acceptButton->setDefault( true );
	QPushButton *pingButton = new QPushButton( i18n("&Refresh"), this );

	QBoxLayout *hbox = new QHBoxLayout( vbox, 20 );
	hbox->addWidget( acceptButton );
	hbox->addWidget( pingButton );
	hbox->addStretch( 1 );

	if (optMenu) {
		QPushButton *menuButton = new QPushButton( i18n("&Menu"), this );
		menuButton->setPopup( optMenu );
		hbox->addWidget( menuButton );
		hbox->addStretch( 1 );
	}

//	QPushButton *helpButton = new QPushButton( i18n("&Help"), this );
//	hbox->addWidget( helpButton );

#ifdef WITH_KDM_XCONSOLE
	if (consoleView)
		vbox->addWidget( consoleView );
#endif

	sn = new QSocketNotifier( rfd, QSocketNotifier::Read, this );
	connect( sn, SIGNAL(activated( int )), SLOT(slotReadPipe()) );

	connect( pingButton, SIGNAL(clicked()), SLOT(pingHosts()) );
	connect( acceptButton, SIGNAL(clicked()), SLOT(accept()) );
//	connect( helpButton, SIGNAL(clicked()), SLOT(slotHelp()) );
	connect( host_view, SIGNAL(doubleClicked(QListViewItem *)), SLOT(accept()) );

	adjustGeometry();
}

/*
void ChooserDlg::slotHelp()
{
	KMessageBox::information(0,
	                         i18n("Choose a host, you want to work on,\n"
	                              "in the list or add one.\n\n"
	                              "After this box, you must press cancel\n"
	                              "in the Host Menu to enter a host. :("));
	iline->setFocus();
}
*/

void ChooserDlg::addHostname()
{
	if (!iline->text().isEmpty()) {
		GSendInt( G_Ch_RegisterHost );
		GSendStr( iline->text().latin1() );
		iline->clear();
	}
}

void ChooserDlg::pingHosts()
{
	GSendInt( G_Ch_Refresh );
}

void ChooserDlg::accept()
{
	if (focusWidget() == iline) {
		if (!iline->text().isEmpty()) {
			GSendInt( G_Ch_DirectChoice );
			GSendStr( iline->text().latin1() );
			iline->clear();
		}
		return;
	} else /*if (focusWidget() == host_view)*/ {
		QListViewItem *item = host_view->currentItem();
		if (item) {
			GSendInt( G_Ready );
			GSendInt( ((ChooserListViewItem *)item)->id );
			::exit( EX_NORMAL );
		}
	}
}

void ChooserDlg::reject()
{
}

QString ChooserDlg::recvStr()
{
	char *arr = GRecvStr();
	if (arr) {
		QString str = QString::fromLatin1( arr );
		free( arr );
		return str;
	} else
		return i18n("<unknown>");
}

QListViewItem *ChooserDlg::findItem( int id )
{
	QListViewItem *itm;
	for (QListViewItemIterator it( host_view ); (itm = it.current()); ++it)
		if (((ChooserListViewItem *)itm)->id == id)
			return itm;
	return 0;
}

void ChooserDlg::slotReadPipe()
{
	int id;
	QString nam, sts;

	int cmd = GRecvInt();
	switch (cmd) {
	case G_Ch_AddHost:
	case G_Ch_ChangeHost:
		id = GRecvInt();
		nam = recvStr();
		sts = recvStr();
		GRecvInt(); /* swallow willing for now */
		if (cmd == G_Ch_AddHost)
			host_view->insertItem(
				new ChooserListViewItem( host_view, id, nam, sts ) );
		else {
			QListViewItem *itm = findItem( id );
			itm->setText( 0, nam );
			itm->setText( 1, sts );
		}
		break;
	case G_Ch_RemoveHost:
		delete findItem( GRecvInt() );
		break;
	case G_Ch_BadHost:
		KFMsgBox::box( this, QMessageBox::Warning, i18n("Unknown host %1").arg( recvStr() ) );
		break;
	case G_Ch_Exit:
		done( ex_exit );
		break;
	default: /* XXX huuh ...? */
		break;
	}
}

#include "kchooser.moc"

#endif
