    /*

    Miscellany functions for kdm's greeter
    $Id$

    Copyright (C) 2001 Oswald Buddenhagen <ossi@kde.org>


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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    */

#include "dm.h"
#include "greet.h"

#include "miscfunc.h"

#ifdef USE_RDWR_WM

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <grp.h>

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

int
rdwr_wm (char *wm, int wml, const char *usr, int rd)
{
    int rv;
    FILE *file;
    char fname[256];
    struct passwd *pwd;

    /* read passwd */
    if (!usr || !usr[0])
	return -2;
    pwd = getpwnam( usr );
    endpwent();
    if (!pwd)
	return -2;

    /* Go user */
    rv = -1;
    setgroups( 0, 0);	/* take away root group - forever! */
    if( !setegid(pwd->pw_gid) ) {
	if( !seteuid(pwd->pw_uid) ) {

	    /* open file as user which is loging in */
	    sprintf(fname, "%s/" WMRC, pwd->pw_dir);
	    if (rd) {
		if ( (file = fopen(fname, "r")) != NULL ) {
		    fgets (wm, wml, file);
		    rv = strlen (wm);
		    if (rv && wm[rv - 1] == '\n')
			wm[--rv] = '\0';
		    fclose (file);
		}
	    } else {
		if ( (file = fopen(fname, "w")) != NULL ) {
		    fputs (wm, file);
		    rv = 1;
		    fclose (file);
		}
	    }

	    seteuid(0);
	}
	setegid(0);
    }

    return rv;
}

#endif /* USE_RDWR_WM */


#ifdef GREET_LIB
/*
 * Function pointers filled in by the initial call into the library
 */
int	(*__xdm_PingServer)(struct display *d, Display *alternateDpy) = NULL;
void	(*__xdm_SessionPingFailed)(struct display *d) = NULL;
void	(*__xdm_Debug)(const char *fmt, ...) = NULL;
void	(*__xdm_RegisterCloseOnFork)(int fd) = NULL;
void	(*__xdm_SecureDisplay)(struct display *d, Display *dpy) = NULL;
void	(*__xdm_UnsecureDisplay)(struct display *d, Display *dpy) = NULL;
void	(*__xdm_ClearCloseOnFork)(int fd) = NULL;
void	(*__xdm_SetupDisplay)(struct display *d) = NULL;
void	(*__xdm_LogError)(const char *fmt, ...) = NULL;
void	(*__xdm_SessionExit)(struct display *d, int status, int removeAuth) = NULL;
void	(*__xdm_DeleteXloginResources)(struct display *d, Display *dpy) = NULL;
int	(*__xdm_source)(char **environ, char *file) = NULL;
char	**(*__xdm_defaultEnv)(void) = NULL;
char	**(*__xdm_setEnv)(char **e, char *name, char *value) = NULL;
char	**(*__xdm_putEnv)(const char *string, char **env) = NULL;
char	**(*__xdm_parseArgs)(char **argv, char *string) = NULL;
void	(*__xdm_printEnv)(char **e) = NULL;
char	**(*__xdm_systemEnv)(struct display *d, char *user, char *home) = NULL;
void	(*__xdm_LogOutOfMem)(const char *fkt) = NULL;
SETGRENT_TYPE	(*__xdm_setgrent)(void) = NULL;
struct group	*(*__xdm_getgrent)(void) = NULL;
void	(*__xdm_endgrent)(void) = NULL;
#if !defined(USE_PAM) && !defined(AIXV3) && defined(USESHADOW)
struct spwd	*(*__xdm_getspnam)(GETSPNAM_ARGS) = NULL;
void	(*__xdm_endspent)(void) = NULL;
#endif
struct passwd	*(*__xdm_getpwnam)(GETPWNAM_ARGS) = NULL;
#ifdef __linux__
void	(*__xdm_endpwent)(void) = NULL;
#endif
char	*(*__xdm_crypt)(CRYPT_ARGS) = NULL;
#ifdef USE_PAM
pam_handle_t	**(*__xdm_thepamh)(void) = NULL;
#endif
#endif


struct display		*d;
struct verify_info	*verify;
struct greet_info	*greet;
Display			**dpy;
int			dpy_is_local;

char name[F_LEN], password[F_LEN], sessarg[F_LEN];

void
init_greet()
{
    greet->name = name;
    greet->password = password;
    greet->string = sessarg;
}

int s_copy (char *dst, const char *src, int idx, int spc)
{
    int dp = 0;

    while (src[idx] == ' ')
	idx++;
    for (; src[idx] >= ' ' && (spc || src[idx] != ' '); idx++)
	if (dp < F_LEN - 1)
	    dst[dp++] = src[idx];
    dst[dp] = '\0';
    return idx;
}

int AccNoPass (const char *un)
{
    int len = strlen(un);
    char *fp = d->noPassUsers;

    /* XXX user selected for auto-login can login without password, too */
    if (!strcmp(un, d->autoUser))
	return 1;

    while ((fp = strstr(fp, un))) {
	if ((fp == d->noPassUsers || *(fp - 1) == ',') && 
	    (!*(fp + len) || *(fp + len) == ','))
	    return 1;
	fp++;
    }

    return 0;
}

VerifyRet
MyVerify (const char *name, const char *password, int *expire, char **nologin)
{
    int ret;

    if (password && password[0] == '\0' && AccNoPass (name))
	password = 0;
    else if ((ret = Verify (name, password)) != V_OK)
	return ret;
    return Restrict (name, expire, nologin);
}

int
AutoLogon ()
{
    init_greet();

    if (!d->autoLogin)
	return 0;
    if (d->hstent->nLogPipe) {
	int cp;
	if (d->hstent->nLogPipe[0] == '\n')
	    return 0;
	cp = s_copy(sessarg, d->hstent->nLogPipe, 0, 0);
	cp = s_copy(name, d->hstent->nLogPipe, cp, 0);
	s_copy(password, d->hstent->nLogPipe, cp, 1);
	return MyVerify (name, password, 0, 0) >= V_OK;
    } else {
	if (d->autoUser[0] != '\0') {
	    if (d->hstent->lastExit > time(0) - d->openDelay) {
		if (d->hstent->goodExit)
		    return 0;
	    } else {
	        if (!d->autoLogin1st)
		    return 0;
	    }
	    greet->name = d->autoUser;
	    if (d->autoPass[0] == '\0')
		greet->password = 0;
	    else {
		strncpy(password, d->autoPass, F_LEN - 1);
		Debug("Password set in auto-login\n");
	    }
	    if (strlen (d->autoString))
		greet->string = d->autoString;
	    else
#ifdef USE_RDWR_WM
		 if (!rdwr_wm (sessarg, F_LEN, name, 1))
#endif
		greet->string = (char *)"default";
	    return 1;
	} else
	    return 0;
    }
}


// we have to return RESERVER_DISPLAY to restart the server
int
IOErrorHandler (Display *dpy)
{
    exit (RESERVER_AL_DISPLAY);
    /* NOTREACHED */
}


int
TempUngrab_Run(int (*func)(size_t), size_t arg)
{
    int ret;

    XUngrabKeyboard(*dpy, CurrentTime);
    ret = func(arg);

    // Secure the keyboard again
    if (XGrabKeyboard (*dpy, DefaultRootWindow (*dpy), 
		       True, GrabModeAsync, GrabModeAsync, CurrentTime) 
	!= GrabSuccess
    ) {
	LogError ("WARNING: keyboard on display %s could not be secured\n",
		  d->name);
	SessionExit (d, RESERVER_DISPLAY, FALSE);
    }

    return ret;
}


/* in kgreeter.cpp */
int kg_main(const char *dname);

greet_user_rtn 
GreetUser(
    struct display	*d2,
    Display		**dpy2,
    struct verify_info	*verify2,
    struct greet_info	*greet2,
    struct dlfuncs	*
#ifdef GREET_LIB
			 dlfuncs
#endif
	 )
{
    int retval;

#ifdef GREET_LIB
/*
 * These must be set before they are used.
 */
    __xdm_PingServer = dlfuncs->_PingServer;
    __xdm_SessionPingFailed = dlfuncs->_SessionPingFailed;
    __xdm_Debug = dlfuncs->_Debug;
    __xdm_RegisterCloseOnFork = dlfuncs->_RegisterCloseOnFork;
    __xdm_SecureDisplay = dlfuncs->_SecureDisplay;
    __xdm_UnsecureDisplay = dlfuncs->_UnsecureDisplay;
    __xdm_ClearCloseOnFork = dlfuncs->_ClearCloseOnFork;
    __xdm_SetupDisplay = dlfuncs->_SetupDisplay;
    __xdm_LogError = dlfuncs->_LogError;
    __xdm_SessionExit = dlfuncs->_SessionExit;
    __xdm_DeleteXloginResources = dlfuncs->_DeleteXloginResources;
    __xdm_source = dlfuncs->_source;
    __xdm_defaultEnv = dlfuncs->_defaultEnv;
    __xdm_setEnv = dlfuncs->_setEnv;
    __xdm_putEnv = dlfuncs->_putEnv;
    __xdm_parseArgs = dlfuncs->_parseArgs;
    __xdm_printEnv = dlfuncs->_printEnv;
    __xdm_systemEnv = dlfuncs->_systemEnv;
    __xdm_LogOutOfMem = dlfuncs->_LogOutOfMem;
    __xdm_setgrent = dlfuncs->_setgrent;
    __xdm_getgrent = dlfuncs->_getgrent;
    __xdm_endgrent = dlfuncs->_endgrent;
#if !defined(USE_PAM) && !defined(AIXV3) && defined(USESHADOW)
    __xdm_getspnam = dlfuncs->_getspnam;
    __xdm_endspent = dlfuncs->_endspent;
#endif
    __xdm_getpwnam = dlfuncs->_getpwnam;
#ifdef __linux__
    __xdm_endpwent = dlfuncs->_endpwent;
#endif
    __xdm_crypt = dlfuncs->_crypt;
#ifdef USE_PAM
    __xdm_thepamh = dlfuncs->_thepamh;
#endif
#endif

    d = d2;
    dpy = dpy2;
    verify = verify2;
    greet = greet2;

    dpy_is_local = d->displayType.location == Local;

    if (!AutoLogon()) {
	if (!(*dpy = XOpenDisplay(d->name))) {
	    LogError("Cannot open display %s\n", d->name);
	    SessionExit(d, RESERVER_DISPLAY, FALSE);
	}

	SecureDisplay (d, *dpy);

	/*
	 * Run the setup script - note this usually will not work when
	 * the server is grabbed, so we don't even bother trying.
	 */
	if (!d->grabServer)
	    SetupDisplay (d);

	retval = kg_main(d->name);

	DeleteXloginResources (d, *dpy);

	UnsecureDisplay (d, *dpy);

	XCloseDisplay(*dpy);

	switch (retval) {
	    case ex_choose:
		return Greet_RunChooser;
	    case ex_reserver:
		SessionExit(d, RESERVER_DISPLAY, FALSE);
		/*NOTREACHED*/
	    case ex_unmanage:
		SessionExit(d, UNMANAGE_DISPLAY, FALSE);
		/*NOTREACHED*/
	    case ex_console:
		SessionExit(d, ALTMODE_DISPLAY, FALSE);
		/*NOTREACHED*/
	}
    }

    if (PrepSession(greet->name, greet->password) != V_OK) {
	LogError("Cannot initialize session for %s\n", greet->name);
	SessionExit (d, OBEYSESS_DISPLAY, FALSE);
    }

    /*
     * Run system-wide initialization file
     */
    if (source (verify->systemEnviron, d->startup) != 0) {
	LogError("Cannot execute startup script %s\n", d->startup);
	SessionExit (d, OBEYSESS_DISPLAY, FALSE);
    }

    if (d->pipefd[1] >= 0) {
	if (d->autoReLogin) {
	    char buf[3 * (F_LEN + 1) + 1];
	    write (d->pipefd[1], buf, 
		sprintf (buf, "%s %s %s\n", greet->string, greet->name, 
			 greet->password ? greet->password : ""));
	} else
	    write (d->pipefd[1], "\n", 1);
    }

    return Greet_Success;
}

