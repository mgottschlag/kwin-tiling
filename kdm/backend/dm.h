/* $TOG: dm.h /main/67 1998/02/09 13:55:01 kaleb $ */
/*

Copyright 1988, 1998  The Open Group

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
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/dm.h,v 3.19 2000/06/14 00:16:14 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * dm.h
 *
 * global xdm core declarations
 */

#ifndef _DM_H_
#define _DM_H_ 1

#include "greet.h"

#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xmd.h>
#include <X11/Xauth.h>
#include <X11/Intrinsic.h>

#if defined(X_POSIX_C_SOURCE)
# define _POSIX_C_SOURCE X_POSIX_C_SOURCE
# include <setjmp.h>
# include <limits.h>
# undef _POSIX_C_SOURCE
#else
# include <setjmp.h>
# include <limits.h>
#endif

#include <time.h>
#define Time_t time_t

#include <errno.h>

#ifdef XDMCP
# if defined(__osf__)
/* someone somewhere defines QUERY under Tru64 which confuses Xdmcp.h */
#  undef QUERY
# endif
# include <X11/Xdmcp.h>
#endif

#ifdef CSRG_BASED
# include <sys/param.h>
#endif
#ifdef pegasus
# undef dirty		/* Some bozo put a macro called dirty in sys/param.h */
#endif

#ifndef X_NOT_POSIX
# ifdef _POSIX_SOURCE
#  include <sys/wait.h>
# else
#  define _POSIX_SOURCE
#  ifdef SCO325
#   include <sys/procset.h>
#   include <sys/siginfo.h>
#  endif
#  include <sys/wait.h>
#  undef _POSIX_SOURCE
# endif
# define waitCode(w)	(WIFEXITED(w) ? WEXITSTATUS(w) : 0)
# define waitSig(w)	(WIFSIGNALED(w) ? WTERMSIG(w) : 0)
# ifdef WCOREDUMP
#  define waitCore(w)	(WCOREDUMP(w))
# else
#  define waitCore(w)	0	/* not in POSIX.  so what? */
# endif
typedef int		waitType;
#else /* X_NOT_POSIX */
# ifdef SYSV
#  define waitCode(w)	(((w) >> 8) & 0x7f)
#  define waitSig(w)	((w) & 0xff)
#  define waitCore(w)	(((w) >> 15) & 0x01)
typedef int		waitType;
# else /* SYSV */
#  include <sys/wait.h>
#  define waitCode(w)	((w).w_T.w_Retcode)
#  define waitSig(w)	((w).w_T.w_Termsig)
#  define waitCore(w)	((w).w_T.w_Coredump)
typedef union wait	waitType;
# endif
#endif /* X_NOT_POSIX */

#define waitCompose(sig,core,code) ((sig) * 256 + (core) * 128 + (code))
#define waitVal(w) waitCompose(waitSig(w), waitCore(w), waitCode(w))
#define WaitCode(w) ((w) & 0x7f)
#define WaitCore(w) (((w) >> 7) & 1)
#define WaitSig(w) (((w) >> 8) & 0xff)

#ifndef FD_ZERO
typedef	struct	my_fd_set { int fds_bits[1]; } my_fd_set;
# define FD_ZERO(fdp)	bzero ((fdp), sizeof (*(fdp)))
# define FD_SET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] |=  (1 << ((f) % (sizeof (int) * 8))))
# define FD_CLR(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] &= ~(1 << ((f) % (sizeof (int) * 8))))
# define FD_ISSET(f,fdp)	((fdp)->fds_bits[(f) / (sizeof (int) * 8)] & (1 << ((f) % (sizeof (int) * 8))))
# define FD_TYPE	my_fd_set
#else
# define FD_TYPE	fd_set
#endif

#if defined(X_NOT_POSIX) || defined(__EMX__) || (defined(__NetBSD__) && defined(__sparc__))
# define Setjmp(e)	setjmp(e)
# define Longjmp(e,v)	longjmp(e,v)
# define Jmp_buf	jmp_buf
#else
# define Setjmp(e)	sigsetjmp(e,1)
# define Longjmp(e,v)	siglongjmp(e,v)
# define Jmp_buf	sigjmp_buf
#endif

#ifdef NEED_SIGNAL
# if defined(X_NOT_POSIX) && defined(SIGNALRETURNSINT)
#  define SIGVAL int
# else
#  define SIGVAL void
# endif
# if (defined(X_NOT_POSIX) && defined(SYSV)) || defined(__EMX__) || defined(ISC)
#  define SIGNALS_RESET_WHEN_CAUGHT
# endif
# ifdef X_POSIX_C_SOURCE
#  define _POSIX_C_SOURCE X_POSIX_C_SOURCE
#  include <signal.h>
#  undef _POSIX_C_SOURCE
# else
#  if defined(X_NOT_POSIX) || defined(_POSIX_SOURCE)
#   include <signal.h>
#  else
#   define _POSIX_SOURCE
#   include <signal.h>
#   undef _POSIX_SOURCE
#  endif
# endif
#endif

typedef struct GPipe {
    int wfd, rfd;
    char *who;
} GPipe;

typedef struct GTalk {
    GPipe *pipe;
    Jmp_buf errjmp;
} GTalk;

typedef struct GProc {
    GPipe pipe;
    int pid;
} GProc;

typedef enum displayStatus { notRunning = 0, running, zombie, phoenix, raiser,
			     textMode, reserve, remoteLogin } DisplayStatus;

typedef enum serverStatus { ignore = 0, awaiting, starting,
			    terminated, killed, pausing } ServerStatus;

typedef struct RcStr {
    struct RcStr	*next;
    char		*str;
    int			cnt;
} RcStr;

typedef struct CfgDep {
    RcStr *name;
    long time;
} CfgDep;

typedef struct CfgArr {
	char		*data;		/* config value array; allocated */
	int		*idx;		/* config index array; alias */
	CfgDep		dep;		/* filestamp */
	int		numCfgEnt;	/* number of config entries */
} CfgArr;

struct display {
	struct display	*next;
	struct disphist	*hstent;	/* display history entry */

	/* basic display information */
	char		*name;		/* DISPLAY name -- also referenced in hstent */
	char		*class2;	/* display class (may be NULL) */
	int		displayType;	/* location/origin/lifetime */
	CfgArr		cfg;		/* config data array */

	/* display state */
	DisplayStatus	status;		/* current status */
	int		zstatus;	/*  substatus while zombie */
	int		pid;		/* process id of child */
	int		serverPid;	/* process id of server (-1 if none) */
	ServerStatus	serverStatus;	/* X server startup state */
	Time_t		lastStart;	/* time of last display start */
	int		startTries;	/* current start try */
	int		stillThere;	/* state during HUP processing */
	int		userSess;	/* -1=nobody, otherwise uid */
	int		fifofd;		/* command fifo */
	GPipe		pipe;		/* comm master <-> slave */
	char		*remoteHost;	/* for X -query type remote login */
#ifdef XDMCP
	/* XDMCP state */
	unsigned	sessionID;	/* ID of active session */
	ARRAY8		peer;		/* display peer address */
	ARRAY8		from;		/* XDMCP port of display */
	unsigned	displayNumber;	/* numerical part of name */
	int		useChooser;	/* Run the chooser for this display */
	ARRAY8		clientAddr;	/* for chooser picking */
	unsigned	connectionType;	/* ... */
#endif

	/* server management resources */
	int		serverAttempts;	/* number of attempts at running X */
	int		serverTimeout;	/* how long to wait for X */
	int		openDelay;	/* after failed server start or XOpenDisplay */
	int		openRepeat;	/* connection open attempts to make */
	int		openTimeout;	/* XOpenDisplay timeout */
	int		startAttempts;	/* number of attempts at starting foreign from file */
	int		pingInterval;	/* interval between XSync */
	int		pingTimeout;	/* timeout for XSync */
	int		terminateServer;/* restart for each session */
	int		resetSignal;	/* signal to reset server */
	int		termSignal;	/* signal to terminate server */
	int		resetForAuth;	/* server reads auth file at reset */
	int		idleTimeout;	/* abort login after that time */
	char		**serverArgv;	/* server program and arguments */
	char		*console;	/* the tty line hidden by the server */

	/* session resources */
	char		*resources;	/* resource file */
	char		*xrdb;		/* xrdb program */
	char		*setup;		/* Xsetup program */
	char		*startup;	/* Xstartup program */
	char		*reset;		/* Xreset program */
	char		*session;	/* Xsession program */
	char		*userPath;	/* path set for session */
	char		*systemPath;	/* path set for startup/reset */
	char		*systemShell;	/* interpreter for startup/reset */
	char		*failsafeClient;/* a client to start when the session fails */
	char		*autoUser;	/* user to log in automatically. */
	char		*autoPass;	/* his password. only for krb5 & sec_rpc */
	char		**noPassUsers;	/* users allowed in without a password */
	char		**sessionsDirs;	/* where session .desktop files are located */
	char		**chooserHosts;	/* hosts to auto-add in "remote login" */
	char		*clientLogFile;	/* xsession-errors template */
	int		loginMode;	/* whether to start chooser or login */
	int		autoReLogin;	/* auto-re-login after crash */
	int		allowNullPasswd;/* allow null password on login */
	int		allowRootLogin;	/* allow direct root login */
	int		allowShutdown;	/* who is allowed to shutdown */
	int		allowNuke;	/* who is allowed to s/d even when other sessions are running */
	int		defSdMode;	/* s/d condition/timing used by default */

	/* authorization resources */
	int		authorize;	/* enable authorization */
	char		**authNames;	/* authorization protocol names */
	unsigned short	*authNameLens;	/* authorization protocol name lens */
	char		*clientAuthFile;/* client specified auth file */
	char		*userAuthDir;	/* backup directory for tickets */

	/* information potentially derived from resources */
	int		authNameNum;	/* number of protocol names */
	Xauth		**authorizations;/* authorization data */
	int		authNum;	/* number of authorizations */
	char		*authFile;	/* file to store authorization in */
	char		*fifoPath;	/* filename of the command fifo */
};

struct disphist {
	struct disphist	*next;
	char		*name;
	Time_t		lastExit;	/* time of last display exit */
	unsigned	rLogin:2,	/* 0=nothing 1=relogin 2=login */
			sd_how:2,	/* 0=none 1=reboot 2=halt (SHUT_*) */
			sd_when:2,	/* 0=maylater 1=trynow 2=forcenow (SHUT_*) */
			lock:1,		/* screen locker running */
			goodExit:1;	/* was the last exit "peaceful"? */
	char		*nuser, *npass, *nargs;
};

#ifdef XDMCP

#define PROTO_TIMEOUT	(30 * 60)   /* 30 minutes should be long enough */

struct protoDisplay {
	struct protoDisplay	*next;
	XdmcpNetaddr		address;   /* UDP address */
	int			addrlen;    /* UDP address length */
	unsigned long		date;	    /* creation date */
	CARD16			displayNumber;
	CARD16			connectionType;
	ARRAY8			connectionAddress;
	CARD32			sessionID;
	Xauth			*fileAuthorization;
	Xauth			*xdmcpAuthorization;
	ARRAY8			authenticationName;
	ARRAY8			authenticationData;
	XdmAuthKeyRec		key;
};
#endif /* XDMCP */

/* status code for RStopDisplay */
#define DS_RESTART	0
#define DS_TEXTMODE	1
#define DS_RESERVE	2
#define DS_REMOTE	3
#define DS_REMOVE	4

/* command codes dpy process -> master process */
#define D_User		1
#define D_ReLogin	2
#define D_Shutdown	3
#define D_ChooseHost	4
#define D_RemoteHost	5
#define D_XConnOk	6

extern int	request_port;
extern int	sourceAddress;
extern int	debugLevel;
extern char	*pidFile;
extern int	lockPidFile;
extern char	*authDir;
extern int	autoRescan;
extern int	removeDomainname;
extern char	*keyFile;
extern char	**exportList;
extern char	*randomFile;
extern char	*randomDevice;
extern char	*willing;
extern int	choiceTimeout;	/* chooser choice timeout */
extern char	*cmdHalt;
extern char	*cmdReboot;
extern char	*PAMService;
extern char	*fifoDir;
extern int	fifoGroup;
extern int	fifoAllowShutdown;
extern int	fifoAllowNuke;
extern char	*dmrcDir;

/* in daemon.c */
extern void BecomeDaemon (void);

/* in dm.c */
extern char *prog, *progpath;
extern void StartDisplay (struct display *d);
extern void StartDisplayP2 (struct display *d);
extern void StopDisplay (struct display *d);
extern void SetTitle (const char *name);

/* in dpylist.c */
extern struct display *displays;	/* that's ugly ... */
extern int AnyDisplaysLeft (void);
extern void ForEachDisplay (void (*f)(struct display *));
extern void RemoveDisplay (struct display *old);
extern struct display
	*FindDisplayByName (const char *name),
#ifdef XDMCP
	*FindDisplayBySessionID (CARD32 sessionID),
	*FindDisplayByAddress (XdmcpNetaddr addr, int addrlen, CARD16 displayNumber),
#endif /* XDMCP */
	*FindDisplayByPid (int pid),
	*FindDisplayByServerPid (int serverPid),
	*NewDisplay (const char *name);
extern int AnyActiveDisplays (void);
extern int AnyRunningDisplays (void);
extern int AnyReserveDisplays (void);
extern int AllLocalDisplaysLocked (struct display *dp);
extern void StartReserveDisplay (int lt);
extern void ReapReserveDisplays (void);

/* in reset.c */
extern void pseudoReset (void);

/* in resource.c */
extern char **FindCfgEnt (struct display *d, int id);
extern int InitResources (char **argv);
extern int LoadDMResources (int force);
extern int LoadDisplayResources (struct display *d);
extern void ScanServers (int force);
extern void CloseGetter (void);
extern int startConfig (int what, CfgDep *dep, int force);
extern RcStr *newStr (char *str);
extern void delStr (RcStr *str);
extern GTalk cnftalk;

/* in session.c */
extern struct display *td;
extern char **baseEnv (const char *user);
extern char **inheritEnv (char **env, const char **what);
extern char **systemEnv (const char *user);
extern int source (char **env, char *file);
extern void DeleteXloginResources (void);
extern void LoadXloginResources (void);
extern void ManageSession (struct display *d);
extern void SetupDisplay (void);

extern GTalk mstrtalk, grttalk;
extern GProc grtproc;
extern void OpenGreeter (void);
extern int CloseGreeter (int force);
extern int CtrlGreeterWait (int wreply);
extern void PrepErrorGreet (void);
extern char *conv_interact (int what, const char *prompt);

/* process.c */
#include <stdlib.h>

#ifdef NEED_SIGNAL
typedef SIGVAL (*SIGFUNC)(int);

SIGVAL (*Signal(int, SIGFUNC Handler))(int);
#endif

extern void RegisterInput (int fd);
extern void UnregisterInput (int fd);
extern void RegisterCloseOnFork (int fd);
extern void CloseNClearCloseOnFork (int fd);
extern int Fork (void);
extern int Wait4 (int pid);
extern void execute (char **argv, char **env);
extern int runAndWait (char **args, char **env);
extern void TerminateProcess (int pid, int sig);

extern void GSet (GTalk *talk);	/* call before GOpen! */
extern int GFork (GPipe *pajp, const char *pname, char *cname);
extern void GClosen (GPipe *pajp);
extern int GOpen (GProc *proc, char **argv, const char *what, char **env, char *cname);
extern int GClose (GProc *proc, int force);

extern void GSendInt (int val);
extern int GRecvInt (void);
extern int GRecvCmd (int *cmd);
extern void GSendArr (int len, const char *data);
extern char *GRecvArr (int *len);
extern int GRecvStrBuf (char *buf);
extern int GRecvArrBuf (char *buf);
extern void GSendStr (const char *buf);
extern void GSendNStr (const char *buf, int len);
extern char *GRecvStr (void);
extern void GSendArgv (char **argv);
extern void GSendStrArr (int len, char **data);
extern char **GRecvStrArr (int *len);
extern char **GRecvArgv (void);

/* client.c */
#define GCONV_NORMAL	0
#define GCONV_HIDDEN	1
#define GCONV_USER	2
#define GCONV_PASS	3
#define GCONV_PASS_ND	4
#define GCONV_BINARY	5
typedef char * (*GConvFunc) (int what, const char *prompt);
extern int Verify (GConvFunc gconv, int rootok);
extern int StartClient (void);
extern void SessionExit (int status) ATTR_NORETURN;
extern int ReadDmrc (void);
extern char **userEnviron, **systemEnviron;
extern char *curuser, *curpass, *curtype, *dmrcuser, *curdmrc, *newdmrc;

/* server.c */
extern void StartServer (struct display *d);
extern void AbortStartServer (struct display *d);
extern void StartServerSuccess (void);
extern void StartServerFailed (void);
extern void StartServerTimeout (void);
struct display *startingServer;

extern void WaitForServer (struct display *d);
extern void ResetServer (struct display *d);
extern int PingServer(struct display *d);
extern Display *dpy;

/* socket.c */
extern void CreateWellKnownSockets (void);

/* in util.c */
extern void *Calloc (size_t nmemb, size_t size);
extern void *Malloc (size_t size);
extern void *Realloc (void *ptr, size_t size);
extern void WipeStr (char *str);
extern int StrCmp (const char *s1, const char *s2);
extern int StrNDup (char **dst, const char *src, int len);
extern int StrDup (char **dst, const char *src);
extern int arrLen (char **arr);
extern void freeStrArr (char **arr);
extern char **initStrArr (char **arr);
extern char **xCopyStrArr (int rn, char **arr);
/* Note: the following functions free the old data even in case of failure */
extern int ReStrN (char **dst, const char *src, int len);
extern int ReStr (char **dst, const char *src);
extern int StrApp (char **dst, ...);
extern char **addStrArr (char **arr, const char *str, int len);
extern char **parseArgs (char **argv, const char *string);
/* End note */
extern char **setEnv (char **e, const char *name, const char *value);
extern char **putEnv (const char *string, char **env);
extern const char *getEnv (char **e, const char *name);
extern const char *localHostname (void);
extern int Reader (int fd, void *buf, int len);
extern void FdGetsCall (int fd, void (*func)(const char *, int, void *), void *ptr);

/* in inifile.c */
extern char *iniLoad (const char *fname);
extern int iniSave (const char *data, const char *fname);
extern char *iniEntry (char *data, const char *section, const char *key, const char *value);
extern char *iniMerge (char *data, const char *newdata);

#ifdef XDMCP

/* in xdmcp.c */
extern char *NetworkAddressToHostname (CARD16 connectionType, ARRAY8Ptr connectionAddress);
extern int AnyWellKnownSockets (void);
extern void DestroyWellKnownSockets (void);
extern void SendFailed (struct display *d, const char *reason);
extern void init_session_id(void);
extern void registerHostname(const char *name, int namelen);
extern int xdmcpFd;

/* in netaddr.c */
extern char *NetaddrAddress(XdmcpNetaddr netaddrp, int *lenp);
extern char *NetaddrPort(XdmcpNetaddr netaddrp, int *lenp);
extern int ConvertAddr (XdmcpNetaddr saddr, int *len, char **addr);
extern int NetaddrFamily (XdmcpNetaddr netaddrp);
extern int addressEqual (XdmcpNetaddr a1, int len1, XdmcpNetaddr a2, int len2);

/* in policy.c */
#if 0
extern ARRAY8Ptr Accept (/* struct sockaddr *from, int fromlen, CARD16 displayNumber */);
#endif
extern ARRAY8Ptr ChooseAuthentication (ARRAYofARRAY8Ptr authenticationNames);
extern int CheckAuthentication (struct protoDisplay *pdpy, ARRAY8Ptr displayID, ARRAY8Ptr name, ARRAY8Ptr data);
extern int SelectAuthorizationTypeIndex (ARRAY8Ptr authenticationName, ARRAYofARRAY8Ptr authorizationNames);
extern int SelectConnectionTypeIndex (ARRAY16Ptr connectionTypes, ARRAYofARRAY8Ptr connectionAddresses);
extern int Willing (ARRAY8Ptr addr, CARD16 connectionType, ARRAY8Ptr authenticationName, ARRAY8Ptr status, xdmOpCode type);

/* in protodpy.c */
extern void DisposeProtoDisplay(struct protoDisplay *pdpy);

extern struct protoDisplay	*FindProtoDisplay (
					XdmcpNetaddr address,
					int          addrlen,
					CARD16       displayNumber);
extern struct protoDisplay	*NewProtoDisplay (
					XdmcpNetaddr address,
					int	     addrlen,
					CARD16	     displayNumber,
					CARD16	     connectionType,
					ARRAY8Ptr    connectionAddress,
					CARD32	     sessionID);

#define FamilyBroadcast 0xffff
typedef void (*ChooserFunc)(CARD16 connectionType, ARRAY8Ptr addr, char *closure);

/* in access.c */
extern ARRAY8Ptr getLocalAddress (void);
extern int AcceptableDisplayAddress (ARRAY8Ptr clientAddress, CARD16 connectionType, xdmOpCode type);
extern int ForEachMatchingIndirectHost (ARRAY8Ptr clientAddress, CARD16 connectionType, ChooserFunc function, char *closure);
extern void ScanAccessDatabase (int force);
extern int UseChooser (ARRAY8Ptr clientAddress, CARD16 connectionType);
extern void ForEachChooserHost (ARRAY8Ptr clientAddress, CARD16 connectionType, ChooserFunc function, char *closure);

/* in choose.c */
extern ARRAY8Ptr IndirectChoice (ARRAY8Ptr clientAddress, CARD16 connectionType);
extern int IsIndirectClient (ARRAY8Ptr clientAddress, CARD16 connectionType);
extern int RememberIndirectClient (ARRAY8Ptr clientAddress, CARD16 connectionType);
extern void ForgetIndirectClient ( ARRAY8Ptr clientAddress, CARD16 connectionType);
extern int RegisterIndirectChoice (ARRAY8Ptr clientAddress, CARD16 connectionType, ARRAY8Ptr choice);
extern int DoChoose (void);

/* in xdmcp.c */
extern void ProcessRequestSocket (void);

#endif /* XDMCP */

/* support -debug 0x200 */
#define nofork_session (debugLevel & DEBUG_NOFORK)

/* automatically fork off reserve display if all displays are locked */
/*#define AUTO_RESERVE 1*/

#endif /* _DM_H_ */
