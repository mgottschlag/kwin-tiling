/*

Copyright 2001-2004 Oswald Buddenhagen <ossi@kde.org>

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
 * Author:  Keith Packard, MIT X Consortium
 *
 * interface to xdm's external greeter and config reader
 */

#ifndef GREET_H
#define GREET_H

#define DEBUG_CORE	0x01
#define DEBUG_CONFIG	0x02
#define DEBUG_GREET	0x04
#define DEBUG_HLPCON	0x08
#define DEBUG_WSESS	0x10
#define DEBUG_WCONFIG	0x20
#define DEBUG_WGREET	0x40
#define DEBUG_NOSYSLOG	0x80
#define DEBUG_AUTH	0x100
#define DEBUG_NOFORK	0x200
#define DEBUG_VALGRIND	0x400
#define DEBUG_STRACE	0x800

#ifndef TRUE
# define TRUE	1
# define FALSE	0
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

#define as(ar) ((int)(sizeof(ar)/sizeof(ar[0])))

#define __stringify(x) #x
#define stringify(x) __stringify(x)

/*
 * Exit codes for fork()ed session process, greeter, and config reader
 */
#define EX_NORMAL		30	/* do whatever seems appropriate */
#define EX_REMANAGE_DPY		31	/* force remanage; same as EX_NORMAL, but cannot return to reserve mode immediately */
#define EX_UNMANAGE_DPY		32	/* force deletion */
#define EX_RESERVER_DPY		33	/* force server termination */
#define EX_AL_RESERVER_DPY	34	/* reserver; maybe, auto-(re-)login */
#define EX_OPENFAILED_DPY	35	/* XOpenDisplay failed, retry */
#define EX_TEXTLOGIN		36	/* start console login */
#define EX_RESERVE		37	/* put in reserve mode */
#define EX_REMOTE		38	/* start -query-ing X-server */
#define EX_MAX EX_REMOTE

/*
 * Command codes core -> greeter
 */
#define G_Greet		1	/* get login; bidi */
#define G_ErrorGreet	2	/* print failed auto-login */
#define G_Choose	3	/* run chooser; bidi */
# define G_Ch_AddHost		301
# define G_Ch_ChangeHost	302
# define G_Ch_RemoveHost	303
# define G_Ch_BadHost		304
# define G_Ch_Exit		305
#define G_SessMan	4	/* start "session manager" */

#define G_Ch_Refresh		10	/* XXX change */
#define G_Ch_RegisterHost	11	/* str name  XXX change */
#define G_Ch_DirectChoice	12	/* str name  XXX change */

/*
 * Status/command codes greeter -> core
 */
#define G_Ready		0	/* nop */
#define G_Cancel	1	/* abort login, etc. */

#define G_DGreet	2	/* get login */
#define G_DChoose	3	/* run chooser */

#define G_Shutdown	101	/* int how; int when; async */
# define SHUT_REBOOT	1	/* how */
# define SHUT_HALT	2
# define SHUT_SCHEDULE	0	/* when */
# define SHUT_TRYNOW	1
# define SHUT_FORCENOW	2
# define SHUT_INTERACT	10
#define G_SessionExit	102	/* int code; async */
#define G_GetCfg	103	/* int what; int sts, <variable>  */
#define G_SetupDpy	104	/* ; int <syncer> */
#define G_ReadDmrc	105	/* str user; int sts - curdmrc */
#define G_GetDmrc	106	/* str key; str value - curdmrc */
/*#define G_ResetDmrc	107*/	/* ; async - newdmrc */
#define G_PutDmrc	108	/* str key, str value; async - newdmrc */
#define G_Verify	109	/* str type; ..., int V_ret */
#define G_VerifyRootOK	110	/* str type; ..., int V_ret */

/*
 * Command codes core -> config reader
 */
#define GC_Files	1	/* get file list */
#define GC_GetConf	2	/* get a config group */
# define GC_gGlobal	1	/* get global config array */
# define GC_gXservers	2	/* get Xservers equivalent */
# define GC_gXaccess	3	/* get Xaccess equivalent */
# define GC_gDisplay	4	/* get per-display config array */

/*
 * Error code core -> greeter
 */
#define GE_Ok		0
#define GE_NoFkt	1	/* no such function (only for extensions!) */
#define GE_Error	2	/* internal error, like OOM */
/* for config reading */
#define GE_NoEnt	10	/* no such config entry */
#define GE_BadType	11	/* unknown config entry type */
/* for dmrc reading */
#define GE_NoUser	20	/* no such user */
#define GE_NoFile	21	/* no such file */
#define GE_Denied	22	/* permission denied */

/*
 * Log levels.
 * Used independently in core, greeter & config reader.
 */
#define DM_DEBUG	0
#define DM_INFO		1
#define DM_ERR		2
#define DM_PANIC	3

/*
 * Status codes from Verify
 */
/* terminal status codes */
#define V_OK		0
#define V_FAIL		10	/* whatever, already reported with V_MSG_* */
#define V_AUTH		11	/* authentication failed */
/* non-terminal status codes */
#define V_MSG_INFO	110	/* info message attached */
#define V_MSG_ERR	111	/* error message attached (null for generic) */
#define V_PUT_USER	112	/* user name attached; only with pam & no user send */
#define V_CHTOK 	113	/* password expired; change now */
#define V_CHTOK_AUTH	114	/* password expired; change now, but authenticate first */
#define V_PRE_OK	115	/* authentication succeeded, continue with password change */
/* queries */
#define V_GET_TEXT	200	/* str prompt, int echo, int ndelay; str return, int tag */
# define V_IS_SECRET		1
# define V_IS_USER		2
# define V_IS_PASSWORD		4
# define V_IS_OLDPASSWORD	8
# define V_IS_NEWPASSWORD	16
#define V_GET_BINARY	201	/* array prompt, int ndelay; array return */

/*
 * Config/Runtime data keys
 */
#define C_WHO_MASK	  0x00ff0000	/* Non-zero for proprietary extensions (see manufacturer table [to be written]) */
#define C_TYPE_MASK	  0x0f000000	/* Type of the value */
# define C_TYPE_INT	  0x00000000	/*  Integer */
# define C_TYPE_STR	  0x01000000	/*  String */
# define C_TYPE_ARGV	  0x02000000	/*  0-terminated Array of Strings */
# define C_TYPE_ARR	  0x03000000	/*  Array (only when XDCMP is enabled) */
#define C_PRIVATE	  0xf0000000	/* Private, don't make it visible to interfaces! */

/* global config */

#define C_autoRescan		(C_TYPE_INT | 0x001)

#define C_PAMService		(C_TYPE_STR | 0x002)

#define C_dmrcDir		(C_TYPE_STR | 0x003)

#define C_dataDir		(C_TYPE_STR | 0x004)

#define C_randomFile		(C_TYPE_STR | 0x005)
#define C_randomDevice		(C_TYPE_STR | 0x006)
#define C_prngdSocket		(C_TYPE_STR | 0x007)
#define C_prngdPort		(C_TYPE_INT | 0x008)

#define C_exportList		(C_TYPE_ARGV | 0x009)

#define C_cmdHalt		(C_TYPE_STR | 0x00a)
#define C_cmdReboot		(C_TYPE_STR | 0x00b)

#define C_pidFile		(C_TYPE_STR | 0x018)
#define C_lockPidFile		(C_TYPE_INT | 0x019)

#define C_authDir		(C_TYPE_STR | 0x01a)

#define C_requestPort		(C_TYPE_INT | 0x01b)
#define C_sourceAddress		(C_TYPE_INT | 0x01c)
#define C_removeDomainname	(C_TYPE_INT | 0x01d)
#define C_choiceTimeout		(C_TYPE_INT | 0x01e)
#define C_keyFile		(C_TYPE_STR | 0x01f)
#define C_willing		(C_TYPE_STR | 0x020)

#define C_fifoDir		(C_TYPE_STR | 0x028)
#define C_fifoGroup		(C_TYPE_INT | 0x029)	
#define C_fifoAllowShutdown	(C_TYPE_INT | 0x02a)
#define C_fifoAllowNuke		(C_TYPE_INT | 0x02b)


/* per-display config */

#define C_serverAttempts	(C_TYPE_INT | 0x100)
#define C_serverTimeout		(C_TYPE_INT | 0x101)
#define C_openDelay		(C_TYPE_INT | 0x102)
#define C_openRepeat		(C_TYPE_INT | 0x103)
#define C_openTimeout		(C_TYPE_INT | 0x104)
#define C_startAttempts		(C_TYPE_INT | 0x105)
#define C_pingInterval		(C_TYPE_INT | 0x106)
#define C_pingTimeout		(C_TYPE_INT | 0x107)	
#define C_terminateServer	(C_TYPE_INT | 0x108)
#define C_resetSignal		(C_TYPE_INT | 0x10b)	
#define C_termSignal		(C_TYPE_INT | 0x10c)	
#define C_resetForAuth		(C_TYPE_INT | 0x10d)
#define C_authorize		(C_TYPE_INT | 0x10e)
#define C_authNames		(C_TYPE_ARGV | 0x110)
#define C_clientAuthFile	(C_TYPE_STR | 0x111)
#define C_resources		(C_TYPE_STR | 0x116)
#define C_xrdb			(C_TYPE_STR | 0x117)
#define C_setup			(C_TYPE_STR | 0x118)
#define C_startup		(C_TYPE_STR | 0x119)
#define C_reset			(C_TYPE_STR | 0x11a)
#define C_session		(C_TYPE_STR | 0x11b)
#define C_userPath		(C_TYPE_STR | 0x11c)
#define C_systemPath		(C_TYPE_STR | 0x11d)
#define C_systemShell		(C_TYPE_STR | 0x11e)
#define C_failsafeClient	(C_TYPE_STR | 0x11f)
#define C_userAuthDir		(C_TYPE_STR | 0x120)
#define C_noPassUsers		(C_TYPE_ARGV | 0x123)
#define C_autoUser		(C_TYPE_STR | 0x124)
#define C_autoPass		(C_TYPE_STR | 0x125)
#define C_autoReLogin		(C_TYPE_INT | 0x128)
#define C_allowNullPasswd	(C_TYPE_INT | 0x129)
#define C_allowRootLogin	(C_TYPE_INT | 0x12a)
#define C_sessSaveFile		(C_TYPE_STR | 0x12b)
#define C_allowShutdown		(C_TYPE_INT | 0x12c)
# define SHUT_NONE	0
# define SHUT_ROOT	1
# define SHUT_ALL	2
#define C_allowNuke		(C_TYPE_INT | 0x12d)	/* see previous */
#define C_defSdMode		(C_TYPE_INT | 0x12e)	/* see G_Shutdown, but no SHUT_INTERACT */
#define C_interactiveSd		(C_TYPE_INT | 0x12f)
#define C_chooserHosts		(C_TYPE_ARGV | 0x130)
#define C_loginMode		(C_TYPE_INT | 0x131)
# define LOGIN_LOCAL_ONLY	0
# define LOGIN_DEFAULT_LOCAL	1
# define LOGIN_DEFAULT_REMOTE	2
# define LOGIN_REMOTE_ONLY	3
#define C_sessionsDirs		(C_TYPE_ARGV | 0x132)
#define C_clientLogFile		(C_TYPE_STR | 0x133)

/* display variables */
#define C_name			(C_TYPE_STR | 0x200)
#define C_class			(C_TYPE_STR | 0x201)
#define C_displayType		(C_TYPE_INT | 0x202)
#define C_serverArgv		(C_TYPE_ARGV | 0x203)
#define C_serverPid		(C_TYPE_INT | 0x204)
#define C_sessionID		(C_TYPE_INT | 0x205)
#define C_peer			(C_TYPE_ARR | 0x206)
#define C_from			(C_TYPE_ARR | 0x207)
#define C_displayNumber		(C_TYPE_INT | 0x208)
#define C_useChooser		(C_TYPE_INT | 0x209)
#define C_clientAddr		(C_TYPE_ARR | 0x20a)
#define C_connectionType	(C_TYPE_INT | 0x20b)
#define C_console		(C_TYPE_STR | 0x20c)

/**
 ** for struct display
 **/

#define d_location	1
#define dLocal		1	/* server runs on local host */
#define dForeign	0	/* server runs on remote host */

#define d_lifetime	6
#define dPermanent	4	/* display restarted when session exits */
#define dReserve	2	/* display not restarted when session exits */
#define dTransient	0	/* display removed when session exits */

#define d_origin	8
#define dFromFile	8	/* started via entry in servers file */
#define dFromXDMCP	0	/* started with XDMCP */

/**
 ** for xdmcp acls
 **/

/*
 * flags in acl entries
 */
#define a_notAllowed	1	/* both direct and indirect */
#define a_notBroadcast	2	/* only direct */
#define a_useChooser	2	/* only indirect */

/*
 * type of host entries
 */
#define HOST_ALIAS	0
#define HOST_ADDRESS	1
#define HOST_PATTERN	2
#define HOST_BROADCAST	3


#endif /* GREET_H */
