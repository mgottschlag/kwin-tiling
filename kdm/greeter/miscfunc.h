#include "dm.h"
#include "greet.h"

#ifdef _AIX
# define __FULL_PROTO 1
# undef HAVE_SETEUID
# undef HAVE_INITGROUPS
# include <sys/id.h>
#endif

#ifndef HAVE_SETEUID
# undef seteuid		/* from config.h */
# define seteuid(euid) setreuid(-1, euid);
# define setegid(egid) setregid(-1, egid);
#endif /* HAVE_SETEUID */

#define F_LEN 50	/* user, password, session string len */

#ifdef __cplusplus
extern "C" {
#endif

int s_copy (char *, const char *, int, int);

#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)
int rdwr_wm (char *wm, int wml, const char *usr, int rd);
#endif

#ifdef __cplusplus
}
#endif
