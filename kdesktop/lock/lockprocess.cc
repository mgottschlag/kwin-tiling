//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Oswald Buddenhagen <ossi@kde.org>
//

//kdesktop keeps running and checks user inactivity
//when it should show screensaver (and maybe lock the session),
//it starts kdesktop_lock, who does all the locking and who
//actually starts the screensaver

//It's done this way to prevent screen unlocking when kdesktop
//crashes (e.g. because it's set to multiple wallpapers and
//some image will be corrupted).

#include <config.h>

#include "lockprocess.h"
#include "lockdlg.h"
#include "autologout.h"
#include "kdesktopsettings.h"

#include <dmctl.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <klibloader.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>
#include <kauthorized.h>

#include <qframe.h>
#include <QLabel>
#include <QLayout>
#include <QCursor>
#include <QTimer>
#include <QFile>
#include <QSocketNotifier>
#include <QDesktopWidget>
#include <QX11Info>
#include <QTextStream>

#include <QDateTime>

#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#ifdef HAVE_SETPRIORITY
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>

#ifdef HAVE_DPMS
extern "C" {
#include <X11/Xmd.h>
#ifndef Bool
#define Bool BOOL
#endif
#include <X11/extensions/dpms.h>

#ifndef HAVE_DPMSINFO_PROTO
Status DPMSInfo ( Display *, CARD16 *, BOOL * );
#endif
}
#endif

#ifdef HAVE_XF86MISC
#include <X11/extensions/xf86misc.h>
#endif

#ifdef HAVE_GLXCHOOSEVISUAL
#include <GL/glx.h>
#endif

#define LOCK_GRACE_DEFAULT          5000
#define AUTOLOGOUT_DEFAULT          600

static Window gVRoot = 0;
static Window gVRootData = 0;
static Atom   gXA_VROOT;
static Atom   gXA_SCREENSAVER_VERSION;

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.f
//
LockProcess::LockProcess(bool child, bool useBlankOnly)
    : QWidget(0L, "saver window", Qt::WX11BypassWM),
      mOpenGLVisual(0),
      child_saver(child),
      mParent(0),
      mUseBlankOnly(useBlankOnly),
      mSuspended(false),
      mVisibility(false),
      mRestoreXF86Lock(false),
      mForbidden(false),
      mAutoLogout(false)
{
    setupSignals();

    kapp->installX11EventFilter(this);

    // Get root window size
    XWindowAttributes rootAttr;
	QX11Info info;
    XGetWindowAttributes(QX11Info::display(), RootWindow(QX11Info::display(),
                        info.screen()), &rootAttr);
    mRootWidth = rootAttr.width;
    mRootHeight = rootAttr.height;
    XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
        SubstructureNotifyMask | rootAttr.your_event_mask );

    // Add non-KDE path
    KGlobal::dirs()->addResourceType("scrsav",
                                    KGlobal::dirs()->kde_default("apps") +
                                    "System/ScreenSavers/");

    // Add KDE specific screensaver path
    QString relPath="System/ScreenSavers/";
    KServiceGroup::Ptr servGroup = KServiceGroup::baseGroup( "screensavers");
    if (servGroup)
    {
      relPath=servGroup->relPath();
      kDebug(1204) << "relPath=" << relPath << endl;
    }
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     relPath);

    // virtual root property
    gXA_VROOT = XInternAtom (QX11Info::display(), "__SWM_VROOT", False);
    gXA_SCREENSAVER_VERSION = XInternAtom (QX11Info::display(), "_SCREENSAVER_VERSION", False);

    connect(&mHackProc, SIGNAL(processExited(KProcess *)),
                        SLOT(hackExited(KProcess *)));

    mSuspendTimer.setSingleShot(true);
    connect(&mSuspendTimer, SIGNAL(timeout()), SLOT(suspend()));

    QStringList dmopt =
        QString::fromLatin1( ::getenv( "XDM_MANAGED" )).split(QChar(','), QString::SkipEmptyParts);
    for (QStringList::ConstIterator it = dmopt.begin(); it != dmopt.end(); ++it)
        if ((*it).startsWith("method="))
            mMethod = (*it).mid(7);

    configure();

#ifdef HAVE_DPMS
    if (mDPMSDepend) {
        BOOL on;
        CARD16 state;
        DPMSInfo(QX11Info::display(), &state, &on);
        if (on)
        {
            connect(&mCheckDPMS, SIGNAL(timeout()), SLOT(checkDPMSActive()));
            mCheckDPMS.start(60000);
        }
    }
#endif

    greetPlugin.library = 0;
}

//---------------------------------------------------------------------------
//
// Destructor - usual cleanups.
//
LockProcess::~LockProcess()
{
    if (greetPlugin.library) {
        if (greetPlugin.info->done)
            greetPlugin.info->done();
        greetPlugin.library->unload();
    }
}

static int sigterm_pipe[2];

static void sigterm_handler(int)
{
    char tmp = 0;
    ::write( sigterm_pipe[1], &tmp, 1);
}

void LockProcess::timerEvent(QTimerEvent *ev)
{
	if (ev->timerId() == mAutoLogoutTimerId)
	{
		killTimer(mAutoLogoutTimerId);
		AutoLogout autologout(this);
		execDialog(&autologout);
	}
}

void LockProcess::setupSignals()
{
    // ignore SIGHUP
    struct sigaction act;
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGHUP);
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, 0L);
    // ignore SIGINT
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGINT);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, 0L);
    // ignore SIGQUIT
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGQUIT);
    act.sa_flags = 0;
    sigaction(SIGQUIT, &act, 0L);
    // exit cleanly on SIGTERM
    act.sa_handler= sigterm_handler;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGTERM);
    act.sa_flags = 0;
    sigaction(SIGTERM, &act, 0L);

    pipe(sigterm_pipe);
    QSocketNotifier* notif = new QSocketNotifier(sigterm_pipe[0],
	QSocketNotifier::Read, this );
    connect( notif, SIGNAL(activated(int)), SLOT(sigtermPipeSignal()));
}


void LockProcess::sigtermPipeSignal()
{
    quitSaver();
}

//---------------------------------------------------------------------------
bool LockProcess::lock()
{
    if (startSaver()) {
        // In case of a forced lock we don't react to events during
        // the dead-time to give the screensaver some time to activate.
        // That way we don't accidentally show the password dialog before
        // the screensaver kicks in because the user moved the mouse after
        // selecting "lock screen", that looks really untidy.
        mBusy = true;
        if (startLock())
        {
            QTimer::singleShot(1000, this, SLOT(slotDeadTimePassed()));
            return true;
        }
        stopSaver();
        mBusy = false;
    }
    return false;
}
//---------------------------------------------------------------------------
void LockProcess::slotDeadTimePassed()
{
    mBusy = false;
}

//---------------------------------------------------------------------------
bool LockProcess::defaultSave()
{
    mLocked = false;
    if (startSaver()) {
        if (mLockGrace >= 0)
            QTimer::singleShot(mLockGrace, this, SLOT(startLock()));
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------
bool LockProcess::dontLock()
{
    mLocked = false;
    return startSaver();
}

//---------------------------------------------------------------------------
void LockProcess::quitSaver()
{
    stopSaver();
    kapp->quit();
}

//---------------------------------------------------------------------------
//
// Read and apply configuration.
//
void LockProcess::configure()
{
    // the configuration is stored in kdesktop's config file
    if( KDesktopSettings::lock() )
    {
        mLockGrace = KDesktopSettings::lockGrace();
        if (mLockGrace < 0)
            mLockGrace = 0;
        else if (mLockGrace > 300000)
            mLockGrace = 300000; // 5 minutes, keep the value sane
    }
    else
        mLockGrace = -1;

    if ( KDesktopSettings::autoLogout() )
    {
        mAutoLogout = true;
        mAutoLogoutTimeout = KDesktopSettings::autoLogoutTimeout();
        mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000); // in milliseconds
    }

#ifdef HAVE_DPMS
    //if the user  decided that the screensaver should run independent from
    //dpms, we shouldn't check for it, aleXXX
    mDPMSDepend = KDesktopSettings::dpmsDependent();
#endif

    mPriority = KDesktopSettings::priority();
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;

    mSaver = KDesktopSettings::saver();
    if (mSaver.isEmpty() || mUseBlankOnly)
        mSaver = "KBlankscreen.desktop";

    readSaver();

    mPlugins = KDesktopSettings::pluginsUnlock();
    if (mPlugins.isEmpty())
        mPlugins = QStringList("classic");
    mPluginOptions = KDesktopSettings::pluginOptions();
}

//---------------------------------------------------------------------------
//
// Read the command line needed to run the screensaver given a .desktop file.
//
void LockProcess::readSaver()
{
    if (!mSaver.isEmpty())
    {
        QString file = locate("scrsav", mSaver);

	bool opengl = KAuthorized::authorizeKAction("opengl_screensavers");
	bool manipulatescreen = KAuthorized::authorizeKAction("manipulatescreen_screensavers");
        KDesktopFile config(file, true);
	if (!config.readEntry("X-KDE-Type").toUtf8().isEmpty())
	{
		QString saverType = config.readEntry("X-KDE-Type").toUtf8();
		QStringList saverTypes = saverType.split( ";");
		for (int i = 0; i < saverTypes.count(); i++)
		{
			if ((saverTypes[i] == "ManipulateScreen") && !manipulatescreen)
			{
				kDebug(1204) << "Screensaver is type ManipulateScreen and ManipulateScreen is forbidden" << endl;
				mForbidden = true;
			}
			if ((saverTypes[i] == "OpenGL") && !opengl)
			{
				kDebug(1204) << "Screensaver is type OpenGL and OpenGL is forbidden" << endl;
				mForbidden = true;
			}
			if (saverTypes[i] == "OpenGL")
			{
				mOpenGLVisual = true;
			}
		}
	}

	kDebug(1204) << "mForbidden: " << (mForbidden ? "true" : "false") << endl;

        if (config.hasActionGroup("Root"))
        {
            config.setActionGroup("Root");
            mSaverExec = config.readPathEntry("Exec");
        }
    }
}

//---------------------------------------------------------------------------
//
// Create a window to draw our screen saver on.
//
void LockProcess::createSaverWindow()
{
    Visual* visual = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = 0;
#ifdef HAVE_GLXCHOOSEVISUAL
    if( mOpenGLVisual )
    {
        int attribs[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, x11Depth(), None };
        if( XVisualInfo* i = glXChooseVisual( x11Display(), x11Screen(), attribs ))
        {
            visual = i->visual;
            static Colormap colormap = 0;
            if( colormap != 0 )
                XFreeColormap( x11Display(), colormap );
            colormap = XCreateColormap( x11Display(), RootWindow( x11Display(), x11Screen()), visual, AllocNone );
            attrs.colormap = colormap;
            flags |= CWColormap;
            XFree( i );
        }
    }
#endif
    Window w = XCreateWindow( x11Display(), RootWindow( x11Display(), x11Screen()),
        x(), y(), width(), height(), 0, x11Depth(), InputOutput, visual, flags, &attrs );
    create( w );

    // Some xscreensaver hacks check for this property
    const char *version = "KDE 2.0";
    XChangeProperty (QX11Info::display(), winId(),
                     gXA_SCREENSAVER_VERSION, XA_STRING, 8, PropModeReplace,
                     (unsigned char *) version, strlen(version));

    XSetWindowAttributes attr;
    attr.event_mask = KeyPressMask | ButtonPressMask | PointerMotionMask |
                        VisibilityChangeMask | ExposureMask;
    XChangeWindowAttributes(QX11Info::display(), winId(),
                            CWEventMask, &attr);

    // erase();

    // set NoBackground so that the saver can capture the current
    // screen state if necessary
    setAttribute(Qt::WA_NoSystemBackground, true);

    setCursor( Qt::BlankCursor );
    setGeometry(0, 0, mRootWidth, mRootHeight);
    hide();

    kDebug(1204) << "Saver window Id: " << winId() << endl;
}

//---------------------------------------------------------------------------
//
// Hide the screensaver window
//
void LockProcess::hideSaverWindow()
{
  hide();
  lower();
  removeVRoot(winId());
  XDeleteProperty(QX11Info::display(), winId(), gXA_SCREENSAVER_VERSION);
  if ( gVRoot ) {
      unsigned long vroot_data[1] = { gVRootData };
      XChangeProperty(QX11Info::display(), gVRoot, gXA_VROOT, XA_WINDOW, 32,
                      PropModeReplace, (unsigned char *)vroot_data, 1);
      gVRoot = 0;
  }
  XSync(QX11Info::display(), False);
}

//---------------------------------------------------------------------------
static int ignoreXError(Display *, XErrorEvent *)
{
    return 0;
}

//---------------------------------------------------------------------------
//
// Save the current virtual root window
//
void LockProcess::saveVRoot()
{
  Window rootReturn, parentReturn, *children;
  unsigned int numChildren;
  QX11Info info;
  Window root = RootWindowOfScreen(ScreenOfDisplay(QX11Info::display(), info.screen()));

  gVRoot = 0;
  gVRootData = 0;

  int (*oldHandler)(Display *, XErrorEvent *);
  oldHandler = XSetErrorHandler(ignoreXError);

  if (XQueryTree(QX11Info::display(), root, &rootReturn, &parentReturn,
      &children, &numChildren))
  {
    for (unsigned int i = 0; i < numChildren; i++)
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytesafter;
      Window *newRoot = (Window *)0;

      if ((XGetWindowProperty(QX11Info::display(), children[i], gXA_VROOT, 0, 1,
          False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
          (unsigned char **) &newRoot) == Success) && newRoot)
      {
        gVRoot = children[i];
        gVRootData = *newRoot;
        XFree ((char*) newRoot);
        break;
      }
    }
    if (children)
    {
      XFree((char *)children);
    }
  }

  XSetErrorHandler(oldHandler);
}

//---------------------------------------------------------------------------
//
// Set the virtual root property
//
void LockProcess::setVRoot(Window win, Window vr)
{
    if (gVRoot)
        removeVRoot(gVRoot);

	QX11Info info;
    unsigned long rw = RootWindowOfScreen(ScreenOfDisplay(QX11Info::display(), info.screen()));
    unsigned long vroot_data[1] = { vr };

    Window rootReturn, parentReturn, *children;
    unsigned int numChildren;
    Window top = win;
    while (1) {
        XQueryTree(QX11Info::display(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren);
        if (children)
            XFree((char *)children);
        if (parentReturn == rw) {
            break;
        } else
            top = parentReturn;
    }

    XChangeProperty(QX11Info::display(), top, gXA_VROOT, XA_WINDOW, 32,
                     PropModeReplace, (unsigned char *)vroot_data, 1);
}

//---------------------------------------------------------------------------
//
// Remove the virtual root property
//
void LockProcess::removeVRoot(Window win)
{
    XDeleteProperty (QX11Info::display(), win, gXA_VROOT);
}

//---------------------------------------------------------------------------
//
// Grab the keyboard. Returns true on success
//
bool LockProcess::grabKeyboard()
{
    int rv = XGrabKeyboard( QX11Info::display(), QApplication::desktop()->winId(),
        True, GrabModeAsync, GrabModeAsync, CurrentTime );

    return (rv == GrabSuccess);
}

#define GRABEVENTS ButtonPressMask | ButtonReleaseMask | PointerMotionMask | \
		   EnterWindowMask | LeaveWindowMask

//---------------------------------------------------------------------------
//
// Grab the mouse.  Returns true on success
//
bool LockProcess::grabMouse()
{
    int rv = XGrabPointer( QX11Info::display(), QApplication::desktop()->winId(),
            True, GRABEVENTS, GrabModeAsync, GrabModeAsync, None,
            QCursor(Qt::BlankCursor).handle(), CurrentTime );

    return (rv == GrabSuccess);
}

//---------------------------------------------------------------------------
//
// Grab keyboard and mouse.  Returns true on success.
//
bool LockProcess::grabInput()
{
    XSync(QX11Info::display(), False);

    if (!grabKeyboard())
    {
        sleep(1);
        if (!grabKeyboard())
        {
            return false;
        }
    }

    if (!grabMouse())
    {
        sleep(1);
        if (!grabMouse())
        {
            XUngrabKeyboard(QX11Info::display(), CurrentTime);
            return false;
        }
    }

    lockXF86();

    return true;
}

//---------------------------------------------------------------------------
//
// Release mouse an keyboard grab.
//
void LockProcess::ungrabInput()
{
    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    XUngrabPointer(QX11Info::display(), CurrentTime);
    unlockXF86();
}

//---------------------------------------------------------------------------
//
// Start the screen saver.
//
bool LockProcess::startSaver()
{
    if (!child_saver && !grabInput())
    {
        kWarning(1204) << "LockProcess::startSaver() grabInput() failed!!!!" << endl;
        return false;
    }
    mBusy = false;

    saveVRoot();

    if (mParent) {
        QSocketNotifier *notifier = new QSocketNotifier(mParent, QSocketNotifier::Read, this, "notifier");
        connect(notifier, SIGNAL( activated (int)), SLOT( quitSaver()));
    }
    createSaverWindow();
    move(0, 0);
    show();
    setCursor( Qt::BlankCursor );

    raise();
    XSync(QX11Info::display(), False);
    setVRoot( winId(), winId() );
    startHack();
    return true;
}

//---------------------------------------------------------------------------
//
// Stop the screen saver.
//
void LockProcess::stopSaver()
{
    kDebug(1204) << "LockProcess: stopping saver" << endl;
    resume();
    stopHack();
    hideSaverWindow();
    mVisibility = false;
    if (!child_saver) {
        if (mLocked)
            DM().setLock( false );
        ungrabInput();
        const char *out = "GOAWAY!";
        for (QList<int>::ConstIterator it = child_sockets.begin(); it != child_sockets.end(); ++it)
            write(*it, out, sizeof(out));
    }
}

// private static
QVariant LockProcess::getConf(void *ctx, const char *key, const QVariant &dflt)
{
    LockProcess *that = (LockProcess *)ctx;
    QString fkey = QLatin1String( key ) + '=';
    for (QStringList::ConstIterator it = that->mPluginOptions.begin();
         it != that->mPluginOptions.end(); ++it)
        if ((*it).startsWith( fkey ))
            return (*it).mid( fkey.length() );
    return dflt;
}

void LockProcess::cantLock( const QString &txt)
{
    msgBox( QMessageBox::Critical, i18n("Will not lock the session, as unlocking would be impossible:\n") + txt );
}

#if 0 // placeholders for later
i18n("Cannot start <i>kcheckpass</i>.");
i18n("<i>kcheckpass</i> is unable to operate. Possibly it is not SetUID root.");
#endif

//---------------------------------------------------------------------------
//
// Make the screen saver password protected.
//
bool LockProcess::startLock()
{
    for (QStringList::ConstIterator it = mPlugins.begin(); it != mPlugins.end(); ++it) {
        GreeterPluginHandle plugin;
        QString path = KLibLoader::self()->findLibrary(
                    ((*it)[0] == '/' ? *it : "kgreet_" + *it ).toLatin1() );
        if (path.isEmpty()) {
            kWarning(1204) << "GreeterPlugin " << *it << " does not exist" << endl;
            continue;
        }
        if (!(plugin.library = KLibLoader::self()->library( path.toLatin1() ))) {
            kWarning(1204) << "Cannot load GreeterPlugin " << *it << " (" << path << ")" << endl;
            continue;
        }
        if (!plugin.library->hasSymbol( "kgreeterplugin_info" )) {
            kWarning(1204) << "GreeterPlugin " << *it << " (" << path << ") is no valid greet widget plugin" << endl;
            plugin.library->unload();
            continue;
        }
        plugin.info = (kgreeterplugin_info*)plugin.library->symbol( "kgreeterplugin_info" );
        if (plugin.info->method && !mMethod.isEmpty() && mMethod != plugin.info->method) {
            kDebug(1204) << "GreeterPlugin " << *it << " (" << path << ") serves " << plugin.info->method << ", not " << mMethod << endl;
            plugin.library->unload();
            continue;
        }
        if (!plugin.info->init( mMethod, getConf, this )) {
            kDebug(1204) << "GreeterPlugin " << *it << " (" << path << ") refuses to serve " << mMethod << endl;
            plugin.library->unload();
            continue;
        }
        kDebug(1204) << "GreeterPlugin " << *it << " (" << plugin.info->method << ", " << plugin.info->name << ") loaded" << endl;
        greetPlugin = plugin;
	mLocked = true;
	DM().setLock( true );
	return true;
    }
    cantLock( i18n("No appropriate greeter plugin configured.") );
    return false;
}

//---------------------------------------------------------------------------
//


bool LockProcess::startHack()
{
    if (mSaverExec.isEmpty())
    {
        return false;
    }

    if (mHackProc.isRunning())
    {
        stopHack();
    }

    mHackProc.clearArguments();

    QTextStream ts(&mSaverExec, QIODevice::ReadOnly);
    QString word;
    ts >> word;
    QString path = KStandardDirs::findExe(word);

    if (!path.isEmpty())
    {
        mHackProc << path;

        kDebug(1204) << "Starting hack: " << path << endl;

        while (!ts.atEnd())
        {
            ts >> word;
            if (word == "%w")
            {
                word = word.setNum(winId());
            }
            mHackProc << word;
        }

	    if (!mForbidden)
	    {

		    if (mHackProc.start() == true)
		    {
#ifdef HAVE_SETPRIORITY
			    setpriority(PRIO_PROCESS, mHackProc.pid(), mPriority);
#endif
 		        //bitBlt(this, 0, 0, &mOriginal);
	            return true;
		    }
	    }
	    else
	    // we aren't allowed to start the specified screensaver either because it didn't run for some reason
	    // according to the kiosk restrictions forbid it
	    {
            QPalette palette;
            palette.setColor(backgroundRole(), Qt::black);
            setPalette(palette);
	    }
    }
    return false;
}

//---------------------------------------------------------------------------
//
void LockProcess::stopHack()
{
    if (mHackProc.isRunning())
    {
        mHackProc.kill();
    }
}

//---------------------------------------------------------------------------
//
void LockProcess::hackExited(KProcess *)
{
	// Hack exited while we're supposed to be saving the screen.
	// Make sure the saver window is black.
    QPalette palette;
    palette.setColor(backgroundRole(), Qt::black);
    setPalette(palette);
}

void LockProcess::suspend()
{
    if(!mSuspended)
    {
        mHackProc.kill(SIGSTOP);
        QApplication::syncX();
        mSavedScreen = QPixmap::grabWindow( winId());
    }
    mSuspended = true;
}

void LockProcess::resume()
{
    if (!mDialogs.isEmpty())
        return; // no resuming with dialog visible
    if(!mVisibility)
        return; // no need to resume, not visible
    if(mSuspended)
    {
        bitBlt( this, 0, 0, &mSavedScreen );
        QApplication::syncX();
        mHackProc.kill(SIGCONT);
    }
    mSuspended = false;
}

//---------------------------------------------------------------------------
//
// Show the password dialog
// This is called only in the master process
//
bool LockProcess::checkPass()
{
    killTimer(mAutoLogoutTimerId);
    PasswordDlg passDlg( this, &greetPlugin);

    int ret = execDialog( &passDlg );

    XWindowAttributes rootAttr;
    XGetWindowAttributes(QX11Info::display(), QX11Info::appRootWindow(), &rootAttr);
    if(( rootAttr.your_event_mask & SubstructureNotifyMask ) == 0 )
    {
        kWarning() << "ERROR: Something removed SubstructureNotifyMask from the root window!!!" << endl;
        XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
            SubstructureNotifyMask | rootAttr.your_event_mask );
    }

    return ret == QDialog::Accepted;
}

static void fakeFocusIn( WId window )
{
    // We have keyboard grab, so this application will
    // get keyboard events even without having focus.
    // Fake FocusIn to make Qt realize it has the active
    // window, so that it will correctly show cursor in the dialog.
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xfocus.display = QX11Info::display();
    ev.xfocus.type = FocusIn;
    ev.xfocus.window = window;
    ev.xfocus.mode = NotifyNormal;
    ev.xfocus.detail = NotifyAncestor;
    XSendEvent( QX11Info::display(), window, False, NoEventMask, &ev );
}

int LockProcess::execDialog( QDialog *dlg )
{
    dlg->adjustSize();

    QRect rect = dlg->geometry();
    rect.moveCenter(KGlobalSettings::desktopGeometry(QCursor::pos()).center());
    dlg->move( rect.topLeft() );

    if (mDialogs.isEmpty())
    {
        suspend();
        XChangeActivePointerGrab( QX11Info::display(), GRABEVENTS,
                QCursor(Qt::ArrowCursor).handle(), CurrentTime);
    }
    mDialogs.prepend( dlg );
    fakeFocusIn( dlg->winId());
    int rt = dlg->exec();
    int pos = mDialogs.indexOf( dlg );
    if (pos != -1)
        mDialogs.remove( pos );
    if( mDialogs.isEmpty() ) {
        XChangeActivePointerGrab( QX11Info::display(), GRABEVENTS,
                QCursor(Qt::BlankCursor).handle(), CurrentTime);
        resume();
    } else
        fakeFocusIn( mDialogs.first()->winId());
    return rt;
}

void LockProcess::preparePopup()
{
    QWidget *dlg = (QWidget *)sender();
    mDialogs.prepend( dlg );
    fakeFocusIn( dlg->winId() );
}

void LockProcess::cleanupPopup()
{
    QWidget *dlg = (QWidget *)sender();

    int pos = mDialogs.indexOf( dlg );
    mDialogs.remove( pos );
    fakeFocusIn( mDialogs.first()->winId() );
}

//---------------------------------------------------------------------------
//
// X11 Event.
//
bool LockProcess::x11Event(XEvent *event)
{
    switch (event->type)
    {
        case KeyPress:
        case ButtonPress:
        case MotionNotify:
            if (mBusy || !mDialogs.isEmpty())
                break;
            mBusy = true;
            if (!mLocked || checkPass())
            {
                stopSaver();
                kapp->quit();
            }
            else if (mAutoLogout) // we need to restart the auto logout countdown
            {
                killTimer(mAutoLogoutTimerId);
                mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout);
            }
            mBusy = false;
            return true;

        case VisibilityNotify:
            if( event->xvisibility.window == winId())
            {  // mVisibility == false means the screensaver is not visible at all
               // e.g. when switched to text console
                mVisibility = !(event->xvisibility.state == VisibilityFullyObscured);
                if(!mVisibility)
                    mSuspendTimer.start(2000);
		else
                {
                    mSuspendTimer.stop();
                    resume();
                }
                if (event->xvisibility.state != VisibilityUnobscured)
                    stayOnTop();
            }
            break;

        case ConfigureNotify: // from SubstructureNotifyMask on the root window
            if(event->xconfigure.event == QX11Info::appRootWindow())
                stayOnTop();
            break;
        case MapNotify: // from SubstructureNotifyMask on the root window
            if( event->xmap.event == QX11Info::appRootWindow())
                stayOnTop();
            break;
    }

    // We have grab with the grab window being the root window.
    // This results in key events being sent to the root window,
    // but they should be sent to the dialog if it's visible.
    // It could be solved by setFocus() call, but that would mess
    // the focus after this process exits.
    // Qt seems to be quite hard to persuade to redirect the event,
    // so let's simply dupe it with correct destination window,
    // and ignore the original one.
    if(!mDialogs.isEmpty() && ( event->type == KeyPress || event->type == KeyRelease)
        && event->xkey.window != mDialogs.first()->winId())
    {
        XEvent ev2 = *event;
        ev2.xkey.window = ev2.xkey.subwindow = mDialogs.first()->winId();
        qApp->x11ProcessEvent( &ev2 );
        return true;
    }

    return false;
}

void LockProcess::stayOnTop()
{
    if(!mDialogs.isEmpty())
    {
        // this restacking is written in a way so that
        // if the stacking positions actually don't change,
        // all restacking operations will be no-op,
        // and no ConfigureNotify will be generated,
        // thus avoiding possible infinite loops
        XRaiseWindow( QX11Info::display(), mDialogs.first()->winId()); // raise topmost
        // and stack others below it
        Window* stack = new Window[ mDialogs.count() + 1 ];
        int count = 0;
        for( QVector< QWidget* >::ConstIterator it = mDialogs.begin();
             it != mDialogs.end();
             ++it )
            stack[ count++ ] = (*it)->winId();
        stack[ count++ ] = winId();
        XRestackWindows( x11Display(), stack, count );
        delete[] stack;
    }
    else
        XRaiseWindow(QX11Info::display(), winId());
}

void LockProcess::checkDPMSActive()
{
#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo(QX11Info::display(), &state, &on);
    if (state == DPMSModeStandby || state == DPMSModeSuspend || state == DPMSModeOff)
    {
       suspend();
    }
#endif
}

#if defined(HAVE_XF86MISC) && defined(HAVE_XF86MISCSETGRABKEYSSTATE)
// see http://cvsweb.xfree86.org/cvsweb/xc/programs/Xserver/hw/xfree86/common/xf86Events.c#rev3.113
// This allows enabling the "Allow{Deactivate/Closedown}Grabs" options in XF86Config,
// and kdesktop_lock will still lock the session.
static enum { Unknown, Yes, No } can_do_xf86_lock = Unknown;
void LockProcess::lockXF86()
{
    if( can_do_xf86_lock == Unknown )
    {
        int major, minor;
        if( XF86MiscQueryVersion( QX11Info::display(), &major, &minor )
            && major >= 0 && minor >= 5 )
            can_do_xf86_lock = Yes;
        else
            can_do_xf86_lock = No;
    }
    if( can_do_xf86_lock != Yes )
        return;
    if( mRestoreXF86Lock )
        return;
    if( XF86MiscSetGrabKeysState( QX11Info::display(), False ) != MiscExtGrabStateSuccess )
        return;
    // success
    mRestoreXF86Lock = true;
}

void LockProcess::unlockXF86()
{
    if( can_do_xf86_lock != Yes )
        return;
    if( !mRestoreXF86Lock )
        return;
    XF86MiscSetGrabKeysState( QX11Info::display(), True );
    mRestoreXF86Lock = false;
}
#else
void LockProcess::lockXF86()
{
}

void LockProcess::unlockXF86()
{
}
#endif

void LockProcess::msgBox( QMessageBox::Icon type, const QString &txt )
{
    QDialog box( 0, "messagebox", true, Qt::WX11BypassWM );
    QFrame *winFrame = new QFrame( &box );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    QLabel *label1 = new QLabel( winFrame );
    label1->setPixmap( QMessageBox::standardIcon( type ) );
    QLabel *label2 = new QLabel( txt, winFrame );
    KPushButton *button = new KPushButton( KStdGuiItem::ok(), winFrame );
    button->setDefault( true );
    button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    connect( button, SIGNAL( clicked() ), &box, SLOT( accept() ) );

    QVBoxLayout *vbox = new QVBoxLayout( &box );
    vbox->addWidget( winFrame );
    QGridLayout *grid = new QGridLayout( winFrame );
    grid->setSpacing( 10 );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    grid->addWidget( button, 1, 0, 1, 2, Qt::AlignCenter );

    execDialog( &box );
}

#include "lockprocess.moc"
