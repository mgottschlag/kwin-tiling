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

#include <qdir.h>
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
  , nnormals( 0 )
  , nspecials( 0 )
  , curprev( -1 )
  , cursel( -1 )
  , prevvalid( true )
  , capslocked( -1 )
  , loginfailed( false )
{
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
    loginLabel = new QLabel( loginEdit, i18n("&Username:"), winFrame );
    // update session type
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(sel_user()) );
    // start login timeout after entered login
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(SetTimer()) );

    passwdEdit = new KPasswordEdit( winFrame, "edit", kdmcfg->_echoMode );
    passwdLabel = new QLabel( passwdEdit, i18n("&Password:"), winFrame );

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
    grid->addMultiCellWidget( failedLabel, 3,3, 0,3, AlignCenter );
#if 0
    grid->addMultiCellWidget( sep, 4,4, 0,3 );
#else
    grid->addMultiCellWidget( sep, 4,4, 0,
	(kdmcfg->_showUsers != SHOW_NONE || 
	 kdmcfg->_logoArea == LOGO_NONE) ? 3 : 4 );
#endif
    grid->setColStretch( 3, 1 );

    goButton = new QPushButton( i18n("L&ogin"), winFrame );
    goButton->setFixedWidth( goButton->sizeHint().width() );
    goButton->setDefault( true );
    connect( goButton, SIGNAL( clicked()), SLOT(accept()) );
    hbox2->addWidget( goButton );

    clearButton = new QPushButton( i18n("&Clear"), winFrame );
    connect( clearButton, SIGNAL(clicked()), SLOT(reject()) );
    hbox2->addWidget( clearButton );

    hbox2->addStretch( 1 );

    sessMenu = new QPopupMenu( winFrame );
    connect( sessMenu, SIGNAL(activated(int)),
	     SLOT(slot_session_selected(int)) );
    insertSessions();

    optMenu = new QPopupMenu( winFrame );
    optMenu->setCheckable( false );

    Inserten( i18n("Session &Type"), sessMenu );

    optMenu->insertSeparator();

    Inserten( disLocal ? i18n("R&estart X Server") : i18n("Clos&e Connection"),
	      SLOT(quit_button_clicked()) );

    if (disLocal && kdmcfg->_loginMode != LOGIN_LOCAL_ONLY)
	Inserten( i18n("&Remote Login"),
		  SLOT( chooser_button_clicked() ) );

    if (dhasConsole)
	Inserten( i18n("Co&nsole Login"),
		  SLOT( console_button_clicked() ) );

    if (kdmcfg->_allowShutdown != SHUT_NONE)
	Inserten( i18n("&Shutdown..."),
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

    loginEdit->setFocus();

    UpdateLock();

    stsfile = new KSimpleConfig( kdmcfg->_stsFile );
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
	QTimer::singleShot( 0, this, SLOT(sel_user()) );
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
    int dp = 0, nd = 0;
    if (kdmcfg->_faceSource == FACE_USER_ONLY ||
	kdmcfg->_faceSource == FACE_PREFER_USER)
	dp = 1;
    if (kdmcfg->_faceSource != FACE_USER_ONLY &&
	kdmcfg->_faceSource != FACE_ADMIN_ONLY)
	nd = 1;
    QImage p;
    // XXX remove seteuid-voodoo when we run as nobody
    seteuid( ps->pw_uid );
    do {
	QString fn = dp ?
		     QFile::decodeName( ps->pw_dir ) + "/.face" :
		     kdmcfg->_faceDir + '/' + username + ".face";
	if (p.load( fn + ".icon" ) || p.load( fn )) {
	    QSize ns( 48, 48 );
	    if (p.size() != ns)
		p = p.convertDepth( 32 ).smoothScale( ns, QImage::ScaleMin );
	    goto gotit;
	}
	dp = 1 - dp;
    } while (--nd >= 0);
    p = default_pix;
  gotit:
    seteuid( 0 );
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
    QImage default_pix;
    if (!default_pix.load( kdmcfg->_faceDir + "/.default.face.icon" ))
	if (!default_pix.load( kdmcfg->_faceDir + "/.default.face" ))
	    LogError("Can't open default user face\n");
    QSize ns( 48, 48 );
    if (default_pix.size() != ns)
	default_pix =
	  default_pix.convertDepth( 32 ).smoothScale( ns, QImage::ScaleMin );
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
KGreeter::putSession(const QString &type, const QString &name, bool hid, const char *exe)
{
    int prio = exe ? (!strcmp( exe, "default" ) ? 0 :
			!strcmp( exe, "custom" ) ? 1 :
			  !strcmp( exe, "failsafe" ) ? 3 : 2) : 2;
    for (uint i = 0; i < sessionTypes.size(); i++)
	if (sessionTypes[i].type == type) {
	    sessionTypes[i].prio = prio;
	    return;
	}
    sessionTypes.append( SessType( name, type, hid, prio ) );
}

void
KGreeter::insertSessions()
{
    QString defsess = i18n("Default"), custsess = i18n("Custom"),
	    safesess = i18n("Failsafe");
    for (QStringList::ConstIterator dit = kdmcfg->_sessionsDirs.begin();
	 dit != kdmcfg->_sessionsDirs.end(); ++dit)
    {
	QStringList ents = QDir( *dit ).entryList();
	for (QStringList::ConstIterator it = ents.begin(); it != ents.end(); ++it)
	    if ((*it).endsWith( ".desktop" )) {
		KSimpleConfig dsk( QString( *dit ).append( '/' ).append( *it ) );
		dsk.setGroup( "Desktop Entry" );
		putSession( (*it).left( (*it).length() - 8 ),
			    dsk.readEntry( "Name" ),
			    (dsk.readBoolEntry( "Hidden", false ) ||
			     (dsk.hasKey( "TryExec" ) &&
			      KStandardDirs::findExe( dsk.readEntry( "TryExec" ) ).isEmpty())),
			    dsk.readEntry( "Exec" ).latin1() );
	    }
    }
    putSession( "default", i18n("Default"), false, "default" );
    putSession( "custom", i18n("Custom"), false, "custom" );
    putSession( "failsafe", i18n("Failsafe"), false, "failsafe" );
    qBubbleSort( sessionTypes );
    for (uint i = 0; i < sessionTypes.size() && !sessionTypes[i].hid; i++) {
	sessMenu->insertItem( sessionTypes[i].name, i );
	switch (sessionTypes[i].prio) {
	case 0: case 1: nspecials++; break;
	case 2: nnormals++; break;
	}
    }
}

void
KGreeter::Inserten( const QString& txt, const char *member )
{
    optMenu->insertItem( txt, this, member, QAccel::shortcutKey( txt ) );
}

void
KGreeter::Inserten( const QString& txt, QPopupMenu *cmnu )
{
    int id = optMenu->insertItem( txt, cmnu );
    optMenu->setAccel( QAccel::shortcutKey( txt ), id );
    optMenu->connectItem( id, this, SLOT(slotActivateMenu( int )) );
    optMenu->setItemParameter( id, id );
}

void
KGreeter::slotActivateMenu( int id )
{
    QPopupMenu *cmnu = optMenu->findItem( id )->popup();
    QSize sh( cmnu->sizeHint() / 2 );
    cmnu->exec( geometry().center() - QPoint( sh.width(), sh.height() ) );
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
KGreeter::slot_session_selected( int id )
{
    if (id != cursel) {
	sessMenu->setItemChecked( cursel, false );
	sessMenu->setItemChecked( id, true );
	cursel = id;
    }
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
    load_wm();
    slot_session_selected( -1 );
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
KGreeter::set_wm( int wm )
{
    if (curprev != wm) {
	if (curprev != -1)
	    sessMenu->changeItem( curprev, sessionTypes[curprev].name );
	if (wm != -1)
	    sessMenu->changeItem( wm, sessionTypes[wm].name + i18n(" (previous)") );
	curprev = wm;
    }
}

void
KGreeter::load_wm()
{
    int len, i, b;
    unsigned long crc, by;
    QCString name;
    char *sess;

    prevvalid = true;
    name = loginEdit->text().local8Bit();
    GSendInt( G_ReadDmrc );
    GSendStr( name.data() );
    GRecvInt(); // ignore status code ...
    if ((len = name.length())) {
	GSendInt( G_GetDmrc );
	GSendStr( "Session" );
	sess = GRecvStr();
	if (!sess) {		/* no such user */
	    if (kdmcfg->_showUsers == SHOW_NONE) { // don't fake if user list shown
		/* simple crc32 */
		for (crc = kdmcfg->_forgingSeed, i = 0; i < len; i++) {
		    by = (crc & 255) ^ name[i];
		    for (b = 0; b < 8; b++)
			by = (by >> 1) ^ (-(by & 1) & 0xedb88320);
		    crc = (crc >> 8) ^ by;
		}
		/* forge a session with this hash - default & custom more probable */
		/* XXX - this should do a statistical analysis of the real users */
#if 1
		set_wm( crc % (nspecials * 2 + nnormals) % (nspecials + nnormals) );
#else
		i = crc % (nspecials * 2 + nnormals);
		if (i < nnormals)
		    set_wm( i + nspecials );
		else
		    set_wm( (i - nnormals) / 2 );
#endif
		return;
	    }
	} else {
	    for (uint i = 0; i < sessionTypes.count(); i++)
		if (sessionTypes[i].type == sess) {
		    free( sess );
		    set_wm( i );
		    return;
		}
	    if (cursel == -1)
		MsgBox( sorrybox, i18n("Your saved session type '%1' is not valid any more.\n"
		    "Please select a new one, otherwise 'default' will be used.").arg(sess) );
	    free( sess );
	    prevvalid = false;
	}
    }
    set_wm( -1 );
}


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
	    goButton->setEnabled( false );
	    loginEdit->setEnabled( false );
	    passwdEdit->setEnabled( false );
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
	    if (cursel != -1) {
		GSendInt (G_PutDmrc);
		GSendStr ("Session");
		GSendStr (sessionTypes[cursel].type.utf8());
	    } else if (!prevvalid) {
		GSendInt (G_PutDmrc);
		GSendStr ("Session");
		GSendStr ("default");
	    }
	    GSendInt (G_Login);
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
    QRect grt( dialog->rect() );
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
