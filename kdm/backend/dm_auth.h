/************************************************************

Copyright 1998 by Thomas E. Dickey <dickey@clark.net>

   All Rights Reserved

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name(s) of the above copyright
holders shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization.

********************************************************/

#ifndef _DM_AUTH_H_
#define _DM_AUTH_H_ 1

#include "dm.h"

void MitInitAuth( unsigned short name_len, const char *name );
Xauth *MitGetAuth( unsigned short namelen, const char *name );

#ifdef HASXDMAUTH
void XdmInitAuth( unsigned short name_len, const char *name );
Xauth *XdmGetAuth( unsigned short namelen, const char *name );
# ifdef XDMCP
void XdmGetXdmcpAuth( struct protoDisplay *pdpy,
                      unsigned short authorizationNameLen,
                      const char *authorizationName );
int XdmCheckAuthentication( struct protoDisplay *pdpy,
                            ARRAY8Ptr displayID,
                            ARRAY8Ptr authenticationName,
                            ARRAY8Ptr authenticationData );
# else
#  define XdmGetXdmcpAuth NULL
# endif
#endif

#ifdef SECURE_RPC
void SecureRPCInitAuth( unsigned short name_len, const char *name );
Xauth *SecureRPCGetAuth( unsigned short name_len, const char *name );
#endif

#ifdef K5AUTH
void Krb5InitAuth( unsigned short name_len, const char *name );
Xauth *Krb5GetAuth( unsigned short name_len, const char *name );

Xauth *Krb5GetAuthFor( unsigned short name_len, const char *name, const char *dname );
char *Krb5Init( const char *user, const char *passwd, const char *dname );
void Krb5Destroy( const char *dname );
#endif

/* auth.c */
int ValidAuthorization( unsigned short name_length, const char *name );


#ifdef XDMCP

void
SetProtoDisplayAuthorization( struct protoDisplay *pdpy,
                              unsigned short authorizationNameLen,
                              const char *authorizationName );

#endif /* XDMCP */

int SaveServerAuthorizations( struct display *d, Xauth **auths, int count );
void CleanUpFileName( const char *src, char *dst, int len );
void RemoveUserAuthorization( struct display *d );
void SetAuthorization( struct display *d );
void SetLocalAuthorization( struct display *d );
void SetUserAuthorization( struct display *d );

/* genauth.c */
int GenerateAuthData( char *auth, int len );
#ifdef NEED_ENTROPY
void AddPreGetEntropy( void );
void AddOtherEntropy( void );
void AddTimerEntropy( void );
#endif

#endif /* _DM_AUTH_H_ */
