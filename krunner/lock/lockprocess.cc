//===========================================================================
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
//

//kdesktop keeps running and checks user inactivity
//when it should show screensaver (and maybe lock the screen),
//it starts kdesktop_lock, who does all the locking and who
//actually starts the screensaver

//It's done this way to prevent screen unlocking when kdesktop
//crashes (e.g. because it's set to multiple wallpapers and
//some image will be corrupted).


// This stuff should be probably rewritten to use override_redirect
// (WX11BypassWM) windows. This process grabs the mouse and the keyboard,
// and basically pretends there's no WM anyway. If there will be some
// more problems with focus, window showing or whatever related,
// tell KWin developers.
// See also PasswordDlg::show().

#include <config.h>

#include <stdlib.h>
#include <qcursor.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kwin.h>
#include <qtimer.h>
#include <qfile.h>
#include <assert.h>
#include <signal.h>
#include <qsocketnotifier.h>

#include "lockprocess.h"
#include "lockprocess.moc"
#include "lockdlg.h"

#ifdef HAVE_SETPRIORITY
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#define LOCK_GRACE_DEFAULT          5000

static Window gVRoot = 0;
static Window gVRootData = 0;
static Atom   gXA_VROOT;
static Atom   gXA_SCREENSAVER_VERSION;

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.
//
LockProcess::LockProcess(bool child, bool useBlankOnly)
    : QWidget(0L, "saver window", WStyle_Customize | WStyle_NoBorder), child_saver(child), parent(0), mUseBlankOnly(useBlankOnly)
{
    KWin::setState( winId(), NET::StaysOnTop );
    KWin::setOnAllDesktops( winId(), true );

    setupSignals();

    kapp->installX11EventFilter(this);

    // Get root window size
    XWindowAttributes rootAttr;
    XGetWindowAttributes(qt_xdisplay(), RootWindow(qt_xdisplay(),
                        qt_xscreen()), &rootAttr);
    mRootWidth = rootAttr.width;
    mRootHeight = rootAttr.height;

    // Add non-KDE path
    KGlobal::dirs()->addResourceType("scrsav",
                                    KGlobal::dirs()->kde_default("apps") +
                                    "System/ScreenSavers/");

    // Add KDE specific screensaver path
    QString relPath="System/ScreenSavers/";
    KServiceGroup::Ptr servGroup = KServiceGroup::baseGroup( "screensavers" );
    if (servGroup)
    {
      relPath=servGroup->relPath();
      kdDebug(1204) << "relPath=" << relPath << endl;
    }
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     relPath);

    mLockOnce = false;

    // virtual root property
    gXA_VROOT = XInternAtom (qt_xdisplay(), "__SWM_VROOT", False);
    gXA_SCREENSAVER_VERSION = XInternAtom (qt_xdisplay(), "_SCREENSAVER_VERSION", False);

    XWindowAttributes attrs;
    XGetWindowAttributes(qt_xdisplay(), winId(), &attrs);
    mColorMap = attrs.colormap;

    connect(&mHackProc, SIGNAL(processExited(KProcess *)),
                        SLOT(hackExited(KProcess *)));

    QStringList dmopt =
        QStringList::split( QChar( ',' ),
                            QString::fromLatin1( ::getenv( "XDM_MANAGED" ) ) );
    if (dmopt.findIndex( "rsvd" ) < 0)
        mXdmFifoName = QString::null;
    else
        mXdmFifoName = dmopt.first();

    configure();
}

//---------------------------------------------------------------------------
//
// Destructor - usual cleanups.
//
LockProcess::~LockProcess()
{
}

static int sigterm_pipe[ 2 ];

static void sigterm_handler( int )
{
    char tmp;
    ::write( sigterm_pipe[ 1 ], &tmp, 1 );
}

void LockProcess::setupSignals()
{
    // ignore SIGHUP
    struct sigaction act;
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGHUP);
    act.sa_flags = 0;
    sigaction( SIGHUP, &act, 0L);
    // ignore SIGINT
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGINT);
    act.sa_flags = 0;
    sigaction( SIGINT, &act, 0L);
    // ignore SIGQUIT
    act.sa_handler=SIG_IGN;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGQUIT);
    act.sa_flags = 0;
    sigaction( SIGQUIT, &act, 0L);
    // exit cleanly on SIGTERM
    act.sa_handler= sigterm_handler;
    sigemptyset(&(act.sa_mask));
    sigaddset(&(act.sa_mask), SIGTERM);
    act.sa_flags = 0;
    sigaction( SIGTERM, &act, 0L);

    pipe( sigterm_pipe );
    QSocketNotifier* notif = new QSocketNotifier( sigterm_pipe[ 0 ],
	QSocketNotifier::Read, this );
    connect( notif, SIGNAL( activated( int )), SLOT( sigtermPipeSignal()));
}


void LockProcess::sigtermPipeSignal()
{
    quitSaver();
}

//---------------------------------------------------------------------------
void LockProcess::lock()
{
    mLockOnce = true;
    startSaver();
}

//---------------------------------------------------------------------------
void LockProcess::defaultSave()
{
    startSaver();
}

//---------------------------------------------------------------------------
void LockProcess::dontLock()
{
    mLock = false;
    startSaver();
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
    KConfig config( "kdesktoprc", true );

    config.setGroup("ScreenSaver");

//    bool e  = config->readBoolEntry("Enabled", false);
    if(config.readBoolEntry("Lock", false))
    {
        int lockGrace = config.readNumEntry("LockGrace", LOCK_GRACE_DEFAULT);

        if (lockGrace < 0)
        {
            lockGrace = 0;
        }
        else if (lockGrace > 300000)
        {
            lockGrace = 300000; // 5 minutes, keep the value sane
        }

        QTimer::singleShot(lockGrace, this, SLOT(actuallySetLock()));
    }
    mLock = false;

    mPriority = config.readNumEntry("Priority", 19);
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;
    mSaver = config.readEntry("Saver");

    if ((mSaver.isEmpty() && mLockOnce) || mUseBlankOnly)
    {
        mSaver = "KBlankscreen.desktop";
    }

    readSaver();
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

        //kdDebug(1204) << "Reading saver: " << mSaver << endl;

        KDesktopFile config(file, true);

        if (config.hasActionGroup("Root"))
        {
            config.setActionGroup("Root");
            mSaverExec = config.readEntry("Exec");
        }

        //kdDebug(1204) << "Saver-exec: " << mSaverExec << endl;
    }
}

//---------------------------------------------------------------------------
//
// Create a window to draw our screen saver on.
//
void LockProcess::createSaverWindow()
{
    // We only create the window once, but we reset its attributes every
    // time.

    // Some xscreensaver hacks check for this property
    const char *version = "KDE 2.0";
    XChangeProperty (qt_xdisplay(), winId(),
                     gXA_SCREENSAVER_VERSION, XA_STRING, 8, PropModeReplace,
                     (unsigned char *) version, strlen(version));

    XSetWindowAttributes attr;
    if (mColorMap != None)
    {
        attr.colormap = mColorMap;
    }
    else
    {
        attr.colormap = DefaultColormapOfScreen(
                                ScreenOfDisplay(qt_xdisplay(), qt_xscreen()));
    }
    attr.event_mask = KeyPressMask | ButtonPressMask | PointerMotionMask |
                        VisibilityChangeMask | ExposureMask;
    XChangeWindowAttributes(qt_xdisplay(), winId(),
                            CWEventMask | CWColormap, &attr);

    // erase();

    // set NoBackground so that the saver can capture the current
    // screen state if necessary
    setBackgroundMode( QWidget::NoBackground );

    setCursor( blankCursor );
    setGeometry(0, 0, mRootWidth, mRootHeight);
    hide();

    kdDebug(1204) << "Saver window Id: " << winId() << endl;
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
  XDeleteProperty(qt_xdisplay(), winId(), gXA_SCREENSAVER_VERSION);
  if ( gVRoot ) {
      unsigned long vroot_data[1] = { gVRootData };
      XChangeProperty(qt_xdisplay(), gVRoot, gXA_VROOT, XA_WINDOW, 32,
                      PropModeReplace, (unsigned char *)vroot_data, 1);
      gVRoot = 0;
  }
  XSync(qt_xdisplay(), False);
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
  Window root = RootWindowOfScreen(ScreenOfDisplay(qt_xdisplay(), qt_xscreen()));

  gVRoot = 0;
  gVRootData = 0;

  int (*oldHandler)(Display *, XErrorEvent *);
  oldHandler = XSetErrorHandler(ignoreXError);

  if (XQueryTree(qt_xdisplay(), root, &rootReturn, &parentReturn,
      &children, &numChildren))
  {
    for (unsigned int i = 0; i < numChildren; i++)
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytesafter;
      Window *newRoot = (Window *)0;

      if ((XGetWindowProperty(qt_xdisplay(), children[i], gXA_VROOT, 0, 1,
          False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
          (unsigned char **) &newRoot) == Success) && newRoot)
      {
        gVRoot = children[i];
        gVRootData = *newRoot;
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
    assert( KWin::info(winId()).mappingState != NET::Withdrawn );

    if (gVRoot)
        removeVRoot(gVRoot);

    unsigned long rw = RootWindowOfScreen(ScreenOfDisplay(qt_xdisplay(), qt_xscreen()));
    unsigned long vroot_data[1] = { vr };

    Window rootReturn, parentReturn, *children;
    unsigned int numChildren;
    Window top = win;
    while (1) {
        XQueryTree(qt_xdisplay(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren);
        if (children)
            XFree((char *)children);
        if (parentReturn == rw) {
            break;
        } else
            top = parentReturn;
    }

    XChangeProperty(qt_xdisplay(), top, gXA_VROOT, XA_WINDOW, 32,
                     PropModeReplace, (unsigned char *)vroot_data, 1);
}

//---------------------------------------------------------------------------
//
// Remove the virtual root property
//
void LockProcess::removeVRoot(Window win)
{
    XDeleteProperty (qt_xdisplay(), win, gXA_VROOT);
}

//---------------------------------------------------------------------------
//
// Grab the keyboard. Returns true on success
//
bool LockProcess::grabKeyboard()
{
    int rv = XGrabKeyboard( qt_xdisplay(), QApplication::desktop()->winId(),
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
    int rv = XGrabPointer( qt_xdisplay(), QApplication::desktop()->winId(),
            True, GRABEVENTS, GrabModeAsync, GrabModeAsync, None,
            blankCursor.handle(), CurrentTime );

    return (rv == GrabSuccess);
}

//---------------------------------------------------------------------------
//
// Grab keyboard and mouse.  Returns true on success.
//
bool LockProcess::grabInput()
{
    XSync(qt_xdisplay(), False);

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
            XUngrabKeyboard(qt_xdisplay(), CurrentTime);
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------
//
// Release mouse an keyboard grab.
//
void LockProcess::ungrabInput()
{
    XUngrabKeyboard(qt_xdisplay(), CurrentTime);
    XUngrabPointer(qt_xdisplay(), CurrentTime);
}

//---------------------------------------------------------------------------
//
// Send KDM (or XDM, if it gets adapted) a command
//
void LockProcess::xdmFifoCmd(const char *cmd)
{
    QFile fifo(mXdmFifoName);
    if (fifo.open(IO_WriteOnly | IO_Raw)) {
        fifo.writeBlock( cmd, ::strlen(cmd) );
        fifo.close();
    }
}

void LockProcess::xdmFifoLockCmd(const char *cmd)
{
    if (!mXdmFifoName.isNull() && (mLock || mLockOnce))
	xdmFifoCmd(cmd);
}

//---------------------------------------------------------------------------
//
// Start the screen saver.
//
void LockProcess::startSaver()
{
    if (!child_saver && !grabInput())
    {
        kdWarning(1204) << "LockProcess::startSaver() grabInput() failed!!!!" << endl;
        return;
    }
    mBusy = false;

    saveVRoot();

    if (parent) {
        QSocketNotifier *notifier = new QSocketNotifier(parent, QSocketNotifier::Read, this, "notifier");
        connect(notifier, SIGNAL( activated (int)), SLOT( quitSaver()));
    }
    createSaverWindow();
    move(0, 0);
    show();
    setCursor( blankCursor );

    raise();
    XSync(qt_xdisplay(), False);
    if (!child_saver)
        xdmFifoLockCmd("lock\n");
    slotStart();
}

void LockProcess::slotStart()
{
    if (KWin::info(winId()).mappingState == NET::Withdrawn) {
        QTimer::singleShot(100, this, SLOT(slotStart()));
        return;
    }
    setVRoot( winId(), winId() );
    if (startHack() == false)
    {
        // failed to start a hack.  Just show a blank screen
        setBackgroundColor(black);
    }
}

//---------------------------------------------------------------------------
//
// Stop the screen saver.
//
void LockProcess::stopSaver()
{
    kdDebug(1204) << "LockProcess: stopping saver" << endl;
    stopHack();
    hideSaverWindow();
    if (!child_saver) {
        xdmFifoLockCmd("unlock\n");
        ungrabInput();
        const char *out = "GOAWAY!";
        for (QValueList<int>::ConstIterator it = child_sockets.begin(); it != child_sockets.end(); ++it)
            write(*it, out, sizeof(out));
    }
    mLockOnce = false;
}

void LockProcess::actuallySetLock()
{
    mLock = true;
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

    QTextStream ts(&mSaverExec, IO_ReadOnly);
    QString word;
    ts >> word;
    QString path = KStandardDirs::findExe(word);

    if (!path.isEmpty())
    {
        mHackProc << path;

        kdDebug(1204) << "Starting hack: " << path << endl;

        while (!ts.atEnd())
        {
            ts >> word;
            if (word == "%w")
            {
                word = word.setNum(winId());
            }
            mHackProc << word;
        }

        if (mHackProc.start() == true)
        {
#ifdef HAVE_SETPRIORITY
            setpriority(PRIO_PROCESS, mHackProc.getPid(), mPriority);
#endif
            return true;
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
void LockProcess::hackExited( KProcess * )
{
	// Hack exited while we're supposed to be saving the screen.
	// Make sure the saver window is black.
        setBackgroundColor(black);
}

//---------------------------------------------------------------------------
//
// Show the password dialog
//
bool LockProcess::checkPass()
{
    PasswordDlg passDlg(this, !mXdmFifoName.isNull());
    connect(&passDlg, SIGNAL(startNewSession()), SLOT(startNewSession()));

    QDesktopWidget *desktop = KApplication::desktop();
    QRect rect = passDlg.geometry();
    if (child_sockets.isEmpty()) {
        QRect desk = KGlobalSettings::desktopGeometry(QCursor::pos());
        rect.moveCenter(desk.center());
    } else {
        rect.moveCenter(desktop->screenGeometry(qt_xscreen()).center());
    }

    passDlg.move(rect.topLeft() );

    XChangeActivePointerGrab( qt_xdisplay(), GRABEVENTS,
	     arrowCursor.handle(), CurrentTime);
    bool rt = passDlg.exec();
    XChangeActivePointerGrab( qt_xdisplay(), GRABEVENTS,
	     blankCursor.handle(), CurrentTime);
    return rt;
}

//---------------------------------------------------------------------------
//
// X11 Event.
//
bool LockProcess::x11Event(XEvent *event)
{
    switch (event->type)
    {
        case FocusIn:
        case FocusOut:
            // Hack to tell dialogs to take focus when the keyboard is grabbed
            event->xfocus.mode = NotifyNormal;
            break;

        case KeyPress:
        case ButtonPress:
        case MotionNotify:
            if (mBusy)
		break;
            mBusy = true;
            if (!(mLock || mLockOnce) || checkPass())
            {
                stopSaver();
                kapp->quit();
            }
            mBusy = false;
            return true;

        case VisibilityNotify:
            if (event->xvisibility.state != VisibilityUnobscured &&
                event->xvisibility.window == winId())
            {
                raise();
                QApplication::flushX();
            }
            break;

        case ConfigureNotify:
//            // Workaround for bug in Qt 2.1, as advised by Matthias Ettrich (David)
//            if (event->xconfigure.window != event->xconfigure.event)
//                return true;

            raise();
            QApplication::flushX();
            break;
    }

    return false;
}

void LockProcess::startNewSession()
{
    xdmFifoCmd("reserve\n");
}

