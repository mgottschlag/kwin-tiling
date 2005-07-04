/*

Shutdown dialog

Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003,2005 Oswald Buddenhagen <ossi@kde.org>


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

#include "kdmshutdown.h"
#include "kdm_greet.h"

#include <kapplication.h>
#include <kseparator.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kprocio.h>
#include <kdialog.h>
#include <kstandarddirs.h>
#include <kuser.h>

#include <qcombobox.h>
#include <qvbuttongroup.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qaccel.h>
#include <qpopupmenu.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qlabel.h>
#include <qdatetime.h>
#include <qlistview.h>
#include <qheader.h>
#include <qdatetime.h>

#define KDmh KDialog::marginHint()
#define KDsh KDialog::spacingHint()

#include <stdlib.h>

int KDMShutdownBase::curPlugin = -1;
PluginList KDMShutdownBase::pluginList;

KDMShutdownBase::KDMShutdownBase( int _uid, QWidget *_parent )
	: inherited( _parent )
	, box( new QVBoxLayout( this, KDmh, KDsh ) )
#ifdef HAVE_VTS
	, willShut( true )
#endif
	, mayNuke( false )
	, doesNuke( false )
	, mayOk( true )
	, maySched( false )
	, rootlab( 0 )
	, verify( 0 )
	, needRoot( -1 )
	, uid( _uid )
{
}

KDMShutdownBase::~KDMShutdownBase()
{
	hide();
	delete verify;
}

void
KDMShutdownBase::complete( QWidget *prevWidget )
{
	QSizePolicy fp( QSizePolicy::Fixed, QSizePolicy::Fixed );

	if (uid &&
	    ((willShut && _allowShutdown == SHUT_ROOT) ||
	     (mayNuke && _allowNuke == SHUT_ROOT)))
	{
		rootlab = new QLabel( i18n("Root authorization required."), this );
		box->addWidget( rootlab );
		if (curPlugin < 0) {
			curPlugin = 0;
			pluginList = KGVerify::init( _pluginsShutdown );
		}
		verify = new KGStdVerify( this, this,
		                          prevWidget, "root",
		                          pluginList, KGreeterPlugin::Authenticate,
		                          KGreeterPlugin::Shutdown );
		verify->selectPlugin( curPlugin );
		box->addLayout( verify->getLayout() );
		QAccel *accel = new QAccel( this );
		accel->insertItem( ALT+Key_A, 0 );
		connect( accel, SIGNAL(activated( int )), SLOT(slotActivatePlugMenu()) );
	}

	box->addWidget( new KSeparator( KSeparator::HLine, this ) );

	QBoxLayout *hlay = new QHBoxLayout( box, KDsh );
	hlay->addStretch( 1 );
	if (mayOk) {
		okButton = new KPushButton( KStdGuiItem::ok(), this );
		okButton->setSizePolicy( fp );
		okButton->setDefault( true );
		hlay->addWidget( okButton );
		hlay->addStretch( 1 );
		connect( okButton, SIGNAL(clicked()), SLOT(accept()) );
	}
	if (maySched) {
		KPushButton *schedButton =
			new KPushButton( KGuiItem( i18n("&Schedule...") ), this );
		schedButton->setSizePolicy( fp );
		hlay->addWidget( schedButton );
		hlay->addStretch( 1 );
		connect( schedButton, SIGNAL(clicked()), SLOT(slotSched()) );
	}
	cancelButton = new KPushButton( KStdGuiItem::cancel(), this );
	cancelButton->setSizePolicy( fp );
	if (!mayOk)
		cancelButton->setDefault( true );
	hlay->addWidget( cancelButton );
	hlay->addStretch( 1 );
	connect( cancelButton, SIGNAL(clicked()), SLOT(reject()) );

	updateNeedRoot();
}

void
KDMShutdownBase::slotActivatePlugMenu()
{
	if (needRoot) {
		QPopupMenu *cmnu = verify->getPlugMenu();
		if (!cmnu)
			return;
		QSize sh( cmnu->sizeHint() / 2 );
		cmnu->exec( geometry().center() - QPoint( sh.width(), sh.height() ) );
	}
}

void
KDMShutdownBase::accept()
{
	if (needRoot == 1)
		verify->accept();
	else
		accepted();
}

void
KDMShutdownBase::slotSched()
{
	done( Schedule );
}

void
KDMShutdownBase::updateNeedRoot()
{
	int nNeedRoot = uid &&
		(((willShut && _allowShutdown == SHUT_ROOT) ||
		  (_allowNuke == SHUT_ROOT && doesNuke)));
	if (verify && nNeedRoot != needRoot) {
		if (needRoot == 1)
			verify->abort();
		needRoot = nNeedRoot;
		rootlab->setEnabled( needRoot );
		verify->setEnabled( needRoot );
		if (needRoot)
			verify->start();
	}
}

void
KDMShutdownBase::accepted()
{
	inherited::done( needRoot ? (int)Authed : (int)Accepted );
}

void
KDMShutdownBase::verifyPluginChanged( int id )
{
	curPlugin = id;
	adjustSize();
}

void
KDMShutdownBase::verifyOk()
{
	accepted();
}

void
KDMShutdownBase::verifyFailed()
{
	okButton->setEnabled( false );
	cancelButton->setEnabled( false );
}

void
KDMShutdownBase::verifyRetry()
{
	okButton->setEnabled( true );
	cancelButton->setEnabled( true );
}

void
KDMShutdownBase::verifySetUser( const QString & )
{
}


static void
doShutdown( int type, const char *os )
{
	GSet( 1 );
	GSendInt( G_Shutdown );
	GSendInt( type );
	GSendInt( 0 );
	GSendInt( 0 );
	GSendInt( SHUT_FORCE );
	GSendInt( 0 ); /* irrelevant, will timeout immediately anyway */
	GSendStr( os );
	GSet( 0 );
}



KDMShutdown::KDMShutdown( int _uid, QWidget *_parent )
	: inherited( _uid, _parent )
{
	QSizePolicy fp( QSizePolicy::Fixed, QSizePolicy::Fixed );

	QHBoxLayout *hlay = new QHBoxLayout( box, KDsh );

	howGroup = new QVButtonGroup( i18n("Shutdown Type"), this );
	hlay->addWidget( howGroup, 0, AlignTop );

	QRadioButton *rb;
	rb = new KDMRadioButton( i18n("&Turn off computer"), howGroup );
	rb->setChecked( true );
	rb->setFocus();

	restart_rb = new KDMRadioButton( i18n("&Restart computer"), howGroup );

	connect( rb, SIGNAL(doubleClicked()), SLOT(accept()) );
	connect( restart_rb, SIGNAL(doubleClicked()), SLOT(accept()) );

	GSet( 1 );
	GSendInt( G_ListBootOpts );
	if (GRecvInt() == BO_OK) { /* XXX show dialog on failure */
		char **tlist = GRecvStrArr( 0 );
		int defaultTarget = GRecvInt();
		oldTarget = GRecvInt();
		QWidget *hlp = new QWidget( howGroup );
		targets = new QComboBox( hlp );
		for (int i = 0; tlist[i]; i++)
			targets->insertItem( QString::fromLocal8Bit( tlist[i] ) );
		freeStrArr( tlist );
		targets->setCurrentItem( oldTarget == -1 ? defaultTarget : oldTarget );
		QHBoxLayout *hb = new QHBoxLayout( hlp, 0, KDsh );
		int spc = kapp->style().pixelMetric( QStyle::PM_ExclusiveIndicatorWidth )
		          + howGroup->insideSpacing();
		hb->addSpacing( spc );
		hb->addWidget( targets );
		connect( targets, SIGNAL(activated( int )), SLOT(slotTargetChanged()) );
	}
	GSet( 0 );

	howGroup->setSizePolicy( fp );

	schedGroup = new QGroupBox( i18n("Scheduling"), this );
	hlay->addWidget( schedGroup, 0, AlignTop );

	le_start = new QLineEdit( schedGroup );
	QLabel *lab1 = new QLabel( le_start, i18n("&Start:"), schedGroup );

	le_timeout = new QLineEdit( schedGroup );
	QLabel *lab2 = new QLabel( le_timeout, i18n("T&imeout:"), schedGroup );

	cb_force = new QCheckBox( i18n("&Force after timeout"), schedGroup );
	if (_allowNuke != SHUT_NONE) {
		connect( cb_force, SIGNAL(clicked()), SLOT(slotWhenChanged()) );
		mayNuke = true;
	} else
		cb_force->setEnabled( false );

	QGridLayout *grid = new QGridLayout( schedGroup, 0, 0, KDmh, KDsh );
	grid->addRowSpacing( 0, schedGroup->fontMetrics().height() - 5 );
	grid->addWidget( lab1, 1, 0, AlignRight );
	grid->addWidget( le_start, 1, 1 );
	grid->addWidget( lab2, 2, 0, AlignRight );
	grid->addWidget( le_timeout, 2, 1 );
	grid->addMultiCellWidget( cb_force, 3,3, 0,1 );

	schedGroup->setSizePolicy( fp );

	le_start->setText( "0" );
	if (_defSdMode == SHUT_SCHEDULE)
		le_timeout->setText( "-1" );
	else {
		le_timeout->setText( "0" );
		if (_defSdMode == SHUT_FORCENOW && cb_force->isEnabled())
			cb_force->setChecked( true );
	}

	complete( schedGroup );
}

static int
get_date( const char *str )
{
	KProcIO prc;
	prc << "/bin/date" << "+%s" << "-d" << str;
	prc.start( KProcess::Block, false );
	QString dstr;
	if (prc.readln( dstr, false, 0 ) < 0)
		return -1;
	return dstr.toInt();
}

void
KDMShutdown::accept()
{
	if (le_start->text() == "0" || le_start->text() == "now")
		sch_st = time( 0 );
	else if (le_start->text()[0] == '+')
		sch_st = time( 0 ) + le_start->text().toInt();
	else if ((sch_st = get_date( le_start->text().latin1() )) < 0) {
		MsgBox( errorbox, i18n("Entered start date is invalid.") );
		le_start->setFocus();
		return;
	}
	if (le_timeout->text() == "-1" || le_timeout->text().startsWith( "inf" ))
		sch_to = TO_INF;
	else if (le_timeout->text()[0] == '+')
		sch_to = sch_st + le_timeout->text().toInt();
	else if ((sch_to = get_date( le_timeout->text().latin1() )) < 0) {
		MsgBox( errorbox, i18n("Entered timeout date is invalid.") );
		le_timeout->setFocus();
		return;
	}

	inherited::accept();
}

void
KDMShutdown::slotTargetChanged()
{
	restart_rb->setChecked( true );
}

void
KDMShutdown::slotWhenChanged()
{
	doesNuke = cb_force->isChecked();
	updateNeedRoot();
}

void
KDMShutdown::accepted()
{
	GSet( 1 );
	GSendInt( G_Shutdown );
	GSendInt( restart_rb->isChecked() ? SHUT_REBOOT : SHUT_HALT );
	GSendInt( sch_st );
	GSendInt( sch_to );
	GSendInt( cb_force->isChecked() ? SHUT_FORCE : SHUT_CANCEL );
	GSendInt( _allowShutdown == SHUT_ROOT ? 0 : -2 );
	GSendStr( (restart_rb->isChecked() &&
	           targets && targets->currentItem() != oldTarget) ?
	          targets->currentText().local8Bit().data() : 0 );
	GSet( 0 );
	inherited::accepted();
}

void
KDMShutdown::scheduleShutdown( QWidget *_parent )
{
	GSet( 1 );
	GSendInt( G_QueryShutdown );
	int how = GRecvInt();
	int start = GRecvInt();
	int timeout = GRecvInt();
	int force = GRecvInt();
	int uid = GRecvInt();
	char *os = GRecvStr();
	GSet( 0 );
	if (how) {
		int ret =
			KDMCancelShutdown( how, start, timeout, force, uid, os,
			                   _parent ).exec();
		if (!ret)
			return;
		doShutdown( 0, 0 );
		uid = ret == Authed ? 0 : -1;
	} else
		uid = -1;
	if (os)
		free( os );
	KDMShutdown( uid, _parent ).exec();
}


KDMRadioButton::KDMRadioButton( const QString &label, QWidget *parent )
	: inherited( label, parent )
{
}

void
KDMRadioButton::mouseDoubleClickEvent( QMouseEvent * )
{
	emit doubleClicked();
}


KDMDelayedPushButton::KDMDelayedPushButton( const KGuiItem &item,
                                            QWidget *parent, 
                                            const char *name )
	: inherited( item, parent, name )
	, pop( 0 )
{
	connect( this, SIGNAL(pressed()), SLOT(slotPressed()) );
	connect( this, SIGNAL(released()), SLOT(slotReleased()) );
	connect( &popt, SIGNAL(timeout()), SLOT(slotTimeout()) );
}

void KDMDelayedPushButton::setPopup( QPopupMenu *p )
{
	pop = p;
	setIsMenuButton( p != 0 );
}

void KDMDelayedPushButton::slotPressed()
{
	if (pop)
		popt.start( QApplication::startDragTime() );
}

void KDMDelayedPushButton::slotReleased()
{
	popt.stop();
}

void KDMDelayedPushButton::slotTimeout()
{
	popt.stop();
	pop->popup( mapToGlobal( rect().bottomLeft() ) );
	setDown( false );
}


KDMSlimShutdown::KDMSlimShutdown( QWidget *_parent )
	: inherited( _parent )
	, targetList( 0 )
{
	QHBoxLayout *hbox = new QHBoxLayout( this, KDmh, KDsh );

	QFrame *lfrm = new QFrame( this );
	lfrm->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	hbox->addWidget( lfrm, AlignCenter );
	QLabel *icon = new QLabel( lfrm );
	icon->setPixmap( QPixmap( locate( "data", "kdm/pics/shutdown.jpg" ) ) );
	QVBoxLayout *iconlay = new QVBoxLayout( lfrm );
	iconlay->addWidget( icon );

	QVBoxLayout *buttonlay = new QVBoxLayout( hbox, KDsh );

	buttonlay->addStretch( 1 );

	KPushButton *btnHalt = new
		KPushButton( KGuiItem( i18n("&Turn Off Computer"), "exit" ), this );
	buttonlay->addWidget( btnHalt );
	connect( btnHalt, SIGNAL(clicked()), SLOT(slotHalt()) );

	buttonlay->addSpacing( KDialog::spacingHint() );

	KDMDelayedPushButton *btnReboot = new
		KDMDelayedPushButton( KGuiItem( i18n("&Restart Computer"), "reload" ), this );
	buttonlay->addWidget( btnReboot );
	connect( btnReboot, SIGNAL(clicked()), SLOT(slotReboot()) );

	GSet( 1 );
	GSendInt( G_ListBootOpts );
	if (GRecvInt() == BO_OK) {
		targetList = GRecvStrArr( 0 );
		/*int def =*/ GRecvInt();
		int cur = GRecvInt();
		QPopupMenu *targets = new QPopupMenu( this );
		for (int i = 0; targetList[i]; i++) {
			QString t( QString::fromLocal8Bit( targetList[i] ) );
			targets->insertItem( i == cur ?
			                     i18n("current option in boot loader",
			                          "%1 (current)").arg( t ) :
			                     t, i );
		}
		btnReboot->setPopup( targets );
		connect( targets, SIGNAL(activated(int)), SLOT(slotReboot(int)) );
	}
	GSet( 0 );

	buttonlay->addStretch( 1 );

	if (_scheduledSd != SHUT_NEVER) {
		KPushButton *btnSched = new
			KPushButton( KGuiItem( i18n("&Schedule...") ), this );
		buttonlay->addWidget( btnSched );
		connect( btnSched, SIGNAL(clicked()), SLOT(slotSched()) );

		buttonlay->addStretch( 1 );
	}

	buttonlay->addWidget( new KSeparator( this ) );

	buttonlay->addSpacing( 0 );

	KPushButton *btnBack = new KPushButton( KStdGuiItem::cancel(), this );
	buttonlay->addWidget( btnBack );
	connect( btnBack, SIGNAL(clicked()), SLOT(reject()) );

	buttonlay->addSpacing( KDialog::spacingHint() );
}

KDMSlimShutdown::~KDMSlimShutdown()
{
	freeStrArr( targetList );
}

void
KDMSlimShutdown::slotSched()
{
	reject();
	KDMShutdown::scheduleShutdown();
}

void
KDMSlimShutdown::slotHalt()
{
	if (checkShutdown( SHUT_HALT, 0 ))
		doShutdown( SHUT_HALT, 0 );
}

void
KDMSlimShutdown::slotReboot()
{
	if (checkShutdown( SHUT_REBOOT, 0 ))
		doShutdown( SHUT_REBOOT, 0 );
}

void
KDMSlimShutdown::slotReboot( int opt )
{
	if (checkShutdown( SHUT_REBOOT, targetList[opt] ))
		doShutdown( SHUT_REBOOT, targetList[opt] );
}

bool
KDMSlimShutdown::checkShutdown( int type, const char *os )
{
	reject();
	dpySpec *sess = fetchSessions( lstRemote | lstTTY );
	if (!sess && _allowShutdown != SHUT_ROOT)
		return true;
	int ret = KDMConfShutdown( -1, sess, type, os ).exec();
	disposeSessions( sess );
	if (ret == Schedule) {
		KDMShutdown::scheduleShutdown();
		return false;
	}
	return ret;
}

void
KDMSlimShutdown::externShutdown( int type, const char *os, int uid )
{
	dpySpec *sess = fetchSessions( lstRemote | lstTTY );
	int ret = KDMConfShutdown( uid, sess, type, os ).exec();
	disposeSessions( sess );
	if (ret == Schedule)
		KDMShutdown( uid ).exec();
	else if (ret)
		doShutdown( type, os );
}


KDMConfShutdown::KDMConfShutdown( int _uid, dpySpec *sess, int type, const char *os,
                                  QWidget *_parent )
	: inherited( _uid, _parent )
{
#ifdef HAVE_VTS
	if (type == SHUT_CONSOLE)
		willShut = false;
#endif
	box->addWidget( new QLabel( QString( "<qt><center><b><nobr>"
	                                     "%1%2"
	                                     "</nobr></b></center><br></qt>" )
	                            .arg( (type == SHUT_HALT) ?
	                                  i18n("Turn Off Computer") :
#ifdef HAVE_VTS
	                                  (type == SHUT_CONSOLE) ?
	                                  i18n("Switch to Console") :
#endif
	                                  i18n("Restart Computer") )
	                            .arg( os ?
	                                  i18n("<br>(Next boot: %1)")
	                                  .arg( QString::fromLocal8Bit( os ) ) :
	                                  QString::null ),
	                            this ) );

	if (sess) {
		if (willShut && _scheduledSd != SHUT_NEVER)
			maySched = true;
		mayNuke = doesNuke = true;
		if (_allowNuke == SHUT_NONE)
			mayOk = false;
		QLabel *lab = new QLabel( mayOk ?
		                          i18n("Abort active sessions:") :
		                          i18n("No permission to abort active sessions:"),
		                          this );
		box->addWidget( lab );
		QListView *lv = new QListView( this );
		lv->setSelectionMode( QListView::NoSelection );
		lv->setResizeMode( QListView::AllColumns );
		lv->setAllColumnsShowFocus( true );
		lv->addColumn( i18n("Session") );
		lv->addColumn( i18n("Location") );
		lv->setColumnWidthMode( 0, QListView::Maximum );
		lv->setColumnWidthMode( 1, QListView::Maximum );
		QListViewItem *itm;
		int ns = 0;
		QString user, loc;
		for (; sess; sess = sess->next, ns++) {
			decodeSess( sess, user, loc );
			itm = new QListViewItem( lv, user, loc );
		}
		int fw = lv->frameWidth() * 2;
		QSize hds( lv->header()->sizeHint() );
		lv->setMinimumWidth( fw + hds.width() +
			(ns > 10 ? style().pixelMetric( QStyle::PM_ScrollBarExtent ) : 0 ) );
		lv->setFixedHeight( fw + hds.height() +
			itm->height() * (ns < 3 ? 3 : ns > 10 ? 10 : ns) );
		lv->header()->adjustHeaderSize();
		box->addWidget( lv );
		complete( lv );
	} else
		complete( 0 );
}


KDMCancelShutdown::KDMCancelShutdown( int how, int start, int timeout,
                                      int force, int uid, const char *os,
                                      QWidget *_parent )
	: inherited( -1, _parent )
{
	if (force == SHUT_FORCE) {
		if (_allowNuke == SHUT_NONE)
			mayOk = false;
		else if (_allowNuke == SHUT_ROOT)
			mayNuke = doesNuke = true;
	}
	QLabel *lab = new QLabel( mayOk ?
	                          i18n("Abort pending shutdown:") :
	                          i18n("No permission to abort pending shutdown:"),
	                          this );
	box->addWidget( lab );
	QDateTime qdt;
	QString strt, end;
	if (start < time( 0 ))
		strt = i18n("now");
	else {
		qdt.setTime_t( start );
		strt = qdt.toString( LocalDate );
	}
	if (timeout == TO_INF)
		end = i18n("infinite");
	else {
		qdt.setTime_t( timeout );
		end = qdt.toString( LocalDate );
	}
	QString trg =
		i18n("Owner: %1"
		     "\nType: %2%5"
		     "\nStart: %3"
		     "\nTimeout: %4")
		.arg( uid == -2 ?
		      i18n("console user") :
		      uid == -1 ?
		      i18n("control socket") :
		      KUser( uid ).loginName() )
		.arg( how == SHUT_HALT ?
		      i18n("turn off computer") :
		      i18n("restart computer") )
		.arg( strt ).arg( end )
		.arg( os ?
		      i18n("\nNext boot: %1").arg( QString::fromLocal8Bit( os ) ) :
		      QString::null );
	if (timeout != TO_INF)
		trg += i18n("\nAfter timeout: %1")
		       .arg( force == SHUT_FORCE ?
		             i18n("abort all sessions") :
		             force == SHUT_FORCEMY ?
		             i18n("abort own sessions") :
		             i18n("cancel shutdown") );
	lab = new QLabel( trg, this );
	box->addWidget( lab );
	complete( 0 );
}

#include "kdmshutdown.moc"
