    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998, 2000 Steffen Hansen <hansen@kde.org>
    Copyright (C) 2000, 2001 Oswald Buddenhagen <ossi@kde.org>


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

#include <qfile.h>
#include <qbitmap.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qaccel.h>

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include "kgreeter.h"
#include "kdmconfig.h"
#include "kdmclock.h" 
#include "kfdialog.h" 
#include "miscfunc.h"

#include <X11/Xlib.h>
#ifdef HAVE_X11_XKBLIB_H
#include <X11/XKBlib.h>
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <utime.h>
#include <time.h>

// Global vars
KDMConfig *kdmcfg = 0;
KGreeter *kgreeter = 0;

/* XXX let it go away */
extern int dpy_is_local;
extern Display **dpy;

void
KLoginLineEdit::focusOutEvent( QFocusEvent *e)
{
    emit lost_focus();
    QLineEdit::focusOutEvent( e);
}

class MyApp : public KApplication {

public:
    MyApp(Display *display, int& argc, char** argv, const QCString& rAppName);
    virtual bool x11EventFilter( XEvent * );
};

MyApp::MyApp(Display *display, int& argc, char** argv, const QCString& rAppName)
    : KApplication(display, argc, argv, rAppName)
{
}

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
    return false;
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
		clearButton->animateClick();
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
/*    if( default_pix.isNull())
	LogError("Can't get default pixmap from \"default.png\"\n");
*/
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

static void
inserten (QPopupMenu *mnu, const QString& txt, 
	  const QObject *receiver, const char *member)
{
    mnu->insertItem(txt, receiver, member, QAccel::shortcutKey(txt));
}

QString enam;

KGreeter::KGreeter(QWidget *parent, const char *t) 
  : QFrame( parent, t, WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
{
    setFrameStyle (QFrame::WinPanel | QFrame::Raised);
    QBoxLayout* vbox = new QBoxLayout(  this, 
					QBoxLayout::TopToBottom, 
					10, 10);
    QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
    QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10);

    QGridLayout* grid = new QGridLayout( 5, 4, 5);

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

    loginEdit = new KLoginLineEdit( this);
    loginLabel = new QLabel( loginEdit, i18n("&Login:"), this);

    passwdEdit = new KPasswordEdit( this, "edit", kdmcfg->_echoMode);
    passwdLabel = new QLabel( passwdEdit, i18n("&Password:"), this);

    sessargBox = new QComboBox( false, this);
    sessargLabel = new QLabel( sessargBox, i18n("Session &Type:"), this);
    sessargBox->insertStringList( kdmcfg->_sessionTypes );
    sessargStat = new QWidget( this);
    sasPrev = new QLabel( i18n("session type", "(previous)"), sessargStat);
    sasSel = new QLabel( i18n("session type", "(selected)"), sessargStat);
    sessargStat->setFixedSize(
	QMAX(sasPrev->sizeHint().width(), sasSel->sizeHint().width()),
	sessargBox->height());

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

    grid->addWidget( loginLabel, 0, 0);
    grid->addMultiCellWidget( loginEdit, 0,0, 1,3);
    grid->addWidget( passwdLabel, 1, 0);
    grid->addMultiCellWidget( passwdEdit, 1,1, 1,3);
    grid->addWidget( sessargLabel, 2, 0);
    grid->addWidget( sessargBox, 2, 1);
    grid->addWidget( sessargStat, 2, 2);
    grid->addMultiCellWidget( failedLabel, 3,3, 0,3, AlignCenter);
    grid->addMultiCellWidget( sepFrame, 4,4, 0,3);
    grid->setColStretch( 3, 1);

    goButton = new QPushButton( i18n("G&o!"), this);
    goButton->setFixedWidth(goButton->sizeHint().width());
    goButton->setDefault( true);
    connect( goButton, SIGNAL( clicked()), SLOT(go_button_clicked()));
    hbox2->addWidget( goButton);

    clearButton = new QPushButton( i18n("&Clear"), this);
    connect( clearButton, SIGNAL(clicked()), SLOT(clear_button_clicked()));
    hbox2->addWidget( clearButton);

    hbox2->addStretch( 1);

    optMenu = new QPopupMenu(this);
    optMenu->setCheckable(false);

    inserten (optMenu, i18n("Co&nsole Login"), 
	      this, SLOT(console_button_clicked()));

    inserten (optMenu, i18n("&Remote Login"), 
	      this, SLOT(chooser_button_clicked()));

    inserten (optMenu, dpy_is_local ? 
		       i18n("R&estart X Server") : 
		       i18n("Clos&e Connection"), 
	      this, SLOT(quit_button_clicked()));

    menuButton = new QPushButton( i18n("&Menu"), this);
    menuButton->setPopup(optMenu);
    hbox2->addWidget( menuButton);

    hbox2->addStretch( 1);

    int sbw = 0;
    if (kdmcfg->_shutdownButton != KDMConfig::SdNone && 
	(kdmcfg->_shutdownButton != KDMConfig::SdConsoleOnly || dpy_is_local))
    {
	  
	shutdownButton = new QPushButton(i18n("&Shutdown..."), this);

	connect( shutdownButton, SIGNAL(clicked()), 
		 SLOT(shutdown_button_clicked()));
	hbox2->addWidget( shutdownButton);
	sbw = shutdownButton->width();
    }

    //vbox->activate();
    timer = new QTimer( this );
    //// Signals/Slots
    // Timer for failed login
    connect( timer, SIGNAL(timeout()),
	     this , SLOT(timerDone()) );
     // Signal to update session type
    connect( loginEdit, SIGNAL(lost_focus()),
	     this, SLOT( load_wm()));
    connect( sessargBox, SIGNAL(activated(int)),
	     this, SLOT(slot_session_selected()));
    if( user_view) {
	connect( user_view, SIGNAL(returnPressed(QIconViewItem*)), 
		 this, SLOT(slot_user_name( QIconViewItem*)));
	connect( user_view, SIGNAL(clicked(QIconViewItem*)), 
		 this, SLOT(slot_user_name( QIconViewItem*)));
    }

    clear_button_clicked();

    if (kdmcfg->_showPrevious) {
	loginEdit->setText (kdmcfg->readEntry (enam));
	loginEdit->selectAll();
	load_wm();
    } else
	kdmcfg->deleteEntry (enam, false);
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
KGreeter::slot_session_selected()
{
    wmstat = WmSel;
    sasSel->show();
    sasPrev->hide();
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
	sessargBox->setEnabled( true);
	clear_button_clicked();
    }
}

void 
KGreeter::clear_button_clicked()
{
    loginEdit->clear();
    passwdEdit->erase();
    loginEdit->setFocus();
    sasPrev->hide();
    sasSel->hide();
    wmstat = WmNone;
    set_wm( "default");
}

void
KGreeter::quit_button_clicked()
{
    QApplication::flushX();
    qApp->exit( ex_reserver );
}

void
KGreeter::chooser_button_clicked()
{
    qApp->exit( ex_choose );
}

void
KGreeter::console_button_clicked()
{
    qApp->exit( ex_console );
}

static int
do_shutdown(size_t arg)
{
    KDMShutdown k( kdmcfg->_shutdownButton,
		   (KGreeter *)arg, "Shutdown",
		   kdmcfg->_shutdown, 
		   kdmcfg->_restart,
		   kdmcfg->_useLilo,
		   kdmcfg->_liloCmd,
		   kdmcfg->_liloMap);
    return k.exec();
}

void
KGreeter::shutdown_button_clicked()
{
    timer->stop();
    if (TempUngrab_Run(do_shutdown, (size_t)this) == QDialog::Accepted)
	qApp->exit( ex_unmanage );
    else
	SetTimer();
}


void
KGreeter::set_wm(const char *cwm)
{
    QString wm = QString::fromLocal8Bit (cwm);
    for (int i = sessargBox->count(); i--;)
	if (sessargBox->text(i) == wm) {
	    sessargBox->setCurrentItem(i);
	    return;
	}
}

void
KGreeter::save_wm()
{
#ifdef USE_RDWR_WM
    rdwr_wm ((char *)sessargBox->currentText().local8Bit().data(), 0, 
	     loginEdit->text().local8Bit().data(), 0);
#endif /* USE_RDWR_WM */
}

void
KGreeter::load_wm()
{
#ifdef USE_RDWR_WM
    int sum, len, i, num;
    QCString name;
    char buf[256];

    if (wmstat == WmSel)
	return;

    name = loginEdit->text().local8Bit();
    if (!(len = name.length())) {
	wmstat = WmNone;
	sasPrev->hide();
	set_wm( "default");
	return;
    }

    wmstat = WmPrev;
    sasPrev->show();
    switch (rdwr_wm (buf, 256, name.data(), 1)) {
	case -2:	/* no such user */
	    /* XXX - voodoo */
	    for (sum = 0, i = 0; i < len; i++)
		sum += (int)name[i] << ((i ^ sum) & 7);
	    sum ^= (sum >> 7);
	    /* forge a session with this hash - default more probable */
	    num = sessargBox->count();
	    i = sum % (num * 4 / 3);
	    if (i < num) {
		sessargBox->setCurrentItem(i);
		return;
	    }
	    /* fall through */
	case -1:	/* cannot read */
	    set_wm( "default");
	    return;
	default:
	    set_wm( buf);
    }
#endif /* USE_RDWR_WM */
}


QString errm;

#define errorbox ((size_t)QMessageBox::Critical)
#define sorrybox ((size_t)QMessageBox::Warning)
#define infobox ((size_t)QMessageBox::Information)

static int
msgbox(size_t typ)
{
    KFMsgBox::box(kgreeter, (QMessageBox::Icon)typ, errm);
    return 0;
}

bool
KGreeter::verifyUser(bool haveto)
{
    int expire;
    char *nologin;

    init_greet();
    /* NOTE: name/password are static -> all zero -> terminator present */
    strncpy( ::name, loginEdit->text().local8Bit(), F_LEN - 1 );
    strncpy( ::password, passwdEdit->password(), F_LEN - 1 );
    strncpy( ::sessarg, sessargBox->currentText().local8Bit(), F_LEN - 1 );

    switch (MyVerify (::name, ::password, &expire, &nologin)) {
	case V_ERROR:
	    errm = i18n("Some critical error occurred.\n"
			"Please look at KDM's logfile for more information\n"
			"or contact your system administrator.");
	    TempUngrab_Run(msgbox, errorbox);
	    break;
	case V_NOHOME:
	    errm = i18n("Home directory not available.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_NOROOT:
	    errm = i18n("Root logins are not allowed.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_BADSHELL:
	    errm = i18n("Your login shell is not listed\n"
			"in /etc/shells.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_AEXPIRED:
	    errm = i18n("Your account has expired.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_PEXPIRED:
	    errm = i18n("Your password has expired.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_BADTIME:
	    errm = i18n("You are not allowed to login\n"
			"at the moment.");
	    TempUngrab_Run(msgbox, sorrybox);
	    break;
	case V_NOLOGIN: 
	    {
		QFile f;
		f.setName(QString::fromLocal8Bit(nologin));
		f.open(IO_ReadOnly);
	        QTextStream t( &f ); 
		errm = "";
		while ( !t.eof() )
		    errm += t.readLine() + '\n';
		f.close();

		if (errm.isEmpty())
		    errm = i18n("Logins are not allowed at the moment.\n"
				"Try again later.");
		TempUngrab_Run(msgbox, sorrybox);
	    } 
	    break;
	case V_AWEXPIRE:
	    if (expire == 1)
		errm = i18n("Your account expires tomorrow.");
	    else
		errm = i18n("Your account expires in %1 days.").arg(expire);
	    TempUngrab_Run(msgbox, infobox);
	    goto oke;
	case V_PWEXPIRE:
	    if (expire == 1)
		errm = i18n("Your password expires tomorrow.");
	    else
		errm = i18n("Your password expires in %1 days.").arg(expire);
	    TempUngrab_Run(msgbox, infobox);
	  oke:
	case V_OK:
	    save_wm();
	    //qApp->desktop()->setCursor( waitCursor);
	    qApp->setOverrideCursor( waitCursor);
	    hide();
	    qApp->exit( ex_login);
	    return true;
	case V_FAIL:
	    if (!haveto)
		return false;
	    failedLabel->setText(i18n("Login failed"));
	    goButton->setEnabled( false);
	    loginEdit->setEnabled( false);
	    passwdEdit->setEnabled( false);
	    sessargBox->setEnabled( false);
	    timer->start( 1500 + kapp->random()/(RAND_MAX/1000), true );// XXX make configurable
	    return true;
    }
//    setActiveWindow();	XXX (tempungrabrun)
    clear_button_clicked();
    return true;
}

void 
KGreeter::go_button_clicked()
{
    verifyUser(true);
}

void
KGreeter::ReturnPressed()
{
    if (!goButton->isEnabled())
	return;
    if (loginEdit->hasFocus()) {
	load_wm();
	if (!verifyUser(false))
	    passwdEdit->setFocus();
    } else if (passwdEdit->hasFocus() ||
	       sessargBox->hasFocus()) {
	verifyUser(true);
    }
}


static int
creat_greet(size_t /* arg */)
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
    return 0;
}

extern bool kde_have_kipc;
extern "C" int IOErrorHandler (Display *);

int argc = 1;
const char *argv[2] = {"kdmgreet", NULL};

extern "C" int
kg_main(const char *dname)
{
    /* KApplication trashes xdm's signal handlers :-( */
    struct sigaction sig;
    sigaction(SIGCHLD, NULL, &sig);

    kde_have_kipc = false;
    MyApp myapp(*dpy, argc, (char **)argv, "kdmgreet");

    QString cfgname (KGlobal::dirs()->resourceDirs("config").last() + 
		     QString::fromLatin1("kdmrc"));
    kdmcfg = new KDMConfig(cfgname);
    struct stat st;
    stat (QFile::encodeName(cfgname).data(), &st);

    KGlobal::dirs()->addResourceType("user_pic", 
				     KStandardDirs::kde_default("data") + 
				     QString::fromLatin1("kdm/pics/users/"));

    QApplication::setOverrideCursor( Qt::waitCursor );

    myapp.setFont( *kdmcfg->_normalFont);

    // this is necessary, since Qt just overwrites the
    // IOErrorHandler that was set by xdm!!!
    XSetIOErrorHandler(IOErrorHandler);
     
    sigaction(SIGCHLD, &sig, NULL);

    enam = QString::fromLocal8Bit("LastUser_") + QString::fromLocal8Bit(dname);

    // Hack! Kdm looses keyboard focus unless
    // the keyboard is ungrabbed during setup
    TempUngrab_Run(creat_greet, 0);
#ifdef HAVE_X11_XKBLIB_H
    //
    //  Activate the correct mapping for modifiers in XKB extension as
    //  grabbed keyboard has its own mapping by default
    //
    int opcode, evbase, errbase, majret, minret;
    unsigned int value = XkbPCF_GrabsUseXKBStateMask;
    if (XkbQueryExtension (*dpy, &opcode, &evbase,
                           &errbase, &majret, &minret))
    {
        if (!XkbSetPerClientControls (*dpy,
                                      XkbPCF_GrabsUseXKBStateMask, &value))
            (void) fprintf(stderr, "XkbSetPerClientControls failed\n");
   }
#endif
    int retval = qApp->exec();
    // Give focus to root window:
    QApplication::desktop()->setActiveWindow();
    delete kgreeter;

    if (retval == ex_login && kdmcfg->_showPrevious)
	kdmcfg->writeEntry (enam, QString::fromLocal8Bit(name));

    qApp->restoreOverrideCursor();
    delete kdmcfg;
    // HACK: KConfigINIBackend sets 600
    chmod (QFile::encodeName(cfgname).data(), 0644);
    // HACK: prevent the xdm core from rereading the config
    struct utimbuf utb;
    utb.actime = st.st_atime;
    utb.modtime = st.st_mtime;
    utime (QFile::encodeName(cfgname).data(), &utb);

    return retval;
}

#include "kgreeter.moc"

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */

