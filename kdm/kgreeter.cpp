    /*

    Greeter module for xdm
    $Id$

    Copyright (C) 1997, 1998 Steffen Hansen
                             stefh@mip.ou.dk


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

#include "kgreeter.h"

#include <sys/types.h>
#include <sys/param.h>

#include <pwd.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)
#  include <grp.h>
#endif

#include <qbitmap.h>
#include <qtextstream.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qcombobox.h>

#include "kdmclock.h" 

#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <kmessagebox.h>
#include <kcmdlineargs.h>
#include <kapp.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "dm.h"
#include "greet.h"

// Make the C++ compiler shut the f... up:
extern "C" {
	int Verify( struct display*, struct greet_info*, struct verify_info*);
	char **parseArgs(char **, const char *);
	void DeleteXloginResources(struct display *, Display *dpy);
	void SetupDisplay(struct display *d);
	void SecureDisplay(struct display *d, Display *);
	void RegisterCloseOnFork(int);
	int source(void*, void*);
	void SessionExit(void*, int, int);
}

#ifdef USESHADOW
	#include <shadow.h>
#endif

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
	#define USE_LOGIN_CAP 1
	#include <login_cap.h>
#ifdef __bsdi__
	// This only works / is needed on BSDi
	struct login_cap_t *lc;
#else
	struct login_cap *lc;
#endif
#endif

#ifdef TEST_KDM
	int Verify(struct display *, struct greet_info *, struct verify_info *) {}
	char **parseArgs( char **, const char *) {}
	void DeleteXloginResources(struct display *, Display *) {}
	void SetupDisplay(struct display *d) {}
	void SecureDisplay(struct display *d, Display *dpy) {}
	void RegisterCloseOnFork(int i) {}
	int source(void *,void *) {}
	void SessionExit(void *, int, int) {}
#endif

static const char *description = 
	I18N_NOOP("KDE Display Manager");

static const char *version = "v0.0.1";


// Global vars
KGreeter* kgreeter = 0;

KDMConfig               *kdmcfg;
struct display          *d;
Display                 ** dpy;
struct verify_info      *verify;
struct greet_info       *greet;

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
MyApp::x11EventFilter( XEvent * ev){
     if( ev->type == KeyPress && kgreeter){
	  // This should go away
	  KeySym ks = XLookupKeysym(&(ev->xkey),0);
	  if (ks == XK_Return ||
	      ks == XK_KP_Enter)
	       kgreeter->ReturnPressed();
     }
     // Hack to tell dialogs to take focus 
     if( ev->type == ConfigureNotify) {
	  QWidget* target = QWidget::find( (( XConfigureEvent *) ev)->window);
	  target = target->topLevelWidget();
	  if( target->isVisible() && !target->isPopup())
	    XSetInputFocus( qt_xdisplay(), target->winId(), 
			    RevertToParent, CurrentTime);
     }
     return FALSE;
}

// Misc helper functions:
static inline int my_seteuid( uid_t euid)
{
#ifdef HAVE_SETEUID
     return seteuid(euid);
#else
     return setreuid(-1, euid);
#endif // HAVE_SETEUID
}

// Misc. functions
static inline int my_setegid( gid_t egid)
{
#ifdef HAVE_SETEUID
     return setegid(egid);
#else
     return setregid(-1, egid);
#endif // HAVE_SETEUID    
}

KGreeter::KGreeter(QWidget *parent = 0, const char *t = 0) 
  : QFrame( parent, t, WStyle_Customize | WStyle_NoBorder | WStyle_Tool)
{
     setFrameStyle(QFrame::WinPanel| QFrame::Raised);
     QBoxLayout* vbox = new QBoxLayout(  this, 
					 QBoxLayout::TopToBottom, 
					 10, 10);
     QBoxLayout* hbox1 = new QBoxLayout( QBoxLayout::LeftToRight, 10);
     QBoxLayout* hbox2 = new QBoxLayout( QBoxLayout::LeftToRight, 10);

     QGridLayout* grid = new QGridLayout( 4, 2, 5);

     QLabel* welcomeLabel = new QLabel( kdmcfg->greetString(), this);
     welcomeLabel->setAlignment(AlignCenter);
     welcomeLabel->setFont( *kdmcfg->greetFont());
     vbox->addWidget( welcomeLabel);
     if( kdmcfg->users()) {
	  user_view = new QIconView( this);
	  user_view->setSelectionMode( QIconView::Single );
	  user_view->setArrangement( QIconView::LeftToRight);
	  user_view->setAutoArrange(true);
	  user_view->setItemsMovable(false);
	  user_view->setResizeMode(QIconView::Adjust);
	  kdmcfg->insertUsers( user_view);	  
	  vbox->addWidget( user_view);
     } else {
	  user_view = NULL;
     }

     pixLabel = 0;
     clock    = 0;

     if( !kdmcfg->useLogo() )
     {
         clock = new KdmClock( this, "clock" );
     }
     else
     {
     pixLabel = new QLabel( this);
     pixLabel->setFrameStyle( QFrame::Panel| QFrame::Sunken);
     pixLabel->setAutoResize( true);
     pixLabel->setIndent(0);
     QPixmap pixmap;
     if( QFile::exists( kdmcfg->logo() ) )
	  pixmap.load( kdmcfg->logo() );
     else
	  pixmap.resize( 100,100);
     pixLabel->setPixmap( pixmap);
     }
     loginLabel = new QLabel( i18n("Login:"), this);
     loginEdit = new KLoginLineEdit( this);

     // The line-edit look _very_ bad if you don't give them 
     // a resonal height that observes a proportional aspect.
     // -- Bernd
     
     // I still think a line-edit show decide it's own height
     // -- Steffen

     loginEdit->setFocus();

     passwdLabel = new QLabel( i18n("Password:"), this);
     passwdEdit = new QLineEdit( this);

     passwdEdit->setEchoMode( QLineEdit::NoEcho);
     vbox->addLayout( hbox1);
     vbox->addLayout( hbox2);
     hbox1->addWidget( pixLabel ? (QWidget*)pixLabel : (QWidget*)clock, 0, AlignTop);
     hbox1->addLayout( grid, 3);
     
     QFrame* sepFrame = new QFrame( this);
     sepFrame->setFrameStyle( QFrame::HLine| QFrame::Sunken);
     sepFrame->setFixedHeight( sepFrame->sizeHint().height());

     failedLabel = new QLabel( this);
     failedLabel->setFont( *kdmcfg->failFont());

     grid->addWidget( loginLabel , 0, 0);
     grid->addWidget( loginEdit  , 0, 1);
     grid->addWidget( passwdLabel, 1, 0);
     grid->addWidget( passwdEdit , 1, 1);
     grid->addMultiCellWidget( failedLabel, 2, 2, 0, 1, AlignCenter);
     grid->addMultiCellWidget( sepFrame, 3, 3, 0, 1);
     grid->setColStretch( 1, 4);

     QLabel* sessionargLabel = new QLabel(i18n("Session Type:"),
					  this);
     sessionargLabel->setAlignment( AlignRight|AlignVCenter);
     hbox2->addWidget( sessionargLabel);
     sessionargBox = new QComboBox( false, this);

     sessionargBox->insertStringList( kdmcfg->sessionTypes() );
     //set_fixed( sessionargBox);
     hbox2->addWidget( sessionargBox);
     
     goButton = new QPushButton( i18n("Go!"), this);
     connect( goButton, SIGNAL( clicked()), SLOT(go_button_clicked()));

     //set_fixed( goButton);
     hbox2->addWidget( goButton, AlignBottom);

#if 0
     chooserButton = new QPushButton( i18n("Chooser"), this);
     connect( chooserButton, SIGNAL(clicked()), SLOT(cancel_button_clicked()));
     //set_fixed( chooserButton);
     hbox2->addWidget( chooserButton, AlignBottom);
#endif
     
     cancelButton = new QPushButton( i18n("Cancel"), this);
     connect( cancelButton, SIGNAL(clicked()), SLOT(cancel_button_clicked()));
     //set_fixed( cancelButton);
     hbox2->addWidget( cancelButton, AlignBottom);

     int sbw;
#ifndef TEST_KDM
     if( kdmcfg->shutdownButton() != KDMConfig::KNone 
	 && ( kdmcfg->shutdownButton() != KDMConfig::ConsoleOnly 
	 || ::d->displayType.location == Local)) {
	  
	  shutdownButton = new QPushButton(i18n("Shutdown..."), this);

	  connect( shutdownButton, SIGNAL(clicked()), 
		   SLOT(shutdown_button_clicked()));
	  //set_fixed( shutdownButton);
	  hbox2->addWidget( shutdownButton, AlignBottom);
	  sbw = shutdownButton->width();
     } else {
	quitButton = new QPushButton( i18n("Quit"), this);
	connect( quitButton, SIGNAL(clicked()), SLOT(quit_button_clicked()));
	//set_fixed( quitButton);
	hbox2->addWidget( quitButton, AlignBottom);
	sbw = 0;
     }
#else
     sbw = 0;
#endif

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
}

void 
KGreeter::slot_user_name( QIconViewItem *item)
{
     if( item != 0) {
	  loginEdit->setText( item->text());
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
     if (failedLabel->isVisible()){
	  failedLabel->setText(QString::null);
	  goButton->setEnabled( true);
	  loginEdit->setEnabled( true);
          passwdEdit->setEnabled( true);
          loginEdit->setFocus();
     }
}

void 
KGreeter::cancel_button_clicked()
{
     loginEdit->clear();
     passwdEdit->clear();
     loginEdit->setFocus();
}

void
KGreeter::quit_button_clicked()
{
   QApplication::flushX();
   SessionExit(::d, UNMANAGE_DISPLAY, FALSE);
}

#if 0
extern void RunChooser(struct display *d);

void
KGreeter::chooser_button_clicked()
{
	if(!fork()) {
		RunChooser(QApplication::display());		
	}
	hide();
  QApplication::flushX();
  SessionExit(::d, UNMANAGE_DISPLAY, FALSE);
}
#endif
void
KGreeter::shutdown_button_clicked()
{
  timer->stop();
  
  KDMShutdown k( kdmcfg->shutdownButton(),
		 this, "Shutdown",
		 kdmcfg->shutdown(), 
		 kdmcfg->restart(),
#ifndef BSD
		 kdmcfg->consoleMode(),
#endif
		 kdmcfg->useLilo(),
		 kdmcfg->liloCmd(),
		 kdmcfg->liloMap());
  k.exec();

  SetTimer();
}

// Switch uid/gids to user described by pwd. Return old gid set
// or 0 if an error occurs
#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)

static inline gid_t* switch_to_user( int *gidset_size, 
				     const struct passwd *pwd)
{
     *gidset_size = getgroups(0,0);
     gid_t *gidset = new gid_t[*gidset_size];
     if( getgroups( *gidset_size, gidset) == -1 ||
	 initgroups(pwd->pw_name, pwd->pw_gid) != 0 ||
	 my_setegid(pwd->pw_gid) != 0 ||
         my_seteuid(pwd->pw_uid) != 0) {
	  // Error, back out
	  my_seteuid(0);
          my_setegid(0);
	  setgroups( *gidset_size, gidset);
	  delete[] gidset;
	  return 0;
     }
     return gidset;
}

// Switch uid back to root, and gids to gidset
static inline void switch_to_root( int gidset_size, gid_t *gidset)
{
     my_seteuid(0);
     my_setegid(0);
     setgroups( gidset_size, gidset);
     delete[] gidset;
}

void
KGreeter::save_wm()
{
     QString file = QFile::decodeName(pwd->pw_dir);
     file += QString::fromLatin1("/" WMRC);

     // open file as user which is loging in
     int gidset_size;
     // Go user
     gid_t *gidset = switch_to_user( &gidset_size, pwd);
     if( gidset == 0) return;

     QFile f(file);
     if ( f.open(IO_WriteOnly) )
     {
        QTextStream t( &f );
	t << sessionargBox->currentText();
        f.close();
     }

     // Go root
     switch_to_root( gidset_size, gidset);
}

void
KGreeter::load_wm()
{
     // read passwd
     pwd = getpwnam( QFile::encodeName( loginEdit->text() ).data() );
     endpwent();
     if (!pwd) return;
     // we don't need the password
     memset(pwd->pw_passwd, 0, strlen(pwd->pw_passwd));

     QString file = QFile::decodeName(pwd->pw_dir);
     file += QString::fromLatin1( "/" WMRC );
     
     int gidset_size;
     // Go user
     gid_t *gidset = switch_to_user( &gidset_size, pwd);
     if( gidset == 0) return;

     // open file as user which is loging in
     QFile f(file);
     if ( f.open(IO_ReadOnly) )
     {
        QTextStream t( &f );
        QString s (t.readLine());
        f.close();

	int i;
	for (i = 0; i < sessionargBox->count(); i++)
	   if (sessionargBox->text(i) == s)
	   {
	      sessionargBox->setCurrentItem(i);
	      break;
	   }
     }

     // Go root
     switch_to_root( gidset_size, gidset);
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

/* This stuff doesn't really belong to the kgreeter, but verify.c is
 * just C, not C++.
 *
 * A restrict_host() should be added.
 */
bool
KGreeter::restrict()
{
     bool rval = false;

     pwd = getpwnam(greet->name);
     endpwent();
     if (!pwd) return false;
     // we don't need the password
     memset(pwd->pw_passwd, 0, strlen(pwd->pw_passwd));

#ifdef USESHADOW
     swd = getspnam(greet->name);
     endspent();
     if (!swd) return false;
     // we don't need the password
     memset(swd->sp_pwdp, 0, strlen(swd->sp_pwdp));
#endif

#ifdef USE_LOGIN_CAP
#ifdef __bsdi__
     lc = login_getclass(pwd->pw_class);
#else
     lc = login_getpwclass(pwd);
#endif
#endif

     if (restrict_nologin() ||
	 restrict_nohome() ||
	 restrict_expired() ||
	 restrict_time())
       rval = true;

#ifdef USE_LOGIN_CAP
     login_close(lc);
     lc = NULL;
#endif

     return rval;
}

bool
KGreeter::restrict_time()
{
#ifdef USE_LOGIN_CAP
     // don't deny a root log in
     if (!pwd->pw_uid) return false;

     if(!auth_timeok(lc, time(NULL))) {
         KMessageBox::sorry(this, i18n("Logins not available right now."));
         return true;
     }
#endif
     return false;
}


bool
KGreeter::restrict_nologin()
{
#ifdef USE_PAM
     // PAM handles /etc/nologin itself.
     return false;
#else /* !USE_PAM */

     // don't deny root to log in
     if (pwd && !pwd->pw_uid) return false;

#ifndef _PATH_NOLOGIN
#define _PATH_NOLOGIN "/etc/nologin"
#endif

#ifdef USE_LOGIN_CAP
     /* Do we ignore a nologin file? */
     if (login_getcapbool(lc, "ignorenologin", 0))
       return false;

     QString file;
     /* Note that <file> will be "" if there is no nologin capability */
     file = QFile::decodeName(login_getcapstr(lc, "nologin", "", NULL));
     if (file.isNull()) {
       KMessageBox::error(this, i18n("Could not access the login capabilities database or out of memory."));
       return true;
     }
#endif

     QFile f;

#ifdef USE_LOGIN_CAP
     if (!file.isNull()) {
       f.setName(file);
       f.open(IO_ReadOnly);
     }
#endif

     if (f.handle() == -1) {
       f.setName(QString::fromLatin1(_PATH_NOLOGIN));
       f.open(IO_ReadOnly);
     }

     if (f.handle() != -1) {
       QString s;
       QTextStream t( &f ); 

       while ( !t.eof() )
         s += t.readLine() + '\n';

       if (s.isEmpty())
         s = i18n("You're not allowed to login at the moment.\n"
                  "try again later.");

       f.close();
       KMessageBox::sorry(this, s);

       return true;
     }

     return false;
}
#endif /* !USE_PAM */

#ifdef HAVE_PW_EXPIRE
bool
KGreeter::restrict_expired(){
#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
     bool quietlog = login_getcapbool(lc, "hushlogin", 0);
     time_t warntime = login_getcaptime(lc, "warnexpire",
				 DEFAULT_WARN, DEFAULT_WARN);
#else
     bool quietlog = false;
     time_t warntime = DEFAULT_WARN;
#endif
     if (pwd->pw_expire)
	  if (pwd->pw_expire <= time(NULL)) {
	       KMessageBox::sorry(this, i18n("Your account has expired."));
	       return true;
	  } else if (pwd->pw_expire - time(NULL) < warntime && !quietlog) {
	       QDateTime dat;
	       dat.setTime_t(pwd->pw_expire);
	       QString str = i18n("Your account expires on %1.")
			   .arg(KGlobal::locale()->formatDateTime(dat));
	       KMessageBox::sorry(this, str);
	  }

     return false;
}
#elif USESHADOW
bool
KGreeter::restrict_expired(){
#define DEFAULT_WARN  (2L * 7L * 86400L)  /* Two weeks */
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

     time_t warntime = DEFAULT_WARN;
     time_t expiresec = swd->sp_expire*86400L; //sven: ctime uses seconds
     
     if (swd->sp_expire != -1)
	 if (expiresec <= time(NULL)) {
	     KMessageBox::sorry(this, i18n("Your account has expired."));
	     return true;
	 } else if (expiresec - time(NULL) < warntime) {
	     QDateTime dat;
	     dat.setTime_t(expiresec);
             QString str = i18n("Your account expires on %1.")
			 .arg(KGlobal::locale()->formatDateTime(dat));
	     KMessageBox::sorry(this, str);
	 }

     return false;
}
#else /*!USESHADOW*/
bool
KGreeter::restrict_expired()
{
     return false;
}
#endif

#if defined(HAVE_LOGIN_CAP_H) && !defined(__NetBSD__)
bool
KGreeter::restrict_nohome(){
     // don't deny root to log in
     if (!pwd->pw_uid) return false;

     my_seteuid(pwd->pw_uid);
     if (!*pwd->pw_dir || chdir(pwd->pw_dir) < 0) {
	  if (login_getcapbool(lc, "requirehome", 0)) {
	       KMessageBox::sorry(this, i18n("Home directory not available"));
	       return true;
	  }
     }
     my_seteuid(0);

     return false;
}
#else
bool
KGreeter::restrict_nohome()
{
     return false;
}
#endif


void 
KGreeter::go_button_clicked()
{
     greet->name = qstrdup( QFile::encodeName(loginEdit->text()).data() );
     greet->password = qstrdup( QFile::encodeName(passwdEdit->text()).data() );
     
     if (!Verify (::d, greet, verify)){
	  failedLabel->setText(i18n("Login failed!"));
	  goButton->setEnabled( false);
	  loginEdit->setEnabled( false);
          passwdEdit->setEnabled( false);
	  cancel_button_clicked();
	  timer->start( 2000, true );
	  return;
     }

     if (restrict()) {
	  setActiveWindow();
	  cancel_button_clicked();
	  return;
     }

     // Set session argument:
     // sessions are in fact filenames and should be encoded that way
     verify->argv = parseArgs( verify->argv,
			       QFile::encodeName( sessionargBox->currentText() ).data() );

     save_wm();
     //qApp->desktop()->setCursor( waitCursor);
     qApp->setOverrideCursor( waitCursor);
     hide();
     DeleteXloginResources( ::d, *dpy);
     qApp->exit();
}

void
KGreeter::ReturnPressed()
{
     if( !goButton->isEnabled())
	  return;
     if( loginEdit->hasFocus()) {
	  passwdEdit->setFocus();
          load_wm();
     }
     else if (passwdEdit->hasFocus()
	      || goButton->hasFocus() 
	      || cancelButton->hasFocus()) {
	  go_button_clicked();
     }
}

static void 
DoIt()
{
     // First initialize display:
     SetupDisplay( d);
     // Hack! Kdm looses keyboard focus unless
     // the keyboard is ungrabbed during setup
     XUngrabKeyboard(qt_xdisplay(), CurrentTime);
     kgreeter = new KGreeter;		   
     // More hack. QIconView wont calculate
     // a correct sizeHint before show()
     kgreeter->move(-1000,-1000);
     kgreeter->show();
     kgreeter->updateGeometry();
     qApp->processEvents(0);
     kgreeter->resize(kgreeter->sizeHint());     
     // Center on screen:
     kgreeter->move( QApplication::desktop()->width()/2  - kgreeter->width()/2,
		     QApplication::desktop()->height()/2 - kgreeter->height()/2 );  
     QApplication::restoreOverrideCursor();
     kgreeter->setActiveWindow();
     // Secure the keyboard again
     if (XGrabKeyboard (qt_xdisplay(), DefaultRootWindow (qt_xdisplay()), 
			True, GrabModeAsync, GrabModeAsync, CurrentTime) 
	 != GrabSuccess) {
	  LogError ("WARNING: keyboard on display %s could not be secured\n",
		    ::d->name);
	  SessionExit (::d, RESERVER_DISPLAY, FALSE);	 
     }
     qApp->exec();
     // Give focus to root window:
     QApplication::desktop()->setActiveWindow();
     delete kgreeter;
}

#include "kgreeter.moc"
#include <qstring.h>
#include <stdlib.h>


#ifdef TEST_KDM

int main(int argc, char **argv)
{
     KCmdLineArgs::init(argc, argv, "kdm", description, version);

     QString path = KStandardDirs::kde_default("data");
     //??????
     QString s;
     s.sprintf("%s", "kdm/pics/users/" );
     path += s;
   

     MyApp app;
     KGlobal::dirs()->addResourceType("user_pic", path );

     kdmcfg = new KDMConfig();

     app.setFont( *kdmcfg->normalFont());

     kgreeter = new KGreeter;
     app.setMainWidget( kgreeter);
     kgreeter->show();
     return app.exec();
}

#endif

static int
IOErrorHandler (Display*)
{
  exit (RESERVER_DISPLAY);
  /* Not reached */
  return 0;
}

greet_user_rtn 
GreetUser(
     struct display          *d2,
     Display                 ** dpy2,
     struct verify_info      *verify2,
     struct greet_info       *greet2,
     struct dlfuncs       */*dlfuncs*/
     )
{
     d = d2;
     dpy = dpy2;
     verify = verify2;
     greet = greet2;
     
     int argc = 3;
     const char* argv[5] = {"kdm", "-display", NULL};
 
     struct sigaction sig;
 
     /* KApplication trashes xdm's signal handlers :-( */
     sigaction(SIGCHLD, NULL, &sig);
 
     argv[2] = ::d->name;
     KCmdLineArgs::init(argc, (char **) argv, "kdm", description, version);

     MyApp myapp;
     KGlobal::dirs()->addResourceType("user_pic", KStandardDirs::kde_default("data") + QString::fromLatin1("kdm/pics/users/"));
     QApplication::setOverrideCursor( Qt::waitCursor );
     kdmcfg = new KDMConfig( );
     
     myapp.setFont( *kdmcfg->normalFont());
     // TODO: myapp.setStyle( kdmcfg->style());

     *dpy = qt_xdisplay();
     
     RegisterCloseOnFork (ConnectionNumber (*dpy));
     SecureDisplay (d, *dpy);

     // this is necessary, since Qt just overwrites the
     // IOErrorHandler that was set by xdm!!!
     // we have to return RESERVER_DISPLAY to restart the server
     XSetIOErrorHandler(IOErrorHandler);
     
     sigaction(SIGCHLD, &sig, NULL);

     DoIt();
     
     /*
      * Run system-wide initialization file
      */
     if (source (verify->systemEnviron, ::d->startup) != 0)
     {
          QString buf = i18n("Startup program %1 exited with non-zero status."
			     "\nPlease contact your system administrator.\n"
			     "Please press OK to retry.")
	       .arg(QFile::decodeName(::d->startup));
	  qApp->restoreOverrideCursor();
	  KMessageBox::error(0, buf, i18n("Login aborted"));
	  SessionExit (::d, OBEYSESS_DISPLAY, FALSE);
     }

     // Clean up and log user in:
     XKillClient( qt_xdisplay(), AllTemporary);
     qApp->restoreOverrideCursor();
     delete kdmcfg;
     //delete myapp;
     return Greet_Success;
}

/*
 * Local variables:
 * mode: c++
 * c-file-style: "k&r"
 * End:
 */

