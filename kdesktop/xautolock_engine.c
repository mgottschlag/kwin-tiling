/*****************************************************************************
 *
 * Authors: Michel Eyckmans (MCE) & Stefan De Troch (SDT)
 *
 * Content: This file is part of version 2.x of xautolock. It implements
 *          the program's core functions.
 *
 *          Please send bug reports etc. to eyckmans@imec.be.
 *
 * --------------------------------------------------------------------------
 * 
 * Copyright 1990,1992-1999,2001-2002 by Stefan De Troch and Michel Eyckmans.
 * 
 * Versions 2.0 and above of xautolock are available under version 2 of the
 * GNU GPL. Earlier versions are available under other conditions. For more
 * information, see the License file.
 * 
 *****************************************************************************/

#include <X11/Xlib.h>
#include <time.h>
#include <config-kdesktop.h>
#include "xautolock_c.h"

/*
 *  Function for querying the idle time from the server.
 *  Only used if either the Xidle or the Xscreensaver
 *  extension is present.
 */
void 
xautolock_queryIdleTime (Display* d)
{
  Time idleTime = 0; /* millisecs since last input event */

#ifdef HasXidle
  if (xautolock_useXidle)
  {
    XGetIdleTime (d, &idleTime);
  }
  else
#endif /* HasXIdle */
  {
#ifdef HAVE_SCREENSAVER
    if( xautolock_useMit )
    {
    static XScreenSaverInfo* mitInfo = 0; 
    if (!mitInfo) mitInfo = XScreenSaverAllocInfo ();
    XScreenSaverQueryInfo (d, DefaultRootWindow (d), mitInfo);
    idleTime = mitInfo->idle;
    }
    else
#endif /* HAVE_SCREENSAVER */
    {
        d = d; /* shut up */
        return; /* DIY */
    }
  }

  if (idleTime < CHECK_INTERVAL )  
  {
    xautolock_resetTriggers ();
  }
}

/*
 *  Function for monitoring pointer movements. This implements the 
 *  `corners' feature and as a side effect also tracks pointer 
 *  related user activity. The latter actually is only needed when
 *  we're using the DIY mode of operations, but it's much simpler
 *  to do it unconditionally.
 */
void 
xautolock_queryPointer (Display* d)
{
  Window           dummyWin;         /* as it says                    */
  int              dummyInt;         /* as it says                    */
  unsigned         mask;             /* modifier mask                 */
  int              rootX;            /* as it says                    */
  int              rootY;            /* as it says                    */
  int              corner;           /* corner index                  */
  time_t           now;              /* as it says                    */
  time_t           newTrigger;       /* temporary storage             */
  int              i;                /* loop counter                  */
  static Window    root;             /* root window the pointer is on */
  static Screen*   screen;           /* screen the pointer is on      */
  static unsigned  prevMask = 0;     /* as it says                    */
  static int       prevRootX = -1;   /* as it says                    */
  static int       prevRootY = -1;   /* as it says                    */
  static Bool      firstCall = True; /* as it says                    */

 /*
  *  Have a guess...
  */
  if (firstCall)
  {
    firstCall = False;
    root = DefaultRootWindow (d);
    screen = ScreenOfDisplay (d, DefaultScreen (d));
  }

 /*
  *  Find out whether the pointer has moved. Using XQueryPointer for this
  *  is gross, but it also is the only way never to mess up propagation
  *  of pointer events.
  */
  if (!XQueryPointer (d, root, &root, &dummyWin, &rootX, &rootY,
                      &dummyInt, &dummyInt, &mask))
  {
   /*
    *  Pointer has moved to another screen, so let's find out which one.
    */
    for (i = -1; ++i < ScreenCount (d); ) 
    {
      if (root == RootWindow (d, i)) 
      {
        screen = ScreenOfDisplay (d, i);
        break;
      }
    }
  }

  if (   rootX == prevRootX
      && rootY == prevRootY
      && mask == prevMask)
  {
  xautolock_corner_t* corners = xautolock_corners;
   /*
    *  If the pointer has not moved since the previous call and 
    *  is inside one of the 4 corners, we act according to the
    *  contents of the "corners" array.
    *
    *  If rootX and rootY are less than zero, don't lock even if
    *  ca_forceLock is set in the upper-left corner. Why? 'cause
    *  on initial server startup, if (and only if) the pointer is
    *  never moved, XQueryPointer() can return values less than 
    *  zero (only some servers, Openwindows 2.0 and 3.0 in 
    *  particular).
    */
    if (   (corner = 0,
               rootX <= cornerSize && rootX >= 0
            && rootY <= cornerSize && rootY >= 0)
        || (corner++,
               rootX >= WidthOfScreen  (screen) - cornerSize - 1
            && rootY <= cornerSize)
        || (corner++,
               rootX <= cornerSize
            && rootY >= HeightOfScreen (screen) - cornerSize - 1)
        || (corner++,
               rootX >= WidthOfScreen  (screen) - cornerSize - 1
            && rootY >= HeightOfScreen (screen) - cornerSize - 1))
    {
      now = time (0);

      switch (corners[corner])
      {
        case ca_forceLock:
#if 0
          newTrigger = now + (useRedelay ? cornerRedelay : cornerDelay) - 1;
#else
          newTrigger = now + 2 - 1;
#endif

#if 0
          if (newTrigger < lockTrigger)
          {
            setLockTrigger (newTrigger - now);
          }
#else
          xautolock_setTrigger( newTrigger );
#endif
          break;

        case ca_dontLock:
          xautolock_resetTriggers ();

#ifdef __GNUC__
	default: ; /* Makes gcc -Wall shut up. */
#endif /* __GNUC__ */
      }
    }
  }
  else
  {
#if 0
    useRedelay = False;
#endif
    prevRootX = rootX;
    prevRootY = rootY;
    prevMask = mask;

    xautolock_resetTriggers ();
  }
}

#if 0
/*
 *  Support for deciding whether to lock or kill.
 */
void
evaluateTriggers (Display* d)
{
  static time_t prevNotification = 0;
  time_t        now = 0;

 /*
  *  Obvious things first.
  *
  *  The triggers are being moved all the time while in disabled
  *  mode in order to make absolutely sure we cannot run into
  *  trouble by an enable message coming in at an odd moment.
  *  Otherwise we possibly might lock or kill too soon.
  */
  if (disabled)
  {
    resetTriggers ();
  }

 /*
  *  Next, wait for (or kill, if we were so told) the previous 
  *  locker (if any). Note that this must also be done while in
  *  disabled mode. Not only to avoid a potential zombie proces
  *  hanging around until we are re-enabled, but also to prevent
  *  us from incorrectly setting a kill trigger at the moment 
  *  when we are finally re-enabled.
  */
#ifdef VMS
  if (vmsStatus == 0)  
  {
#else /* VMS */
  if (lockerPid)
  {
#if !defined (UTEKV) && !defined (SYSV) && !defined (SVR4)
    union wait  status;      /* child's process status */
#else /* !UTEKV && !SYSV && !SVR4 */
    int         status = 0;  /* child's process status */
#endif /* !UTEKV && !SYSV && !SVR4 */

    if (unlockNow && !disabled)
    {
      (void) kill (lockerPid, SIGTERM);
    }

#if !defined (UTEKV) && !defined (SYSV) && !defined (SVR4)
    if (wait3 (&status, WNOHANG, 0))
#else /* !UTEKV && !SYSV && !SVR4 */
    if (waitpid (-1, &status, WNOHANG)) 
#endif /* !UTEKV && !SYSV && !SVR4 */
    {
     /*
      *  If the locker exited normally, we disable any pending kill
      *  trigger. Otherwise, we assume that it either has crashed or
      *  was not able to lock the display because of an existing
      *  locker (which may have been started manually). In both of
      *  the later cases, disabling the kill trigger would open a
      *  loop hole.
      */
      if (   WIFEXITED (status)
	  && WEXITSTATUS (status) == EXIT_SUCCESS)
      {
        disableKillTrigger ();
      }

      useRedelay = True;
      lockerPid = 0;
    }
#endif /* VMS */

    setLockTrigger (lockTime);

   /*
    *  No return here! The pointer may be sitting in a corner, while
    *  parameter settings may be such that we need to start another
    *  locker without further delay. If you think this cannot happen,
    *  consider the case in which the locker simply crashed.
    */
  }

  unlockNow = False;

 /*
  *  Note that the above lot needs to be done even when we're in 
  *  disabled mode, since we may have entered said mode with an
  *  active locker around. 
  */
  if (disabled) return;

 /*
  *  Is it time to run the killer command?
  */
  now = time (0);

  if (killTrigger && now >= killTrigger)
  {
   /*
    *  There is a dirty trick here. On the one hand, we don't want
    *  to block until the killer returns, but on the other one
    *  we don't want to have it interfere with the wait() stuff we 
    *  do to keep track of the locker. To obtain both, the killer
    *  command has already been patched by KillerChecker() so that
    *  it gets backgrounded by the shell started by system().
    *
    *  For the time being, VMS users are out of luck: their xautolock
    *  will indeed block until the killer returns.
    */
    (void) system (killer);
    setKillTrigger (killTime);
  }

 /*
  *  Now trigger the notifier if required. 
  */
  if (   notifyLock
      && now + notifyMargin >= lockTrigger
      && prevNotification < now - notifyMargin - 1)
  {
    if (notifierSpecified)
    {
     /*
      *  Here we use the same dirty trick as for the killer command.
      */
      (void) system (notifier);
    }
    else
    {
      (void) XBell (d, bellPercent);
      (void) XSync (d, 0);
    }

    prevNotification = now;
  }

 /*
  *  Finally fire up the locker if time has somehow come. 
  */
  if (   lockNow
      || now >= lockTrigger)
  {
#ifdef VMS
    if (vmsStatus != 0)
#else /* VMS */
    if (!lockerPid)
#endif /* VMS */
    {
      switch (lockerPid = vfork ())
      {
        case -1:
          lockerPid = 0;
          break;
  
        case 0:
          (void) close (ConnectionNumber (d));
#ifdef VMS
          vmsStatus = 0;
          lockerPid = lib$spawn ((lockNow ? &nowLockerDescr : &lockerDescr),
	                         0, 0, &1, 0, 0, &vmsStatus);

	  if (!(lockerPid & 1)) exit (lockerPid);

#ifdef SLOW_VMS
          (void) sleep (SLOW_VMS_DELAY); 
#endif /* SLOW_VMS */
#else /* VMS */
          (void) execl ("/bin/sh", "/bin/sh", "-c", 
	                (lockNow ? nowLocker : locker), 0);
#endif /* VMS */
          _exit (EXIT_FAILURE);
  
        default:
         /*
          *  In general xautolock should keep its fingers off the real
          *  screensaver because no universally acceptable policy can 
          *  be defined. In no case should it decide to disable or enable 
          *  it all by itself. Setting the screensaver policy is something
          *  the locker should take care of. After all, xautolock is not
          *  supposed to know what the "locker" does and doesn't do. 
          *  People might be using xautolock for totally different
          *  purposes (which, by the way, is why it will accept a
          *  different set of X resources after being renamed).
          *
          *  Nevertheless, simply resetting the screensaver is a
          *  convenience action that aids many xlock users, and doesn't
          *  harm anyone (*). The problem with older versions of xlock 
	  *  is that they can be told to replace (= disable) the real
	  *  screensaver, but forget to reset that same screensaver if
	  *  it was already active at the time xlock starts. I guess 
	  *  xlock initially wasn't designed to be run without a user
	  *  actually typing the comand ;-).
	  *
	  *  (*) Well, at least it used not to harm anyone, but with the
	  *      advent of DPMS monitors, it now can mess up the power
	  *      saving setup. Hence we better make it optional. 
	  *
	  *      Also, some xlock versions also unconditionally call
	  *      XResetScreenSaver, yielding the same kind of problems
	  *      with DPMS that xautolock did. The latest and greatest
	  *      xlocks also have a -resetsaver option for this very
	  *      reason. You may want to upgrade.
          */
	  if (resetSaver) (void) XResetScreenSaver(d);
  
          setLockTrigger (lockTime);
          (void) XSync (d,0);
      }

     /*
      *  Once the locker is running, all that needs to be done is to 
      *  set the killTrigger if needed. Notice that this must be done 
      *  even if we actually failed to start the locker. Otherwise
      *  the error would "propagate" from one feature to another.
      */
      if (killerSpecified) setKillTrigger (killTime);

      useRedelay = False;
    }

    lockNow = False;
  }
}
#endif
