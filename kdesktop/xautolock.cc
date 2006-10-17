//----------------------------------------------------------------------------
//
// This file is part of the KDE project
//
// Copyright (c) 1999 Martin R. Jones <mjones@kde.org>
// Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
//
// KDE screensaver engine
//

#include <config.h>
#include <config-kdesktop.h>
#include "xautolock.h"
#include "xautolock.moc"

#include <kapplication.h>
#include <kdebug.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <ctime>
#include "xautolock_c.h"
//Added by qt3to4:
#include <QTimerEvent>
#include <QX11Info>

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

int xautolock_useXidle = 0;
int xautolock_useMit = 0;
xautolock_corner_t xautolock_corners[ 4 ];

static XAutoLock* self = NULL;

static int catchFalseAlarms(Display *, XErrorEvent *)
{
    return 0;
}

//===========================================================================
//
// Detect user inactivity.
// Named XAutoLock after the program that it is based on.
//
XAutoLock::XAutoLock()
{
    self = this;
    int dummy = 0;
    dummy = dummy; // shut up
    xautolock_useXidle = 0;
    xautolock_useMit = 0;
#ifdef HAVE_XIDLE
    useXidle = XidleQueryExtension( QX11Info::display(), &dummy, &dummy );
#endif
#ifdef HAVE_SCREENSAVER
    if( !xautolock_useXidle )
        xautolock_useMit = XScreenSaverQueryExtension( QX11Info::display(), &dummy, &dummy );
#endif
    if( !xautolock_useXidle && !xautolock_useMit )
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

    time(&mLastTimeout);
    mActive = false;

    mTimerId = startTimer( CHECK_INTERVAL );

}

//---------------------------------------------------------------------------
//
// Destructor.
//
XAutoLock::~XAutoLock()
{
  self = NULL;
}

//---------------------------------------------------------------------------
//
// The time in seconds of continuous inactivity.
//
void XAutoLock::setTimeout(int t)
{
    mTimeout = t;
    resetTrigger();
}

void XAutoLock::setDPMS(bool s)
{
    mDPMS = s;
}

//---------------------------------------------------------------------------
//
// Start watching Activity
//
void XAutoLock::start()
{
    resetTrigger();
    time(&mLastTimeout);
    mActive = true;
}

//---------------------------------------------------------------------------
//
// Stop watching Activity
//
void XAutoLock::stop()
{
    mActive = false;
}

//---------------------------------------------------------------------------
//
// Reset the trigger time.
//
void XAutoLock::resetTrigger()
{
    mTrigger = time(0) + mTimeout;
}

//---------------------------------------------------------------------------
//
// Set the remaining time to 't', if it's shorter than already set.
//
void XAutoLock::setTrigger( time_t t )
{
    if( t < mTrigger )
        mTrigger = t;
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

    int (*oldHandler)(Display *, XErrorEvent *) = NULL;
    if( !xautolock_useXidle && !xautolock_useMit )
    { // only the diy way needs special X handler
        XSync( QX11Info::display(), False );
        oldHandler = XSetErrorHandler(catchFalseAlarms);
    }

    xautolock_processQueue();

    time_t now = time(0);
    if ((now > mLastTimeout && now - mLastTimeout > TIME_CHANGE_LIMIT) ||
        (mLastTimeout > now && mLastTimeout - now > TIME_CHANGE_LIMIT+1))
    {
        /* the time has changed in one large jump.  This could be because
           the date was changed, or the machine was suspended.  We'll just
           reset the triger. */
        resetTrigger();
    }

    mLastTimeout = now;

    xautolock_queryIdleTime( QX11Info::display());
    xautolock_queryPointer( QX11Info::display());

    if( !xautolock_useXidle && !xautolock_useMit )
        XSetErrorHandler(oldHandler);

    bool activate = false;

    kDebug() << now << " " << mTrigger << endl;
    if (now >= mTrigger)
    {
        resetTrigger();
        activate = true;
    }

#ifdef HAVE_DPMS
    BOOL on;
    CARD16 state;
    DPMSInfo( QX11Info::display(), &state, &on );

    kDebug() << "DPMSInfo " << state << " " << on << endl;
    // If DPMS is active, it makes XScreenSaverQueryInfo() report idle time
    // that is always smaller than DPMS timeout (X bug I guess). So if DPMS
    // saving is active, simply always activate our saving too, otherwise
    // this could prevent locking from working.
    if(state == DPMSModeStandby || state == DPMSModeSuspend || state == DPMSModeOff)
        activate = true;
    if(!on && mDPMS) {
        activate = false;
        resetTrigger();
    }
#endif
    
#ifdef HAVE_SCREENSAVER
    static XScreenSaverInfo* mitInfo = 0;
    if (!mitInfo) mitInfo = XScreenSaverAllocInfo ();
    if (XScreenSaverQueryInfo (QX11Info::display(), QX11Info::appRootWindow(), mitInfo)) {
        kDebug() << "XScreenSaverQueryInfo " << mitInfo->state << " " << ScreenSaverDisabled << endl;
        if (mitInfo->state == ScreenSaverDisabled)
	    activate = false;
    }
#endif

    if(mActive && activate)
        emit timeout();
}

bool XAutoLock::x11Event( XEvent* ev )
{
    xautolock_processEvent( ev );
// don't futher process key events that were received only because XAutoLock wants them
    if( ev->type == KeyPress && !ev->xkey.send_event
        && !xautolock_useXidle && !xautolock_useMit
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

extern "C"
void xautolock_resetTriggers()
{
  self->resetTrigger();
}

extern "C"
void xautolock_setTrigger( time_t t )
{
  self->setTrigger( t );
}

extern "C"
int xautolock_ignoreWindow( Window w )
{
   return self->ignoreWindow( w );
}
