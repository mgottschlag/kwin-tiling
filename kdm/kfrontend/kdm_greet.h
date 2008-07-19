/*

KDE Greeter module for xdm

Copyright (C) 2001-2003 Oswald Buddenhagen <ossi@kde.org>

This file contains code from the old xdm core,
Copyright 1988, 1998  Keith Packard, MIT X Consortium/The Open Group

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef _KDM_GREET_H_
#define _KDM_GREET_H_

#include <greet.h>

#ifdef __cplusplus
extern "C" {
#endif

void gSet( int master );
void gSendInt( int val );
void gSendStr( const char *buf );
/*void gSendNStr( const char *buf, int len );*/
void gSendArr( int len, const char *buf );
int gRecvInt( void );
char *gRecvStr( void );
char **gRecvStrArr( int *len );
char *gRecvArr( int *len );

int getCfgInt( int id );
char *getCfgStr( int id );
char **getCfgStrArr( int id, int *len );

void freeStrArr( char **arr );

extern int debugLevel;
void debug( const char *fmt, ... );
void logInfo( const char *fmt, ... );
void logWarn( const char *fmt, ... );
void logError( const char *fmt, ... );
void logPanic( const char *fmt, ... ) ATTR_NORETURN;

struct _XDisplay;

void secureKeyboard( struct _XDisplay *dpy );
void securePointer( struct _XDisplay *dpy );
void secureInputs( struct _XDisplay *dpy );
void unsecureInputs( struct _XDisplay *dpy );
void secureDisplay( struct _XDisplay *dpy );
void unsecureDisplay( struct _XDisplay *dpy );
int pingServer( struct _XDisplay *dpy );

void setupModifiers( struct _XDisplay *mdpy, int numlock );
void restoreModifiers( void );

void setCursor( struct _XDisplay *mdpy, int window, int shape );


extern int rfd; /* for select() loops */
extern int mrfd, mwfd, srfd, swfd; /* for main */

extern char *dname; /* d->name */

#ifdef __cplusplus
}
#endif

#endif /* _KDM_GREET_H_ */
