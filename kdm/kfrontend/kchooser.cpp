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

#include "kchooser.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kdm_greet.h"

#include <klocale.h>

#include <qaccel.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qsocketnotifier.h>
#include <qlistview.h>
#include <qlineedit.h>

#include <stdlib.h>

class ChooserListViewItem : public QListViewItem {
public:
    ChooserListViewItem( QListView* parent, int _id, const QString& nam, const QString& sts )
	: QListViewItem( parent, nam, sts ) { id = _id; };

    int id;
};


ChooserDlg::ChooserDlg(QWidget * parent, const char *name)
    : inherited(parent, name)
{
    QBoxLayout *vbox = new QVBoxLayout(winFrame, 10, 10);

    QLabel *title = new QLabel(i18n("XDMCP Host Menu"), winFrame);
    title->setAlignment(AlignCenter);
    vbox->addWidget(title);

    host_view = new QListView(winFrame, "hosts");
    host_view->addColumn(i18n("Hostname"));
    host_view->setColumnWidth(0, fontMetrics().width("login.crap.net"));
    host_view->addColumn(i18n("Status"));
    host_view->setMinimumWidth(fontMetrics().width("login.crap.com Display not authorized to connect this server"));
    host_view->setResizeMode(QListView::LastColumn);
    vbox->addWidget(host_view);

    QBoxLayout *hibox = new QHBoxLayout(vbox, 10);
    iline = new QLineEdit(winFrame);
    iline->setEnabled(TRUE);
    QLabel *itxt = new QLabel(iline, i18n("Hos&t:"), winFrame);
    QPushButton *addButton = new QPushButton(i18n("A&dd"), winFrame);
    hibox->addWidget(itxt);
    hibox->addWidget(iline);
    hibox->addWidget(addButton);
    connect(addButton, SIGNAL(clicked()), SLOT(addHostname()));

    // Buttons
    QPushButton *acceptButton = new QPushButton(i18n("&Accept"), winFrame);
    acceptButton->setDefault(true);
    QPushButton *pingButton = new QPushButton(i18n("&Refresh"), winFrame);

    QPopupMenu *optMenu = new QPopupMenu( winFrame );
    optMenu->setCheckable( false );

    Inserten( optMenu, disLocal ?
		       i18n("R&estart X Server") :
		       i18n("Clos&e Connection"),
	      SLOT(quit_button_clicked()) );

    if (disLocal && kdmcfg->_loginMode != LOGIN_REMOTE_ONLY)
	Inserten( optMenu, i18n("&Local Login"),
		  SLOT( local_button_clicked() ) );

    if (dhasConsole)
	Inserten( optMenu, i18n("Co&nsole Login"),
		  SLOT( console_button_clicked() ) );

    if (kdmcfg->_allowShutdown != SHUT_NONE)
	Inserten( optMenu, i18n("&Shutdown..."),
		  SLOT(shutdown_button_clicked()) );

    QPushButton *menuButton = new QPushButton( i18n("&Menu"), winFrame );
    menuButton->setPopup( optMenu );

//    QPushButton *helpButton = new QPushButton(i18n("&Help"), winFrame);

    QBoxLayout *hbox = new QHBoxLayout(vbox, 20);
    hbox->addWidget(acceptButton);
    hbox->addWidget(pingButton);
    hbox->addStretch( 1 );
    hbox->addWidget(menuButton);
    hbox->addStretch( 1 );
//    hbox->addWidget(helpButton);

    sn = new QSocketNotifier (rfd, QSocketNotifier::Read, this);
    connect (sn, SIGNAL(activated(int)), SLOT(slotReadPipe()));

    connect(pingButton, SIGNAL(clicked()), SLOT(pingHosts()));
    connect(acceptButton, SIGNAL(clicked()), SLOT(accept()));
//    connect(helpButton, SIGNAL(clicked()), SLOT(slotHelp()));
    connect(host_view, SIGNAL(doubleClicked(QListViewItem *)), SLOT(accept()));

}

void
ChooserDlg::Inserten( QPopupMenu *mnu, const QString& txt, const char *member )
{
    mnu->insertItem( txt, this, member, QAccel::shortcutKey(txt) );
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
	GSendInt (G_Ch_RegisterHost);
	GSendStr (iline->text().latin1 ());
	iline->clear();
    }
}

void ChooserDlg::pingHosts()
{
    GSendInt (G_Ch_Refresh);
}

void ChooserDlg::accept()
{
    if (focusWidget() == iline) {
	if (!iline->text().isEmpty()) {
	    GSendInt (G_Ch_DirectChoice);
	    GSendStr (iline->text().latin1 ());
	    iline->clear();
	}
	return;
    } else /*if (focusWidget() == host_view)*/ {
	QListViewItem *item = host_view->currentItem();
	if (item) {
	    GSendInt (G_Ready);
	    GSendInt (((ChooserListViewItem *)item)->id);
	    SessionExit(EX_NORMAL);
	}
    }
}

void ChooserDlg::reject()
{
}

void ChooserDlg::quit_button_clicked()
{
    SessionExit(disLocal ? EX_RESERVER_DPY : EX_NORMAL);	/* XXX */
}

void ChooserDlg::local_button_clicked()
{
    done( ex_greet );
}

void ChooserDlg::console_button_clicked()
{
    SessionExit( EX_TEXTLOGIN );
}

void ChooserDlg::shutdown_button_clicked()
{
    KDMShutdown k( winFrame );
    k.exec();
}

QString ChooserDlg::recvStr()
{
    char *arr = GRecvStr();
    if (arr) {
	QString str = QString::fromLatin1 (arr);
	free (arr);
	return str;
    } else
	return i18n("<unknown>");
}

QListViewItem *ChooserDlg::findItem (int id)
{
    QListViewItem *itm;
    for (QListViewItemIterator it(host_view); (itm = it.current()); ++it)
	if (((ChooserListViewItem *)itm)->id == id)
	    return itm;
    return 0;
}

void ChooserDlg::slotReadPipe()
{
    int id;
    QString nam, sts;

    int cmd = GRecvInt ();
    switch (cmd) {
    case G_Ch_AddHost:
    case G_Ch_ChangeHost:
	id = GRecvInt ();
	nam = recvStr ();
	sts = recvStr ();
	GRecvInt ();	/* swallow willing for now */
	if (cmd == G_Ch_AddHost)
	    host_view->insertItem (
		new ChooserListViewItem (host_view, id, nam, sts));
	else {
	    QListViewItem *itm = findItem (id);
	    itm->setText (0, nam);
	    itm->setText (1, sts);
	}
	break;
    case G_Ch_RemoveHost:
	delete findItem (GRecvInt ());
	break;
    case G_Ch_BadHost:
	KFMsgBox::box( this, QMessageBox::Warning, i18n("Unknown host %1").arg(recvStr()) );
	break;
    case G_Ch_Exit:
	done( ex_exit );
	break;
    default:	/* XXX huuh ...? */
	break;
    }
}

#include "kchooser.moc"
