/*

Copyright 1994, 1998  The Open Group

All Rights Reserved.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/greet.h,v 1.5 2000/05/31 07:15:11 eich Exp $ */

/*
 * greetest.c - a dummy xdm for testing greeter modules
 */


#include "dm.h"
#include "dm_socket.h"
#include "dm_auth.h"
#include "greet.h"

#include <X11/Xlib.h>

#include <dlfcn.h>
#ifndef RTLD_NOW
# define RTLD_NOW 1
#endif

#include <stdio.h>
#include <stdarg.h>

#include <grp.h>	/* for initgroups */

#ifdef HAVE_CRYPT_H
# include <crypt.h>
#else
# define _XOPEN_SOURCE
# include <unistd.h>
#endif
#ifdef HAVE_RPC_RPC_H
# include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_KEY_PROT_H
# include <rpc/key_prot.h>
#endif
#ifdef K5AUTH
# include <krb5/krb5.h>
#endif
#ifdef USE_PAM
# include <security/pam_appl.h>
#endif
#ifdef USESHADOW
# include <shadow.h>
#endif

#ifdef KRB4
#  include <krb.h>
#  ifdef AFS
#    include <kafs.h>
#  endif
#endif

#ifndef GREET_USER_STATIC
# include <dlfcn.h>
# ifndef RTLD_NOW
#  define RTLD_NOW 1
# endif
#endif

#ifdef CSRG_BASED
# include <sys/param.h>
# ifdef HAS_SETUSERCONTEXT
#  include <login_cap.h>
#  include <pwd.h>
# endif
#endif

#ifdef HAVE_LOGIN_CAP_H		/* maybe, HAS_SETUSERCONTEXT fits better */
# include <login_cap.h>		/* BSDI-like login classes */
#endif

#if defined(CSRG_BASED) || defined(__osf__)
# include <sys/types.h>
# include <grp.h>
#else
/* should be in <grp.h> */
extern	void	setgrent(void);
extern	struct group	*getgrent(void);
extern	void	endgrent(void);
#endif

#if defined(CSRG_BASED)
# include <pwd.h>
# include <unistd.h>
#else
extern	struct passwd	*getpwnam(GETPWNAM_ARGS);
# ifdef linux
extern  void	endpwent(void);
# endif
extern	char	*crypt(CRYPT_ARGS);
#endif

Jmp_buf jb;

int     PingServer(struct display *d, Display *alternateDpy) {return 1;}
void    SessionPingFailed(struct display *d) {exit (3);}
void    RegisterCloseOnFork(int fd) {}
void    SecureDisplay(struct display *d, Display *dpy) {}
void    UnsecureDisplay(struct display *d, Display *dpy) {}
void    ClearCloseOnFork(int fd) {}
void    SetupDisplay(struct display *d) {}
void    SessionExit(struct display *d, int status, int removeAuth) { Longjmp(jb, status);} 
void    DeleteXloginResources(struct display *d, Display *dpy) {}
int     source(char **environ, char *file) {return 0;}
char    **defaultEnv(void) {return 0;}
char    **setEnv(char **e, char *name, char *value) {return 0;}
char    **putEnv(const char *string, char **env) {return 0;}
char    **parseArgs(char **argv, char *string) {return 0;}
void    printEnv(char **e) {}
char    **systemEnv(struct display *d, char *user, char *home) {return 0;}
#ifdef USE_PAM
pam_handle_t    **thepamh(void) {static pam_handle_t *pamh = 0; return &pamh;}
#endif

int debugLevel = 1;
char *errorLogFile = "";

char prog[16];

#undef HAVE_VSYSLOG
#undef HAVE_SYSLOG_H
#include "error.c"

int autoLogin = 0;
#define addressEqual(a1, l1, a2, l2) 1
#include "dpylist.c"

/* duplicate src */
int
StrDup (char **dst, const char *src)
{
    if (src) {
	int len = strlen (src) + 1;
	if (!(*dst = malloc ((unsigned) len)))
	    return 0;
	memcpy (*dst, src, len);
    } else
	*dst = NULL;
    return 1;
}

/* append src to dst */
int
StrApp(char **dst, const char *src)
{
    int olen, len;
    char *bk;

    if (*dst) {
	if (!src)
	    return 1;
	len = strlen(src) + 1;
	olen = strlen(*dst);
	if (!(bk = malloc (olen + len)))
	    return 0;
	memcpy(bk, *dst, olen);
	memcpy(bk + olen, src, len);
	free(*dst);
	*dst = bk;
	return 1;
    } else
	return StrDup (dst, src);
}

static	struct dlfuncs	dlfuncs = {
	PingServer,
	SessionPingFailed,
	Debug,
	RegisterCloseOnFork,
	SecureDisplay,
	UnsecureDisplay,
	ClearCloseOnFork,
	SetupDisplay,
	LogError,
	SessionExit,
	DeleteXloginResources,
	source,
	defaultEnv,
	setEnv,
	putEnv,
	parseArgs,
	printEnv,
	systemEnv,
	LogOutOfMem,
	setgrent,
	getgrent,
	endgrent,
#ifdef USESHADOW
	getspnam,
#ifndef QNX4
	endspent,
#endif /* QNX4 doesn't use endspent */
#endif
	getpwnam,
#ifdef linux
	endpwent,
#endif
	crypt,
#ifdef USE_PAM
	thepamh,
#endif
	};

#define AgreeterLib	(argc > 1 && argv[1][0] != '\0' ? argv[1] : "libKdmGreet.so")
#define Aname		(argc > 2 ? argv[2] : ":0")

static struct greet_info	greet;
static struct verify_info	verify;

int main(int argc, char **argv)
{
    struct display *d;
    Display	*dpy;
    static GreetUserProc greet_user_proc = NULL;
    void	*greet_lib_handle;
    int rt;
    char *pptr;

    strncpy(prog, (pptr = strrchr(argv[0], '/')) ? pptr + 1 : argv[0], 
	    sizeof(prog) - 1);

    if (!(d = NewDisplay (Aname, 0)))
	return 1;

    greet_lib_handle = dlopen(AgreeterLib, RTLD_NOW);
    if (greet_lib_handle != NULL)
	greet_user_proc = (GreetUserProc)dlsym(greet_lib_handle, "GreetUser");
    if (greet_user_proc == NULL)
    {
	LogError("%s while loading %s\n", dlerror(), AgreeterLib);
	return 2;
    }

    verify.version = 1;
    greet.version = 1;

    if ((rt = Setjmp(jb)) != 0)
	Debug("Greeter aborted: %d\n", rt);
    else {
	int rslt = (*greet_user_proc)(d, &dpy, &verify, &greet, &dlfuncs);
	Debug("Greeter exited: %d\n", rslt);
    }

#if 0
    dlclose(greet_lib_handle);
    Debug("Greeter unloaded\n");
#endif

    return 0;
}
