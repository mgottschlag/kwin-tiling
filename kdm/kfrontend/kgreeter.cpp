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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include <config.h>

#include <qfile.h>
#include <qbitmap.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qaccel.h>
#include <qcursor.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kseparator.h>
#include <kapplication.h>

#include "kgreeter.h"
#include "kdmconfig.h"
#include "kdmclock.h"
#include "kfdialog.h"
#include "kdm_greet.h"

extern "C" {
#ifdef HAVE_XKB
// note: some XKBlib.h versions contain a global variable definition
// called "explicit". This keyword is not allowed on some C++ compilers so ->
# define explicit __explicit_dummy
# include <X11/XKBlib.h>
#endif
};

#include <sys/param.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>

KGreeter *kgreeter = 0;

void
KLoginLineEdit::focusOutEvent( QFocusEvent *e )
{
    emit lost_focus();
    QLineEdit::focusOutEvent( e );
}

class MyApp : public KApplication {

public:
    MyApp( int& argc, char** argv ) : KApplication( argc, argv, "kdmgreet" ) {};
    virtual bool x11EventFilter( XEvent * );
};

bool
MyApp::x11EventFilter( XEvent * ev )
{
    switch (ev->type) {
    case FocusIn:
    case FocusOut:
	// Hack to tell dialogs to take focus when the keyboard is grabbed
	ev->xfocus.mode = NotifyNormal;
	break;
    case ButtonPress:
    case ButtonRelease:
	// Hack to let the RMB work (nearly) as LMB
	if (ev->xbutton.button == 3)
	    ev->xbutton.button = 1;
	break;
    case KeyPress:
    case KeyRelease:
	if (kgreeter)
	    kgreeter->UpdateLock();
	break;
    }
    return false;
}

void KGreeter::keyPressEvent( QKeyEvent *e )
{
    if (!(~e->state() & (AltButton | ControlButton)) &&
	e->key() == Key_Delete && kdmcfg->_allowShutdown != SHUT_NONE) {
	shutdown_button_clicked();
	return;
    }
    FDialog::keyPressEvent( e );
}

#define CHECK_STRING( x) (x != 0 && x[0] != 0)

void
KGreeter::insertUsers( QIconView *iconview)
{
    QPixmap default_pix( locate("user_pic", QString::fromLatin1("default.png")));
    if( default_pix.isNull())
	LogError("Can't open default pixmap \"default.png\"\n");
    if( kdmcfg->_showUsers == SHOW_ALL ) {
	struct passwd *ps;
	for( setpwent(); (ps = getpwent()) != 0; ) {
	    // usernames are stored in the same encoding as files
	    QString username = QFile::decodeName ( ps->pw_name );
	    if( CHECK_STRING(ps->pw_dir) &&
		CHECK_STRING(ps->pw_shell) &&
		(ps->pw_uid >= (unsigned)kdmcfg->_lowUserId || 
		 username == "root") &&
		ps->pw_uid <= (unsigned)kdmcfg->_highUserId &&
        	!kdmcfg->_noUsers.contains( username )
	    ) {
		// we might have a real user, insert him/her
		QPixmap p( locate("user_pic",
				  username + QString::fromLatin1(".png")));
		if( p.isNull())
		    p = default_pix;
		QIconViewItem *item = new QIconViewItem( iconview, username, p);
		item->setDragEnabled(false);
	    }
	}
    } else {
	QStringList::ConstIterator it = kdmcfg->_users.begin();
	for( ; it != kdmcfg->_users.end(); ++it) {
	    if (getpwnam((*it).latin1())) {
		QPixmap p( locate("user_pic",
				  *it + QString::fromLatin1(".png")));
		if( p.isNull())
		    p = default_pix;
		QIconViewItem *item = new QIconViewItem( iconview, *it, p);
		item->setDragEnabled(false);
	    }
	}
    }
    endpwent();
    if (kdmcfg->_sortUsers)
        iconview->sort();
}

#undef CHECK_STRING

void
KGreeter::Inserten( QPopupMenu *mnu, const QString& txt, const char *member )
{
    mnu->insertItem( txt, this, member, QAccel::shortcutKey(txt) );
}

KGreeter::KGreeter()
  : FDialog( 0, 0, true )
  , user_view( 0 )
  , clock( 0 )
  , pixLabel( 0 )
  , capslocked( -1 )
  , loginfailed( false )
{
    QBoxLayout* vbox = new QBoxLayout( winFrame,
				       QBoxLayout::TopToBottom,
				       10, 10 );
    QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10 );
    QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10 );

    QGridLayout* grid = new QGridLayout( 5, 4, 5 );

    if (!kdmcfg->_greetString.isEmpty()) {
	QLabel* welcomeLabel = new QLabel( kdmcfg->_greetString, winFrame );
	welcomeLabel->setAlignment( AlignCenter );
	welcomeLabel->setFont( kdmcfg->_greetFont );
	vbox->addWidget( welcomeLabel );
    }
    if (kdmcfg->_showUsers != SHOW_NONE) {
	user_view = new QIconView( winFrame );
	user_view->setSelectionMode( QIconView::Single );
	user_view->setArrangement( QIconView::LeftToRight );
	user_view->setAutoArrange( true );
	user_view->setItemsMovable( false );
	user_view->setResizeMode( QIconView::Adjust );
	insertUsers( user_view );
	vbox->addWidget( user_view );
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

    passwdEdit = new KPasswordEdit( winFrame, "edit", kdmcfg->_echoMode );
    passwdLabel = new QLabel( passwdEdit, i18n("&Password:"), winFrame );

    sessargBox = new QComboBox( false, winFrame );
    sessargLabel = new QLabel( sessargBox, i18n("Session &type:"), winFrame );
    sessargBox->insertStringList( kdmcfg->_sessionTypes );
    sessargStat = new QWidget( winFrame );
    sasPrev = new QLabel( i18n("session type", "(previous)"), sessargStat );
    sasSel = new QLabel( i18n("session type", "(selected)"), sessargStat );
    sessargStat->setFixedSize(
	QMAX(sasPrev->sizeHint().width(), sasSel->sizeHint().width()),
	sessargBox->height() );

    vbox->addLayout( hbox1 );
    vbox->addLayout( hbox2 );
    hbox1->addLayout( grid, 3 );
    if (clock)
	hbox1->addWidget( clock, 0, AlignTop );
    else if (pixLabel)
	hbox1->addWidget( pixLabel, 0, AlignTop );

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
    grid->addMultiCellWidget( sep, 4,4, 0,3 );
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

    if (dhasConsole)
	Inserten( optMenu, i18n("Co&nsole Login"),
		  SLOT( console_button_clicked() ) );

//    Inserten( optMenu, i18n("&Remote Login"),
//	      SLOT( chooser_button_clicked() ) );

    Inserten( optMenu, disLocal ?
		       i18n("R&estart X Server") :
		       i18n("Clos&e Connection"),
	      SLOT(quit_button_clicked()) );

    menuButton = new QPushButton( i18n("&Menu"), winFrame );
    menuButton->setPopup( optMenu );
    hbox2->addWidget( menuButton );

    hbox2->addStretch( 1 );

    if (kdmcfg->_allowShutdown != SHUT_NONE) {
	shutdownButton = new QPushButton( i18n("&Shutdown..."), winFrame );
	connect( shutdownButton, SIGNAL(clicked()),
		 SLOT(shutdown_button_clicked()) );
	hbox2->addWidget( shutdownButton );
    }

    timer = new QTimer( this );
    // clear fields
    connect( timer, SIGNAL(timeout()), SLOT(timerDone()) );
    // update session type
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(load_wm()) );
    // start login timeout after entered login
    connect( loginEdit, SIGNAL(lost_focus()), SLOT(SetTimer()) );
    // update sessargStat
    connect( sessargBox, SIGNAL(activated(int)),
	     SLOT(slot_session_selected()) );
    if (user_view) {
	connect( user_view, SIGNAL(returnPressed(QIconViewItem*)),
		 SLOT(slot_user_name( QIconViewItem*)) );
	connect( user_view, SIGNAL(clicked(QIconViewItem*)),
		 SLOT(slot_user_name( QIconViewItem*)) );
    }

    reject();

    UpdateLock();

    stsfile = new KSimpleConfig( QString::fromLatin1(KDE_CONFDIR "/kdm/kdmsts") );
    stsfile->setGroup( "PrevUser" );
    enam = QString::fromLocal8Bit( dname );
    if (kdmcfg->_preselUser != PRESEL_PREV)
	stsfile->deleteEntry( enam, false );
    if (kdmcfg->_preselUser != PRESEL_NONE) {
	if (kdmcfg->_preselUser == PRESEL_PREV) {
	    loginEdit->setText( stsfile->readEntry( enam ) );
	} else
	    loginEdit->setText( kdmcfg->_defaultUser );
	if (kdmcfg->_focusPasswd && !loginEdit->text().isEmpty())
	    passwdEdit->setFocus();
	else
	    loginEdit->selectAll();
	load_wm();
    }
}

KGreeter::~KGreeter()
{
    delete stsfile;
}

void
KGreeter::slot_user_name( QIconViewItem *item )
{
    if (item) {
	loginEdit->setText( item->text() );
	passwdEdit->erase();
	passwdEdit->setFocus();
	load_wm();
	SetTimer();
    }
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
	failedLabel->setText( i18n("Warning: Caps locked") );
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
//    kapp->exit( ex_choose );
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
    int sum, len, i, num, dummy;
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
	    /* XXX - voodoo */
	    for (sum = 0, i = 0; i < len; i++)
		sum += (int)name[i] << ((i ^ sum) & 7);
	    sum ^= (sum >> 7);
	    /* forge a session with this hash - default more probable */
	    num = sessargBox->count();
	    i = sum % (num * 4 / 3);
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
		if (expire == 1)
		    MsgBox (infobox, i18n("Your account expires tomorrow."));
		else
		    MsgBox (infobox, 
			i18n("Your account expires in %1 days.").arg(expire));
		break;
	    case V_PWEXPIRE:
		expire = GRecvInt ();
		if (!expire)
		    MsgBox (infobox, i18n("Your password expires today."));
		if (expire == 1)
		    MsgBox (infobox, i18n("Your password expires tomorrow."));
		else
		    MsgBox (infobox, 
			i18n("Your password expires in %1 days.").arg(expire));
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
	    kapp->quit();
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
    } else
	verifyUser(true);
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
    kde_have_kipc = false;
#if QT_VERSION >= 300
    KApplication::disableAutoDcopRegistration();
#endif
    MyApp myapp( argc, argv );

    Display *dpy = qt_xdisplay();

    kdmcfg = new KDMConfig();

    KGlobal::dirs()->addResourceType( "user_pic",
				      KStandardDirs::kde_default("data") +
				      QString::fromLatin1("kdm/pics/users/") );


    myapp.setFont( kdmcfg->_normalFont );

#ifdef HAVE_XKBSETPERCLIENTCONTROLS
    //
    //  Activate the correct mapping for modifiers in XKB extension as
    //  grabbed keyboard has its own mapping by default
    //
    int opcode, evbase, errbase, majret, minret;
    unsigned int value = XkbPCF_GrabsUseXKBStateMask;
    if (XkbQueryExtension( dpy, &opcode, &evbase,
                           &errbase, &majret, &minret ))
        XkbSetPerClientControls( dpy, value, &value );
#endif
    setup_modifiers( dpy, kdmcfg->_numLockStatus );
    SecureDisplay( dpy );
    if (!dgrabServer) {
	GSendInt( G_SetupDpy );
	GRecvInt();
    }
    QDesktopWidget *dsk = kapp->desktop();
    QRect scr = dsk->screenGeometry(
	kdmcfg->_greeterScreen == -1 ?
	    dsk->screenNumber( QPoint( 0, 0 ) ) :
	    kdmcfg->_greeterScreen == -2 ?
		dsk->screenNumber( QPoint( dsk->width() - 1, 0 ) ) :
		kdmcfg->_greeterScreen );
    kgreeter = new KGreeter;
    kgreeter->setMaximumSize( scr.size() );
    kgreeter->move( -10000, -10000 );
    kgreeter->show();
    QRect grt( QPoint( 0, 0 ), kgreeter->sizeHint() );
    if (kdmcfg->_greeterPosX >= 0) {
	grt.moveCenter( QPoint( kdmcfg->_greeterPosX, kdmcfg->_greeterPosY ) );
	moveInto( grt, scr );
    } else {
	grt.moveCenter( scr.center() );
    }
    kgreeter->setGeometry( grt );
    if (dsk->screenNumber( QCursor::pos() ) !=
	dsk->screenNumber( grt.center() ))
	QCursor::setPos( grt.center() );
    XUndefineCursor( dpy, RootWindow( dpy, DefaultScreen( dpy ) ) );
    Debug ("entering event loop\n");
    kgreeter->exec();
    delete kgreeter;
    delete kdmcfg;
    UnsecureDisplay( dpy );
    restore_modifiers();
}

#include "kgreeter.moc"

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */

