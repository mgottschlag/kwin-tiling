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

#ifndef _MISCFUNC_H
#define _MISCFUNC_H

#ifdef __cplusplus
extern "C" {
#endif

/* in verify.c */
typedef enum verifyRet { V_ERROR, V_FAIL, V_NOHOME, V_NOLOGIN, V_NOROOT, 
	V_BADSHELL, V_BADTIME, V_AEXPIRED, V_PEXPIRED, 
	V_OK, V_AWEXPIRE, V_PWEXPIRE } VerifyRet;
VerifyRet Verify (const char *name, const char *password);
VerifyRet Restrict (const char *name, int *expire, char **nologin);
VerifyRet PrepSession (const char *name, const char *password);

/* in miscfunc.c */
#if defined( HAVE_INITGROUPS) && defined( HAVE_GETGROUPS) && defined( HAVE_SETGROUPS)
# define USE_RDWR_WM
# define WMRC ".wmrc"
int rdwr_wm (char *wm, int wml, const char *usr, int rd);
#endif

#define F_LEN 50	/* user, password, session string len */
extern char name[F_LEN], password[F_LEN], sessarg[F_LEN];

#define ex_login	0
#define ex_choose	1
#define ex_console	2
#define ex_reserver	3
#define ex_unmanage	4

void init_greet();
int s_copy (char *, const char *, int, int);
VerifyRet MyVerify (const char *name, const char *password, int *expire, char **nologin);
int TempUngrab_Run(int (*func)(size_t), size_t arg);

#ifdef __cplusplus
}
#endif

#endif /* _MISCFUNC_H */
