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

#include "xautolock_c.h"

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
      switch (corners[corner])
      {
        case ca_forceLock:
#if 0
          xautolock_setTrigger( (useRedelay ? cornerRedelay : cornerDelay) - 1 );
#else
          xautolock_setTrigger( 0 );
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
