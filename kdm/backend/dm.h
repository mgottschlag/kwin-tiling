/*

Copyright 1988, 1998  The Open Group
Copyright 2000-2005 Oswald Buddenhagen <ossi@kde.org>

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
 * global xdm core declarations
 */

#ifndef _DM_H_
#define _DM_H_ 1

#include "greet.h"

#define WANT_CORE_DECLS
#include <config.ci>

#include <X11/X.h> /* FamilyInternet6 */
#include <X11/Xos.h>
#include <X11/Xfuncs.h>
#include <X11/Xmd.h>
#include <X11/Xauth.h>

#include <sys/param.h>
#ifdef HAVE_LIMITS_H
# include <limits.h>
#endif

#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <errno.h>

extern char **environ;

#ifdef XDMCP
# if defined(__osf__)
/* someone somewhere defines QUERY under Tru64 which confuses Xdmcp.h */
#  undef QUERY
# endif
# include <X11/Xdmcp.h>
#endif

#ifndef PATH_MAX
# ifdef MAXPATHLEN
#  define PATH_MAX MAXPATHLEN
# else
#  define PATH_MAX 1024
# endif
#endif

#include <sys/wait.h>
#define waitCode(w) (WIFEXITED(w) ? WEXITSTATUS(w) : 0)
#define waitSig(w) (WIFSIGNALED(w) ? WTERMSIG(w) : 0)
#ifdef WCOREDUMP
# define waitCore(w) (WCOREDUMP(w))
#else
# define waitCore(w) 0 /* not in POSIX.  so what? */
#endif

#define wcCompose(sig,core,code) ((sig) * 256 + (core) * 128 + (code))
#define wcFromWait(w) wcCompose(waitSig(w), waitCore(w), waitCode(w))
#define wcCode(w) ((w) & 0x7f)
#define wcCore(w) (((w) >> 7) & 1)
#define wcSig(w) (((w) >> 8) & 0xff)

#include <setjmp.h>
#if defined(__EMX__) || (defined(__NetBSD__) && defined(__sparc__)) /* XXX netbsd? */
# define Setjmp(e) setjmp(e)
# define Longjmp(e,v) longjmp(e,v)
# define Jmp_buf jmp_buf
#else
# define Setjmp(e) sigsetjmp(e,1)
# define Longjmp(e,v) siglongjmp(e,v)
# define Jmp_buf sigjmp_buf
#endif

#ifdef HAVE_UTMPX
# include <utmpx.h>
# define STRUCTUTMP struct utmpx
# define UTMPNAME utmpxname
# define SETUTENT setutxent
# define GETUTENT getutxent
# define PUTUTLINE pututxline
# define ENDUTENT endutxent
# define ut_time ut_tv.tv_sec
#else
# include <utmp.h>
# define STRUCTUTMP struct utmp
# define UTMPNAME utmpname
# define SETUTENT setutent
# define GETUTENT getutent
# define PUTUTLINE pututline
# define ENDUTENT endutent
# ifndef HAVE_STRUCT_UTMP_UT_USER
#  define ut_user ut_name
# endif
#endif
#ifndef WTMP_FILE
# ifdef _PATH_WTMPX
#  define WTMP_FILE _PATH_WTMPX
# elif defined(_PATH_WTMP)
#  define WTMP_FILE _PATH_WTMP
# else
#  define WTMP_FILE "/usr/adm/wtmp"
# endif
#endif
#ifndef UTMP_FILE
# ifdef _PATH_UTMPX
#  define UTMP_FILE _PATH_UTMPX
# elif defined(_PATH_UTMP)
#  define UTMP_FILE _PATH_UTMP
# else
#  define UTMP_FILE "/etc/utmp"
# endif
#endif

#ifdef HAVE_NETCONFIG_H
# define STREAMSCONN
#else
# define UNIXCONN
# define TCPCONN
# ifdef FamilyInternet6
#  define IPv6
# endif
# ifdef HAVE_NETDNET_DN_H
#  define DNETCONN
# endif
#endif

#if !defined(HAVE_ARC4RANDOM) && !defined(DEV_RANDOM)
# define NEED_ENTROPY
#endif

typedef struct GPipe {
#if defined(__FreeBSD__) || defined(UNIXCONN)
# define SINGLE_PIPE
    union
#else
    struct
#endif
    {
        int w, r;
    } fd;
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

typedef enum displayStatus {
    notRunning = 0, /* waiting for being started */
    running,        /* was started */
    zombie,         /* manager and server killed, remove/suspend when both are gone */
    phoenix,        /* server killed, restart when it dies */
    raiser,         /* manager killed, restart when it dies */
    textMode,       /* suspended, console mode */
    reserve,        /* suspended, reserve display */
    remoteLogin     /* running X -query */
} DisplayStatus;

typedef enum serverStatus {
    ignore = 0,     /* error in this state is no error */
    awaiting,       /* waking for being started */
    starting,       /* process launched, wait max serverTimeout secs */
    terminated,     /* process SIGTERMed, wait max serverTimeout secs */
    killed,         /* process SIGKILLed, wait max 10 secs */
    pausing         /* startup failed, wait openDelay secs */
} ServerStatus;

typedef struct {
    unsigned how:2,    /* 0=none 1=reboot 2=halt (SHUT_*) */
             force:2;
    int uid;
    int start;
    int timeout;
    char *osname;
    time_t bmstamp;
    int osindex;
} SdRec;

typedef struct RcStr {
    struct RcStr *next;
    char *str;
    int cnt;
} RcStr;

typedef struct CfgDep {
    RcStr *name;
    long time;
} CfgDep;

typedef struct CfgArr {
    void *data;     /* config value array; allocated */
    int *idx;      /* config index array; alias */
    CfgDep dep;     /* filestamp */
    int numCfgEnt;  /* number of config entries */
} CfgArr;

struct bsock {
    int fd;
    int buflen;
    char *buffer;
};

struct cmdsock {
    struct cmdsock *next;
    struct bsock sock;    /* buffered fd of the socket */
};

typedef struct {
    struct cmdsock *css;  /* open connections */

    char *path;           /* filename of the socket */
#ifndef HONORS_SOCKET_PERMS
    char *realdir;        /* real dirname of the socket */
#endif
    int fd;               /* fd of the socket */
    int gid;              /* owner group of the socket */
} CtrlRec;

struct display {
    struct display *next;
    struct disphist *hstent;    /* display history entry */

    /* basic display information */
    char *name;                 /* DISPLAY name -- also referenced in hstent */
    char *class2;               /* display class (may be 0) */
    int displayType;            /* location/origin/lifetime */
    CfgArr cfg;                 /* config data array */

    /* display state */
    DisplayStatus status;       /* current status */
    int zstatus;                /* substatus while zombie */
    int pid;                    /* process id of child */
    int serverPid;              /* process id of server (-1 if none) */
#ifdef HAVE_VTS
    int serverVT;               /* server VT (0 = none, -1 = pending) */
    struct display *follower;   /* on exit, hand VT to this display */
#endif
    ServerStatus serverStatus;  /* X server startup state */
    time_t lastStart;           /* time of last display start */
    int startTries;             /* current start try */
    int stillThere;             /* state during HUP processing */
    int userSess;               /* -1=nobody, otherwise uid */
    char *userName;
    char *sessName;
    SdRec sdRec;                /* user session requested shutdown */
    CtrlRec ctrl;               /* command socket */
    GPipe pipe;                 /* comm master <-> slave */
    GPipe gpipe;                /* comm master <-> greeter */
#ifdef XDMCP
    char *remoteHost;           /* for X -query type remote login */
    /* XDMCP state */
    unsigned sessionID;         /* ID of active session */
    ARRAY8 peer;                /* display peer address (sockaddr) */
    ARRAY8 from;                /* XDMCP peer address (sockaddr) */
    unsigned displayNumber;     /* numerical part of name */
    int useChooser;             /* Run the chooser for this display */
    ARRAY8 clientAddr;          /* for chooser picking */
    ARRAY8 clientPort;          /* ... */
    unsigned connectionType;    /* ... */
    int xdmcpFd;
#endif

    CONF_CORE_LOCAL_DEFS

    /* information potentially derived from resources */
    Xauth **authorizations;     /* authorization data */
    int authNum;                /* number of authorizations */
    char *authFile;             /* file to store authorization in */
    char *greeterAuthFile;      /* file to store authorization for greeter in */
};

#define d_location   1
#define dLocal          1       /* server runs on local host */
#define dForeign        0       /* server runs on remote host */

#define d_lifetime   6
#define dPermanent      4       /* display restarted when session exits */
#define dReserve        2       /* display not restarted when session exits */
#define dTransient      0       /* display removed when session exits */

#ifdef XDMCP
#define d_origin     24
#else
#define d_origin     16 /* clever, huh? :) */
#endif
#define dFromCommand   16       /* started via command socket */
#define dFromXDMCP      8       /* started with XDMCP */
#define dFromFile       0       /* started via entry in servers file */

struct disphist {
    struct disphist *next;
    char *name;
    time_t lastExit;      /* time of last display exit */
    unsigned rLogin:2,    /* 0=nothing 1=relogin 2=login */
             lock:1,      /* screen locker running */
             goodExit:1;  /* was the last exit "peaceful"? */
    char *nuser, *npass, *nargs;
};

#ifdef XDMCP

#define PROTO_TIMEOUT (30 * 60)  /* 30 minutes should be long enough */

struct protoDisplay {
    struct protoDisplay *next;
    XdmcpNetaddr address;       /* UDP address */
    int addrlen;                /* UDP address length */
    unsigned long date;         /* creation date */
    CARD16 displayNumber;
    CARD16 connectionType;
    ARRAY8 connectionAddress;
    CARD32 sessionID;
    Xauth *fileAuthorization;
    Xauth *xdmcpAuthorization;
    ARRAY8 authenticationName;
    ARRAY8 authenticationData;
    XdmAuthKeyRec key;
};
#endif /* XDMCP */

/* status code for rStopDisplay */
#define DS_MASK      255
# define DS_RESTART  0
# define DS_TEXTMODE 1
# define DS_RESERVE  2
# define DS_REMOTE   3
# define DS_REMOVE   4
#define DS_SCHEDULE  256 /* flag for DS_TEXTMODE */

/* command codes dpy process -> master process */
#define D_User       1
#define D_ReLogin    2
#define D_ChooseHost 4
#define D_RemoteHost 5
#define D_XConnOk    6
#define D_UnUser     7

extern int debugLevel;

CONF_CORE_GLOBAL_DECLS

/* in daemon.c */
void becomeDaemon(void);

/* in dm.c */
#if KDM_LIBEXEC_STRIP != -1
extern char *progpath;
#endif
#if KDM_LIBEXEC_STRIP
extern char *progname;
#endif
extern char *prog;
extern time_t now;
extern SdRec sdRec;
void startDisplayP2(struct display *d);
void stopDisplay(struct display *d);
#if !defined(HAVE_SETPROCTITLE) && !defined(NOXDMTITLE)
void setproctitle(const char *fmt, ...);
#endif
void wakeDisplays(void);
void switchToX(struct display *d);
void setNLogin(struct display *d,
               const char *nuser, const char *npass, const char *nargs,
               int rl);
void cancelShutdown(void);
int TTYtoVT(const char *tty);
int activateVT(int vt);

#ifndef _POSIX_MONOTONIC_CLOCK
# define _POSIX_MONOTONIC_CLOCK -1
#endif
#if (_POSIX_MONOTONIC_CLOCK > 0)
# define nowMonotonic 1
#elif (_POSIX_MONOTONIC_CLOCK < 0)
# define nowMonotonic 0
#else
extern int nowMonotonic;
#endif
void updateNow(void);

/* in ctrl.c */
void openCtrl(struct display *d);
void closeCtrl(struct display *d);
int handleCtrl(fd_set *reads, struct display *d);
void chownCtrl(CtrlRec *cr, int uid);
void updateCtrl(void);

/* in dpylist.c */
extern struct display *displays; /* that's ugly ... */
int anyDisplaysLeft(void);
void forEachDisplay(void (*f)(struct display *));
#ifdef HAVE_VTS
void forEachDisplayRev(void (*f)(struct display *));
#endif
void removeDisplay(struct display *old);
struct display
    *findDisplayByName(const char *name),
#ifdef XDMCP
    *findDisplayBySessionID(CARD32 sessionID),
    *findDisplayByAddress(XdmcpNetaddr addr, int addrlen, CARD16 displayNumber),
#endif /* XDMCP */
    *findDisplayByPid(int pid),
    *findDisplayByServerPid(int serverPid),
    *newDisplay(const char *name);
int anyRunningDisplays(void);
int anyReserveDisplays(void);
int idleReserveDisplays(void);
int startReserveDisplay(void);
const char *displayName(struct display *);

/* in reset.c */
void pseudoReset(void);

/* in resource.c */
void **findCfgEnt(struct display *d, int id);
int initResources(char **argv);
int loadDMResources(int force);
int loadDisplayResources(struct display *d);
void scanServers(void);
void closeGetter(void);
int startConfig(int what, CfgDep *dep, int force);
RcStr *newStr(char *str);
void delStr(RcStr *str);
extern GTalk cnftalk;

/* in session.c */
extern struct display *td;
extern const char *td_setup;
char **baseEnv(char **env, const char *user);
char **inheritEnv(char **env, const char **what);
char **systemEnv(char **env, const char *user);
int source(char **env, const char *file, const char *arg);
void manageSession(void);

extern GTalk mstrtalk, grttalk;
extern GProc grtproc;
void openGreeter(void);
int closeGreeter(int force);
int ctrlGreeterWait(int wreply, time_t *startTime);
void prepareErrorGreet(void);
void finishGreet(void);
char *conv_interact(int what, const char *prompt);

/* process.c */
typedef void (*SIGFUNC)(int);
SIGFUNC Signal(int, SIGFUNC handler);

void registerInput(int fd);
void unregisterInput(int fd);
void registerCloseOnFork(int fd);
void clearCloseOnFork(int fd);
void closeNclearCloseOnFork(int fd);
int Fork(volatile int *pid);
int Wait4(volatile int *pid);
void execute(char **argv, char **env);
int runAndWait(char **args, char **env);
FILE *pOpen(char **what, char m, volatile int *pid);
int pClose(FILE *f, volatile int *pid);
char *locate(const char *exe);
void terminateProcess(int pid, int sig);
void blockTerm(void);
void unblockTerm(void);

void gSet(GTalk *talk); /* call before gOpen! */
void gCloseOnExec(GPipe *pajp);
int gFork(GPipe *pajp, const char *pname, char *cname,
          GPipe *ogp, char *cgname, GPipe *igp, volatile int *pid);
void gClosen(GPipe *pajp);
int gOpen(GProc *proc,
          char **argv, const char *what, char **env, char *cname,
          const char *user, const char *authfile, GPipe *igp);
int gClose(GProc *proc, GPipe *gp, int force);

void gSendInt(int val);
int gRecvInt(void);
int gRecvCmd(int *cmd);
void gSendArr(int len, const char *data);
char *gRecvArr(int *len);
int gRecvStrBuf(char *buf);
int gRecvArrBuf(char *buf);
void gSendStr(const char *buf);
void gSendNStr(const char *buf, int len); /* exact len, buf != 0 */
void gSendStrN(const char *buf, int len); /* maximal len */
char *gRecvStr(void);
void gSendArgv(char **argv);
void gSendStrArr(int len, char **data);
char **gRecvStrArr(int *len);
char **gRecvArgv(void);

/* client.c */
#define GCONV_NORMAL  0
#define GCONV_HIDDEN  1
#define GCONV_USER    2
#define GCONV_PASS    3
#define GCONV_PASS_ND 4
#define GCONV_BINARY  5
typedef char *(*GConvFunc)(int what, const char *prompt);
int verify(GConvFunc gconv, int rootok);
int startClient(volatile int *pid);
void clientExited(void);
void sessionExit(int status) ATTR_NORETURN;
int readDmrc(void);
int changeUser(const char *user, const char *authfile);
extern char **userEnviron, **systemEnviron;
extern char *curuser, *curpass, *curtype, *newpass,
            *dmrcuser, *curdmrc, *newdmrc;
extern int cursource;
#define PWSRC_MANUAL 0
#define PWSRC_AUTOLOGIN 1
#define PWSRC_RELOGIN 2

/* server.c */
char **prepareServerArgv(struct display *d, const char *args);
void startServer(struct display *d);
void abortStartServer(struct display *d);
void startServerSuccess(void);
void startServerFailed(void);
void startServerTimeout(void);
extern struct display *startingServer;
extern time_t serverTimeout;

void waitForServer(struct display *d);
void resetServer(struct display *d);
int pingServer(struct display *d);
extern struct _XDisplay *dpy;

/* in util.c */
void *Calloc(size_t nmemb, size_t size);
void *Malloc(size_t size);
void *Realloc(void *ptr, size_t size);
void wipeStr(char *str);
int strCmp(const char *s1, const char *s2);
#ifndef HAVE_STRNLEN
int strnlen(const char *s, int max);
#endif
int strNDup(char **dst, const char *src, int len);
int strDup(char **dst, const char *src);
int arrLen(char **arr);
void freeStrArr(char **arr);
char **initStrArr(char **arr);
char **xCopyStrArr(int rn, char **arr);
/* Note: the following functions free the old data even in case of failure */
int reStrN(char **dst, const char *src, int len);
int reStr(char **dst, const char *src);
int strApp(char **dst, ...);
char **addStrArr(char **arr, const char *str, int len);
char **parseArgs(char **argv, const char *string);
/* End note */
char **setEnv(char **e, const char *name, const char *value);
char **putEnv(const char *string, char **env);
const char *getEnv(char **e, const char *name);
const char *localHostname(void);
int reader(int fd, void *buf, int len);
int writer(int fd, const void *buf, int len);
int fGets(char *buf, int max, FILE *f);
time_t mTime(const char *fn);
void randomStr(char *s);
int hexToBinary(char *out, const char *in);
void listSessions(int flags, struct display *d, void *ctx,
                  void (*emitXSess)(struct display *, struct display *, void *),
                  void (*emitTTYSess)(STRUCTUTMP *, struct display *, void *));
int anyUserLogins(int exclude_uid);

struct expando {
    char key;
    int uses;
    const char *val;
};
char *expandMacros(const char *str, struct expando *expandos);

/* in inifile.c */
char *iniLoad(const char *fname);
int iniSave(const char *data, const char *fname);
char *iniEntry(char *data, const char *section, const char *key, const char *value);
char *iniMerge(char *data, const char *newdata);

/* in bootman.c */
int getBootOptions(char ***opts, int *def, int *cur);
int setBootOption(const char *opt, SdRec *sdr);
void commitBootOption(void);

/* in netaddr.c */
CARD8 *netaddrAddress(char *netaddrp, int *lenp);
CARD8 *netaddrPort(char *netaddrp, int *lenp);
int convertAddr(char *saddr, int *len, CARD8 **addr);
int netaddrFamily(char *netaddrp);
int addressEqual(char *a1, int len1, char *a2, int len2);

#ifdef XDMCP

/* in xdmcp.c */
char *networkAddressToHostname(CARD16 connectionType, ARRAY8Ptr connectionAddress);
void sendFailed(struct display *d, const char *reason);
void initXdmcp(void);

/* in policy.c */
struct sockaddr;
ARRAY8Ptr isAccepting(struct sockaddr *from, int fromlen, CARD16 displayNumber);
ARRAY8Ptr chooseAuthentication(ARRAYofARRAY8Ptr authenticationNames);
int checkAuthentication(struct protoDisplay *pdpy, ARRAY8Ptr displayID, ARRAY8Ptr name, ARRAY8Ptr data);
int selectAuthorizationTypeIndex(ARRAY8Ptr authenticationName, ARRAYofARRAY8Ptr authorizationNames);
int selectConnectionTypeIndex(ARRAY16Ptr connectionTypes, ARRAYofARRAY8Ptr connectionAddresses);
int isWilling(ARRAY8Ptr addr, CARD16 connectionType, ARRAY8Ptr authenticationName, ARRAY8Ptr status, xdmOpCode type);

/* in protodpy.c */
void disposeProtoDisplay(struct protoDisplay *pdpy);

struct protoDisplay *findProtoDisplay(XdmcpNetaddr address, int addrlen,
                                      CARD16 displayNumber);
struct protoDisplay *newProtoDisplay(XdmcpNetaddr address, int addrlen,
                                     CARD16 displayNumber,
                                     CARD16 connectionType,
                                     ARRAY8Ptr connectionAddress,
                                     CARD32 sessionID);

#define FamilyBroadcast 0xffff
typedef void (*ChooserFunc)(CARD16 connectionType, ARRAY8Ptr addr, char *closure);
typedef void (*ListenFunc)(ARRAY8Ptr addr, void **closure);

/* in access.c */
ARRAY8Ptr getLocalAddress(void);
int acceptableDisplayAddress(ARRAY8Ptr clientAddress, CARD16 connectionType, xdmOpCode type);
int forEachMatchingIndirectHost(ARRAY8Ptr clientAddress, ARRAY8Ptr clientPort, CARD16 connectionType,
                                ChooserFunc function, char *closure);
void scanAccessDatabase(int force);
int useChooser(ARRAY8Ptr clientAddress, CARD16 connectionType);
void forEachChooserHost(ARRAY8Ptr clientAddress, CARD16 connectionType, ChooserFunc function, char *closure);
void forEachListenAddr(ListenFunc listenfunction, ListenFunc mcastfcuntion, void **closure);

/* in choose.c */
time_t disposeIndirectHosts(void);
ARRAY8Ptr indirectChoice(ARRAY8Ptr clientAddress, ARRAY8Ptr clientPort, CARD16 connectionType);
int checkIndirectChoice(ARRAY8Ptr clientAddress, ARRAY8Ptr clientPort, CARD16 connectionType);
void registerIndirectChoice(ARRAY8Ptr clientAddress, ARRAY8Ptr clientPort, CARD16 connectionType,
                            ARRAY8Ptr choice);
int doChoose(time_t *startTime);

/* socket.c or streams.c */
void updateListenSockets(void);
int anyListenSockets(void);
int processListenSockets(fd_set *reads);

/* in xdmcp.c */
void processRequestSocket(int fd);

#endif /* XDMCP */

/* in sessreg.c */
void sessreg(struct display *d, int pid, const char *user, int uid);

#endif /* _DM_H_ */
