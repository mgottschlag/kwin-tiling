/* $TOG: resource.c /main/48 1998/02/09 13:56:04 kaleb $ */
/* $Id$ */
/*

Copyright 1988, 1998  The Open Group

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
/* $XFree86: xc/programs/xdm/resource.c,v 3.5 1998/12/06 06:08:49 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * resource.c
 */

# include "dm.h"
# include "dm_error.h"

# include <X11/Intrinsic.h>

char	*config;

char	*servers;
int	request_port;
int	debugLevel;
char	*errorLogFile;
int	daemonMode;
char	*pidFile;
int	lockPidFile;
int	sourceAddress;
char	*authDir;
int	autoRescan;
int	removeDomainname;
char	*keyFile;
char	*accessFile;
char	**exportList;
char	*randomFile;
char	*greeterLib;
char	*willing;
int	choiceTimeout;	/* chooser choice timeout */
int	autoLogin;

# define DM_STRING	0
# define DM_INT		1
# define DM_BOOL	2
# define DM_ARGV	3

/*
 * the following constants are supposed to be set in the makefile from
 * parameters set util/imake.includes/site.def (or *.macros in that directory
 * if it is server-specific).  DO NOT CHANGE THESE DEFINITIONS!
 */
#ifndef __EMX__
#define Quote(s) #s
#define QUOTE(s) Quote(s)
#ifndef DEF_SERVER_LINE 
#define DEF_SERVER_LINE ":0 local " QUOTE(XBINDIR) "/X :0"
#endif
#ifndef XRDB_PROGRAM
/* use krdb instead? */
#define XRDB_PROGRAM QUOTE(XBINDIR) "/xrdb"
#endif
#ifndef DEF_SETUP
/* Should be run this as default? */
#define DEF_SETUP XDMDIR"/Xsetup"
#endif
#ifndef DEF_SESSION
#define DEF_SESSION XDMDIR"/Xsession" /* QUOTE(XBINDIR) "xterm -ls" */
#endif
#ifndef DEF_USER_PATH
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(linux)
#    define DEF_USER_PATH "/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  else
#    define DEF_USER_PATH "/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin:/usr/ucb"
#  endif
#endif
#ifndef DEF_SYSTEM_PATH
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(linux)
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin"
#  else
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" QUOTE(XBINDIR) ":/usr/local/bin:/etc:/usr/ucb"
#  endif
#endif
#ifndef DEF_SYSTEM_SHELL
#  ifdef _PATH_BSHELL
#    define DEF_SYSTEM_SHELL _PATH_BSHELL
#  else
#    define DEF_SYSTEM_SHELL "/bin/sh"
#  endif
#endif
#ifndef DEF_FAILSAFE_CLIENT
#define DEF_FAILSAFE_CLIENT QUOTE(XBINDIR) "/xterm"
#endif
#ifndef DEF_XDM_CONFIG
#define DEF_XDM_CONFIG XDMDIR "/xdm-config"
#endif
#ifndef DEF_CHOOSER
#define DEF_CHOOSER XDMDIR "/chooser"
#endif
#ifndef DEF_AUTH_NAME
#ifdef HASXDMAUTH
#define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1 MIT-MAGIC-COOKIE-1"
#else
#define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
#endif
#endif
#ifndef DEF_AUTH_DIR
/* This may be read only */
#define DEF_AUTH_DIR XDMDIR
#endif
#ifndef DEF_USER_AUTH_DIR
#define DEF_USER_AUTH_DIR	"/tmp"
#endif
#ifndef DEF_KEY_FILE
#define DEF_KEY_FILE	""
#endif
#ifndef DEF_ACCESS_FILE
#define DEF_ACCESS_FILE	""
#endif
#ifndef DEF_RANDOM_FILE
#define DEF_RANDOM_FILE _PATH_MEM
#endif
#ifndef DEF_GREETER_LIB
#define DEF_GREETER_LIB "libKdmGreet.so"
#endif
#ifndef DEF_PID_FILE
#  if defined(__FreeBSD__) || defined(__NetBSD__)
#    define DEF_PID_FILE _PATH_VARRUN"kdm.pid"
#  else
/* this may be readonly
 */
#    define DEF_PID_FILE XDMDIR "/kdm-pid"
#  endif
#endif
#else	/* __EMX__ */
/* unfortunately I have to declare all of them, because there is a limit
 * in argument size in OS/2
 * but everything needs to be fixed again
 */
#define DEF_SERVER_LINE ":0 local /XFree86/bin/X :0"
#ifndef XRDB_PROGRAM
#define XRDB_PROGRAM "/XFree86/bin/xrdb"
#endif
#ifndef DEF_SESSION
#define DEF_SESSION "/XFree86/bin/xterm -ls"
#endif
#ifndef DEF_USER_PATH
#define DEF_USER_PATH "c:\\os2;c:\\os2\apps;\\XFree86\\bin"
#endif
#ifndef DEF_SYSTEM_PATH
#define DEF_SYSTEM_PATH "c:\\os2;c:\\os2\apps;\\XFree86\\bin"
#endif
#ifndef DEF_SYSTEM_SHELL
#define DEF_SYSTEM_SHELL "sh"
#endif
#ifndef DEF_FAILSAFE_CLIENT
#define DEF_FAILSAFE_CLIENT "/XFree86/bin/xterm"
#endif
#ifndef DEF_XDM_CONFIG
#define DEF_XDM_CONFIG "/XFree86/lib/X11/xdm/xdm-config"
#endif
#ifndef DEF_CHOOSER
#define DEF_CHOOSER "/XFree86/lib/X11/xdm/chooser"
#endif
#ifndef DEF_AUTH_NAME
#ifdef HASXDMAUTH
#define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1 MIT-MAGIC-COOKIE-1"
#else
#define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
#endif
#endif
#ifndef DEF_AUTH_DIR
#define DEF_AUTH_DIR "/XFree86/lib/X11/xdm"
#endif
#ifndef DEF_USER_AUTH_DIR
#define DEF_USER_AUTH_DIR	"/tmp"
#endif
#ifndef DEF_KEY_FILE
#define DEF_KEY_FILE	""
#endif
#ifndef DEF_ACCESS_FILE
#define DEF_ACCESS_FILE	""
#endif
#ifndef DEF_RANDOM_FILE
#define DEF_RANDOM_FILE ""
#endif
#ifndef DEF_GREETER_LIB
#define DEF_GREETER_LIB "/XFree86/lib/X11/xdm/libXdmGreet.so"
#endif

#endif /* __EMX__ */

#define DEF_UDP_PORT	"177"	    /* registered XDMCP port, dont change */

struct dmResources {
	char	*name, *class2;
	int	type;
	char	**dm_value;
	char	*default_value;
} DmResources[] = {
{ "servers",	"Servers", 	DM_STRING,	&servers,
				DEF_SERVER_LINE} ,
{ "requestPort","RequestPort",	DM_INT,		(char **) &request_port,
				DEF_UDP_PORT} ,
{ "debugLevel",	"DebugLevel",	DM_INT,		(char **) &debugLevel,
				"0"} ,
{ "errorLogFile","ErrorLogFile",	DM_STRING,	&errorLogFile,
				""} ,
{ "daemonMode",	"DaemonMode",	DM_BOOL,	(char **) &daemonMode,
				"true"} ,
{ "pidFile",	"PidFile",	DM_STRING,	&pidFile,
				""} ,
{ "lockPidFile","LockPidFile",	DM_BOOL,	(char **) &lockPidFile,
				"true"} ,
{ "authDir",	"authDir",	DM_STRING,	&authDir,
				DEF_AUTH_DIR} ,
{ "autoRescan",	"AutoRescan",	DM_BOOL,	(char **) &autoRescan,
				"true"} ,
{ "removeDomainname","RemoveDomainname",DM_BOOL,(char **) &removeDomainname,
				"true"} ,
{ "keyFile",	"KeyFile",	DM_STRING,	&keyFile,
				DEF_KEY_FILE} ,
{ "accessFile",	"AccessFile",	DM_STRING,	&accessFile,
				DEF_ACCESS_FILE} ,
{ "exportList",	"ExportList",	DM_ARGV,	(char **) &exportList,
				""} ,
{ "randomFile",	"RandomFile",	DM_STRING,	&randomFile,
				DEF_RANDOM_FILE} ,
{ "greeterLib",	"GreeterLib",	DM_STRING,	&greeterLib,
				DEF_GREETER_LIB} ,
{ "choiceTimeout","ChoiceTimeout",DM_INT,	(char **) &choiceTimeout,
				"15"} ,
{ "sourceAddress","SourceAddress",DM_BOOL,	(char **) &sourceAddress,
				"false"} ,
{ "willing",	"Willing",	DM_STRING,	&willing,
				""} ,
{ "autoLogin",	"AutoLogin",	DM_BOOL,	(char **) &autoLogin,
				"true"} ,
};

# define NUM_DM_RESOURCES	(sizeof DmResources / sizeof DmResources[0])

# define boffset(f)	XtOffsetOf(struct display, f)

struct displayResource {
	char	*name, *class2;
	int	type;
	int	offset;
	char	*default_value;
};

/* resources for managing the server */

struct displayResource serverResources[] = {
{ "serverAttempts","ServerAttempts",DM_INT,	boffset(serverAttempts),
				"1" },
{ "openDelay",	"OpenDelay",	DM_INT,		boffset(openDelay),
				"15" },
{ "openRepeat",	"OpenRepeat",	DM_INT,		boffset(openRepeat),
				"5" },
{ "openTimeout","OpenTimeout",	DM_INT,		boffset(openTimeout),
				"120" },
{ "startAttempts","StartAttempts",DM_INT,	boffset(startAttempts),
				"4" },
{ "pingInterval","PingInterval",DM_INT,		boffset(pingInterval),
				"5" },
{ "pingTimeout","PingTimeout",	DM_INT,		boffset(pingTimeout),
				"5" },
{ "terminateServer","TerminateServer",DM_BOOL,	boffset(terminateServer),
				"false" },
{ "grabServer",	"GrabServer",	DM_BOOL,	boffset(grabServer),
				"false" },
{ "grabTimeout","GrabTimeout",	DM_INT,		boffset(grabTimeout),
				"3" },
{ "resetSignal","Signal",	DM_INT,		boffset(resetSignal),
				"1" },	/* SIGHUP */
{ "termSignal",	"Signal",	DM_INT,		boffset(termSignal),
				"15" },	/* SIGTERM */
{ "resetForAuth","ResetForAuth",DM_BOOL,	boffset(resetForAuth),
				"false" },
{ "authorize",	"Authorize",	DM_BOOL,	boffset(authorize),
				"true" },
{ "authComplain","AuthComplain",DM_BOOL,	boffset(authComplain),
				"true" },
{ "authName",	"AuthName",	DM_ARGV,	boffset(authNames),
				DEF_AUTH_NAME },
{ "authFile",	"AuthFile",	DM_STRING,	boffset(clientAuthFile),
				"" },
{ "fifoCreate",	"FifoCreate",	DM_BOOL,	boffset(fifoCreate),
				"false" },
{ "fifoGroup",	"FifoGroup",	DM_INT,		boffset(fifoGroup),
				"0" },
{ "fifoMode",	"FifoMode",	DM_INT,		boffset(fifoMode),
				"0" },
{ "startInterval","StartInterval",DM_INT,	boffset(startInterval),
				"30" },
};

# define NUM_SERVER_RESOURCES	(sizeof serverResources/\
				 sizeof serverResources[0])

/* resources which control the session behaviour */

struct displayResource sessionResources[] = {
{ "resources",	"Resources",	DM_STRING,	boffset(resources),
				"" },
{ "xrdb",	"Xrdb",		DM_STRING,	boffset(xrdb),
				XRDB_PROGRAM },
{ "setup",	"Setup",	DM_STRING,	boffset(setup),
				"" },
{ "startup",	"Startup",	DM_STRING,	boffset(startup),
				"" },
{ "reset",	"Reset",	DM_STRING,	boffset(reset),
				"" },
{ "session",	"Session",	DM_STRING,	boffset(session),
				DEF_SESSION },
{ "userPath",	"Path",		DM_STRING,	boffset(userPath),
				DEF_USER_PATH },
{ "systemPath",	"Path",		DM_STRING,	boffset(systemPath),
				DEF_SYSTEM_PATH },
{ "systemShell","Shell",	DM_STRING,	boffset(systemShell),
				DEF_SYSTEM_SHELL },
{ "failsafeClient","FailsafeClient", DM_STRING,	boffset(failsafeClient),
				DEF_FAILSAFE_CLIENT },
{ "userAuthDir","UserAuthDir",	DM_STRING,	boffset(userAuthDir),
				DEF_USER_AUTH_DIR },
{ "chooser",	"Chooser",	DM_STRING,	boffset(chooser),
				DEF_CHOOSER },
{ "noPassUsers", "NoPassUsers",	DM_STRING,	boffset(noPassUsers),
				"" },
{ "autoUser",	"AutoUser",	DM_STRING,	boffset(autoUser),
                        	"" },
{ "autoPass",	"AutoPass",	DM_STRING,	boffset(autoPass),
                        	"" },
{ "autoString",	"AutoString",	DM_STRING,	boffset(autoString),
                        	"" },
{ "autoLogin1st", "AutoLogin1st", DM_BOOL,	boffset(autoLogin1st),
				"true" },
{ "autoReLogin", "AutoReLogin",	DM_BOOL,	boffset(autoReLogin),
				"false" },
};

# define NUM_SESSION_RESOURCES	(sizeof sessionResources/\
				 sizeof sessionResources[0])

XrmDatabase	DmResourceDB;

static void
GetResource (
    char    *name,
    char    *class2,
    int	    valueType,
    char    **valuep,
    char    *default_value)
{
    char	*type;
    XrmValue	value;
    char	*string, *new_string;
    char	str_buf[50];
    int	len;

    if (DmResourceDB && XrmGetResource (DmResourceDB,
	name, class2,
	&type, &value))
    {
	string = value.addr;
	len = value.size;
    }
    else
    {
	string = default_value;
	len = strlen (string);
    }

    Debug ("%s/%s value %*.*s\n", name, class2, len, len, string);

    switch (valueType) {
    case DM_STRING:
        if (*valuep)
        {
	    if (strlen (*valuep) == len && !strncmp (*valuep, string, len))
		return;
	    else
		free (*valuep);
        }
	new_string = malloc ((unsigned) (len+1));
	if (!new_string) {
		LogOutOfMem ("GetResource");
		return;
	}
	strncpy (new_string, string, len);
	new_string[len] = '\0';
	*(valuep) = new_string;
	break;
    case DM_INT:
	strncpy (str_buf, string, sizeof (str_buf));
	str_buf[sizeof (str_buf)-1] = '\0';
	sscanf (str_buf, "%i", (int *) valuep);
	break;
    case DM_BOOL:
	strncpy (str_buf, string, sizeof (str_buf));
	str_buf[sizeof (str_buf)-1] = '\0';
	if (!strcasecmp (str_buf, "true") ||
	    !strcasecmp (str_buf, "on") ||
	    !strcasecmp (str_buf, "yes"))
		*((int *) valuep) = 1;
	else if (!strcasecmp (str_buf, "false") ||
		 !strcasecmp (str_buf, "off") ||
		 !strcasecmp (str_buf, "no"))
		*((int *) valuep) = 0;
	break;
    case DM_ARGV:
	freeArgs (*(char ***) valuep);
	*((char ***) valuep) = parseArgs ((char **) 0, string);
	break;
    }
}

XrmOptionDescRec configTable [] = {
{"-server",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-udpPort",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-error",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-resources",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-session",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-debug",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionSkipArg,	(caddr_t) NULL },
{"-config",	".configFile",		XrmoptionSepArg,	(caddr_t) NULL }
};

XrmOptionDescRec optionTable [] = {
{"-server",	".servers",		XrmoptionSepArg,	(caddr_t) NULL },
{"-udpPort",	".requestPort",		XrmoptionSepArg,	(caddr_t) NULL },
{"-error",	".errorLogFile",	XrmoptionSepArg,	(caddr_t) NULL },
{"-resources",	"*resources",		XrmoptionSepArg,	(caddr_t) NULL },
{"-session",	"*session",		XrmoptionSepArg,	(caddr_t) NULL },
{"-debug",	"*debugLevel",		XrmoptionSepArg,	(caddr_t) NULL },
{"-xrm",	NULL,			XrmoptionResArg,	(caddr_t) NULL },
{"-autolog",	".autoLogin",		XrmoptionNoArg,		"true"         },
{"-noautolog",	".autoLogin",		XrmoptionNoArg,		"false"        },
{"-daemon",	".daemonMode",		XrmoptionNoArg,		"true"         },
{"-nodaemon",	".daemonMode",		XrmoptionNoArg,		"false"        }
};

static int	originalArgc;
static char	**originalArgv;

void
InitResources (int argc, char **argv)
{
	XrmInitialize ();
	originalArgc = argc;
	originalArgv = argv;
	ReinitResources ();
}

void
ReinitResources (void)
{
    int	argc;
    char	**a;
    char	**argv;
    XrmDatabase newDB;

    argv = (char **) malloc ((originalArgc + 1) * sizeof (char *));
    if (!argv)
	LogPanic ("no space for argument realloc\n");
    for (argc = 0; argc < originalArgc; argc++)
	argv[argc] = originalArgv[argc];
    argv[argc] = 0;
    if (DmResourceDB)
	XrmDestroyDatabase (DmResourceDB);
    DmResourceDB = XrmGetStringDatabase ("");
    /* pre-parse the command line to get the -config option, if any */
    XrmParseCommand (&DmResourceDB, configTable,
		     sizeof (configTable) / sizeof (configTable[0]),
		     "DisplayManager", &argc, argv);
    GetResource ("DisplayManager.configFile", "DisplayManager.ConfigFile",
		 DM_STRING, &config, DEF_XDM_CONFIG);
    newDB = XrmGetFileDatabase ( config );
    if (newDB)
    {
	if (DmResourceDB)
	    XrmDestroyDatabase (DmResourceDB);
	DmResourceDB = newDB;
    }
    else if (argc != originalArgc)
	LogError ("Can't open configuration file %s\n", config );
    XrmParseCommand (&DmResourceDB, optionTable,
		     sizeof (optionTable) / sizeof (optionTable[0]),
		     "DisplayManager", &argc, argv);
    if (argc > 1)
    {
	LogError ("extra arguments on command line:");
	for (a = argv + 1; *a; a++)
		LogError (" \"%s\"", *a);
	LogError ("\n");
    }
    free (argv);
}

void
LoadDMResources (void)
{
	int	i;
	char	name[1024], class2[1024];

	for (i = 0; i < NUM_DM_RESOURCES; i++) {
		sprintf (name, "DisplayManager.%s", DmResources[i].name);
		sprintf (class2, "DisplayManager.%s", DmResources[i].class2);
		GetResource (name, class2, DmResources[i].type,
			      (char **) DmResources[i].dm_value,
			      DmResources[i].default_value);
	}
}

static void
CleanUpName (char *src, char *dst, int len)
{
    while (*src) {
	if (--len <= 0)
		break;
	switch (*src)
	{
	case ':':
	case '.':
	    *dst++ = '_';
	    break;
	default:
	    *dst++ = *src;
	}
	++src;
    }
    *dst = '\0';
}

static void
LoadDisplayResources (
    struct display	    *d,
    struct displayResource  *resources,
    int			    numResources)
{
    int	i;
    char	name[1024], class2[1024];
    char	dpyName[512], dpyClass[512];

    CleanUpName (d->name, dpyName, sizeof (dpyName));
    CleanUpName (d->class2 ? d->class2 : d->name, dpyClass, sizeof (dpyClass));
    for (i = 0; i < numResources; i++) {
	    sprintf (name, "DisplayManager.%s.%s", 
		    dpyName, resources[i].name);
	    sprintf (class2, "DisplayManager.%s.%s",
		    dpyClass, resources[i].class2);
	    GetResource (name, class2, resources[i].type,
			  (char **) (((char *) d) + resources[i].offset),
			  resources[i].default_value);
    }
}

void
LoadServerResources (struct display *d)
{
    LoadDisplayResources (d, serverResources, NUM_SERVER_RESOURCES);
}

void
LoadSessionResources (struct display *d)
{
    LoadDisplayResources (d, sessionResources, NUM_SESSION_RESOURCES);
}
