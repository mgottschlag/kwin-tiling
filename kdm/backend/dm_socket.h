/************************************************************

Copyright 1998 by Thomas E. Dickey <dickey@clark.net>
Copyright 2002-2004 Oswald Buddenhagen <ossi@kde.org>

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

#ifndef _DM_SOCKET_H_
#define _DM_SOCKET_H_ 1

#ifndef __Lynx__
# include <sys/socket.h>
#else
# include <socket.h>
#endif

#ifdef TCPCONN
# include <netinet/in.h>
#endif

#ifdef UNIXCONN
# ifndef __Lynx__
#  include <sys/un.h>
# else
#  include <un.h>
# endif
#endif

#ifdef DNETCONN
# include <netdnet/dn.h>
#endif

#if (defined(__svr4__) && !defined(__sun__)) && defined(SIOCGIFCONF)
# define SYSV_SIOCGIFCONF
int ifioctl( int fd, int cmd, char *arg );
#else
# define ifioctl ioctl
#endif

#ifdef BSD
# if (BSD >= 199103)
#  define VARIABLE_IFREQ
# endif
#endif

#endif /* _DM_SOCKET_H_ */
