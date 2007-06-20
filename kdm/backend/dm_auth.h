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

void mitInitAuth( unsigned short name_len, const char *name );
Xauth *mitGetAuth( unsigned short namelen, const char *name );

#ifdef HASXDMAUTH
void xdmInitAuth( unsigned short name_len, const char *name );
Xauth *xdmGetAuth( unsigned short namelen, const char *name );
# ifdef XDMCP
void xdmGetXdmcpAuth( struct protoDisplay *pdpy,
                      unsigned short authorizationNameLen,
                      const char *authorizationName );
int xdmcheckAuthentication( struct protoDisplay *pdpy,
                            ARRAY8Ptr displayID,
                            ARRAY8Ptr authenticationName,
                            ARRAY8Ptr authenticationData );
# else
#  define xdmGetXdmcpAuth NULL
# endif
#endif

#ifdef SECURE_RPC
void secureRPCInitAuth( unsigned short name_len, const char *name );
Xauth *secureRPCGetAuth( unsigned short name_len, const char *name );
#endif

#ifdef K5AUTH
void krb5InitAuth( unsigned short name_len, const char *name );
Xauth *krb5GetAuth( unsigned short name_len, const char *name );

Xauth *krb5GetAuthFor( unsigned short name_len, const char *name, const char *dname );
char *krb5Init( const char *user, const char *passwd, const char *dname );
void krb5Destroy( const char *dname );
#endif

/* auth.c */
int validAuthorization( unsigned short name_length, const char *name );


#ifdef XDMCP

void
setProtoDisplayAuthorization( struct protoDisplay *pdpy,
                              unsigned short authorizationNameLen,
                              const char *authorizationName );

#endif /* XDMCP */

int saveServerAuthorizations( struct display *d, Xauth **auths, int count );
void cleanUpFileName( const char *src, char *dst, int len );
void removeUserAuthorization( struct display *d );
void setAuthorization( struct display *d );
void setLocalAuthorization( struct display *d );
void setUserAuthorization( struct display *d );

/* genauth.c */
int generateAuthData( char *auth, int len );
#ifdef NEED_ENTROPY
void addPreGetEntropy( void );
void addOtherEntropy( void );
void addTimerEntropy( void );
#endif

#endif /* _DM_AUTH_H_ */
