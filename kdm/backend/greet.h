/*

Copyright 2001-2005 Oswald Buddenhagen <ossi@kde.org>

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from the copyright holder.

*/

/*
 * xdm - display manager daemon
 * Author: Keith Packard, MIT X Consortium
 *
 * interface to xdm's external greeter and config reader
 */

#ifndef GREET_H
#define GREET_H

#include <config-workspace.h>
#include <config-unix.h>
#include <config-kdm.h>

#if defined(__sun) && !defined(__sun__)
# define __sun__
#endif

#ifdef HAVE_PAM
# define USE_PAM
#elif defined(HAVE_GETSPNAM)
# define USESHADOW
#endif
#ifdef HAVE_VSYSLOG
# define USE_SYSLOG
#endif

#ifdef __linux__
/* This needs to be run-time configurable, additionally. */
# define HAVE_VTS
#endif

#define DEBUG_CORE     0x01
#define DEBUG_CONFIG   0x02
#define DEBUG_GREET    0x04
#define DEBUG_HLPCON   0x08
#define DEBUG_WSESS    0x10
#define DEBUG_WCONFIG  0x20
#define DEBUG_WGREET   0x40
#define DEBUG_NOSYSLOG 0x80
#define DEBUG_AUTH     0x100
#define DEBUG_THEMING  0x200
#define DEBUG_VALGRIND 0x400
#define DEBUG_STRACE   0x800

#ifndef True
# define True  1
# define False 0
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define ATTR_UNUSED __attribute__((unused))
# define ATTR_NORETURN __attribute__((noreturn))
# define ATTR_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
# define ATTR_UNUSED
# define ATTR_NORETURN
# define ATTR_PRINTFLIKE(fmt,var)
#endif

#ifndef offsetof
# define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#define as(ar) ((int)(sizeof(ar)/sizeof(ar[0])))

#define __stringify(x) #x
#define stringify(x) __stringify(x)

/*
 * Exit codes for fork()ed session process, greeter, and config reader
 */
#define EX_NORMAL           30  /* do whatever seems appropriate */
#define EX_REMANAGE_DPY     31  /* force remanage; same as EX_NORMAL, but cannot return to reserve mode immediately */
#define EX_UNMANAGE_DPY     32  /* force deletion */
#define EX_RESERVER_DPY     33  /* force server termination */
#define EX_AL_RESERVER_DPY  34  /* reserver; maybe, auto-(re-)login */
#define EX_OPENFAILED_DPY   35  /* XOpenDisplay failed, retry */
#define EX_RESERVE          37  /* put in reserve mode */
#ifdef XDMCP
#define EX_REMOTE           38  /* start -query-ing X-server */
#define EX_MAX EX_REMOTE
#else
#define EX_MAX EX_RESERVE
#endif

/*
 * Command codes core -> greeter
 */
#define G_Greet             1   /* get login; bidi */
#define G_ErrorGreet        2   /* print failed auto-login */
#ifdef XDMCP
#define G_Choose            3   /* run chooser; bidi */
# define G_Ch_AddHost         301
# define G_Ch_ChangeHost      302
# define G_Ch_RemoveHost      303
# define G_Ch_BadHost         304
# define G_Ch_Exit            305
#endif
#define G_SessMan           4   /* start "session manager" */
#define G_ConfShutdown      5   /* confirm forced shutdown */
#define G_GreetTimed        6   /* get login; timed login permitted */

#ifdef XDMCP
#define G_Ch_Refresh       10   /* XXX change */
#define G_Ch_RegisterHost  11   /* str name      XXX change */
#define G_Ch_DirectChoice  12   /* str name      XXX change */
#endif

/*
 * Status/command codes greeter -> core
 */
#define G_Ready    0    /* nop */
#define G_Cancel   1    /* abort login, etc. */

#define G_DGreet   2    /* get login */
#ifdef XDMCP
#define G_DChoose  3    /* run chooser */
#endif

#define G_Interact 4    /* greeter got user input. possible crash is probably not spontaneous. */

#define G_Shutdown      101 /* 5*int, string; async */
# define SHUT_REBOOT      1     /* how */
# define SHUT_HALT        2
# define SHUT_CONSOLE     -1 /* pseudo-code */
# define SHUT_SCHEDULE    0     /* when; config only */
# define SHUT_TRYNOW      1
# define SHUT_FORCENOW    2
# define SHUT_CANCEL      0     /* force */
# define SHUT_FORCEMY     1
# define SHUT_FORCE       2
# define SHUT_ASK         3
# define TO_INF           0x7fffffff
#define G_SessionExit   102 /* int code; async */
#define G_GetCfg        103 /* int what; int sts, <variable>  */
#define G_SetupDpy      104 /* ; int <syncer> */
#define G_ReadDmrc      105 /* str user; int sts - curdmrc */
#define G_GetDmrc       106 /* str key; str value - curdmrc */
/*#define G_ResetDmrc   107*/ /* ; async - newdmrc */
#define G_PutDmrc       108 /* str key, str value; async - newdmrc */
#define G_Verify        109 /* str type; ..., int V_ret */
#define G_VerifyRootOK  110 /* str type; ..., int V_ret */
#define G_List          111 /* int flags; ?*(str,str,[int,]str,str,int), int 0 */
# define lstRemote        1
# define lstPassive       2
# define lstTTY           4
# define isSelf           1
# define isTTY            2
#define G_QueryShutdown 112 /* ; 5*int; string */
#define G_Activate      113 /* int vt; async */
#define G_ListBootOpts  114 /* ; int sts, [argv opts, int dflt, int cur] */
# define BO_OK      0
# define BO_NOMAN  -1
# define BO_NOENT  -2
# define BO_IO     -3
#define G_Console       116 /* ; async */
#define G_AutoLogin     117 /* ; async */
#define G_QryDpyShutdown 118 /* ; int, int, str */

/*
 * Command codes core -> config reader
 */
#define GC_Files        1       /* get file list */
#define GC_GetConf      2       /* get a config group */
# define GC_gGlobal       1     /* get global config array */
#ifdef XDMCP
# define GC_gXaccess      3     /* get Xaccess equivalent */
#endif
# define GC_gDisplay      4     /* get per-display config array */

/*
 * Error code core -> greeter
 */
#define GE_Ok       0
#define GE_NoFkt    1   /* no such function (only for extensions!) */
#define GE_Error    2   /* internal error, like OOM */
/* for config reading */
#define GE_NoEnt   10   /* no such config entry */
#define GE_BadType 11   /* unknown config entry type */
/* for dmrc reading */
#define GE_NoUser  20   /* no such user */
#define GE_NoFile  21   /* no such file */
#define GE_Denied  22   /* permission denied */

/*
 * Log levels.
 * Used independently in core, greeter & config reader.
 */
#define DM_DEBUG  0
#define DM_INFO   1
#define DM_WARN   2
#define DM_ERR    3
#define DM_PANIC  4

/*
 * Status codes from verify
 */
/* terminal status codes */
#define V_OK              0
#define V_FAIL           10     /* whatever, already reported with V_MSG_* */
#define V_AUTH           11     /* authentication failed */
/* non-terminal status codes */
#define V_MSG_INFO      110 /* info message attached; 0 return */
#define V_MSG_ERR       111 /* error message attached (null for generic); 0 return */
#define V_PUT_USER      112 /* user name attached; only with pam & no user send */
#define V_CHTOK         113 /* password expired; change now */
#define V_CHTOK_AUTH    114 /* password expired; change now, but authenticate first */
#define V_PRE_OK        115 /* authentication succeeded, continue with password change */
#define V_MSG_INFO_AUTH 116 /* info message during auth attached; 0 return */
/* queries */
#define V_GET_TEXT      200 /* str prompt, int echo, int ndelay; str return, int tag */
# define V_IS_SECRET        1
# define V_IS_USER          2
# define V_IS_PASSWORD      4
# define V_IS_OLDPASSWORD   8
# define V_IS_NEWPASSWORD  16
#define V_GET_BINARY    201 /* array prompt, int ndelay; array return */

/*
 * Config/Runtime data keys
 */
#define C_WHO_MASK    0x00ff0000        /* Non-zero for proprietary extensions (see manufacturer table [to be written]) */
#define C_TYPE_MASK   0x0f000000        /* Type of the value */
# define C_TYPE_INT      0x00000000     /*   Integer */
# define C_TYPE_STR      0x01000000     /*   String */
# define C_TYPE_ARGV     0x02000000     /*   0-terminated Array of Strings */
# define C_TYPE_ARR      0x03000000     /*   Array (only when XDCMP is enabled) */
#define C_PRIVATE     0xf0000000        /* Private, don't make it visible to interfaces! */

/* display variables */
#define C_isLocal       (C_TYPE_INT | 0x200)
#define C_hasConsole    (C_TYPE_INT | 0x201)
#define C_isAuthorized  (C_TYPE_INT | 0x202)
#define C_isReserve     (C_TYPE_INT | 0x203)

#ifdef XDMCP
/**
 ** for xdmcp acls
 **/

/*
 * flags in acl entries
 */
#define a_notAllowed    1       /* both direct and indirect */
#define a_notBroadcast  2       /* only direct */
#define a_useChooser    2       /* only indirect */

/*
 * type of host entries
 */
#define HOST_ALIAS      0
#define HOST_ADDRESS    1
#define HOST_PATTERN    2
#define HOST_BROADCAST  3

#endif

#endif /* GREET_H */
