#ifndef _MISCFUNC_H
#define _MISCFUNC_H

#include "dm.h"
#include "greet.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _AIX
# define __FULL_PROTO 1
# include <sys/id.h>
int setegid (gid_t egid);
#endif

#ifndef HAVE_SETEUID
# undef seteuid		/* from config.h */
# define seteuid(euid) setreuid(-1, euid)
# define setegid(egid) setregid(-1, egid)
#endif /* HAVE_SETEUID */

#define F_LEN 50	/* user, password, session string len */

int s_copy (char *, const char *, int, int);

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)
# define USE_RDWR_WM
# define WMRC ".wmrc"
int rdwr_wm (char *wm, int wml, const char *usr, int rd);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _MISCFUNC_H */
