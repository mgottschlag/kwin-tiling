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
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#ifndef _KDM_GREET_H_
#define _KDM_GREET_H_

#include <greet.h> /* for the ATTR_ defines */
#include <config.ci> /* for the HAVE_VTS define */

#ifdef __cplusplus
extern "C" {
#endif

void GSet( int master );
void GSendInt( int val );
void GSendStr( const char *buf );
/*void GSendNStr( const char *buf, int len );*/
void GSendArr( int len, const char *buf );
int GRecvInt( void );
char *GRecvStr( void );
char **GRecvStrArr( int *len );
char *GRecvArr( int *len );

int GetCfgInt( int id );
char *GetCfgStr( int id );
char **GetCfgStrArr( int id, int *len );

typedef struct dpySpec {
	struct dpySpec *next;
	char *display, *from, *user, *session;
#ifdef HAVE_VTS
	int vt;
#endif
	int flags;
	int count;
} dpySpec;

dpySpec *fetchSessions( int flags );
void disposeSessions( dpySpec *sess );

void freeStrArr( char **arr );

void Debug( const char *fmt, ... );
void LogInfo( const char *fmt, ... );
void LogError( const char *fmt, ... );
void LogPanic( const char *fmt, ... ) ATTR_NORETURN;

struct _XDisplay;

void SecureDisplay( struct _XDisplay *dpy );
void UnsecureDisplay( struct _XDisplay *dpy );
int PingServer( struct _XDisplay *dpy );

void setup_modifiers( struct _XDisplay *mdpy, int numlock );
void restore_modifiers( void );

void setCursor( struct _XDisplay *mdpy, int window, int shape );


extern int rfd; /* for select() loops */

extern char *dname; /* d->name */

#ifdef __cplusplus
}
#endif

#endif /* _KDM_GREET_H_ */
