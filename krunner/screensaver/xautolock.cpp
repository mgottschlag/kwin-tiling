//----------------------------------------------------------------------------
//
// This file is part of the KDE project
//
// Copyright 1999 Martin R. Jones <mjones@kde.org>
// Copyright 2003 Lubos Lunak <l.lunak@kde.org>
//
// KDE screensaver engine
//

#include <config-workspace.h>

#include "xautolock.h"
#include "xautolock_c.h"

#include <kapplication.h>
#include <kdebug.h>

#include <QTimerEvent>
#include <QX11Info>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

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

#include <ctime>

xautolock_corner_t xautolock_corners[ 4 ];

static XAutoLock* self = NULL;

extern "C" {
static int catchFalseAlarms(Display *, XErrorEvent *)
{
    return 0;
}
}

//===========================================================================
//
// Detect user inactivity.
// Named XAutoLock after the program that it is based on.
//
XAutoLock::XAutoLock()
{
    self = this;
#ifdef HAVE_XSCREENSAVER
    mMitInfo = 0;
    int dummy;
    if (XScreenSaverQueryExtension( QX11Info::display(), &dummy, &dummy ))
    {
        mMitInfo = XScreenSaverAllocInfo();
    }
    else
#endif
    {
        kapp->installX11EventFilter( this );
        int (*oldHandler)(Display *, XErrorEvent *);
        oldHandler = XSetErrorHandler(catchFalseAlarms);
        XSync(QX11Info::display(), False );
        xautolock_initDiy( QX11Info::display());
        XSync(QX11Info::display(), False );
        XSetErrorHandler(oldHandler);
    }

    mTimeout = DEFAULT_TIMEOUT;
    mDPMS = true;
    resetTrigger();

    mActive = false;

    mTimerId = startTimer( CHECK_INTERVAL );
    // This is an internal clock timer (in seconds), used instead of querying system time.
    // It is incremented manually, preventing from problems with clock jumps.
    // In other words, this is the 'now' time and the reference point for other times here.
    mElapsed = 0;
}

//---------------------------------------------------------------------------
//
// Destructor.
//
XAutoLock::~XAutoLock()
{
    stop();
    self = NULL;
}

//---------------------------------------------------------------------------
//
// The time in seconds of continuous inactivity.
//
void XAutoLock::setTimeout(int t)
{
    mTimeout = t;
}

void XAutoLock::setDPMS(bool s)
{
#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo( QX11Info::display(), &state, &on );
    if (!on)
        s = false;
#endif
    mDPMS = s;
}

//---------------------------------------------------------------------------
//
// Start watching Activity
//
void XAutoLock::start()
{
    mActive = true;
    resetTrigger();
}

//---------------------------------------------------------------------------
//
// Stop watching Activity
//
void XAutoLock::stop()
{
    mActive = false;
    resetTrigger();
}

//---------------------------------------------------------------------------
//
// Reset the trigger time.
//
void XAutoLock::resetTrigger()
{
    // Time of the last user activity (used only when the internal XScreensaver
    // idle counter is not available).
    mLastReset = mElapsed;
    // Time when screensaver should be activated.
    mTrigger = mElapsed + mTimeout;
#ifdef HAVE_XSCREENSAVER
    mLastIdle = 0;
#endif
    // Do not reset the internal X screensaver here (no XForceScreenSaver())
}

//---------------------------------------------------------------------------
//
// Move the trigger time in order to postpone (repeat) emitting of timeout().
//
void XAutoLock::postpone()
{
    mTrigger = mElapsed + 60; // delay by 60sec
}

//---------------------------------------------------------------------------
//
// Set the remaining time to 't', if it's shorter than already set.
//
void XAutoLock::setTrigger( int t )
{
    time_t newT = mElapsed + qMax(t, 0);
    if (mTrigger > newT)
        mTrigger = newT;
}

//---------------------------------------------------------------------------
//
// Process new windows and check the mouse.
//
void XAutoLock::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() != mTimerId)
    {
        return;
    }
    mElapsed += CHECK_INTERVAL / 1000;

#ifdef HAVE_XSCREENSAVER
    if (!mMitInfo)
#endif
    { // only the diy way needs special X handler
        XSync( QX11Info::display(), False );
        int (*oldHandler)(Display *, XErrorEvent *) =
                XSetErrorHandler(catchFalseAlarms);

        xautolock_processQueue();

        XSetErrorHandler(oldHandler);
    }

#ifdef HAVE_XSCREENSAVER
    if (mMitInfo)
    {
        Display *d = QX11Info::display();
        // Check user idle time. If it is smaller than before, it is either
        // clock jump or user activity, so reset the trigger time. Checking whether
        // there is user inactivity timeout is done below using mTrigger and mElapsed.
        XScreenSaverQueryInfo(d, DefaultRootWindow(d), mMitInfo);
        if (mLastIdle < mMitInfo->idle)
            mLastIdle = mMitInfo->idle;
        else
            resetTrigger();
    }
#endif /* HAVE_XSCREENSAVER */

    // This needs to be after the above check, so it overrides it.
    xautolock_queryPointer( QX11Info::display());

    bool activate = false;

    // This is the test whether to activate screensaver. If we have reached the time
    // and for the whole timeout period there was no activity (which would change mTrigger
    // again), activate.
    if (mElapsed >= mTrigger)
        activate = true;

#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    CARD16 timeout1, timeout2, timeout3;
    DPMSInfo( QX11Info::display(), &state, &on );
    DPMSGetTimeouts( QX11Info::display(), &timeout1, &timeout2, &timeout3 );

    // kDebug() << "DPMSInfo " << state << on;
    // If DPMS is active, it makes XScreenSaverQueryInfo() report idle time
    // that is always smaller than DPMS timeout (X bug I guess). So if DPMS
    // saving is active, simply always activate our saving too, otherwise
    // this could prevent locking from working.
    // X.Org 7.4: With this version activating DPMS resets the screensaver idle timer,
    // so keep this. It probably makes sense to always do this anyway.
    if(state == DPMSModeStandby || state == DPMSModeSuspend || state == DPMSModeOff)
        activate = true;
    // If we are DPMS-dependent and either DPMS is turned off completely or all
    // three DPMS modes are turned off, don't activate (apps use this to turn off
    // screensavers).
    if(mDPMS && (!on || (timeout1 == 0 && timeout2 == 0 && timeout3 == 0 ))) {
        activate = false;
        resetTrigger();
    }
#endif

    // Do not check whether internal X screensaver is enabled or disabled, since we
    // have disabled it ourselves. Some apps might try to disable it too to prevent
    // screensavers, but then our logic breaks[*]. Those apps need to disable DPMS anyway,
    // or they will still have problems, so the DPMS code above should be enough.
    // Besides, I doubt other screensaver implementations check this either.
    // [*] We can't run with X screensaver enabled, since then sooner or later
    // the internal screensaver will activate instead of our screensaver and we cannot
    // prevent its activation by resetting the idle counter since that would also
    // reset DPMS saving.

    if(mActive && activate)
        emit timeout();
}

bool XAutoLock::x11Event( XEvent* ev )
{
    xautolock_processEvent( ev );
// don't futher process key events that were received only because XAutoLock wants them
    if( ev->type == KeyPress && !ev->xkey.send_event
#ifdef HAVE_XSCREENSAVER
        && !mMitInfo
#endif
        && !QWidget::find( ev->xkey.window ))
        return true;
    return false;
}

bool XAutoLock::ignoreWindow( WId w )
{
    if( w != QX11Info::appRootWindow() && QWidget::find( w ))
        return true;
    return false;
}

time_t XAutoLock::idleTime()
{
#ifdef HAVE_XSCREENSAVER
    if (mMitInfo)
        return mMitInfo->idle / 1000;
#endif
    return mElapsed - mLastReset;
}

extern "C"
void xautolock_resetTriggers()
{
  self->resetTrigger();
}

extern "C"
void xautolock_setTrigger( int t )
{
  self->setTrigger( t );
}

extern "C"
int xautolock_ignoreWindow( Window w )
{
   return self->ignoreWindow( w );
}

#include "xautolock.moc"
