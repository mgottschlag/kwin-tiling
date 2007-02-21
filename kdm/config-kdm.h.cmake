/* Define if your system needs _NSGetEnviron to set up the environment */
#cmakedefine HAVE_NSGETENVIRON 1
/* Define to 1 if you have the <crt_externs.h> header file - needed for "environ". */
#cmakedefine HAVE_CRT_EXTERNS_H 1
/* environ - needed by kdm */
#if defined(HAVE_NSGETENVIRON) && defined(HAVE_CRT_EXTERNS_H)
# include <sys/time.h>
# include <crt_externs.h>
# define environ (*_NSGetEnviron())
#endif

/* Define to 1 if you have the `auth_timeok' function. */
#cmakedefine HAVE_AUTH_TIMEOK 1

/* Define to 1 if you have the `getusershell' function. */
#cmakedefine HAVE_GETUSERSHELL 1

/* Define to 1 if you have the `initgroups' function. */
#cmakedefine HAVE_INITGROUPS 1

/* Define to 1 if you have the <lastlog.h> header file. */
#cmakedefine HAVE_LASTLOG_H 1

/* Define to 1 if you have the `login_getclass' function. */
#cmakedefine HAVE_LOGIN_GETCLASS 1

/* Define to 1 if you have the `mkstemp' function. */
#cmakedefine HAVE_MKSTEMP 1

/* Define to 1 if you have the `pam_getenvlist' function. */
#cmakedefine HAVE_PAM_GETENVLIST 1

/* Define to 1 if you have the `seteuid' function. */
#cmakedefine HAVE_SETEUID 1
#if !defined(HAVE_SETEUID)
#define seteuid(_eu) setresuid(-1, _eu, -1)
#endif

/* Define to 1 if you have the `setlogin' function. */
#cmakedefine HAVE_SETLOGIN 1

/* Define to 1 if you have the `setusercontext' function. */
#cmakedefine HAVE_SETUSERCONTEXT 1

/* Define to 1 if `pw_expire' is member of `struct passwd'. */
#cmakedefine HAVE_STRUCT_PASSWD_PW_EXPIRE 1

/* Define to 1 if `ut_user' is member of `struct utmp'. */
#cmakedefine HAVE_STRUCT_UTMP_UT_USER 1

/* Define to 1 if you have the `sysinfo' function. */
#cmakedefine HAVE_SYSINFO 1

/* Define to 1 if you have the `arc4random' function. */
#cmakedefine HAVE_ARC4RANDOM 1

/* Define the system's entropy device */
#cmakedefine DEV_RANDOM ${DEV_RANDOM}

/* Define if the system uses a BSD-style init */
#cmakedefine BSD_INIT 1

/* Define if the system has no getutent */
#cmakedefine BSD_UTMP 1

/* Define if the system uses extended utmp */
#cmakedefine HAVE_UTMPX 1

/* Define if kdm should use Sun's secure RPC for Xauth cookies. */
#cmakedefine SECURE_RPC 1

/* Define if kdm should use Kerberos 5 for Xauth cookies. */
#cmakedefine K5AUTH 1

/* Define if kdm should use Kerberos IV */
#cmakedefine KERBEROS 1

/* Define if kdm should not use AFS */
#cmakedefine AFS 1

/* Define if kdm should be built with XDMCP support */
#cmakedefine XDMCP 1

/* Define if kdm should be built with XDMAUTH support */
#cmakedefine HASXDMAUTH 1

/* Build kdm with built-in xconsole */
#cmakedefine WITH_KDM_XCONSOLE 1

/* Define if you have the XKB extension */
#cmakedefine HAVE_XKB 1

/* Define if you have XkbSetPerClientControls */
#cmakedefine HAVE_XKBSETPERCLIENTCONTROLS 1

