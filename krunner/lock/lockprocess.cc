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
//it starts krunner_lock, who does all the locking and who
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

#include <kdisplaymanager.h>

#include <KStandardDirs>
#include <KApplication>
#include <KServiceGroup>
#include <KDebug>
#include <KMessageBox>
#include <KGlobalSettings>
#include <KLocale>
#include <KLibrary>
#include <KPushButton>
#include <KStandardGuiItem>
#include <KAuthorized>
#include <KDesktopFile>
#include <kservicetypetrader.h>
#include <kmacroexpander.h>
#include <kshell.h>

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

const int TIMEOUT_CODE = 2; //from PasswordDlg

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.f
//
LockProcess::LockProcess(bool child, bool useBlankOnly)
    : QWidget(0L, Qt::X11BypassWindowManagerHint),
      mLocked(false),
      mBusy(false),
      mPlasmaDBus(0),
      mSetupMode(false),
      mOpenGLVisual(false),
      child_saver(child),
      mParent(0),
      mUseBlankOnly(useBlankOnly),
      mSuspended(false),
      mVisibility(false),
      mRestoreXF86Lock(false),
      mForbidden(false),
      mAutoLogoutTimerId(0)
{
    setObjectName("save window");
    setupSignals();

    new LockProcessAdaptor(this);
    QDBusConnection::sessionBus().registerService("org.kde.krunner_lock");
    QDBusConnection::sessionBus().registerObject("/LockProcess", this);

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
            // we can save CPU if we stop it as quickly as possible
            // but we waste CPU if we check too often -> so take 10s
            mCheckDPMS.start(10000);
        }
    }
#endif

    greetPlugin.library = 0;

    mSuppressUnlock.setSingleShot(true);
    connect(&mSuppressUnlock, SIGNAL(timeout()), SLOT(deactivatePlasma()));
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
    struct sigaction act;
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
    // SIGHUP forces lock
    act.sa_handler= sighup_handler;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGHUP);
    act.sa_flags = 0;
    sigaction(SIGHUP, &act, 0L);

    pipe(signal_pipe);
    QSocketNotifier* notif = new QSocketNotifier(signal_pipe[0], QSocketNotifier::Read, this);
    connect( notif, SIGNAL(activated(int)), SLOT(signalPipeSignal()));
}


void LockProcess::signalPipeSignal()
{
    char tmp;
    ::read( signal_pipe[0], &tmp, 1);
    if( tmp == 'T' )
        quitSaver();
    else if( tmp == 'H' ) {
        if( !mLocked )
            startLock();
    }
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

bool LockProcess::startSetup()
{
    if (!mPlasmaEnabled) {
        return defaultSave();
    }

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

    if ( KScreenSaverSettings::autoLogout() ) {
        mAutoLogoutTimeout = KScreenSaverSettings::autoLogoutTimeout();
        mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000); // in milliseconds
    }

#ifdef HAVE_DPMS
    mDPMSDepend = KScreenSaverSettings::suspendWhenInvisible();
#endif

    mPriority = KScreenSaverSettings::priority();
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;

    mSaver = KScreenSaverSettings::saver();
    if (mSaver.isEmpty() || mUseBlankOnly) {
        mSaver = "kblank.desktop";
    }

    readSaver();

    mPlasmaEnabled = KScreenSaverSettings::plasmaEnabled();

    mSuppressUnlockTimeout = qMax(0, KScreenSaverSettings::timeout() * 1000);
    mSuppressUnlockTimeout = qMax(mSuppressUnlockTimeout, 30 * 1000); //min. 30 secs FIXME is this a good idea?

    mPlugins = KScreenSaverSettings::pluginsUnlock();
    if (mPlugins.isEmpty()) {
        mPlugins << "classic" << "generic";
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
        if( entryName.endsWith( ".desktop" ))
            entryName = entryName.left( entryName.length() - 8 ); // strip it
        KService::List offers = KServiceTypeTrader::self()->query( "ScreenSaver",
            "DesktopEntryName == '" + entryName.toLower() + '\'' );
        if( offers.count() == 0 )
        {
            kDebug(1204) << "Cannot find screesaver: " << mSaver;
            return;
        }
        QString file = KStandardDirs::locate("services", offers.first()->entryPath());

        bool opengl = KAuthorized::authorizeKAction("opengl_screensavers");
        bool manipulatescreen = KAuthorized::authorizeKAction("manipulatescreen_screensavers");
        KDesktopFile config( file );
        KConfigGroup desktopGroup = config.desktopGroup();
        if (!desktopGroup.readEntry("X-KDE-Type").toUtf8().isEmpty())
        {
            QString saverType = desktopGroup.readEntry("X-KDE-Type").toUtf8();
            QStringList saverTypes = saverType.split( ";");
            for (int i = 0; i < saverTypes.count(); i++)
            {
                if ((saverTypes[i] == "ManipulateScreen") && !manipulatescreen)
                {
                    kDebug(1204) << "Screensaver is type ManipulateScreen and ManipulateScreen is forbidden";
                    mForbidden = true;
                }
                if ((saverTypes[i] == "OpenGL") && !opengl)
                {
                    kDebug(1204) << "Screensaver is type OpenGL and OpenGL is forbidden";
                    mForbidden = true;
                }
                if (saverTypes[i] == "OpenGL")
                {
                    mOpenGLVisual = true;
                }
            }
        }

        kDebug(1204) << "mForbidden: " << (mForbidden ? "true" : "false");

        if (config.hasActionGroup("Root"))
        {
            mSaverExec = config.actionGroup("Root").readPathEntry("Exec", QString());
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
        static int attribs[][ 15 ] =
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
            if( XVisualInfo* info = glXChooseVisual( x11Info().display(), x11Info().screen(), attribs[ i ] ))
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
        kWarning(1204) << "LockProcess::startSaver() grabInput() failed!!!!" ;
        return false;
    }
    mBusy = false;

    saveVRoot();

    if (mParent) {
        QSocketNotifier *notifier = new QSocketNotifier(mParent, QSocketNotifier::Read, this);
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
    startPlasma();
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
        if (mLocked)
            KDisplayManager().setLock( false );
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
    for (QStringList::ConstIterator it = mPlugins.begin(); it != mPlugins.end(); ++it) {
        GreeterPluginHandle plugin;
        KLibrary *lib = new KLibrary( (*it)[0] == '/' ? *it : "kgreet_" + *it );
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
        if (plugin.info->method && !mMethod.isEmpty() && mMethod != plugin.info->method) {
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
    keyMap.insert('w', QString::number(winId()));
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
    XSetWindowBackground(QX11Info::display(), winId(), 0);
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
        //FIXME I don't like hardcoded strings
        //mPlasmaDBus = new QDBusInterface("org.kde.plasma-overlay", "/MainApplication", QString(),
        mPlasmaDBus = new org::kde::plasmaoverlay::App("org.kde.plasma-overlay", "/App",
                                                       QDBusConnection::sessionBus(), this);
        //FIXME this might-already-be-running stuff seems really really Wrong.
    }

    if (mPlasmaDBus->isValid()) {
        kDebug() << "weird, plasma-overlay is already running";
        mPlasmaDBus->call(QDBus::NoBlock, "setActive", mSetupMode);
        // aseigo: put a dbus call here to the "setup" mode method
        //         that method should also take care of initial visibility
        return true;
    }

    kDebug () << "...not found" << "starting plasma-overlay";
    delete mPlasmaDBus;
    mPlasmaDBus = 0;

    connect(QDBusConnection::sessionBus().interface(), SIGNAL(serviceOwnerChanged(QString, QString, QString)),
            this, SLOT(newService(QString, QString, QString)));

    //KProcess plasmaProc;
    mPlasmaProc.setProgram("plasma-overlay");
    if (mSetupMode) {
        mPlasmaProc << "--setup";
    }
    mPlasmaProc.start();
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
        mPlasmaDBus->call(QDBus::NoBlock, "quit");
    } else {
        kDebug() << "cannot stop plasma-overlay";
    }
}

void LockProcess::newService(QString name, QString oldOwner, QString newOwner)
{
    Q_UNUSED(oldOwner);
    if (name != "org.kde.plasma-overlay") {
        return;
    }

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
    mPlasmaDBus = new org::kde::plasmaoverlay::App(name, "/App",
            QDBusConnection::sessionBus(), this);

    //XXX this isn't actually used any more iirc
    connect(mPlasmaDBus, SIGNAL(hidden()), SLOT(unSuppressUnlock()));

    if (!mDialogs.isEmpty()) {
        //whoops, activation probably failed earlier
        mPlasmaDBus->call(QDBus::NoBlock, "activate");
    }
}

void LockProcess::deactivatePlasma()
{
    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, "deactivate");
    }
    if (!mLocked && mLockGrace >=0) {
        QTimer::singleShot(mLockGrace, this, SLOT(startLock())); //this is only ok because any activity will quit
    }
}

void LockProcess::lockPlasma()
{
    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, "lock");
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
        QApplication::syncX();
        mSavedScreen = QPixmap::grabWindow( winId());
    }
    mSuspended = true;
}

void LockProcess::resume( bool force )
{
    if( !force && (!mDialogs.isEmpty() || !mVisibility ))
        return; // no resuming with dialog visible or when not visible
    if( mSuspended && mHackProc.state() == QProcess::Running )
    {
        XForceScreenSaver(QX11Info::display(), ScreenSaverReset );
        QPainter p( this );
        p.drawPixmap( 0, 0, mSavedScreen );
        p.end();
        mSavedScreen = QPixmap();
        QApplication::syncX();
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
    killTimer(mAutoLogoutTimerId);

    if (isPlasmaValid()) {
        mPlasmaDBus->call(QDBus::NoBlock, "activate");
    }

    PasswordDlg passDlg( this, &greetPlugin);
    int ret = execDialog( &passDlg );

    if (isPlasmaValid()) {
        if (ret == QDialog::Rejected) {
            mSuppressUnlock.start(mSuppressUnlockTimeout);
        } else if (ret == TIMEOUT_CODE) {
            mPlasmaDBus->call(QDBus::NoBlock, "deactivate");
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
    PasswordDlg passDlg(this, &greetPlugin, reason);
    int ret = execDialog( &passDlg );
    kDebug() << ret;

    //FIXME do we need to copy&paste that SubstructureNotifyMask code above?
    if (ret == QDialog::Accepted) {
        //we don't quit on a custom checkpass, but we do unlock
        //so that the user doesn't have to type their password twice
        mLocked = false;
        KDisplayManager().setLock(false);
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
        resume( false );
    }
    updateFocus();

    dlg->removeEventFilter(this);
    mFrames.remove(dlg);

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
    updateFocus();
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
            } else if (!mLocked || checkPass()) {
                quitSaver();
                mBusy = false;
                return true; //it's better not to forward any input while quitting, right?
            }
            if (mAutoLogoutTimerId) // we need to restart the auto logout countdown
            {
                killTimer(mAutoLogoutTimerId);
                mAutoLogoutTimerId = startTimer(mAutoLogoutTimeout * 1000);
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
                    kDebug() << "fully obscured";
                } else {
                    kDebug() << "not fully obscured";
                    mSuspendTimer.stop();
                    resume( false );
                }
                if (mForeignWindows.isEmpty() && event->xvisibility.state != VisibilityUnobscured) {
                    kDebug() << "no plasma; saver obscured";
                    stayOnTop();
                }
            } else if (!mForeignWindows.isEmpty() && event->xvisibility.window == mForeignWindows.last() &&
                    event->xvisibility.state != VisibilityUnobscured) {
                //FIXME now that we have several plasma winids this doesn't feel valid
                //but I don't know what to do about it!
                kDebug() << "plasma obscured!";
                stayOnTop();
            }
            break;

        case ConfigureNotify: // from SubstructureNotifyMask on the root window
            if(event->xconfigure.event == QX11Info::appRootWindow()) {
                //kDebug() << "ConfigureNotify:"; 
                //the stacking order changed, so let's change the stacking order!
                stayOnTop();
            }
            break;
        case MapNotify: // from SubstructureNotifyMask on the root window
            if( event->xmap.event == QX11Info::appRootWindow()) {
                kDebug() << "MapNotify:" << event->xmap.window;
                WindowType type = windowType(event->xmap.window);
                if (type != IgnoreWindow) {
                    if (mForeignWindows.contains(event->xmap.window)) {
                        kDebug() << "uhoh! duplicate!";
                    } else {
                        //ordered youngest-on-top
                        mForeignWindows.prepend(event->xmap.window);
                    }
                    if (type & InputWindow) {
                        kDebug() << "input window";
                        if (mForeignInputWindows.contains(event->xmap.window)) {
                            kDebug() << "uhoh! duplicate again"; //never happens
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
            if (event->xmap.event == QX11Info::appRootWindow()) {
                kDebug() << "UnmapNotify:" << event->xunmap.window;
                mForeignWindows.removeAll(event->xunmap.window);
                if (mForeignInputWindows.removeAll(event->xunmap.window)) {
                    updateFocus();
                }
            }
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
        if ((event->type == KeyPress || event->type == KeyRelease) &&
                event->xkey.window != mDialogs.first()->winId()) {
            //kDebug() << "forward to dialog";
            XEvent ev2 = *event;
            ev2.xkey.window = ev2.xkey.subwindow = mDialogs.first()->winId();
            qApp->x11ProcessEvent( &ev2 );
            ret = true;
        }
    } else if (!mForeignInputWindows.isEmpty()) {
        //when there are no dialogs, forward some events to plasma
        switch (event->type) {
        case KeyPress:
        case KeyRelease:
        case ButtonPress:
        case ButtonRelease:
        case MotionNotify:
            {
                //kDebug() << "forward to plasma";
                XEvent ev2 = *event;
                ev2.xkey.window = ev2.xkey.subwindow = mForeignInputWindows.first();
                XSendEvent(QX11Info::display(), ev2.xkey.window, False, NoEventMask, &ev2);
                ret = true;
            }
        default:
            break;
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
            kDebug() << "malformed property";
        } else {
            kDebug() << "i can haz plasma window?" << data[0];
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
/*    if (result != Success) {
        return false;
    }
    if (actualType == tag) {
        return true;
    }
    //managed windows will have a pesky frame we have to bypass
    XWindowAttributes attr;
    XGetWindowAttributes(display, id, &attr);
    if (!attr.override_redirect) {
        //check the real client window
        if (Window client = XmuClientWindow(display, id)) {
            result = XGetWindowProperty(display, client, tag, 0, 0, False, tag, &actualType,
                    &actualFormat, &nitems, &remaining, &data);
            kDebug() << (result == Success) << (actualType == tag);
            if (data) {
                XFree(data);
            }
            return (result == Success) && (actualType == tag);
*        }
    }
    return false;*/
}

void LockProcess::stayOnTop()
{
    if(!(mDialogs.isEmpty() && mForeignWindows.isEmpty()))
    {
        // this restacking is written in a way so that
        // if the stacking positions actually don't change,
        // all restacking operations will be no-op,
        // and no ConfigureNotify will be generated,
        // thus avoiding possible infinite loops
        Window* stack = new Window[ mDialogs.count() + mForeignWindows.count() + 1 ];
        int count = 0;
        if (!mDialogs.isEmpty()) {
            XRaiseWindow( QX11Info::display(), mDialogs.first()->winId()); // raise topmost
            // and stack others below it
            for( QVector< QWidget* >::ConstIterator it = mDialogs.begin();
                    it != mDialogs.end();
                    ++it )
                stack[ count++ ] = (*it)->winId();
        } else {
            XRaiseWindow( QX11Info::display(), mForeignWindows.first()); // raise topmost
        }
        //now the plasma stuff below the dialogs
        foreach (const WId w, mForeignWindows) {
            stack[count++] = w;
        }
        //finally, the saver window
        stack[ count++ ] = winId();
        XRestackWindows( x11Info().display(), stack, count );
        //kDebug() << "restacked" << count;
        delete[] stack;
    } else {
        XRaiseWindow(QX11Info::display(), winId());
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
    {
       suspend();
    } else if ( mSuspended )
    {
        resume( true );
    }
#endif
}

#if defined(HAVE_XF86MISC) && defined(HAVE_XF86MISCSETGRABKEYSSTATE)
// see http://cvsweb.xfree86.org/cvsweb/xc/programs/Xserver/hw/xfree86/common/xf86Events.c#rev3.113
// This allows enabling the "Allow{Deactivate/Closedown}Grabs" options in XF86Config,
// and krunner_lock will still lock the session.
static enum { Unknown, Yes, No } can_do_xf86_lock = Unknown;
void LockProcess::lockXF86()
{
    if( can_do_xf86_lock == Unknown )
    {
        int major, minor;
        if( XF86MiscQueryVersion( QX11Info::display(), &major, &minor )
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
    box.setModal( true );

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

#include "lockprocess.moc"
