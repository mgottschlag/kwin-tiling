/*****************************************************************************
 *
 * Authors: Michel Eyckmans (MCE) & Stefan De Troch (SDT)
 *
 * Content: This file is part of version 2.x of xautolock. It takes care
 *          of most OS dependencies, and defines the program's default
 *          settings.
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

#ifndef __xautolock_c_h
#define __xautolock_c_h

#include <X11/Xlib.h>
#ifdef __cplusplus
# include <fixx11h.h>
#endif

#define DEFAULT_TIMEOUT           600

#define CHECK_INTERVAL           5000      /* ms */

#define CREATION_DELAY             30      /* should be > 10 and
                                              < min (45,(MIN_MINUTES*30))  */
#define TIME_CHANGE_LIMIT         120      /* if the time changes by more
                                              than x secs then we will
                                              assume someone has changed
                                              date or machine has suspended */

#define cornerSize                  5

#define cornerDelay                 5

#define cornerRedelay               5

typedef enum { ca_nothing, ca_dontLock, ca_forceLock } xautolock_corner_t;

#ifdef __cplusplus
extern "C"
{
#endif
void xautolock_processEvent( XEvent* ev );
void xautolock_processQueue( void );
void xautolock_queryPointer (Display* d);
void xautolock_initDiy (Display* d);
void xautolock_resetTriggers( void );
void xautolock_setTrigger( int );
int xautolock_ignoreWindow( Window );
extern xautolock_corner_t xautolock_corners[ 4 ];
#ifdef __cplusplus
}
#endif


#endif
