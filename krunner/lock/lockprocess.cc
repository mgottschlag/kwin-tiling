//===========================================================================
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Oswald Buddenhagen <ossi@kde.org>
// Copyright 2008 Chani Armitage <chanika@gmail.com>
//

//krunner keeps running and checks user inactivity
//when it should show screensaver (and maybe lock the session),
//it starts kscreenlocker, who does all the locking and who
//actually starts the screensaver

//It's done this way to prevent screen unlocking when krunner
//crashes

#include "lockprocess.h"
#include "lockprocessadaptor.h"

#include <config-workspace.h>
#include <config-X11.h>
#include <config-krunner-lock.h>
#include "lockdlg.h"
#include "autologout.h"
#include "kscreensaversettings.h"

#include <kephal/screens.h>

#include <kworkspace/kdisplaymanager.h>

#include <KStandardDirs>
#include <KApplication>
#include <KServiceGroup>
#include <KDebug>
#include <KMessageBox>
#include <KGlobalSettings>
#include <KLocale>
#include <KLibrary>
#include <KNotification>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KAuthorized>
#include <KDesktopFile>
#include <kservicetypetrader.h>
#include <kmacroexpander.h>
#include <kshell.h>
#include <kxerrorhandler.h>

#include <QtGui/QFrame>
#include <QLabel>
#include <QLayout>
#include <QCursor>
#include <QTimer>
#include <QFile>
#include <QSocketNotifier>
#include <QDesktopWidget>
#include <QX11Info>
#include <QTextStream>
#include <QPainter>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include <QtCore/QStringBuilder> // % operator for QString

#include <QDateTime>

#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <unistd.h>
#ifdef HAVE_SETPRIORITY
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

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

//#define CHECK_XSELECTINPUT
#ifdef CHECK_XSELECTINPUT
#include <dlfcn.h>
static bool check_xselectinput = false;
extern "C"
int XSelectInput( Display* dpy, Window w, long e )
{
    typedef int (*ptr)(Display*, Window, long);
    static ptr fun = NULL;
    if( fun == NULL )
        fun = (ptr)dlsym( RTLD_NEXT, "XSelectInput" );
    if( check_xselectinput && w == DefaultRootWindow( dpy ))
        kDebug() << kBacktrace();
    return fun( dpy, w, e );
}
#endif

static QLatin1String s_overlayServiceName("org.kde.plasma-overlay");

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.f
//
LockProcess::LockProcess(bool child, bool useBlankOnly)
    : QWidget(0L, Qt::X11BypassWindowManagerHint),
      mInitialLock(false),
      mLocked(false),
      mBusy(false),
      mPlasmaDBus(0),
      mServiceWatcher(0),
      mSetupMode(false),
      mOpenGLVisual(false),
      child_saver(child),
      mParent(0),
      mUseBlankOnly(useBlankOnly),
      mSuspended(false),
      mVisibility(false),
      mEventRecursed(false),
      mRestoreXF86Lock(false),
      mForbidden(false),
      mAutoLogoutTimerId(0)
{
    setObjectName(QLatin1String( "save window" ));
    setupSignals();

    new LockProcessAdaptor(this);
    QDBusConnection::sessionBus().registerService(QLatin1String( "org.kde.screenlocker" ));
    QDBusConnection::sessionBus().registerObject(QLatin1String( "/LockProcess" ), this);

    kapp->installX11EventFilter(this);

    // Get root window size
    XWindowAttributes rootAttr;
    QX11Info info;
    XGetWindowAttributes(QX11Info::display(), RootWindow(QX11Info::display(),
                                                         info.screen()), &rootAttr);
    kapp->desktop(); // make Qt set its event mask on the root window first
    XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
                  SubstructureNotifyMask | rootAttr.your_event_mask );
#ifdef CHECK_XSELECTINPUT
    check_xselectinput = true;
#endif
    setGeometry(0, 0, rootAttr.width, rootAttr.height);

    // virtual root property
    gXA_VROOT = XInternAtom (QX11Info::display(), "__SWM_VROOT", False);
    gXA_SCREENSAVER_VERSION = XInternAtom (QX11Info::display(), "_SCREENSAVER_VERSION", False);

    connect(&mHackProc, SIGNAL(finished(int, QProcess::ExitStatus)),
            SLOT(hackExited()));

    mSuspendTimer.setSingleShot(true);
    connect(&mSuspendTimer, SIGNAL(timeout()), SLOT(suspend()));

    const QStringList dmopt =
        QString::fromLatin1( ::getenv( "XDM_MANAGED" )).split(QLatin1Char(','), QString::SkipEmptyParts);
    for (QStringList::ConstIterator it = dmopt.constBegin(); it != dmopt.constEnd(); ++it)
        if ((*it).startsWith(QLatin1String( "method=" )))
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
            // we can save CPU if we stop it as quickly as possible
            // but we waste CPU if we check too often -> so take 10s
            mCheckDPMS.start(10000);
        }
    }
#endif

    greetPlugin.library = 0;

    mSuppressUnlock.setSingleShot(true);
    connect(&mSuppressUnlock, SIGNAL(timeout()), SLOT(deactivatePlasma()));

    // read the initial information about all toplevel windows
    Window r, p;
    Window* real;
    unsigned nreal;
    if( XQueryTree( x11Info().display(), x11Info().appRootWindow(), &r, &p, &real, &nreal )
        && real != NULL ) {
        KXErrorHandler err; // ignore X errors here
        for( unsigned i = 0; i < nreal; ++i ) {
            XWindowAttributes winAttr;
            if (XGetWindowAttributes(QX11Info::display(), real[ i ], &winAttr)) {
                WindowInfo info;
                info.window = real[ i ];
                info.viewable = ( winAttr.map_state == IsViewable );
                windowInfo.append( info ); // ordered bottom to top
            }
        }
        XFree( real );
    }
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

static int signal_pipe[2];

static void sigterm_handler(int)
{
    char tmp = 'T';
    ::write( signal_pipe[1], &tmp, 1);
}

static void sighup_handler(int)
{
    char tmp = 'H';
    ::write( signal_pipe[1], &tmp, 1);
}

static void sigusr1_handler(int)
{
    char tmp = '1';
    ::write(signal_pipe[1], &tmp, 1);
}

void LockProcess::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mAutoLogoutTimerId)
    {
        AutoLogout autologout(this);
        execDialog(&autologout);
    }
}

void LockProcess::setupSignals()
{
    struct sigaction act;
    sigemptyset(&(act.sa_mask));
    act.sa_flags = 0;
    // ignore SIGINT
    act.sa_handler=SIG_IGN;
    sigaction(SIGINT, &act, 0L);
    // ignore SIGQUIT
    //act.sa_handler=SIG_IGN;
    sigaction(SIGQUIT, &act, 0L);
    // exit cleanly on SIGTERM
    act.sa_handler= sigterm_handler;
    sigaction(SIGTERM, &act, 0L);
    // SIGHUP forces lock
    act.sa_handler= sighup_handler;
    sigaction(SIGHUP, &act, 0L);
    // SIGUSR1 simulates user activity
    act.sa_handler= sigusr1_handler;
    sigaction(SIGUSR1, &act, 0L);

    pipe(signal_pipe);
    QSocketNotifier* notif = new QSocketNotifier(signal_pipe[0], QSocketNotifier::Read, this);
    connect( notif, SIGNAL(activated(int)), SLOT(signalPipeSignal()));
}


void LockProcess::signalPipeSignal()
{
    char tmp;
    ::read( signal_pipe[0], &tmp, 1);
    if (tmp == 'T') {
        quitSaver();
    } else if (tmp == '1') {
        // In case SimulateUserActivity (SIGUSR1) is called during the dead-time (mBusy == true).
        mInitialLock = true;
        if (!mBusy && mDialogs.isEmpty()) {
            mBusy = true;
            quit();
            mBusy = false;
        }
    } else if (tmp == 'H') {
        if( !mLocked )
            startLock();
    }
}

//---------------------------------------------------------------------------
bool LockProcess::lock(bool initial)
{
    if (startSaver()) {
        // In case of a forced lock we don't react to events during
        // the dead-time to give the screensaver some time to activate.
        // That way we don't accidentally show the password dialog before
        // the screensaver kicks in because the user moved the mouse after
        // selecting "lock screen", that looks really untidy.
        mBusy = true;
        mInitialLock = initial;
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
    if (mInitialLock)
        quit();
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

bool LockProcess::startSetup()
{
    mPlasmaEnabled = true; //force it on in case the user didn't click apply yet
    mLocked = false;
    mSetupMode = true;
    return startSaver();
    //plasma startup will handle the suppressunlock bit
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
    qApp->quit();
}

//---------------------------------------------------------------------------
//
// Read and apply configuration.
//
void LockProcess::configure()
{
    // the configuration is stored in krunner's config file
    if( KScreenSaverSettings::lock() ) {
        mLockGrace = KScreenSaverSettings::lockGrace();
        if (mLockGrace < 0)
            mLockGrace = 0;
        else if (mLockGrace > 300000)
            mLockGrace = 300000; // 5 minutes, keep the value sane
    } else {
        mLockGrace = -1;
    }

    mAutoLogoutTimeout = KScreenSaverSettings::autoLogout() ?
                         KScreenSaverSettings::autoLogoutTimeout() : 0;

#ifdef HAVE_DPMS
    mDPMSDepend = KScreenSaverSettings::suspendWhenInvisible();
#endif

    mPriority = KScreenSaverSettings::priority();
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;

    mSaver = KScreenSaverSettings::saver();
    if (mSaver.isEmpty() || mUseBlankOnly) {
        mSaver = QLatin1String( "kblank.desktop" );
    }

    readSaver();

    mPlasmaEnabled = KScreenSaverSettings::plasmaEnabled();

    mSuppressUnlockTimeout = qMax(0, KScreenSaverSettings::timeout() * 1000);
    mSuppressUnlockTimeout = qMax(mSuppressUnlockTimeout, 30 * 1000); //min. 30 secs FIXME is this a good idea?

    mPlugins = KScreenSaverSettings::pluginsUnlock();
    if (mPlugins.isEmpty()) {
        mPlugins << QLatin1String( "classic" ) << QLatin1String( "generic" );
    }
    mPluginOptions = KScreenSaverSettings::pluginOptions();
}

//---------------------------------------------------------------------------
//
// Read the command line needed to run the screensaver given a .desktop file.
//
void LockProcess::readSaver()
{
    if (!mSaver.isEmpty())
    {
        QString entryName = mSaver;
        if( entryName.endsWith( QLatin1String( ".desktop" ) ))
            entryName = entryName.left( entryName.length() - 8 ); // strip it
        const KService::List offers = KServiceTypeTrader::self()->query( QLatin1String( "ScreenSaver" ),
            QLatin1String( "DesktopEntryName == '" ) + entryName.toLower() + QLatin1Char( '\'' ) );
        if( offers.isEmpty() )
        {
            kDebug(1204) << "Cannot find screesaver: " << mSaver;
            return;
        }
        const QString file = KStandardDirs::locate("services", offers.first()->entryPath());

        const bool opengl = KAuthorized::authorizeKAction(QLatin1String( "opengl_screensavers" ));
        const bool manipulatescreen = KAuthorized::authorizeKAction(QLatin1String( "manipulatescreen_screensavers" ));
        KDesktopFile config( file );
        KConfigGroup desktopGroup = config.desktopGroup();
        foreach (const QString &type, desktopGroup.readEntry("X-KDE-Type").split(QLatin1Char(';'))) {
            if (type == QLatin1String("ManipulateScreen")) {
                if (!manipulatescreen) {
                    kDebug(1204) << "Screensaver is type ManipulateScreen and ManipulateScreen is forbidden";
                    mForbidden = true;
                }
            } else if (type == QLatin1String("OpenGL")) {
                mOpenGLVisual = true;
                if (!opengl) {
                    kDebug(1204) << "Screensaver is type OpenGL and OpenGL is forbidden";
                    mForbidden = true;
                }
            }
        }

        kDebug(1204) << "mForbidden: " << (mForbidden ? "true" : "false");

        if (config.hasActionGroup(QLatin1String( "Root" )))
        {
            mSaverExec = config.actionGroup(QLatin1String( "Root" )).readPathEntry("Exec", QString());
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
    int depth = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = CWOverrideRedirect;
#ifdef HAVE_GLXCHOOSEVISUAL
// this code is (partially) duplicated in kdebase/workspace/kcontrol/screensaver
    if( mOpenGLVisual )
    {
        static const int attribs[][ 15 ] =
        {
        #define R GLX_RED_SIZE
        #define G GLX_GREEN_SIZE
        #define B GLX_BLUE_SIZE
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, R, 8, G, 8, B, 8, GLX_DEPTH_SIZE, 8, None },
            { GLX_RGBA, R, 4, G, 4, B, 4, GLX_DEPTH_SIZE, 4, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_STENCIL_SIZE, 1, None },
            { GLX_RGBA, GLX_DEPTH_SIZE, 8, None }
        #undef R
        #undef G
        #undef B
        };
        for( unsigned int i = 0;
             i < sizeof( attribs ) / sizeof( attribs[ 0 ] );
             ++i )
        {
            if( XVisualInfo* info = glXChooseVisual( x11Info().display(), x11Info().screen(), const_cast<int*>(attribs[ i ]) ))
            {
                visual = info->visual;
                depth = info->depth;
                static Colormap colormap = 0;
                if( colormap != 0 )
                    XFreeColormap( x11Info().display(), colormap );
                colormap = XCreateColormap( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()), visual, AllocNone );
                attrs.colormap = colormap;
                flags |= CWColormap;
                XFree( info );
                break;
            }
        }
    }
#endif
    attrs.override_redirect = 1;
    hide();
    Window w = XCreateWindow( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()),
        x(), y(), width(), height(), 0, depth, InputOutput, visual, flags, &attrs );

    create( w, false, true );

    // Some xscreensaver hacks check for this property
    const char *version = "KDE 4.0";
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
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_PaintOutsidePaintEvent, true); // for bitBlt in resume()

    setCursor( Qt::BlankCursor );

    kDebug(1204) << "Saver window Id: " << winId();
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
      unsigned char *newRoot = 0;

      if ((XGetWindowProperty(QX11Info::display(), children[i], gXA_VROOT, 0, 1,
          False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
          &newRoot) == Success) && newRoot)
      {
        gVRoot = children[i];
        Window *dummy = (Window*)newRoot;
        gVRootData = *dummy;
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
        if (!XQueryTree(QX11Info::display(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren))
            return;
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
        kWarning(1204) << "LockProcess::startSaver() grabInput() failed!!!!" ;
        return false;
    }
    mBusy = false;

    saveVRoot();

    if (mParent) {
        QSocketNotifier *notifier = new QSocketNotifier(mParent, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL( activated (int)), SLOT( quitSaver()));
    }
    if (mAutoLogoutTimeout && !mSetupMode)
        mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000); // in milliseconds
    createSaverWindow();
    move(0, 0);
    show();
    setCursor( Qt::BlankCursor );

    raise();
    XSync(QX11Info::display(), False);

    setVRoot( winId(), winId() );
    startHack();
    startPlasma();
    KNotification::event( QLatin1String( "savingstarted" ) );
    return true;
}

//---------------------------------------------------------------------------
//
// Stop the screen saver.
//
void LockProcess::stopSaver()
{
    kDebug(1204) << "LockProcess: stopping saver";
    resume( true );
    stopPlasma();
    stopHack();
    hideSaverWindow();
    mVisibility = false;
    if (!child_saver) {
        if (mLocked) {
            KDisplayManager().setLock( false );
            mLocked = false;
            KNotification *u = new KNotification( QLatin1String( "unlocked" ) );
	    u->sendEvent();
        }
        ungrabInput();
        const char *out = "GOAWAY!";
        for (QList<int>::ConstIterator it = child_sockets.constBegin(); it != child_sockets.constEnd(); ++it)
            write(*it, out, sizeof(out));
    }
    KNotification *s = new KNotification( QLatin1String( "savingstopped" ) );
    s->sendEvent();
}

// private static
QVariant LockProcess::getConf(void *ctx, const char *key, const QVariant &dflt)
{
    LockProcess *that = (LockProcess *)ctx;
    QString fkey = QLatin1String( key ) % QLatin1Char( '=' );
    for (QStringList::ConstIterator it = that->mPluginOptions.constBegin();
         it != that->mPluginOptions.constEnd(); ++it)
        if ((*it).startsWith( fkey ))
            return (*it).mid( fkey.length() );
    return dflt;
}

void LockProcess::cantLock( const QString &txt)
{
    msgBox( 0, QMessageBox::Critical, i18n("Will not lock the session, as unlocking would be impossible:\n") + txt );
}

#if 0 // placeholders for later
i18n("Cannot start <i>kcheckpass</i>.");
i18n("<i>kcheckpass</i> is unable to operate. Possibly it is not setuid root.");
#endif

//---------------------------------------------------------------------------
//
// Make the screen saver password protected.
//
bool LockProcess::startLock()
{
    if (loadGreetPlugin()) {
        mLocked = true;
        KDisplayManager().setLock(true);
        lockPlasma();
        KNotification::event( QLatin1String( "locked" ) );
        return true;
    }
    return false;
}

bool LockProcess::loadGreetPlugin()
{
    if (greetPlugin.library) {
        //we were locked once before, so all the plugin loading's done already
        //FIXME should I be unloading the plugin on unlock instead?
        return true;
    }
    for (QStringList::ConstIterator it = mPlugins.constBegin(); it != mPlugins.constEnd(); ++it) {
        GreeterPluginHandle plugin;
        KLibrary *lib = new KLibrary( (*it)[0] == QLatin1Char( '/' ) ? *it : QLatin1String( "kgreet_" ) + *it );
        if (lib->fileName().isEmpty()) {
            kWarning(1204) << "GreeterPlugin " << *it << " does not exist" ;
            delete lib;
            continue;
        }
        if (!lib->load()) {
            kWarning(1204) << "Cannot load GreeterPlugin " << *it << " (" << lib->fileName() << ")" ;
            delete lib;
            continue;
        }
        plugin.library = lib;
        plugin.info = (KGreeterPluginInfo *)lib->resolveSymbol( "kgreeterplugin_info" );
        if (!plugin.info ) {
            kWarning(1204) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") is no valid greet widget plugin" ;
            lib->unload();
            delete lib;
            continue;
        }
        if (plugin.info->method && !mMethod.isEmpty() && mMethod != QLatin1String(  plugin.info->method )) {
            kDebug(1204) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") serves " << plugin.info->method << ", not " << mMethod;
            lib->unload();
            delete lib;
            continue;
        }
        if (!plugin.info->init( mMethod, getConf, this )) {
            kDebug(1204) << "GreeterPlugin " << *it << " (" << lib->fileName() << ") refuses to serve " << mMethod;
            lib->unload();
            delete lib;
            continue;
        }
        kDebug(1204) << "GreeterPlugin " << *it << " (" << plugin.info->method << ", " << plugin.info->name << ") loaded";
        greetPlugin = plugin;
        return true;
    }
    cantLock( i18n("No appropriate greeter plugin configured.") );
    return false;
}

//---------------------------------------------------------------------------
//


bool LockProcess::startHack()
{
    kDebug(1204) << "Starting hack:" << mSaverExec;

    if (mSaverExec.isEmpty() || mForbidden)
    {
        hackExited();
        return false;
    }

    QHash<QChar, QString> keyMap;
    keyMap.insert(QLatin1Char( 'w' ), QString::number(winId()));
    mHackProc << KShell::splitArgs(KMacroExpander::expandMacrosShellQuote(mSaverExec, keyMap));

    mHackProc.start();
    if (mHackProc.waitForStarted())
    {
#ifdef HAVE_SETPRIORITY
        setpriority(PRIO_PROCESS, mHackProc.pid(), mPriority);
#endif
        return true;
    }

    hackExited();
    return false;
}

//---------------------------------------------------------------------------
//
void LockProcess::stopHack()
{
    if (mHackProc.state() != QProcess::NotRunning)
    {
        mHackProc.terminate();
        if (!mHackProc.waitForFinished(10000))
        {
            mHackProc.kill();
        }
    }
}

//---------------------------------------------------------------------------
//
void LockProcess::hackExited()
{
    // Hack exited while we're supposed to be saving the screen.
    // Make sure the saver window is black.
    XSetWindowBackground(QX11Info::display(), winId(), BlackPixel( QX11Info::display(), QX11Info::appScreen()));
    XClearWindow(QX11Info::display(), winId());
}

bool LockProcess::startPlasma()
{
    if (!mPlasmaEnabled) {
        return false;
    }

    if (mSetupMode) {
        mSuppressUnlock.start(mSuppressUnlockTimeout);
        XChangeActivePointerGrab(QX11Info::display(), GRABEVENTS,
                                 QCursor(Qt::ArrowCursor).handle(), CurrentTime);
    }

    kDebug() << "looking for plasma-overlay";
    if (!mPlasmaDBus) {
        //try to get it, in case it's already running somehow
        //mPlasmaDBus = new QDBusInterface(s_overlayServiceName, "/MainApplication", QString(),
        mPlasmaDBus = new org::kde::plasmaoverlay::App(s_overlayServiceName, QLatin1String( "/App" ),
                                                       QDBusConnection::sessionBus(), this);
        //FIXME this might-already-be-running stuff seems really really Wrong.
    }

    if (mPlasmaDBus->isValid()) {
        kDebug() << "weird, plasma-overlay is already running";
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "setup" ), mSetupMode);
        return true;
    }

    kDebug () << "...not found" << "starting plasma-overlay";
    delete mPlasmaDBus;
    mPlasmaDBus = 0;

    if (!mServiceWatcher) {
        mServiceWatcher = new QDBusServiceWatcher(s_overlayServiceName, QDBusConnection::sessionBus(),
                                                   QDBusServiceWatcher::WatchForOwnerChange, this);
        connect(mServiceWatcher, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
                this, SLOT(newService(QString,QString,QString)));
    }

    KProcess *plasmaProc = new KProcess;
    plasmaProc->setProgram(QLatin1String( "plasma-overlay" ));
    if (mSetupMode) {
        *plasmaProc << QLatin1String( "--setup" );
    }

    //make sure it goes away when it's done (and not before)
    connect(plasmaProc, SIGNAL(finished(int,QProcess::ExitStatus)), plasmaProc, SLOT(deleteLater()));

    plasmaProc->start();
    kDebug() << "process begun";

    //plasma gets 15 seconds to load, or we assume it failed
    QTimer::singleShot(15 * 1000, this, SLOT(checkPlasma()));
    return true;
}

void LockProcess::checkPlasma()
{
    if (!mPlasmaEnabled) {
        kDebug() << "You're Doing It Wrong!";
        return;
    }
    if (mPlasmaDBus && mPlasmaDBus->isValid()) {
        //hooray, looks like it started ok
        kDebug() << "success!";
        //...but just in case, make sure we're not waiting on it
        mSetupMode = false;
        return;
    }

    kDebug() << "ohnoes. plasma = teh fail.";
    disablePlasma();
}

bool LockProcess::isPlasmaValid()
{
    //FIXME I'm assuming that if it's valid, calls will succeed. so if that's not the case we'll
    //need to change things so that plasma's disabled properly if it fails
    //damn. isValid is not quite enough. a call may still fail, and then we need to bail.
    if (!(mPlasmaEnabled && mPlasmaDBus)) {
        return false; //no plasma, at least not yet
    }
    if (mPlasmaDBus->isValid()) {
        return true;
    }
    //oh crap, it ran away on us.
    disablePlasma();
    return false;
}

void LockProcess::disablePlasma()
{
    kDebug();
    mPlasmaEnabled = false;
    mSetupMode = false;
    mSuppressUnlock.stop(); //FIXME we might need to start the lock timer ala deactivatePlasma()
    //actually we could be lazy and just call deactivatePlasma() TODO check that this'll really work
    delete mPlasmaDBus;
    mPlasmaDBus=0;
}

void LockProcess::stopPlasma()
{
    if (mPlasmaDBus && mPlasmaDBus->isValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "quit" ));
    } else {
        kDebug() << "cannot stop plasma-overlay";
    }
}

void LockProcess::newService(QString name, QString oldOwner, QString newOwner)
{
    Q_UNUSED(name);
    Q_UNUSED(oldOwner);

    if (mPlasmaDBus) {
        if (newOwner.isEmpty()) {
            kDebug() << "plasma ran away?";
            disablePlasma();
        } else {
            kDebug() << "I'm confused!!";
        }
        return;
    }

    kDebug() << "plasma! yaay!";
    mPlasmaDBus = new org::kde::plasmaoverlay::App(s_overlayServiceName, QLatin1String( "/App" ), QDBusConnection::sessionBus(), this);

    //XXX this isn't actually used any more iirc
    connect(mPlasmaDBus, SIGNAL(hidden()), SLOT(unSuppressUnlock()));

    if (!mDialogs.isEmpty()) {
        //whoops, activation probably failed earlier
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "setActive" ), true);
    }
}

void LockProcess::deactivatePlasma()
{
    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "setActive" ), false);
    }
    if (!mLocked && mLockGrace >=0) {
        QTimer::singleShot(mLockGrace, this, SLOT(startLock())); //this is only ok because any activity will quit
    }
}

void LockProcess::lockPlasma()
{
    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "lock" ));
    }
}

void LockProcess::unSuppressUnlock()
{
    //note: suppressing unlock also now means suppressing quit-on-activity
    //maybe some var renaming is in order.
    mSuppressUnlock.stop();
}

void LockProcess::quit()
{
    mSuppressUnlock.stop();
    if (!mLocked || checkPass()) {
        quitSaver();
    }
}

void LockProcess::suspend()
{
    if( !mSuspended && mHackProc.state() == QProcess::Running )
    {
        ::kill(mHackProc.pid(), SIGSTOP);
        // We actually want to wait for the stopped hack's X commands
        // having been handled, but that would require a custom
        // protocol which would cause the hack to call XSync() and
        // freeze itself. So just go to sleep and hope that the X
        // server will have enough time ...
        usleep(100000);
        mSavedScreen = QPixmap::grabWindow( winId());
        mSnapshotTimer.setSingleShot(true);
        mSnapshotTimer.start(1000);
    }
    mSuspended = true;
}

void LockProcess::resume( bool force )
{
    if( !force && (!mDialogs.isEmpty() || !mVisibility ))
        return; // no resuming with dialog visible or when not visible
    if( mSuspended && mHackProc.state() == QProcess::Running )
    {
        QPainter p( this );
        if (!mSavedScreen.isNull())
            p.drawPixmap( 0, 0, mSavedScreen );
        else
            p.fillRect( rect(), Qt::black );
        p.end();
        QApplication::syncX();
        mSavedScreen = QPixmap();
        ::kill(mHackProc.pid(), SIGCONT);
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
    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "setActive" ), true);
    }

    PasswordDlg passDlg( this, &greetPlugin);
    const int ret = execDialog( &passDlg );

    if (isPlasmaValid()) {
        if (ret == QDialog::Rejected) {
            mSuppressUnlock.start(mSuppressUnlockTimeout);
        } else if (ret == TIMEOUT_CODE) {
            mPlasmaDBus->call(QDBus::NoBlock, QLatin1String( "setActive" ), false);
        }
    }

    XWindowAttributes rootAttr;
    XGetWindowAttributes(QX11Info::display(), QX11Info::appRootWindow(), &rootAttr);
    if(( rootAttr.your_event_mask & SubstructureNotifyMask ) == 0 )
    {
        kWarning() << "ERROR: Something removed SubstructureNotifyMask from the root window!!!" ;
        XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
            SubstructureNotifyMask | rootAttr.your_event_mask );
    }

    return ret == QDialog::Accepted;
}

bool LockProcess::checkPass(const QString &reason)
{
    if (! mLocked) {
        //we were never locked... how can we unlock?!
        //if anyone finds a use case for checking the password while unlocked, they'll have to load
        //the greetplugin n'stuff
        return false;
    }
    PasswordDlg passDlg(this, &greetPlugin, reason);
    const int ret = execDialog( &passDlg );
//    kDebug() << ret;

    //FIXME do we need to copy&paste that SubstructureNotifyMask code above?
    if (ret == QDialog::Accepted) {
        //we don't quit on a custom checkpass, but we do unlock
        //so that the user doesn't have to type their password twice
        mLocked = false;
        KDisplayManager().setLock(false);
        KNotification::event( QLatin1String( "unlocked" ) );
        //FIXME while suppressUnlock *should* always be running, if it isn't
        //(say if someone's doing things they shouldn't with dbus) then it won't get started by this
        //which means that a successful unlock will never re-lock
        //in fact, the next bit of activity would lead to the screensaver quitting.
        //possible solutions:
        //-treat this function like activity: quit if already unlocked, ensure suppress is started
        //if we're locked and the dialog's rejected
        //-return true if already unlocked, without doing anything, same as above if locked
        //-let it quit, and tell people not to do such silly things :P
        return true;
    }
    return false;
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

bool LockProcess::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Resize) {
        QWidget *w = static_cast<QWidget *>(o);
        mFrames.value(w)->resize(w->size());
    }
    return false;
}

int LockProcess::execDialog( QDialog *dlg )
{

    QFrame *winFrame = new QFrame( dlg );
    winFrame->setFrameStyle( QFrame::WinPanel | QFrame::Raised );
    winFrame->setLineWidth( 2 );
    winFrame->lower();
    mFrames.insert(dlg, winFrame);
    dlg->installEventFilter(this);

    dlg->adjustSize();

    int screen = Kephal::ScreenUtils::primaryScreenId();
    if (Kephal::ScreenUtils::numScreens() > 1) {
        screen = Kephal::ScreenUtils::screenId(QCursor::pos());
    }

    const QRect screenRect = Kephal::ScreenUtils::screenGeometry(screen);
    QRect rect = dlg->geometry();
    rect.moveCenter(screenRect.center());
    dlg->move(rect.topLeft());

    if (mDialogs.isEmpty())
    {
        if (mAutoLogoutTimerId)
            killTimer(mAutoLogoutTimerId);
        suspend();
        XChangeActivePointerGrab( QX11Info::display(), GRABEVENTS,
                QCursor(Qt::ArrowCursor).handle(), CurrentTime);
    }
    mDialogs.prepend( dlg );
    fakeFocusIn( dlg->winId());
    const int rt = dlg->exec();
    const int pos = mDialogs.indexOf( dlg );
    if (pos != -1)
        mDialogs.remove( pos );
    if( mDialogs.isEmpty() ) {
        resume( false );
        if (mAutoLogoutTimerId)
            mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000);
    }
    updateFocus();

    dlg->removeEventFilter(this);
    mFrames.remove(dlg);

    return rt;
}

void LockProcess::updateFocus()
{
    if (mDialogs.isEmpty()) {
        if (mForeignInputWindows.isEmpty()) {
            XChangeActivePointerGrab( QX11Info::display(), GRABEVENTS,
                    QCursor(Qt::BlankCursor).handle(), CurrentTime);
        } else {
            fakeFocusIn(mForeignInputWindows.first());
        }
    } else {
        fakeFocusIn(mDialogs.first()->winId());
    }
}

//---------------------------------------------------------------------------
//
// X11 Event.
//
bool LockProcess::x11Event(XEvent *event)
{
    if (mEventRecursed)
        return false;

    bool ret = false;
    switch (event->type)
    {
        case ButtonPress:
            if (!mDialogs.isEmpty() && event->xbutton.window == event->xbutton.root) {
                //kDebug() << "close" << mDialogs.first()->effectiveWinId();
                KDialog *dlg = qobject_cast<KDialog*>(mDialogs.first());
                if (dlg) {
                    //kDebug() << "casting success";
                    dlg->reject();
                }
                break;
            }
        case KeyPress:
        case MotionNotify:
            if (mBusy || !mDialogs.isEmpty()) {
                //kDebug() << "busy";
                //FIXME shouldn't we be resetting some timers?
                break;
            }
            mBusy = true;
            //something happened. do we quit, ask for a password or forward it to plasma?
            //if we're supposed to be forwarding, we check that there's actually a plasma window up
            //so that the user isn't trapped if plasma crashes or is slow to load.
            //however, if plasma started in setup mode, we don't want to let anything happen until
            //it has a chance to load.
            //note: mSetupMode should end when we either get a winid or hit the checkPlasma timeout
            if (mSuppressUnlock.isActive() && (mSetupMode || !mForeignInputWindows.isEmpty())) {
                mSuppressUnlock.start(); //help, help, I'm being suppressed!
                if (mAutoLogoutTimerId) {
                    killTimer(mAutoLogoutTimerId);
                    mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000);
                }
            } else if (!mLocked) {
                quitSaver();
                mBusy = false;
                return true; //it's better not to forward any input while quitting, right?
            } else {
                if (event->type == KeyPress) {
                    // Bounce the keypress to the dialog
                    QByteArray chars;
                    chars.resize(513);
                    KeySym keysym;
                    XLookupString(&event->xkey, chars.data(), chars.size(), &keysym, 0);
                    switch (keysym) {
                    // These would cause immediate failure
                    case XK_Escape:
                    case XK_Return:
                    case XK_KP_Enter:
                    // These just make no sense
                    case XK_Tab:
                    case XK_space:
                        break;
                    default:
                        mEventQueue.enqueue(*event);
                    }
                }
                if (checkPass()) {
                    quitSaver();
                    mBusy = false;
                    return true; //it's better not to forward any input while quitting, right?
                }
            }
            mBusy = false;
            ret = true;
            break;

        case VisibilityNotify:
            if( event->xvisibility.window == winId())
            {  // mVisibility == false means the screensaver is not visible at all
               // e.g. when switched to text console
               // ...or when plasma's over it non-compositely?
               // hey, this gives me free "suspend saver when plasma obscures it"
                mVisibility = !(event->xvisibility.state == VisibilityFullyObscured);
                if (!mVisibility) {
                    mSuspendTimer.start(2000);
                    kDebug(1204) << "fully obscured";
                } else {
                    kDebug(1204) << "not fully obscured";
                    mSuspendTimer.stop();
                    resume( false );
                }
                if (mForeignWindows.isEmpty() && event->xvisibility.state != VisibilityUnobscured) {
                    kDebug(1204) << "no plasma; saver obscured";
                    stayOnTop();
                }
            } else if (!mForeignWindows.isEmpty() && event->xvisibility.window == mForeignWindows.last() &&
                    event->xvisibility.state != VisibilityUnobscured) {
                //FIXME now that we have several plasma winids this doesn't feel valid
                //but I don't know what to do about it!
                kDebug(1204) << "plasma obscured!";
                stayOnTop();
            }
            break;

        case ConfigureNotify: // from SubstructureNotifyMask on the root window
            if(event->xconfigure.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xconfigure.window );
                if( index >= 0 ) {
                    int index2 = event->xconfigure.above ? findWindowInfo( event->xconfigure.above ) : 0;
                    if( index2 < 0 )
                        kDebug(1204) << "Unknown above for ConfigureNotify";
                    else { // move just above the other window
                        if( index2 < index )
                            ++index2;
                        windowInfo.move( index, index2 );
                    }
                } else
                    kDebug(1204) << "Unknown toplevel for ConfigureNotify";
                //kDebug() << "ConfigureNotify:";
                //the stacking order changed, so let's change the stacking order again to what we want
                stayOnTop();
            }
            break;
        case MapNotify: // from SubstructureNotifyMask on the root window
            if( event->xmap.event == QX11Info::appRootWindow()) {
                kDebug(1204) << "MapNotify:" << event->xmap.window;
                if (!mDialogs.isEmpty() && mDialogs.first()->winId() == event->xmap.window)
                    mVisibleDialogs.append(event->xmap.window);
                int index = findWindowInfo( event->xmap.window );
                if( index >= 0 )
                    windowInfo[ index ].viewable = true;
                else
                    kDebug(1204) << "Unknown toplevel for MapNotify";
                KXErrorHandler err; // ignore X errors here
                WindowType type = windowType(event->xmap.window);
                if (type != IgnoreWindow) {
                    if (mForeignWindows.contains(event->xmap.window)) {
                        kDebug(1204) << "uhoh! duplicate!";
                    } else {
                        //ordered youngest-on-top
                        mForeignWindows.prepend(event->xmap.window);
                    }
                    if (type & InputWindow) {
                        kDebug(1204) << "input window";
                        if (mForeignInputWindows.contains(event->xmap.window)) {
                            kDebug(1204) << "uhoh! duplicate again"; //never happens
                        } else {
                            //ordered youngest-on-top
                            mForeignInputWindows.prepend(event->xmap.window);
                            fakeFocusIn(event->xmap.window);
                        }
                        mSetupMode = false; //no more waiting for plasma
                    }
                }
                stayOnTop();
            }
            break;
        case UnmapNotify:
            if (event->xunmap.event == QX11Info::appRootWindow()) {
                kDebug(1204) << "UnmapNotify:" << event->xunmap.window;
                int index = findWindowInfo( event->xunmap.window );
                if( index >= 0 )
                    windowInfo[ index ].viewable = false;
                else
                    kDebug(1204) << "Unknown toplevel for MapNotify";
                mVisibleDialogs.removeAll(event->xunmap.window);
                mForeignWindows.removeAll(event->xunmap.window);
                if (mForeignInputWindows.removeAll(event->xunmap.window)) {
                    updateFocus();
                }
            }
            break;
        case CreateNotify:
            if (event->xcreatewindow.parent == QX11Info::appRootWindow()) {
                kDebug(1204) << "CreateNotify:" << event->xcreatewindow.window;
                int index = findWindowInfo( event->xcreatewindow.window );
                if( index >= 0 )
                    kDebug(1204) << "Already existing toplevel for CreateNotify";
                else {
                    WindowInfo info;
                    info.window = event->xcreatewindow.window;
                    info.viewable = false;
                    windowInfo.append( info );
                }
            }
            break;
        case DestroyNotify:
            if (event->xdestroywindow.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xdestroywindow.window );
                if( index >= 0 )
                    windowInfo.removeAt( index );
                else
                    kDebug(1204) << "Unknown toplevel for DestroyNotify";
            }
            break;
        case ReparentNotify:
            if (event->xreparent.event == QX11Info::appRootWindow() && event->xreparent.parent != QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xreparent.window );
                if( index >= 0 )
                    windowInfo.removeAt( index );
                else
                    kDebug(1204) << "Unknown toplevel for ReparentNotify away";
            } else if (event->xreparent.parent == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xreparent.window );
                if( index >= 0 )
                    kDebug(1204) << "Already existing toplevel for ReparentNotify";
                else {
                    WindowInfo info;
                    info.window = event->xreparent.window;
                    info.viewable = false;
                    windowInfo.append( info );
                }
            }
            break;
        case CirculateNotify:
            if (event->xcirculate.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xcirculate.window );
                if( index >= 0 ) {
                    windowInfo.move( index, event->xcirculate.place == PlaceOnTop ? windowInfo.size() - 1 : 0 );
                } else
                    kDebug(1204) << "Unknown toplevel for CirculateNotify";
            }
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
    if (!mDialogs.isEmpty()) {
        if (event->type == KeyPress || event->type == KeyRelease) {
            mEventQueue.enqueue(*event);
            ret = true;
        }
        if (!mVisibleDialogs.isEmpty())
            while (!mEventQueue.isEmpty()) {
                //kDebug() << "forward to dialog";
                XEvent ev2 = mEventQueue.dequeue();
                ev2.xkey.window = ev2.xkey.subwindow = mVisibleDialogs.last();
                mEventRecursed = true;
                qApp->x11ProcessEvent( &ev2 );
                mEventRecursed = false;
            }
    } else {
        mEventQueue.clear();
        if (!mForeignInputWindows.isEmpty()) {
            //when there are no dialogs, forward some events to plasma
            switch (event->type) {
            case KeyPress:
            case KeyRelease:
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify: {
                //kDebug() << "forward to plasma";
                XEvent ev2 = *event;
                Window root_return;
                int x_return, y_return;
                unsigned int width_return, height_return, border_width_return, depth_return;
                WId targetWindow = 0;
                //kDebug() << "root is" << winId();
                //kDebug() << "search window under pointer with" << mForeignInputWindows.size() << "windows";
                KXErrorHandler err; // ignore X errors
                foreach(WId window, mForeignInputWindows)
                {
                    if( XGetGeometry(QX11Info::display(), window, &root_return,
                                &x_return, &y_return,
                                &width_return, &height_return,
                                &border_width_return, &depth_return)
                        &&
                        (event->xkey.x>=x_return && event->xkey.x<=x_return+(int)width_return)
                        &&
                        (event->xkey.y>=y_return && event->xkey.y<=y_return+(int)height_return) )
                    {
                        //kDebug() << "found window" << window;
                        targetWindow = window;
                        ev2.xkey.window = ev2.xkey.subwindow = targetWindow;
                        ev2.xkey.x = event->xkey.x - x_return;
                        ev2.xkey.y = event->xkey.y - y_return;
                        break;
                    }
                }
                XSendEvent(QX11Info::display(), targetWindow, False, NoEventMask, &ev2);
                ret = true;
                break; }
            default:
                break;
            }
        }
    }

    return ret;
}

LockProcess::WindowType LockProcess::windowType(WId id)
{
    Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREENSAVER_OVERRIDE", False);
    Atom actualType;
    int actualFormat;
    unsigned long nitems, remaining;
    unsigned char *data = 0;
    Display *display = QX11Info::display();

    int result = XGetWindowProperty(display, id, tag, 0, 1, False, tag, &actualType,
            &actualFormat, &nitems, &remaining, &data);

    //kDebug() << (result == Success) << (actualType == tag);
    WindowType type = IgnoreWindow;
    if (result == Success && actualType == tag) {
        if (nitems != 1 || actualFormat != 8) {
            kDebug(1204) << "malformed property";
        } else {
            kDebug(1204) << "i can haz plasma window?" << data[0];
            switch (data[0]) {
                case 0: //FIXME magic numbers
                    type = SimpleWindow;
                    break;
                case 1:
                    type = InputWindow;
                    break;
                case 2:
                    type = DefaultWindow;
                    break;
            }
        }
    }
    if (data) {
        XFree(data);
    }
    return type;
}

void LockProcess::stayOnTop()
{
    // this restacking is written in a way so that
    // if the stacking positions actually don't change,
    // all restacking operations will be no-op,
    // and no ConfigureNotify will be generated,
    // thus avoiding possible infinite loops
    QVector< Window > stack( mDialogs.count() + mForeignWindows.count() + 1 );
    int count = 0;
    // dialogs first
    foreach( QWidget* w, mDialogs )
        stack[ count++ ] = w->winId();
    // now the plasma stuff below the dialogs
    foreach( WId w, mForeignWindows )
        stack[ count++ ] = w;
    // finally, the saver window
    stack[ count++ ] = winId();
    // We actually have to check the current stacking order. When an override-redirect
    // window is shown or raised, it can get above the screensaver window and there's not
    // much to do to prevent it (only the compositing manager can prevent that). This
    // is detected by the screenlocker and handled here, but the contents of the window
    // may remain visible, since some screensavers don't react to Expose events and
    // don't repaint as necessary. Therefore, if a window is detected above any of the windows
    // related to screenlocking, I don't see any better possibility than to completely
    // erase the screenlocker window.
    // It is important to first detect, then restack and then erase.
    // Another catch here is that only viewable windows matter, but checking here whether
    // a window is viewable is a race condition, since a window may map, paint and unmap
    // before we reach this point, thus making this code fail to detect the need to do
    // a repaint. Therefore we track all relevant X events about mapping state of toplevel
    // windows (which ensures proper ordering) and here just consult the information.
    bool needs_erase = false;
    bool found_ours = false;
    foreach( const WindowInfo& info, windowInfo ) {
        if( stack.contains( info.window )) {
            found_ours = true;
        } else if( found_ours && info.viewable ) {
            kDebug(1204) << "found foreign window above screensaver";
            needs_erase = true;
            break;
        }
    }
    // do the actual restacking if needed
    XRaiseWindow( x11Info().display(), stack[ 0 ] );
    if( count > 1 )
        XRestackWindows( x11Info().display(), stack.data(), count );
    if( needs_erase ) {
        // if the snapshot was taken recently it is possible that the rogue
        // window was snapshotted at well.
        if (mSnapshotTimer.isActive())
            mSavedScreen = QPixmap();
        QPainter p( this );
        if (!mSavedScreen.isNull())
            p.drawPixmap( 0, 0, mSavedScreen );
        else
            p.fillRect( rect(), Qt::black );
        p.end();
        QApplication::syncX();
    }
}

void LockProcess::checkDPMSActive()
{
#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo(QX11Info::display(), &state, &on);
    //kDebug() << "checkDPMSActive " << on << " " << state;
    if (state == DPMSModeStandby || state == DPMSModeSuspend || state == DPMSModeOff)
        suspend();
    else
        resume( false );
#endif
}

#if defined(HAVE_XF86MISC) && defined(HAVE_XF86MISCSETGRABKEYSSTATE)
// see http://cvsweb.xfree86.org/cvsweb/xc/programs/Xserver/hw/xfree86/common/xf86Events.c#rev3.113
// This allows enabling the "Allow{Deactivate/Closedown}Grabs" options in XF86Config,
// and kscreenlocker will still lock the session.
static enum { Unknown, Yes, No } can_do_xf86_lock = Unknown;
void LockProcess::lockXF86()
{
    if( can_do_xf86_lock == Unknown )
    {
        int major, minor, dummy;
        if( XF86MiscQueryExtension( QX11Info::display(), &dummy, &dummy )
            && XF86MiscQueryVersion( QX11Info::display(), &major, &minor )
            && (major > 0 || minor >= 5) )
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

void LockProcess::msgBox( QWidget *parent, QMessageBox::Icon type, const QString &txt )
{
    QDialog box( parent, Qt::X11BypassWindowManagerHint );

    QLabel *label1 = new QLabel( &box );
    label1->setPixmap( QMessageBox::standardIcon( type ) );
    QLabel *label2 = new QLabel( txt, &box );
    KPushButton *button = new KPushButton( KStandardGuiItem::ok(), &box );
    button->setDefault( true );
    button->setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred ) );
    connect( button, SIGNAL( clicked() ), &box, SLOT( accept() ) );

    QGridLayout *grid = new QGridLayout( &box );
    grid->setSpacing( 10 );
    grid->addWidget( label1, 0, 0, Qt::AlignCenter );
    grid->addWidget( label2, 0, 1, Qt::AlignCenter );
    grid->addWidget( button, 1, 0, 1, 2, Qt::AlignCenter );

    execDialog( &box );
}

int LockProcess::findWindowInfo( Window w )
{
    for( int i = 0;
         i < windowInfo.size();
         ++i )
        if( windowInfo[ i ].window == w )
            return i;
    return -1;
}

#include "lockprocess.moc"
