    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000 Oswald Buddenhagen <ossi@kde.org>


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

#include <strings.h>
#include "kgreeter.h"
#include "kdmconfig.h"
#include "kdmclock.h" 

#include <sys/types.h>
#include <sys/param.h>

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <qfile.h>
#include <qbitmap.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qstring.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include <X11/Xmd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "dm.h"
#include "dm_error.h"
#include "greet.h"
#include "miscfunc.h"

#ifdef GREET_LIB
extern "C" {
/*
 * Function pointers filled in by the initial call into the library
 */
int	(*__xdm_PingServer)(struct display *d, Display *alternateDpy) = NULL;
void	(*__xdm_SessionPingFailed)(struct display *d) = NULL;
void	(*__xdm_Debug)(const char *fmt, ...) = NULL;
void	(*__xdm_RegisterCloseOnFork)(int fd) = NULL;
void	(*__xdm_SecureDisplay)(struct display *d, Display *dpy) = NULL;
void	(*__xdm_UnsecureDisplay)(struct display *d, Display *dpy) = NULL;
void	(*__xdm_ClearCloseOnFork)(int fd) = NULL;
void	(*__xdm_SetupDisplay)(struct display *d) = NULL;
void	(*__xdm_LogError)(const char *fmt, ...) = NULL;
void	(*__xdm_SessionExit)(struct display *d, int status, int removeAuth) = NULL;
void	(*__xdm_DeleteXloginResources)(struct display *d, Display *dpy) = NULL;
int	(*__xdm_source)(char **environ, char *file) = NULL;
char	**(*__xdm_defaultEnv)(void) = NULL;
char	**(*__xdm_setEnv)(char **e, char *name, char *value) = NULL;
char	**(*__xdm_putEnv)(const char *string, char **env) = NULL;
char	**(*__xdm_parseArgs)(char **argv, char *string) = NULL;
void	(*__xdm_printEnv)(char **e) = NULL;
char	**(*__xdm_systemEnv)(struct display *d, char *user, char *home) = NULL;
void	(*__xdm_LogOutOfMem)(const char *fmt, ...) = NULL;
SETGRENT_TYPE	(*__xdm_setgrent)(void) = NULL;
struct group	*(*__xdm_getgrent)(void) = NULL;
void	(*__xdm_endgrent)(void) = NULL;
#ifdef USESHADOW
struct spwd	*(*__xdm_getspnam)(GETSPNAM_ARGS) = NULL;
void	(*__xdm_endspent)(void) = NULL;
#endif
struct passwd	*(*__xdm_getpwnam)(GETPWNAM_ARGS) = NULL;
#ifdef __linux__
void	(*__xdm_endpwent)(void) = NULL;
#endif
char	*(*__xdm_crypt)(CRYPT_ARGS) = NULL;
#ifdef USE_PAM
pam_handle_t	**(*__xdm_thepamh)(void) = NULL;
#endif
}
#endif

#ifdef SECURE_RPC
# include <rpc/rpc.h>
# include <rpc/key_prot.h>
#endif

#ifdef K5AUTH
# include <krb5/krb5.h>
#endif

#define ex_login 0
#define ex_choose 1

static const char *description = 
	I18N_NOOP("KDE Display Manager");

static const char *version = "v0.90";


// Global vars
KGreeter *kgreeter = 0;
KDMConfig *kdmcfg = 0;

struct display		*d;
Display			**dpy;
struct verify_info	*verify;
struct greet_info	*greet;

char name[F_LEN], password[F_LEN], sessarg[F_LEN];

void
KLoginLineEdit::focusOutEvent( QFocusEvent *e)
{
    emit lost_focus();
    QLineEdit::focusOutEvent( e);
}

class MyApp:public KApplication {

public:
    virtual bool x11EventFilter( XEvent * );
};

bool 
MyApp::x11EventFilter( XEvent * ev)
{
    // Hack to tell dialogs to take focus 
    if( ev->type == ConfigureNotify) {
	QWidget* target = QWidget::find( (( XConfigureEvent *) ev)->window);
	if (target) {
	    target = target->topLevelWidget();
	    if( target->isVisible() && !target->isPopup())
		XSetInputFocus( qt_xdisplay(), target->winId(), 
				RevertToParent, CurrentTime);
	}
    }
    return FALSE;
}

static void
TempUngrab_Run(void (*func)(void *), void *ptr)
{
    XUngrabKeyboard(qt_xdisplay(), CurrentTime);
    func(ptr);
    // Secure the keyboard again
    if (XGrabKeyboard (qt_xdisplay(), 
		       kgreeter ? kgreeter->winId() : DefaultRootWindow (qt_xdisplay()), 
		       True, GrabModeAsync, GrabModeAsync, CurrentTime) 
	!= GrabSuccess
    ) {
	LogError ("WARNING: keyboard on display %s could not be secured\n",
		  ::d->name);
	SessionExit (::d, RESERVER_DISPLAY, FALSE);	 
    }
}

void KGreeter::keyPressEvent( QKeyEvent *e )
{
    if ( e->state() == 0 || ( e->state() & Keypad && e->key() == Key_Enter ) ) {
	switch ( e->key() ) {
	    case Key_Enter:
	    case Key_Return:
		ReturnPressed();
		return;
	    case Key_Escape:
		clear_button_clicked();
		return;
	}
    }
    e->ignore();
}

#define CHECK_STRING( x) (x != 0 && x[0] != 0)

void
KGreeter::insertUsers( QIconView *iconview)
{
    QPixmap default_pix( locate("user_pic", QString::fromLatin1("default.png")));
    if( default_pix.isNull())
	LogError("Can't get default pixmap from \"default.png\"\n");

    if( kdmcfg->_showUsers == KDMConfig::UsrAll ) {
	struct passwd *ps;
	for( setpwent(); (ps = getpwent()) != 0; ) {
	    // usernames are stored in the same encoding as files
	    QString username = QFile::decodeName ( ps->pw_name );
	    if( CHECK_STRING(ps->pw_dir) &&
		CHECK_STRING(ps->pw_shell) &&
		(ps->pw_uid >= (unsigned)kdmcfg->_lowUserId || 
		ps->pw_uid == 0) &&
                ( kdmcfg->_noUsers.contains( username ) == 0)
	    ) {
		// we might have a real user, insert him/her
		QPixmap p( locate("user_pic",
				  username + QString::fromLatin1(".png")));
		if( p.isNull())
		    p = default_pix;
		    QIconViewItem *item = new QIconViewItem( iconview, 
							     username, p);
		    item->setDragEnabled(false);
	    }
	}
	endpwent();
    } else {
	QStringList::ConstIterator it = kdmcfg->_users.begin();
	for( ; it != kdmcfg->_users.end(); ++it) {
	    QPixmap p( locate("user_pic",
			      *it + QString::fromLatin1(".png")));
	    if( p.isNull())
		p = default_pix;
	    QIconViewItem *item = new QIconViewItem( iconview, *it, p);
	    item->setDragEnabled(false);
	}
    }
    if( kdmcfg->_sortUsers)
        iconview->sort();
}

#undef CHECK_STRING

KGreeter::KGreeter(QWidget *parent, const char *t) 
  : QFrame( parent, t, WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
{
    setFrameStyle(QFrame::WinPanel| QFrame::Raised);
    QBoxLayout* vbox = new QBoxLayout(  this, 
					QBoxLayout::TopToBottom, 
					10, 10);
    QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
    QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10);

    QGridLayout* grid = new QGridLayout( 5, 2, 5);

    QLabel* welcomeLabel = new QLabel( kdmcfg->_greetString, this);
    welcomeLabel->setAlignment(AlignCenter);
    welcomeLabel->setFont( *kdmcfg->_greetFont);
    vbox->addWidget( welcomeLabel);
    if( kdmcfg->_showUsers != KDMConfig::UsrNone) {
	user_view = new QIconView( this);
	user_view->setSelectionMode( QIconView::Single );
	user_view->setArrangement( QIconView::LeftToRight);
	user_view->setAutoArrange(true);
	user_view->setItemsMovable(false);
	user_view->setResizeMode(QIconView::Adjust);
	insertUsers( user_view);
	vbox->addWidget( user_view);
    } else {
	user_view = NULL;
    }

    pixLabel = 0;
    clock    = 0;

    switch( kdmcfg->_logoArea ) {
	case KDMConfig::KdmClock:
	    clock = new KdmClock( this, "clock" );
	    break;
	case KDMConfig::KdmLogo:
	    {
		QPixmap pixmap;
		if ( pixmap.load( kdmcfg->_logo ) ) {
		    pixLabel = new QLabel( this);
		    pixLabel->setFrameStyle( QFrame::Panel | QFrame::Sunken);
		    pixLabel->setAutoResize( true);
		    pixLabel->setIndent(0);
		    pixLabel->setPixmap( pixmap);
		}
	    }
	    break;
	case KDMConfig::KdmNone:
	    break;
    }

    // The line-edit look _very_ bad if you don't give them 
    // a resonal height that observes a proportional aspect.
    // -- Bernd
     
    // I still think a line-edit show decide it's own height
    // -- Steffen

    loginLabel = new QLabel( i18n("Login:"), this);
    loginEdit = new KLoginLineEdit( this);
    loginLabel->setBuddy(loginEdit);
    QString enam = QString::fromLatin1("LastUser_") + 
		   QString::fromLatin1(::d->name);
    loginEdit->setFocus();

    passwdLabel = new QLabel( i18n("Password:"), this);
    passwdEdit = new KPasswordEdit( this, "edit", kdmcfg->_echoMode);
    passwdLabel->setBuddy(passwdEdit);

    sessionargLabel = new QLabel(i18n("Session Type:"), this);
    sessionargBox = new QComboBox( false, this);
    sessionargLabel->setBuddy(sessionargBox);
    sessionargBox->insertStringList( kdmcfg->_sessionTypes );

    vbox->addLayout( hbox1);
    vbox->addLayout( hbox2);
    hbox1->addLayout( grid, 3);
    if (clock)
	hbox1->addWidget( (QWidget*)clock, 0, AlignTop);
    else if (pixLabel)
	hbox1->addWidget( (QWidget*)pixLabel, 0, AlignTop);

    QFrame* sepFrame = new QFrame( this);
    sepFrame->setFrameStyle( QFrame::HLine | QFrame::Sunken);
    sepFrame->setFixedHeight( sepFrame->sizeHint().height());

    failedLabel = new QLabel( this);
    failedLabel->setFont( *kdmcfg->_failFont);

    grid->addWidget( loginLabel , 0, 0);
    grid->addWidget( loginEdit  , 0, 1);
    grid->addWidget( passwdLabel, 1, 0);
    grid->addWidget( passwdEdit , 1, 1);
    grid->addWidget( sessionargLabel, 2, 0);
    grid->addWidget( sessionargBox  , 2, 1);
    grid->addMultiCellWidget( failedLabel, 3, 3, 0, 1, AlignCenter);
    grid->addMultiCellWidget( sepFrame, 4, 4, 0, 1);
    grid->setColStretch( 1, 4);

    goButton = new QPushButton( i18n("G&o!"), this);
    connect( goButton, SIGNAL( clicked()), SLOT(go_button_clicked()));

    hbox2->addWidget( goButton, AlignBottom);

    clearButton = new QPushButton( i18n("&Clear"), this);
    connect( clearButton, SIGNAL(clicked()), SLOT(clear_button_clicked()));
    //set_fixed( clearButton);
    hbox2->addWidget( clearButton, AlignBottom);

    if (kdmcfg->_allowChooser) {
	chooserButton = new QPushButton( i18n("C&hooser"), this);
	connect( chooserButton, SIGNAL(clicked()), SLOT(chooser_button_clicked()));
	//set_fixed( chooserButton);
	hbox2->addWidget( chooserButton, AlignBottom);
    }

    quitButton = new QPushButton( ::d->displayType.location == Local ? i18n("R&estart X Server") : i18n("Clos&e Connection"), this);
    connect( quitButton, SIGNAL(clicked()), SLOT(quit_button_clicked()));
    //set_fixed( quitButton);
    hbox2->addWidget( quitButton, AlignBottom);

    int sbw;
    if( kdmcfg->_shutdownButton != KDMConfig::SdNone 
	&& ( kdmcfg->_shutdownButton != KDMConfig::SdConsoleOnly 
	     || ::d->displayType.location == Local)
    ) {
	  
	shutdownButton = new QPushButton(i18n("&Shutdown..."), this);

	connect( shutdownButton, SIGNAL(clicked()), 
		 SLOT(shutdown_button_clicked()));
	//set_fixed( shutdownButton);
	hbox2->addWidget( shutdownButton, AlignBottom);
	sbw = shutdownButton->width();
    } else {
	sbw = 0;
    }

    if (kdmcfg->_showPrevious) {
	loginEdit->setText (kdmcfg->readEntry (enam));
	loginEdit->selectAll();
	load_wm();
    } else
	kdmcfg->deleteEntry (enam, false);

    //vbox->activate();
    timer = new QTimer( this );
    //// Signals/Slots
    // Timer for failed login
    connect( timer, SIGNAL(timeout()),
	     this , SLOT(timerDone()) );
     // Signal to update session type
    connect( loginEdit, SIGNAL(lost_focus()),
	     this, SLOT( load_wm()));
    if( user_view) {
	connect( user_view, SIGNAL(returnPressed(QIconViewItem*)), 
		 this, SLOT(slot_user_name( QIconViewItem*)));
	connect( user_view, SIGNAL(clicked(QIconViewItem*)), 
		 this, SLOT(slot_user_name( QIconViewItem*)));
    }

    srand(time(0) + (unsigned)kdmcfg);	/* random enough? call core fkt? */
}

void 
KGreeter::slot_user_name( QIconViewItem *item)
{
    if( item != 0) {
	loginEdit->setText( item->text());
	passwdEdit->erase();
	passwdEdit->setFocus();
	load_wm();
    }
}

void 
KGreeter::SetTimer()
{
    if (failedLabel->text().isNull())
	timer->start( 40000, TRUE );
}

void 
KGreeter::timerDone()
{
    if (failedLabel->isVisible()) {
	failedLabel->setText(QString::null);
	goButton->setEnabled( true);
	loginEdit->setEnabled( true);
        passwdEdit->setEnabled( true);
        loginEdit->setFocus();
    }
}

void 
KGreeter::clear_button_clicked()
{
    loginEdit->clear();
    passwdEdit->erase();
    loginEdit->setFocus();
}

void
KGreeter::quit_button_clicked()
{
    QApplication::flushX();
    SessionExit(::d, RESERVER_DISPLAY, FALSE);
}

void
KGreeter::chooser_button_clicked()
{
    qApp->exit( ex_choose );
}

static void
do_shutdown(void *ptr)
{
    KDMShutdown k( kdmcfg->_shutdownButton,
		   (KGreeter *)ptr, "Shutdown",
		   kdmcfg->_shutdown, 
		   kdmcfg->_restart,
#ifndef BSD
		   kdmcfg->_consoleMode,
#endif
		   kdmcfg->_useLilo,
		   kdmcfg->_liloCmd,
		   kdmcfg->_liloMap);
    k.exec();
}

void
KGreeter::shutdown_button_clicked()
{
    timer->stop();
    TempUngrab_Run(do_shutdown, (void *)this);
    SetTimer();
}

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)

void
KGreeter::save_wm()
{
    rdwr_wm ((char*)sessionargBox->currentText().latin1(), 0, 
	     loginEdit->text().latin1(), 0);
}

void
KGreeter::load_wm()
{
    char buf[256];
    const char *name;
    int sum, len, i, num;

    num = sessionargBox->count();
    name = loginEdit->text().latin1();
    switch (rdwr_wm (buf, 256, name, 1)) {
	case -2:	/* no such user */
	    for (sum = 0, i = 0, len = strlen(name); i < len; i++)
		sum += (int)name[i] << ((i ^ sum) & 7);	/* voodoo */
	    i = (sum ^ (sum >> 7)) % (num * 4 / 3);
	    if (i < num) {
		sessionargBox->setCurrentItem(i);
		return;
	    }
	    /* fall through */
	case -1:	/* cannot read */
	    strcpy (buf, "default");
	    break;
    }

    for (i = 0; i < num; i++)
	if (sessionargBox->text(i) == QString::fromLatin1 (buf)) {
	    sessionargBox->setCurrentItem(i);
	    return;
	}
}

#else

void
KGreeter::load_wm()
{
}

void
KGreeter::save_wm()
{
}

#endif /* HAVE_INITGROUPS && HAVE_SETGROUPS && HAVE_GETGROUPS */

QString errm;

static void
errorbox(void *)
{
    KMessageBox::error(0, errm);
}

static void
sorrybox(void *)
{
    KMessageBox::sorry(0, errm);
}

static void
infobox(void *)
{
    KMessageBox::information(0, errm);
}

void 
KGreeter::go_button_clicked()
{
    time_t expire;
    char *nologin;
    QDateTime dat;

    /* NOTE: name/password are static -> all zero -> terminator present */
    greet->name = ::name;
    strncpy( ::name, loginEdit->text().latin1(), F_LEN - 1 );
    greet->password = ::password;
    strncpy( ::password, passwdEdit->password(), F_LEN - 1 );
    greet->string = ::sessarg;
    strncpy( ::sessarg, sessionargBox->currentText().latin1(), F_LEN - 1 );

    switch (MyVerify (::d, greet, verify, &expire, &nologin)) {
	case V_ERROR:
	    errm = i18n("Could not access the login capabilities database or out of memory.");
	    TempUngrab_Run(errorbox, 0);
	    break;
	case V_NOHOME:
	    errm = i18n("Home directory not available.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_NOROOT:
	    errm = i18n("Root logins are not allowed.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_BADSHELL:
	    errm = i18n("Your login shell is not listed in /etc/shells.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_AEXPIRED:
	    errm = i18n("Your account has expired.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_PEXPIRED:
	    errm = i18n("Your password has expired.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_BADTIME:
	    errm = i18n("You are not allowed to login at the moment.");
	    TempUngrab_Run(sorrybox, 0);
	    break;
	case V_NOLOGIN: {
	    QFile f;
	    f.setName(QString::fromLatin1(nologin));
	    f.open(IO_ReadOnly);
	    QTextStream t( &f ); 
	    errm = "";
	    while ( !t.eof() )
		errm += t.readLine() + '\n';
	    f.close();

	    if (errm.isEmpty())
		errm = i18n("Logins are not allowed at the moment.\n"
			 "Try again later.");
	    TempUngrab_Run(sorrybox, 0);
	    } break;
	case V_AWEXPIRE:
	    dat.setTime_t(expire);
	    errm = i18n("Your account expires on %1.")
			.arg(KGlobal::locale()->formatDateTime(dat));
	    TempUngrab_Run(infobox, 0);
	    goto oke;
	case V_PWEXPIRE:
	    dat.setTime_t(expire);
	    errm = i18n("Your password expires on %1.")
			.arg(KGlobal::locale()->formatDateTime(dat));
	    TempUngrab_Run(infobox, 0);
	  oke:
	case V_OK:
	    save_wm();
	    //qApp->desktop()->setCursor( waitCursor);
	    qApp->setOverrideCursor( waitCursor);
	    hide();
	    qApp->exit( ex_login);
	    DeleteXloginResources( ::d, *dpy);
	    return;
	case V_FAIL:
	    failedLabel->setText(i18n("Login failed!"));
	    goButton->setEnabled( false);
	    loginEdit->setEnabled( false);
	    passwdEdit->setEnabled( false);
	    clear_button_clicked();
	    timer->start( 1500 + rand()*1000/RAND_MAX, true );	// XXX make configurable
	    return;
    }
    setActiveWindow();
    clear_button_clicked();
}

void
KGreeter::ReturnPressed()
{
    if( !goButton->isEnabled())
	return;
    else if( loginEdit->hasFocus()) {
	passwdEdit->setFocus();
        load_wm();
	if (AccNoPass (::d, loginEdit->text().latin1()))
	    go_button_clicked();
    } else if (passwdEdit->hasFocus() || 
	       goButton->hasFocus() ||
	       sessionargBox->hasFocus()) {
	go_button_clicked();
    }
}


// we have to return RESERVER_DISPLAY to restart the server
static int
IOErrorHandler (Display*)
{
    exit (RESERVER_AL_DISPLAY);
    /* Not reached */
    return 0;
}

int AccNoPass (
    struct display	*d,
    const char		*un)
{
    QString user (QString::fromLatin1 (un));
    return (!strcmp (d->name, ":0") &&
            (kdmcfg->_autoUser == user ||
    	     kdmcfg->_noPassUsers.contains (user))) ||
	   (d->autoLogin &&
	    (QString::fromLatin1 (d->autoUser) == user ||
	     QStringList::split (QString::fromLatin1 (","), 
				 QString::fromLatin1 (d->noPassUsers)
				).contains (user)));
}

int
MyVerify (
    struct display	*d,
    struct greet_info	*greet,
    struct verify_info	*verify,
    time_t		*expire,
    char		**nologin)
{
    if (greet->password && greet->password[0] == '\0' && 
        AccNoPass (d, greet->name))
	greet->password = 0;
    return Verify (d, greet, verify, expire, nologin);
}

int
AutoLogon (
    struct display	*d,
    struct greet_info	*greet,
    struct verify_info	*verify)
{
    greet->string = sessarg;
    greet->name = name;
    greet->password = password;

    if (d->hstent->nLogPipe) {
	if (d->hstent->nLogPipe[0] == '\n')
	    return 0;
	int cp = s_copy(sessarg, d->hstent->nLogPipe, 0, 0);
	cp = s_copy(name, d->hstent->nLogPipe, cp, 0);
	s_copy(password, d->hstent->nLogPipe, cp, 1);
    } else {
	if (!d->autoLogin)
	    return 0;
	if (d->autoUser[0] != '\0') {
	    // resource specified autologin
	    if (d->hstent->lastExit > time(0) - d->openDelay) {
		if (d->hstent->goodExit)
		    return 0;
	    } else {
	        if (!d->autoLogin1st)
		    return 0;
	    }
	    greet->name = d->autoUser;
	    if (d->autoPass[0] == '\0')
		greet->password = 0;
	    else {
		strncpy(password, d->autoPass, F_LEN - 1);
		Debug("Password set in auto-login\n");
	    }
	    greet->string = strlen (d->autoString) ? 
			    d->autoString : (char *)"default";
	} else if (!strcmp(d->name, ":0") && !kdmcfg->_autoUser.isEmpty()) {
	    // kcontol specified autologin
	    if (d->hstent->lastExit > time(0) - d->openDelay) {
		if (d->hstent->goodExit)
		    return 0;
	    } else {
		if (!kdmcfg->_autoLogin1st)
		    return 0;
	    }
	    strncpy(name, kdmcfg->_autoUser.latin1(), F_LEN - 1);
	    greet->password = 0;
	    if (!rdwr_wm (sessarg, F_LEN, name, 1))
		greet->string = (char *)"default";
	} else	// no autologin
	    return 0;
    }
    return MyVerify (d, greet, verify, 0, 0) >= V_OK;
}


static void
creat_greet(void * /* ptr */)
{
    kgreeter = new KGreeter;		   
    kgreeter->updateGeometry();
    kapp->processEvents(0);
    kgreeter->resize(kgreeter->sizeHint());     
    int dw = QApplication::desktop()->width();
    int dh = QApplication::desktop()->height();
    int gw = kgreeter->width();
    int gh = kgreeter->height();
    int x, y;
    if (kdmcfg->_greeterPosX >= 0) {
	x = kdmcfg->_greeterPosX;
	y = kdmcfg->_greeterPosY;
    } else {
	x = dw/2;
	y = dh/2;
    }
    x -= gw/2;
    y -= gh/2;
    if (x + gw > dw)
	x = dw - gw;
    if (y + gh > dh)
	y = dh - gh;
    kgreeter->move( x < 0 ? 0 : x, y < 0 ? 0 : y );  
    kgreeter->show();
    kgreeter->setActiveWindow();
    QApplication::restoreOverrideCursor();
}

extern bool kde_have_kipc;

/* XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
   This function is a BIG mess. It needs A LOT of cleanup.
*/
greet_user_rtn 
GreetUser(
    struct display	*d2,
    Display		**dpy2,
    struct verify_info	*verify2,
    struct greet_info	*greet2,
    struct dlfuncs	*
#ifdef GREET_LIB
			 dlfuncs
#endif
	 )
{
#ifdef GREET_LIB
/*
 * These must be set before they are used.
 */
    __xdm_PingServer = dlfuncs->_PingServer;
    __xdm_SessionPingFailed = dlfuncs->_SessionPingFailed;
    __xdm_Debug = dlfuncs->_Debug;
    __xdm_RegisterCloseOnFork = dlfuncs->_RegisterCloseOnFork;
    __xdm_SecureDisplay = dlfuncs->_SecureDisplay;
    __xdm_UnsecureDisplay = dlfuncs->_UnsecureDisplay;
    __xdm_ClearCloseOnFork = dlfuncs->_ClearCloseOnFork;
    __xdm_SetupDisplay = dlfuncs->_SetupDisplay;
    __xdm_LogError = dlfuncs->_LogError;
    __xdm_SessionExit = dlfuncs->_SessionExit;
    __xdm_DeleteXloginResources = dlfuncs->_DeleteXloginResources;
    __xdm_source = dlfuncs->_source;
    __xdm_defaultEnv = dlfuncs->_defaultEnv;
    __xdm_setEnv = dlfuncs->_setEnv;
    __xdm_putEnv = dlfuncs->_putEnv;
    __xdm_parseArgs = dlfuncs->_parseArgs;
    __xdm_printEnv = dlfuncs->_printEnv;
    __xdm_systemEnv = dlfuncs->_systemEnv;
    __xdm_LogOutOfMem = dlfuncs->_LogOutOfMem;
    __xdm_setgrent = dlfuncs->_setgrent;
    __xdm_getgrent = dlfuncs->_getgrent;
    __xdm_endgrent = dlfuncs->_endgrent;
#ifdef USESHADOW
    __xdm_getspnam = dlfuncs->_getspnam;
    __xdm_endspent = dlfuncs->_endspent;
#endif
    __xdm_getpwnam = dlfuncs->_getpwnam;
#ifdef __linux__
    __xdm_endpwent = dlfuncs->_endpwent;
#endif
    __xdm_crypt = dlfuncs->_crypt;
#ifdef USE_PAM
    __xdm_thepamh = dlfuncs->_thepamh;
#endif
#endif

    int retval;

    d = d2;
    dpy = dpy2;
    verify = verify2;
    greet = greet2;
     
    int argc = 3;
    const char* argv[5] = {"kdm", "-display", NULL, NULL};
 
    struct sigaction sig;
 
    /* KApplication trashes xdm's signal handlers :-( */
    sigaction(SIGCHLD, NULL, &sig);
 
    argv[2] = ::d->name;
    KCmdLineArgs::init(argc, (char **) argv, "kdm", description, version);

    kde_have_kipc = false;
    MyApp myapp;
    KGlobal::dirs()->addResourceType("user_pic", KStandardDirs::kde_default("data") + QString::fromLatin1("kdm/pics/users/"));
    QApplication::setOverrideCursor( Qt::waitCursor );

    kdmcfg = new KDMConfig();
     
    myapp.setFont( *kdmcfg->_normalFont);

    *dpy = qt_xdisplay();
     
    // this is necessary, since Qt just overwrites the
    // IOErrorHandler that was set by xdm!!!
    XSetIOErrorHandler(IOErrorHandler);
     
    sigaction(SIGCHLD, &sig, NULL);


    if (AutoLogon (d, greet, verify))
	retval = ex_login;
    else {

	SecureDisplay (d, *dpy);

	// First initialize display:
	SetupDisplay( d);
	// Hack! Kdm looses keyboard focus unless
	// the keyboard is ungrabbed during setup
	TempUngrab_Run(creat_greet, 0);
	retval = qApp->exec();
	// Give focus to root window:
	QApplication::desktop()->setActiveWindow();
	delete kgreeter;

	UnsecureDisplay (d, *dpy);

	if (retval != ex_login)
	    goto exgrt;
    }	/* AutoLogon */

    if (kdmcfg->_showPrevious)
	kdmcfg->writeEntry (QString::fromLatin1("LastUser_") + 
				QString::fromLatin1(d->name), 
			    QString::fromLatin1(greet->name));

    /*
     * Run system-wide initialization file
     */
    if (source (verify->systemEnviron, ::d->startup) != 0)
    {
	qApp->restoreOverrideCursor();
	KMessageBox::error(0, 
			i18n("Startup program %1 exited with non-zero status."
			     "\nPlease contact your system administrator.\n"
			     "Please press OK to retry.")
	       .arg(QFile::decodeName(::d->startup)));
	SessionExit (::d, OBEYSESS_DISPLAY, FALSE);
    }

    if (::d->pipefd[1] >= 0) {
	if (::d->autoReLogin || kdmcfg->_autoReLogin) {
	    char buf[3 * (F_LEN + 1) + 1];
	    write (::d->pipefd[1], buf, 
		sprintf (buf, "%s %s %s\n", greet->string, greet->name, 
			 greet->password ? greet->password : ""));
	} else
	    write (::d->pipefd[1], "\n", 1);
    }

    /*
     * for user-based authorization schemes,
     * add the user to the server's allowed "hosts" list.
     */
    for (int i = 0; i < d->authNum; i++)
    {
#ifdef SECURE_RPC
	if (d->authorizations[i]->name_length == 9 &&
	    memcmp(d->authorizations[i]->name, "SUN-DES-1", 9) == 0)
	{
	    XHostAddress	addr;
	    char		netname[MAXNETNAMELEN+1];
	    char		domainname[MAXNETNAMELEN+1];
    
	    getdomainname(domainname, sizeof domainname);
	    user2netname (netname, verify->uid, domainname);
	    addr.family = FamilyNetname;
	    addr.length = strlen (netname);
	    addr.address = netname;
	    XAddHost (*dpy, &addr);
	}
#endif
#ifdef K5AUTH
	if (d->authorizations[i]->name_length == 14 &&
	    memcmp(d->authorizations[i]->name, "MIT-KERBEROS-5", 14) == 0)
	{
	    /* Update server's auth file with user-specific info.
	     * Don't need to AddHost because X server will do that
	     * automatically when it reads the cache we are about
	     * to point it at.
	     */
	    extern Xauth *Krb5GetAuthFor();

	    XauDisposeAuth (d->authorizations[i]);
	    d->authorizations[i] =
		Krb5GetAuthFor(14, "MIT-KERBEROS-5", d->name);
	    SaveServerAuthorizations (d, d->authorizations, d->authNum);
	} 
#endif
    }

exgrt:
     // Clean up
    qApp->restoreOverrideCursor();
    delete kdmcfg;

    return retval == ex_choose ? Greet_RunChooser : Greet_Success;
}

#include "kgreeter.moc"

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */

