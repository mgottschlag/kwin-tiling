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

#include <config.h>

#include <stdlib.h>
#include <qcursor.h>

#include <kstandarddirs.h>
#include <kapplication.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kwin.h>
#include <qtimer.h>
#include <qfile.h>
#include <assert.h>
#include <signal.h>
#include <qsocketnotifier.h>
#include <qtimer.h>
#include <kdebug.h>

#include "lockprocess.h"
#include "lockprocess.moc"

#ifdef HAVE_SETPRIORITY
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#define PASSDLG_HIDE_TIMEOUT        10000
#define LOCK_GRACE_TIMEOUT          5000

int ignoreXError(Display *, XErrorEvent *);
static Window gVRoot = 0;
static Window gVRootData = 0;
static Atom   gXA_VROOT;
static Atom   gXA_SCREENSAVER_VERSION;

//===========================================================================
//
// Screen saver handling process.  Handles screensaver window,
// starting screensaver hacks, and password entry.
//
LockProcess::LockProcess(bool child)
    : QWidget(0L, "saver window", WStyle_Customize | WStyle_NoBorder), child_saver(child), parent(0)
{
    KWin::setState( winId(), NET::StaysOnTop );

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

    mState = Saving;
    mPassDlg = 0;
    mHidePassTimerId = 0;
    mCheckPassTimerId = 0;
    mCheckingPass = false;
    mLockOnce = false;

    // virtual root property
    gXA_VROOT = XInternAtom (qt_xdisplay(), "__SWM_VROOT", False);
    gXA_SCREENSAVER_VERSION = XInternAtom (qt_xdisplay(), "_SCREENSAVER_VERSION", False);

    XWindowAttributes attrs;
    XGetWindowAttributes(qt_xdisplay(), winId(), &attrs);
    mColorMap = attrs.colormap;

    connect(&mPassProc, SIGNAL(processExited(KProcess *)),
                        SLOT(passwordChecked(KProcess *)));
    connect(&mHackProc, SIGNAL(processExited(KProcess *)),
                        SLOT(hackExited(KProcess *)));

    QStringList dmopt =
        QStringList::split( QChar( ',' ),
                            QString::fromLatin1( ::getenv( "XDM_MANAGED" ) ) );
    if ( dmopt.isEmpty() || dmopt.first()[0] != QChar( '/' ) )
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
    hidePassDlg();
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
    if (mState == Saving)
    {
        stopSaver();
    }
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
        QTimer::singleShot(LOCK_GRACE_TIMEOUT, this, SLOT(actuallySetLock()));
    }
    else
    {
        mLock = false;
    }
    
    mPriority = config.readNumEntry("Priority", 19);
    if (mPriority < 0) mPriority = 0;
    if (mPriority > 19) mPriority = 19;
    mSaver = config.readEntry("Saver");

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

//        kdDebug(1204) << "Reading saver: " << saver << endl;

        KDesktopFile config(file, true);

        if (config.hasActionGroup("Root"))
        {
            config.setActionGroup("Root");
            mSaverExec = config.readEntry("Exec");
        }

//        kdDebug(1204) << "Saver-exec: " << mSaverExec << endl;
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
    attr.event_mask = KeyPressMask | ButtonPressMask | MotionNotify |
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

//---------------------------------------------------------------------------
//
// Grab the mouse.  Returns true on success
//
bool LockProcess::grabMouse()
{
    int rv = XGrabPointer( qt_xdisplay(), QApplication::desktop()->winId(),
            True, ButtonPressMask
            | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask
            | PointerMotionMask | PointerMotionHintMask | Button1MotionMask
            | Button2MotionMask | Button3MotionMask | Button4MotionMask
            | Button5MotionMask | ButtonMotionMask | KeymapStateMask,
            GrabModeAsync, GrabModeAsync, None, blankCursor.handle(),
            CurrentTime );

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
// Send KDM (or XDM, if it gets adapted) a command if we lock the screen
//
void LockProcess::xdmFifoCmd(const char *cmd)
{
    if (!mXdmFifoName.isNull() && (mLock || mLockOnce)) {
	QFile fifo(mXdmFifoName);
	if (fifo.open(IO_WriteOnly | IO_Raw)) {
            fifo.writeBlock( cmd, ::strlen(cmd) );
            fifo.close();
	}
    }
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
    mState = Saving;

    saveVRoot();

    if (parent) {
        QSocketNotifier *notifier = new QSocketNotifier(parent, QSocketNotifier::Read, this, "notifier");
        connect(notifier, SIGNAL( activated (int)), SLOT( quitSaver()));
        notifier = new QSocketNotifier(parent, QSocketNotifier::Exception, this, "notifier_ex");
        connect(notifier, SIGNAL( activated (int)), SLOT( quitSaver()));
    }
    createSaverWindow();
    move(0, 0);
    show();
    setCursor( blankCursor );

    raise();
    XSync(qt_xdisplay(), False);
    if (!child_saver)
        xdmFifoCmd("lock\n");
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
    xdmFifoCmd("unlock\n");
    hideSaverWindow();
    hidePassDlg();
    if (!child_saver) {
        ungrabInput();
        const char *out = "GOAWAY!";
        for (QValueList<int>::ConstIterator it = child_sockets.begin(); it != child_sockets.end(); ++it)
            write(*it, out, sizeof(out));
    }
    mLockOnce = false;
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
void LockProcess::showPassDlg()
{
    if (mPassDlg)
    {
        hidePassDlg();
    }
    mPassDlg = new PasswordDlg(this);
    QDesktopWidget *desktop = KApplication::desktop();

    QRect rect = mPassDlg->geometry();
    if (child_sockets.isEmpty()) {
        rect.moveCenter( desktop->screenGeometry(desktop->screenNumber(QCursor::pos())).center());
    } else
        rect.moveCenter( desktop->screenGeometry(qt_xscreen()).center());
    mPassDlg->move(rect.topLeft() );

    mPassDlg->show();
    setPassDlgTimeout(PASSDLG_HIDE_TIMEOUT);
}

//---------------------------------------------------------------------------
//
// Hide the password dialog
//
void LockProcess::hidePassDlg()
{
    if (mPassDlg)
    {
        delete mPassDlg;
        mPassDlg = 0;
        killPassDlgTimeout();
    }
}

//---------------------------------------------------------------------------
//
// Hide the password dialog in "t" seconds.
//
void LockProcess::setPassDlgTimeout(int t)
{
    if (mHidePassTimerId)
    {
        killTimer(mHidePassTimerId);
    }
    mHidePassTimerId = startTimer(t);
}

//---------------------------------------------------------------------------
//
// Kill the password dialog hide timer.
//
void LockProcess::killPassDlgTimeout()
{
    if (mHidePassTimerId)
    {
        killTimer(mHidePassTimerId);
        mHidePassTimerId = 0;
    }
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
            return handleKeyPress((XKeyEvent *)event);

        case ButtonPress:
        case MotionNotify:
            if (mState == Saving)
            {
                if (mLock || mLockOnce)
                {
                    showPassDlg();
                    mState = Password;
                }
                else
                {
                    stopSaver();
		    kapp->quit();
                }
            }
            break;

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

            if (mState == Saving || mState == Password)
            {
                raise();
                QApplication::flushX();
            }
            break;
    }

    return false;
}

//---------------------------------------------------------------------------
//
// Handle key press event.
//
bool LockProcess::handleKeyPress(XKeyEvent *xke)
{
    switch (mState)
    {
        case Password:
            if (!mCheckingPass)
            {
                KeySym keysym = XLookupKeysym(xke, 0);
                switch (keysym)
                {
                    case XK_Escape:
                        hidePassDlg();
                        mState = Saving;
                        break;

                    case XK_Return:
                    case XK_KP_Enter:
                        startCheckPassword();
                        break;

                    default:
                        setPassDlgTimeout(PASSDLG_HIDE_TIMEOUT);
                        mPassDlg->keyPressed(xke);
                }
            }
	    break;

        case Saving:
            if (mLock || mLockOnce)
            {
                showPassDlg();
                mState = Password;
            }
            else
            {
                stopSaver();
		kapp->quit();
            }
	    break;
    }
    return true;
}

//---------------------------------------------------------------------------
//
// Starts the kcheckpass process to check the user's password.
//
// Serge Droz <serge.droz@pso.ch> 10.2000
// Define ACCEPT_ENV if you want to pass an environment variable to
// kcheckpass. Define ACCEPT_ARGS if you want to pass command line
// arguments to kcheckpass
#define ACCEPT_ENV
//#define ACCEPT_ARGS
void LockProcess::startCheckPassword()
{
    const char *passwd = mPassDlg->password().ascii();
    if (passwd)
    {
        QString kcp_binName = KStandardDirs::findExe("kcheckpass");

        mPassProc.clearArguments();
        mPassProc << kcp_binName;

#ifdef HAVE_PAM
# ifdef ACCEPT_ENV
        setenv("KDE_PAM_ACTION", KSCREENSAVER_PAM_SERVICE, 1);
# elif defined(ACCEPT_ARGS)
        mPassProc << "-c" << KSCREENSAVER_PAM_SERVICE;
# endif
#endif
	bool ret = mPassProc.start(KProcess::NotifyOnExit, KProcess::Stdin);
#ifdef HAVE_PAM
# ifdef ACCEPT_ENV
        unsetenv("KDE_PAM_ACTION");
# endif
#endif
	if (ret == false)
        {
            kdDebug(1204) << "kcheckpass failed to start" << endl;
            return;
        }

        // write Password to stdin
        mPassProc.writeStdin(passwd, strlen(passwd));
        mPassProc.closeStdin();

        killPassDlgTimeout();

        mCheckingPass = true;
    }
}

//---------------------------------------------------------------------------
//
// The kcheckpass process has exited.
//
void LockProcess::passwordChecked(KProcess *proc)
{
    if (proc == &mPassProc)
    {
	    /* the exit codes of kcheckpass:
	       0: everything fine
		   1: authentification failed
		   2: passwd access failed [permissions/misconfig]
	    */
        if (mPassProc.normalExit() && (mPassProc.exitStatus() != 1))
        {
            stopSaver();
	    if ( mPassProc.exitStatus() == 2 )
	    {
		KMessageBox::error(0,
		  i18n( "<h1>Screen Locking Failed!</h1>"
		  "Your screen was not locked because the <i>kcheckpass</i> "
		  "program was not able to check your password. This is "
		  "usually the result of kcheckpass not being installed "
		  "correctly. If you installed KDE yourself, reinstall "
		  "kcheckpass as root. If you are using a pre-compiled "
		  "package, contact the packager." ),
		  i18n( "Screen Locking Failed" ) );
	    }
	    kapp->quit();
        }
        else
        {
            mPassDlg->showFailed();
            mPassDlg->resetPassword();
            setPassDlgTimeout(PASSDLG_HIDE_TIMEOUT);
        }

        mCheckingPass = false;
    }
}

//---------------------------------------------------------------------------
//
// Handle our timer events.
//
void LockProcess::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == mHidePassTimerId && !mCheckingPass)
    {
        hidePassDlg();
        mState = Saving;
    }
}

//---------------------------------------------------------------------------
int ignoreXError(Display *, XErrorEvent *)
{
    return 0;
}

void LockProcess::actuallySetLock()
{
    mLock = true;
}

