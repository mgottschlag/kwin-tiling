    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000-2002 Oswald Buddenhagen <ossi@kde.org>


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

#include <config.h>

#include "kgreeter.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kdmclock.h"
#include "kchooser.h"
#include "kdm_greet.h"
#include "kdm_config.h"

#include <klocale.h>
#include <kstandarddirs.h>
#include <kseparator.h>
#include <kprocess.h>
#include <kpassdlg.h>
#include <klistview.h>
#include <ksimpleconfig.h>

#include <qfile.h>
#include <qimage.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qaccel.h>
#include <qcursor.h>
#include <qheader.h>
#include <qstyle.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

#include <sys/param.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

#include <X11/Xlib.h>

void
KLoginLineEdit::focusOutEvent( QFocusEvent *e )
{
    emit lost_focus();
    inherited::focusOutEvent( e );
}


class UserListView : public KListView {
public:
    UserListView( QWidget* parent = 0, const char *name = 0 )
	: KListView( parent, name )
	, cachedSizeHint( -1, 0 )
    {
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Ignored );
	header()->hide();
	addColumn( QString::null );
	setColumnAlignment( 0, AlignVCenter );
	setResizeMode( QListView::LastColumn );
    }

    mutable QSize cachedSizeHint;

protected:
    virtual QSize sizeHint() const
    {
	if (!cachedSizeHint.isValid()) {
	    constPolish();
	    uint maxw = 0;
	    for (QListViewItem *itm = firstChild(); itm; itm = itm->nextSibling()) {
		uint thisw = itm->width( fontMetrics(), this, 0 );
		if (thisw > maxw)
		    maxw = thisw;
	    }
	    cachedSizeHint.setWidth(
		style().pixelMetric( QStyle::PM_ScrollBarExtent ) +
		frameWidth() * 2 + maxw );
	}
	return cachedSizeHint;
    }
};


KGreeter::KGreeter()
  : inherited( 0, 0, true )
  , user_view( 0 )
  , clock( 0 )
  , pixLabel( 0 )
  , capslocked( -1 )
  , loginfailed( false )
{
    user_pic_dir = KGlobal::dirs()->resourceDirs( "data" ).last() +
				      QString::fromLatin1("kdm/pics/users/");	/* XXX standardize */

    QGridLayout* main_grid = new QGridLayout( winFrame, 4, 2, 10 );
    QBoxLayout* hbox1 = new QHBoxLayout( 10 );
    QBoxLayout* hbox2 = new QHBoxLayout( 10 );

    QGridLayout* grid = new QGridLayout( 5, 4, 5 );

    if (!kdmcfg->_greetString.isEmpty()) {
	QLabel* welcomeLabel = new QLabel( kdmcfg->_greetString, winFrame );
	welcomeLabel->setAlignment( AlignCenter );
	welcomeLabel->setFont( kdmcfg->_greetFont );
	main_grid->addWidget( welcomeLabel, 0, 1 );
    }
    if (kdmcfg->_showUsers != SHOW_NONE) {
	user_view = new UserListView( winFrame );
	insertUsers( user_view );
	main_grid->addMultiCellWidget(user_view, 0, 3, 0, 0);
	connect( user_view, SIGNAL(clicked( QListViewItem * )),
		 SLOT(slot_user_name( QListViewItem * )) );
	connect( user_view, SIGNAL(doubleClicked( QListViewItem * )),
		 SLOT(slot_user_doubleclicked()) );
    }

    switch (kdmcfg->_logoArea) {
	case LOGO_CLOCK:
	    clock = new KdmClock( winFrame, "clock" );
	    break;
	case LOGO_LOGO:
	    {
		QPixmap pixmap;
		if (pixmap.load( kdmcfg->_logo )) {
		    pixLabel = new QLabel( winFrame );
		    pixLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
		    pixLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken );
		    pixLabel->setAutoResize( true );
		    pixLabel->setIndent( 0 );
		    pixLabel->setPixmap( pixmap );
		}
	    }
	    break;
    }

    loginEdit = new KLoginLineEdit( winFrame );
    loginLabel = new QLabel( loginEdit, i18n("&Login:"), winFrame );
    // update session type
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(sel_user()) );
    // start login timeout after entered login
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(SetTimer()) );

    passwdEdit = new KPasswordEdit( winFrame, "edit", kdmcfg->_echoMode );
    passwdLabel = new QLabel( passwdEdit, i18n("&Password:"), winFrame );

    sessargBox = new QComboBox( false, winFrame );
    sessargLabel = new QLabel( sessargBox, i18n("Session &type:"), winFrame );
    sessargBox->insertStringList( kdmcfg->_sessionTypes );
    // update sessargStat
    connect( sessargBox, SIGNAL(activated(int)),
	     SLOT(slot_session_selected()) );
    sessargStat = new QWidget( winFrame );
    sasPrev = new QLabel( i18n("session type", "(previous)"), sessargStat );
    sasSel = new QLabel( i18n("session type", "(selected)"), sessargStat );
    sasPrev->setFixedSize( sasPrev->sizeHint() );
    sasSel->setFixedSize( sasSel->sizeHint() );
    sessargStat->setFixedSize(
	QMAX(sasPrev->size().width(), sasSel->size().width()),
	QMAX(sasPrev->size().height(), sasSel->size().height()) );

    main_grid->addItem( hbox1, 1, 1 );
    main_grid->addItem( grid, 2, 1 );
    main_grid->addItem( hbox2, 3, 1 );
    if (kdmcfg->_showUsers != SHOW_NONE) {
	if (clock)
	    hbox1->addWidget( clock, 0, AlignTop );
	else if (pixLabel)
	    hbox1->addWidget( pixLabel, 0, AlignTop );
    } else {
#if 0
	if (clock)
	    main_grid->addMultiCellWidget( clock, 0,3, 2,2, AlignCenter );
	else if (pixLabel)
	    main_grid->addMultiCellWidget( pixLabel, 0,3, 2,2, AlignCenter );
#else
	if (clock)
	    grid->addMultiCellWidget( clock, 0,3, 4,4, AlignTop );
	else if (pixLabel)
	    grid->addMultiCellWidget( pixLabel, 0,3, 4,4, AlignTop );
#endif
    }

    KSeparator* sep = new KSeparator( KSeparator::HLine, winFrame );

    failedLabel = new QLabel( winFrame );
    failedLabel->setFont( kdmcfg->_failFont );

    grid->addWidget( loginLabel, 0, 0 );
    grid->addMultiCellWidget( loginEdit, 0,0, 1,3 );
    grid->addWidget( passwdLabel, 1, 0 );
    grid->addMultiCellWidget( passwdEdit, 1,1, 1,3 );
    grid->addWidget( sessargLabel, 2, 0 );
    grid->addWidget( sessargBox, 2, 1 );
    grid->addWidget( sessargStat, 2, 2 );
    grid->addMultiCellWidget( failedLabel, 3,3, 0,3, AlignCenter );
#if 0
    grid->addMultiCellWidget( sep, 4,4, 0,3 );
#else
    grid->addMultiCellWidget( sep, 4,4, 0,
	(kdmcfg->_showUsers != SHOW_NONE || 
	 kdmcfg->_logoArea == LOGO_NONE) ? 3 : 4 );
#endif
    grid->setColStretch( 3, 1 );

    goButton = new QPushButton( i18n("G&o!"), winFrame );
    goButton->setFixedWidth( goButton->sizeHint().width() );
    goButton->setDefault( true );
    connect( goButton, SIGNAL( clicked()), SLOT(accept()) );
    hbox2->addWidget( goButton );

    clearButton = new QPushButton( i18n("&Clear"), winFrame );
    connect( clearButton, SIGNAL(clicked()), SLOT(reject()) );
    hbox2->addWidget( clearButton );

    hbox2->addStretch( 1 );

    optMenu = new QPopupMenu( winFrame );
    optMenu->setCheckable( false );

    Inserten( optMenu, disLocal ?
		       i18n("R&estart X Server") :
		       i18n("Clos&e Connection"),
	      SLOT(quit_button_clicked()) );

    if (disLocal && kdmcfg->_loginMode != LOGIN_LOCAL_ONLY)
	Inserten( optMenu, i18n("&Remote Login"),
		  SLOT( chooser_button_clicked() ) );

    if (dhasConsole)
	Inserten( optMenu, i18n("Co&nsole Login"),
		  SLOT( console_button_clicked() ) );

    if (kdmcfg->_allowShutdown != SHUT_NONE)
	Inserten( optMenu, i18n("&Shutdown..."),
		  SLOT(shutdown_button_clicked()) );

    menuButton = new QPushButton( i18n("&Menu"), winFrame );
    menuButton->setPopup( optMenu );
    hbox2->addWidget( menuButton );

    hbox2->addStretch( 1 );

//helpButton

#ifdef BUILTIN_XCONSOLE
    if (kdmcfg->_showLog) {
	consoleView = new KConsole( this, kdmcfg->_logSource );
	main_grid->addMultiCellWidget( consoleView, 4,4, 0,1 );
    }
#endif

    timer = new QTimer( this );
    // clear fields
    connect( timer, SIGNAL(timeout()), SLOT(timerDone()) );

    reject();

    UpdateLock();

    stsfile = new KSimpleConfig( QString::fromLatin1(KDE_CONFDIR "/kdm/kdmsts") ); // XXX get from kdm_config
    stsfile->setGroup( "PrevUser" );
    enam = QString::fromLocal8Bit( dname );
    if (kdmcfg->_preselUser != PRESEL_PREV)
	stsfile->deleteEntry( enam, false );
    if (kdmcfg->_preselUser != PRESEL_NONE) {
	if (kdmcfg->_preselUser == PRESEL_PREV)
	    loginEdit->setText( stsfile->readEntry( enam ) );
	else
	    loginEdit->setText( kdmcfg->_defaultUser );
	if (kdmcfg->_focusPasswd && !loginEdit->text().isEmpty())
	    passwdEdit->setFocus();
	else
	    loginEdit->selectAll();
	sel_user();
    }
}

KGreeter::~KGreeter()
{
    delete stsfile;
}

class UserListViewItem : public KListViewItem {
public:
    UserListViewItem( UserListView *parent, const QString &text,
		      const QPixmap &pixmap, const QString &username )
	: KListViewItem( parent )
	, login( username )
    {
	setPixmap( 0, pixmap );
	setMultiLinesEnabled( true );
	setText( 0, text );
	parent->cachedSizeHint.setWidth( -1 );
    }

    QString login;
};

void
KGreeter::insertUser( UserListView *listview, const QImage &default_pix,
		      const QString &username, struct passwd *ps )
{
    QImage p;
    if (kdmcfg->_faceSource != FACE_USER_ONLY &&
	kdmcfg->_faceSource != FACE_PREFER_USER)
	p = QImage( user_pic_dir + username + ".png" );
    if (p.isNull() && kdmcfg->_faceSource != FACE_ADMIN_ONLY) {
	// XXX remove seteuid-voodoo when we run as nobody
	seteuid( ps->pw_uid );
	p = QImage( QFile::decodeName( ps->pw_dir ) + "/.face.icon" );
	seteuid( 0 );
    }
    if (p.isNull() && kdmcfg->_faceSource != FACE_USER_ONLY)
	p = QImage( user_pic_dir + username + ".png" );
    if (p.isNull())
	p = default_pix;
    else
	p = p.smoothScale( 48, 48, QImage::ScaleMin );
    QString realname = QFile::decodeName( ps->pw_gecos );
    realname.truncate( realname.find( ',' ) );
    if (realname.isEmpty() || realname == username)
	new UserListViewItem( listview, username, QPixmap( p ), username );
    else {
	realname.append( "\n" ).append( username );
	new UserListViewItem( listview, realname, QPixmap( p ), username );
    }
}

void
KGreeter::insertUsers( UserListView *listview )
{
    QImage default_pix( user_pic_dir + QString::fromLatin1("default.png") );
    if (default_pix.isNull())
	LogError("Can't open default pixmap\n");
    default_pix = default_pix.smoothScale( 48, 48, QImage::ScaleMin );
    struct passwd *ps;
    if (kdmcfg->_showUsers == SHOW_ALL) {
 	QDict<int> users( 1000 );
	for (setpwent(); (ps = getpwent()) != 0;) {
	    // usernames are stored in the same encoding as files
	    QString username = QFile::decodeName( ps->pw_name );
	    if (*ps->pw_dir && *ps->pw_shell &&
		(ps->pw_uid >= (unsigned)kdmcfg->_lowUserId ||
		 !ps->pw_uid && kdmcfg->_showRoot) &&
		ps->pw_uid <= (unsigned)kdmcfg->_highUserId &&
		!kdmcfg->_noUsers.contains( username ) &&
		!users.find( username )
	    ) {
		// we might have a real user, insert him/her
		users.insert( username, (int *)-1 );
		insertUser( listview, default_pix, username, ps );
	    }
	}
    } else {
	QStringList::ConstIterator it = kdmcfg->_users.begin();
	for (; it != kdmcfg->_users.end(); ++it)
	    if ((ps = getpwnam( (*it).latin1() )))
		insertUser( listview, default_pix, *it, ps );
    }
    endpwent();
    if (kdmcfg->_sortUsers)
        listview->sort();
}

void
KGreeter::Inserten( QPopupMenu *mnu, const QString& txt, const char *member )
{
    mnu->insertItem( txt, this, member, QAccel::shortcutKey(txt) );
}

void KGreeter::keyPressEvent( QKeyEvent *e )
{
    if (!(~e->state() & (AltButton | ControlButton)) &&
	e->key() == Key_Delete && kdmcfg->_allowShutdown != SHUT_NONE) {
	shutdown_button_clicked();
	return;
    }
    inherited::keyPressEvent( e );
    UpdateLock();
}

void KGreeter::keyReleaseEvent( QKeyEvent *e )
{
    inherited::keyReleaseEvent( e );
    UpdateLock();
}

void
KGreeter::sel_user()
{
    if (user_view) {
	QString login = loginEdit->text();
	QListViewItem *item;
	for (item = user_view->firstChild(); item; item = item->nextSibling())
	    if (((UserListViewItem *)item)->login == login) {
		user_view->setCurrentItem( item );
		user_view->ensureItemVisible( item );
		break;
	    }
    }
    load_wm();
}

void
KGreeter::slot_user_name( QListViewItem *item )
{
    if (item) {
	loginEdit->setText( ((UserListViewItem *)item)->login );
	passwdEdit->erase();
	passwdEdit->setFocus();
	load_wm();
	SetTimer();
    }
}

void
KGreeter::slot_user_doubleclicked()
{
    verifyUser( true );
}

void
KGreeter::slot_session_selected()
{
    wmstat = WmSel;
    sasSel->show();
    sasPrev->hide();
}

void
KGreeter::UpdateLock()
{
    unsigned int lmask;
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    XQueryPointer( qt_xdisplay(), DefaultRootWindow( qt_xdisplay() ),
		   &dummy1, &dummy2, &dummy3, &dummy4, &dummy5, &dummy6,
		   &lmask );
    int nlock = lmask & LockMask;
    if (nlock != capslocked) {
	capslocked = nlock;
	if (!loginfailed)
	    updateStatus();
    }
}

void
KGreeter::updateStatus()
{
    if (loginfailed) {
	failedLabel->setPaletteForegroundColor( Qt::black );
	failedLabel->setText( i18n("Login failed") );
    } else if (capslocked) {
	failedLabel->setPaletteForegroundColor( Qt::red );
	failedLabel->setText( i18n("Warning: Caps Lock on") );
    } else
	failedLabel->clear();
}

void
KGreeter::SetTimer()
{
    if (!loginfailed && !loginEdit->text().isEmpty())
	timer->start( 40000, TRUE );
}

void
KGreeter::timerDone()
{
    if (loginfailed) {
	loginfailed = false;
	updateStatus();
	goButton->setEnabled( true );
	loginEdit->setEnabled( true );
	passwdEdit->setEnabled( true );
	sessargBox->setEnabled( true );
	if (!loginEdit->text().isEmpty()) {
	    passwdEdit->erase();
	    passwdEdit->setFocus();
	    SetTimer();
	    return;
	}
    }
    reject();
}

void
KGreeter::reject()
{
    if (!loginfailed) {
	timer->stop();
	loginEdit->setFocus();
    }
    loginEdit->clear();
    passwdEdit->erase();
    sasPrev->hide();
    sasSel->hide();
    wmstat = WmNone;
    set_wm( "default" );
}

void
KGreeter::quit_button_clicked()
{
    SessionExit( EX_RESERVER_DPY );
}

void
KGreeter::chooser_button_clicked()
{
    done( ex_choose );
}

void
KGreeter::console_button_clicked()
{
    SessionExit( EX_TEXTLOGIN );
}

void
KGreeter::shutdown_button_clicked()
{
    KDMShutdown k( winFrame );
    k.exec();
    UpdateLock();
}

void
KGreeter::set_wm( const char *cwm )
{
    QString wm = QString::fromLocal8Bit( cwm );
    for (int i = sessargBox->count(); i--;)
	if (sessargBox->text( i ) == wm) {
	    sessargBox->setCurrentItem( i );
	    return;
	}
}

void
KGreeter::load_wm()
{
    int len, i, b, num, dummy;
    unsigned long crc, by;
    QCString name;
    char **ptr;

    if (wmstat == WmSel)
	return;

    name = loginEdit->text().local8Bit();
    if (!(len = name.length())) {
	wmstat = WmNone;
	sasPrev->hide();
    } else {
	wmstat = WmPrev;
	sasPrev->show();
	GSendInt( G_GetSessArg );
	GSendStr( name.data() );
	ptr = GRecvStrArr( &dummy );
	if (!ptr) {		/* no such user */
	    /* simple crc32 */
	    for (crc = kdmcfg->_forgingSeed, i = 0; i < len; i++) {
		by = (crc & 255) ^ name[i];
		for (b = 0; b < 8; b++)
		    by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
		crc = (crc >> 8) ^ by;
	    }
	    /* forge a session with this hash - default more probable */
	    num = sessargBox->count();
	    i = crc % (num * 4 / 3);
	    if (i < num) {
		sessargBox->setCurrentItem( i );
		return;
	    }
	} else if (!ptr[0]) {	/* cannot read */
	    free( ptr );
	} else {
	    set_wm( ptr[0] );
	    for (i = 0; ptr[i]; i++)
		free( ptr[i] );
	    free( ptr );
	    return;
	}
    }
    set_wm( "default" );
}


#define errorbox QMessageBox::Critical
#define sorrybox QMessageBox::Warning
#define infobox QMessageBox::Information

bool
KGreeter::verifyUser(bool haveto)
{
    int ret, expire;
    char *msg;

    GSendInt (G_Verify);
    GSendStr (loginEdit->text().local8Bit());
    GSendStr (passwdEdit->password());
    ret = GRecvInt ();
    if (ret == V_OK) {
	GSendInt (G_Restrict);
	GSendStr (loginEdit->text().local8Bit());
	ret = GRecvInt ();
    }
    switch (ret) {
	case V_ERROR:
	    MsgBox (errorbox, i18n("A critical error occurred.\n"
			"Please look at KDM's logfile for more information\n"
			"or contact your system administrator."));
	    break;
	case V_NOHOME:
	    MsgBox (sorrybox, i18n("Home directory not available."));
	    break;
	case V_NOROOT:
	    MsgBox (sorrybox, i18n("Root logins are not allowed."));
	    break;
	case V_BADSHELL:
	    MsgBox (sorrybox, 
		    i18n("Your login shell is not listed in /etc/shells."));
	    break;
	case V_AEXPIRED:
	    MsgBox (sorrybox, i18n("Your account has expired."));
	    break;
	case V_PEXPIRED:
	    MsgBox (sorrybox, i18n("Your password has expired."));
	    break;
	case V_BADTIME:
	    MsgBox (sorrybox, i18n("You are not allowed to login\n"
					"at the moment."));
	    break;
	case V_NOLOGIN:
	    msg = GRecvStr();
	    {
		QFile f;
		f.setName(QString::fromLocal8Bit(msg));
		f.open(IO_ReadOnly);
	        QTextStream t( &f );
		QString mesg;
		while ( !t.eof() )
		    mesg += t.readLine() + '\n';
		f.close();

		if (mesg.isEmpty())
		    MsgBox( sorrybox, 
			    i18n("Logins are not allowed at the moment.\n"
					"Try again later.") );
		else
		    MsgBox( sorrybox, mesg );
	    }
	    free (msg);
	    break;
	case V_MSGERR:
	    msg = GRecvStr ();
	    MsgBox (sorrybox, QString::fromLocal8Bit (msg));
	    free (msg);
	    break;
	case V_AUTH:
	    if (!haveto)
		return false;
	    loginfailed = true;
	    updateStatus();
	    goButton->setEnabled( false);
	    loginEdit->setEnabled( false);
	    passwdEdit->setEnabled( false);
	    sessargBox->setEnabled( false);
	    timer->start( 1500 + kapp->random()/(RAND_MAX/1000), true );
	    return true;
	default:
	    switch (ret) {
	    default:
		LogPanic ("Unknown V_xxx code %d from core\n", ret);
	    case V_MSGINFO:
		msg = GRecvStr ();
		MsgBox (infobox, QString::fromLocal8Bit (msg));
		free (msg);
		break;
	    case V_AWEXPIRE:
		expire = GRecvInt ();
		if (!expire)
		    MsgBox (infobox, i18n("Your account expires today."));
		else
		    MsgBox (infobox, 
			i18n("Your account expires tomorrow.", "Your account expires in %n days.", expire));
		break;
	    case V_PWEXPIRE:
		expire = GRecvInt ();
		if (!expire)
		    MsgBox (infobox, i18n("Your password expires today."));
		else
		    MsgBox (infobox, 
			i18n("Your password expires tomorrow.", "Your password expires in %n days.", expire));
		break;
	    case V_OK:
		break;
	    }
	    hide();
	    GSendInt (G_Login);
	    GSendStr (loginEdit->text().local8Bit());
	    GSendStr (passwdEdit->password());
	    GSendInt (2);	//3
	    GSendStr (sessargBox->currentText().utf8());
	    // GSendStr (langBox->currentText().utf8());
	    GSendInt (0);
	    if (kdmcfg->_preselUser == PRESEL_PREV)
		stsfile->writeEntry (enam, loginEdit->text());
	    done( ex_exit );
	    return true;
    }
    reject();
    return true;
}

void
KGreeter::accept()
{
    if (loginEdit->hasFocus()) {
	load_wm();
//	if (!verifyUser(false))
	    passwdEdit->setFocus();
    } else if (user_view && user_view->hasFocus())
	slot_user_name( user_view->currentItem() );
    else
	verifyUser(true);
}

GreeterApp::GreeterApp( int& argc, char** argv )
    : inherited( argc, argv, "kdmgreet" )
{
    pingInterval = GetCfgInt( C_pingInterval );
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

void
GreeterApp::sigAlarm( int )
{
    ExitGreeter( 1 );
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

static void 
moveInto (QRect &what, const QRect &where)
{
    int di;

    if ((di = where.right() - what.right()) < 0)
	what.moveBy( di, 0);
    if ((di = where.left() - what.left()) > 0)
	what.moveBy( di, 0);
    if ((di = where.bottom() - what.bottom()) < 0)
	what.moveBy( 0, di);
    if ((di = where.top() - what.top()) > 0)
	what.moveBy( 0, di);
}

extern bool kde_have_kipc;

extern "C" void
kg_main( int argc, char **argv )
{
    KProcess *proc = 0;
    char *ppath = argv[0];

    kde_have_kipc = false;
    KApplication::disableAutoDcopRegistration();
    setenv( "KDE_DEBUG", "1", 1 );	// prevent KCrash installation
    GreeterApp app( argc, argv );

    Display *dpy = qt_xdisplay();

    kdmcfg = new KDMConfig();

    app.setFont( kdmcfg->_normalFont );

    setup_modifiers( dpy, kdmcfg->_numLockStatus );
    SecureDisplay( dpy );
    if (!dgrabServer) {
	if (GetCfgInt( C_UseBackground )) {
	    proc = new KProcess;
	    *proc << QCString( ppath, argv[0] - ppath + 1 ) + "krootimage";
	    char *conf = GetCfgStr( C_BackgroundCfg );
	    *proc << conf;
	    free( conf );
	    proc->start();
	}
	GSendInt( G_SetupDpy );
	GRecvInt();
    }

    GSendInt( G_Ready );

    QDesktopWidget *dsk = kapp->desktop();
    dsk->setCursor( Qt::ArrowCursor );

  redo:
    app.setOverrideCursor( Qt::WaitCursor );
    bool greet = GRecvInt() == G_Greet;	// alt: G_Choose

    QRect scr = dsk->screenGeometry(
	kdmcfg->_greeterScreen == -1 ?
	    dsk->screenNumber( QPoint( 0, 0 ) ) :
	    kdmcfg->_greeterScreen == -2 ?
		dsk->screenNumber( QPoint( dsk->width() - 1, 0 ) ) :
		kdmcfg->_greeterScreen );
    FDialog *dialog;
    if (greet)
	dialog = new KGreeter;
    else
	dialog = new ChooserDlg;
    dialog->setMaximumSize( scr.size() * .9 );
    dialog->adjustSize();
    QRect grt( 0, 0, dialog->width(), dialog->height() );
    if (kdmcfg->_greeterPosX >= 0) {
	grt.moveCenter( QPoint( kdmcfg->_greeterPosX, kdmcfg->_greeterPosY ) );
	moveInto( grt, scr );
    } else {
	grt.moveCenter( scr.center() );
    }
    dialog->setGeometry( grt );
    if (dsk->screenNumber( QCursor::pos() ) !=
	dsk->screenNumber( grt.center() ))
	QCursor::setPos( grt.center() );
    if (!greet) {
	GSendInt (G_Ready);	/* tell chooser to go into async mode */
	GRecvInt ();		/* ack */
    }
    app.restoreOverrideCursor();
    Debug ("entering event loop\n");
    int rslt = dialog->exec();
    Debug ("left event loop\n");
    delete dialog;
    switch (rslt) {
    case ex_greet:
	GSendInt (G_DGreet);
	goto redo;
    case ex_choose:
	GSendInt (G_DChoose);
	goto redo;
    default:
	break;
    }

    delete proc;
    UnsecureDisplay( dpy );
    restore_modifiers();

    delete kdmcfg;
}


#include "kgreeter.moc"

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */
