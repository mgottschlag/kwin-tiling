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

/* Define to 1 if `ut_host' is member of `struct utmp'. */
#cmakedefine HAVE_STRUCT_UTMP_UT_HOST 1

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

/* Define if the system has the updwtmp function */
#cmakedefine HAVE_UPDWTMP 1

/* Define if the system uses extended utmp */
#cmakedefine HAVE_UTMPX 1

/* Define if the system uses extended lastlog */
#cmakedefine HAVE_LASTLOGX 1

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

/* Define to 1 if you have the `getifaddrs' function. */
#cmakedefine HAVE_GETIFADDRS 1

/* Define to 1 if you have the `getloadavg' function. */
#cmakedefine HAVE_GETLOADAVG 1

/* Define to 1 if you have the `setproctitle' function. */
#cmakedefine HAVE_SETPROCTITLE 1

/* Define to 1 if you have the `strnlen' function. */
#cmakedefine HAVE_STRNLEN 1

/* Define to 1 if `sin6_len' is member of `struct sockaddr_in6'. */
#cmakedefine HAVE_STRUCT_SOCKADDR_IN6_SIN6_LEN

/* Define to 1 if `sin_len' is member of `struct sockaddr_in'. */
#cmakedefine HAVE_STRUCT_SOCKADDR_IN_SIN_LEN

/* Define if you have getdomainname */
#cmakedefine HAVE_GETDOMAINNAME 1

/* Define to 1 if you have the <termio.h> header file. */
#cmakedefine HAVE_TERMIO_H 1

/* Define to 1 if you have the <termios.h> header file. */
#cmakedefine HAVE_TERMIOS_H 1

/* $PATH defaults set by KDM */
#cmakedefine KDM_DEF_USER_PATH "${KDM_DEF_USER_PATH}"
#cmakedefine KDM_DEF_SYSTEM_PATH "${KDM_DEF_SYSTEM_PATH}"

/* Parameters to get from bindir to libexecdir */
#cmakedefine KDM_LIBEXEC_STRIP ${KDM_LIBEXEC_STRIP}
#cmakedefine KDM_LIBEXEC_SUFFIX "${KDM_LIBEXEC_SUFFIX}/"
