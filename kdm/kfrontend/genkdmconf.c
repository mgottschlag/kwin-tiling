    /*

    Create a suitable configuration for kdm taking old xdm/kdm 
    installations into account

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

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include <config.h>

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <fcntl.h>

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define ATTR_UNUSED __attribute__((unused))
#else
# define ATTR_UNUSED
#endif

#define as(ar) ((int)(sizeof(ar)/sizeof(ar[0])))

#define KDMCONF KDE_CONFDIR "/kdm"

static int no_old, copy_files;
static const char *newdir = KDMCONF, *oldxdm, *oldkde;


typedef struct StrList {
    struct StrList	*next;
    const char		*str;
} StrList;


#define NO_LOGGER
#define STATIC static
#include <printf.c>

typedef struct {
    char *buf;
    int clen, blen, tlen;
} OCABuf;

static void
OutCh_OCA (void *bp, char c)
{
    OCABuf *ocabp = (OCABuf *)bp;
    char *nbuf;
    int nlen;

    ocabp->tlen++;
    if (ocabp->clen >= ocabp->blen) {
	if (ocabp->blen < 0)	
	    return;
	nlen = ocabp->blen * 3 / 2 + 100;
	nbuf = realloc (ocabp->buf, nlen);
	if (!nbuf) {
	    free (ocabp->buf);
	    ocabp->blen = -1;
	    ocabp->buf = 0;
	    ocabp->clen = 0;
	    fprintf (stderr, "Out of memory\n");
	    return;
	}
	ocabp->blen = nlen;
	ocabp->buf = nbuf;
    }
    ocabp->buf[ocabp->clen++] = c;
}

static int 
VASPrintf (char **strp, const char *fmt, va_list args)
{
    OCABuf ocab = { 0, 0, 0, -1 };

    DoPr(OutCh_OCA, &ocab, fmt, args);
    OutCh_OCA(&ocab, 0);
    *strp = realloc (ocab.buf, ocab.clen);
    if (!*strp)
	*strp = ocab.buf;
    return ocab.tlen;
}

static int 
ASPrintf (char **strp, const char *fmt, ...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = VASPrintf (strp, fmt, args);
    va_end(args);
    return len;
}


static char *
sed (const char *text, const char *alt, const char *neu)
{
    const char *cptr, *ncptr;
    char *ntext, *dptr;
    int alen, nlen, tlen, ntlen, nma;

    alen = strlen(alt);
    nlen = strlen(neu);
    tlen = strlen(text);
    for (cptr = text, nma = 0; 
	 (cptr = strstr (cptr, alt)); 
	 cptr += alen, nma++);
    ntlen = tlen + (nlen - alen) * nma;
    if ((ntext = malloc (ntlen))) {
	for (cptr = text, dptr = ntext; 
	     (ncptr = strstr (cptr, alt)); 
	     cptr = ncptr + alen, dptr += nlen)
	{
	    memcpy (dptr, cptr, (ncptr - cptr));
	    memcpy ((dptr += (ncptr - cptr)), neu, nlen);    
	}
	memcpy (dptr, cptr, (text + tlen - cptr) + 1);
    }
    return ntext;
}


/* #define WANT_CLOSE 1 */

typedef struct File {
	char *buf, *eof;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
	int ismapped;
#endif
} File;

static int
readFile (File *file, const char *fn)
{
    int fd;
    off_t flen;

    if ((fd = open (fn, O_RDONLY)) < 0)
	return 0;

    flen = lseek (fd, 0, SEEK_END);
#ifdef HAVE_MMAP
# ifdef WANT_CLOSE
    file->ismapped = 0;
# endif
    file->buf = mmap(0, flen + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
# ifdef WANT_CLOSE
    if (file->buf)
	file->ismapped = 1;
    else
# else
    if (!file->buf)
# endif
#endif
    {
	if (!(file->buf = malloc (flen + 1))) {
	    close (fd);
	    fprintf (stderr, "Out of memory\n");
	    return 0;
	}
	lseek (fd, 0, SEEK_SET);
	if (read (fd, file->buf, flen) != flen) {
	    free (file->buf);
	    close (fd);
	    fprintf (stderr, "Cannot read file\n");
	    return 0;
	}
    }
    file->eof = file->buf + flen;
    close (fd);
    return 1;
}

#ifdef WANT_CLOSE
static void
freeFile (File *file)
{
# ifdef HAVE_MMAP
    if (file->ismapped)
	munmap(file->buf, file->eof - file->buf);
    else
# endif
	free (file->buf);
}
#endif


/*
 * defaults
 */

#ifndef HALT_CMD
# ifdef BSD
#  define HALT_CMD	"/sbin/shutdown -h now"
#  define REBOOT_CMD	"/sbin/shutdown -r now"
# elif defined(__SVR4)
#  define HALT_CMD	"/usr/sbin/halt"
#  define REBOOT_CMD	"/usr/sbin/reboot"
# else
#  define HALT_CMD	"/sbin/halt"
#  define REBOOT_CMD	"/sbin/reboot"
# endif
#endif

#ifndef DEF_USER_PATH
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__linux__)
#    define DEF_USER_PATH "/bin:/usr/bin:" XBINDIR ":/usr/local/bin"
#  else
#    define DEF_USER_PATH "/bin:/usr/bin:" XBINDIR ":/usr/local/bin:/usr/ucb"
#  endif
#endif
#ifndef DEF_SYSTEM_PATH
#  if defined(__FreeBSD__) || defined(__NetBSD__) || defined(__linux__)
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" XBINDIR ":/usr/local/bin"
#  else
#    define DEF_SYSTEM_PATH "/sbin:/usr/sbin:/bin:/usr/bin:" XBINDIR ":/usr/local/bin:/etc:/usr/ucb"
#  endif
#endif

#ifndef DEF_AUTH_NAME
# ifdef HASXDMAUTH
#  define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1,MIT-MAGIC-COOKIE-1"
# else
#  define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
# endif
#endif

#ifdef __linux__
# define DEF_SERVER_LINE ":0 local@tty1 " XBINDIR "/X vt7"
#elif defined(sun)
# define DEF_SERVER_LINE ":0 local@console " XBINDIR "/X"
#elif defined(_AIX)
# define DEF_SERVER_LINE ":0 local@lft0 " XBINDIR "/X"
#else
# define DEF_SERVER_LINE ":0 local " XBINDIR "/X"
#endif

const char def_xaccess[] = 
"# Xaccess - Access control file for XDMCP connections\n"
"#\n"
"# To control Direct and Broadcast access:\n"
"#\n"
"#	pattern\n"
"#\n"
"# To control Indirect queries:\n"
"#\n"
"# 	pattern		list of hostnames and/or macros ...\n"
"#\n"
"# To use the chooser:\n"
"#\n"
"#	pattern		CHOOSER BROADCAST\n"
"#\n"
"# or\n"
"#\n"
"#	pattern		CHOOSER list of hostnames and/or macros ...\n"
"#\n"
"# To define macros:\n"
"#\n"
"#       %name		list of hosts ...\n"
"#\n"
"# The first form tells xdm which displays to respond to itself.\n"
"# The second form tells xdm to forward indirect queries from hosts matching\n"
"# the specified pattern to the indicated list of hosts.\n"
"# The third form tells xdm to handle indirect queries using the chooser;\n"
"# the chooser is directed to send its own queries out via the broadcast\n"
"# address and display the results on the terminal.\n"
"# The fourth form is similar to the third, except instead of using the\n"
"# broadcast address, it sends DirectQuerys to each of the hosts in the list\n"
"#\n"
"# In all cases, xdm uses the first entry which matches the terminal;\n"
"# for IndirectQuery messages only entries with right hand sides can\n"
"# match, for Direct and Broadcast Query messages, only entries without\n"
"# right hand sides can match.\n"
"#\n"
"\n"
"*					#any host can get a login window\n"
"\n"
"#\n"
"# To hardwire a specific terminal to a specific host, you can\n"
"# leave the terminal sending indirect queries to this host, and\n"
"# use an entry of the form:\n"
"#\n"
"\n"
"#terminal-a	host-a\n"
"\n"
"\n"
"#\n"
"# The nicest way to run the chooser is to just ask it to broadcast\n"
"# requests to the network - that way new hosts show up automatically.\n"
"# Sometimes, however, the chooser can't figure out how to broadcast,\n"
"# so this may not work in all environments.\n"
"#\n"
"\n"
"*		CHOOSER BROADCAST	#any indirect host can get a chooser\n"
"\n"
"#\n"
"# If you'd prefer to configure the set of hosts each terminal sees,\n"
"# then just uncomment these lines (and comment the CHOOSER line above)\n"
"# and edit the %hostlist line as appropriate\n"
"#\n"
"\n"
"#%hostlist	host-a host-b\n"
"\n"
"#*		CHOOSER %hostlist	#\n";

const char def_xservers[] = 
"# Xservers - local X-server list\n"
"#\n"
"# This file should contain an entry to start the server on the\n"
"# local display; if you have more than one display (not screen),\n"
"# you can add entries to the list (one per line).\n"
"# If you also have some X terminals connected which do not support XDMCP,\n"
"# you can add them here as well; you will want to leave those terminals\n"
"# on and connected to the network, else kdm will have a tougher time\n"
"# managing them. Each X terminal line should look like:\n"
"#       XTerminalName:0 foreign\n"
"#\n"
"\n" DEF_SERVER_LINE "\n\n";

const char def_willing[] = 
"#!/bin/sh\n"
"# The output of this script is displayed in the chooser window.\n"
"# (instead of \"Willing to manage\")\n"
"\n"
"load=\"`uptime|sed -e 's/^.*load[^0-9]*//'`\"\n"
"nrusers=\"`who|cut -c 1-8|sort -u|wc -l|sed 's/^[        ]*//'`\"\n"
"s=\"\"; [ \"$nrusers\" != 1 ] && s=s\n"
"\n"
"echo \"${nrusers} user${s}, load: ${load}\"\n";

const char def_setup[] = 
"#!/bin/sh\n"
"# Xsetup - run as root before the login dialog appears\n"
"\n"
KDE_BINDIR "/kdmdesktop &\n";

const char def_startup[] = 
"#!/bin/sh\n"
"# Xstartup - run as root before session starts\n"
"\n"
"# By convention, both xconsole and xterm -C check that the\n"
"# console is owned by the invoking user and is readable before attaching\n"
"# the console output.  This way a random user can invoke xterm -C without\n"
"# causing serious grief.\n"
"# This is not required if you use PAM, as pam_console should handle it.\n"
"#\n"
#ifdef HAVE_PAM
"#chown $USER /dev/console\n"
#else
"chown $USER /dev/console\n"
#endif
"\n"
"#exec sessreg -a -l $DISPLAY "
#ifdef BSD
"-x " KDMCONF "/Xservers "
#endif
"$USER\n";

const char def_reset[] = 
"#!/bin/sh\n"
"# Xreset - run as root after session exits\n"
"\n"
"# Reassign ownership of the console to root, this should disallow\n"
"# assignment of console output to any random users's xterm\n"
"# This is not required if you use PAM, as pam_console should handle it.\n"
"#\n"
#ifdef HAVE_PAM
"#chown root /dev/console\n"
"#chmod 622 /dev/console\n"
#else
"chown root /dev/console\n"
"chmod 622 /dev/console\n"
#endif
"\n"
"#exec sessreg -d -l $DISPLAY "
#ifdef BSD
"-x " KDMCONF "/Xservers "
#endif
"$USER\n";

const char def_session[] = 
"#!/bin/sh\n"
"# Xsession - run as user\n"
"\n"
"# redirect errors to a file in user's home directory if we can\n"
"for errfile in \"$HOME/.xsession-errors\" \"${TMPDIR-/tmp}/xses-$USER\" \"/tmp/xses-$USER\"\n"
"do\n"
"	if ( cp /dev/null \"$errfile\" 2> /dev/null )\n"
"	then\n"
"		chmod 600 \"$errfile\"\n"
"		exec > \"$errfile\" 2>&1\n"
"		break\n"
"	fi\n"
"done\n"
"\n"
"test -f $HOME/.xprofile && . $HOME/.xprofile\n"
"\n"
"sess=\"$1\"\n"
"shift\n"
"\n"
"case \"$sess\" in\n"
"    failsafe)\n"
"	exec xterm -geometry 80x24-0-0 $*\n"
"	;;\n"
"    \"\"|default)\n"
"	exec $HOME/.xsession $*\n"
"	;;\n"
"esac\n"
"\n"
"# start windowmanager\n"
"type \"$sess\" >/dev/null 2>&1 && exec \"$sess\" $*\n"
"type \"start$sess\" >/dev/null 2>&1 && exec \"start$sess\" $*\n"
"type \"$sess-session\" >/dev/null 2>&1 && exec \"$sess-session\" $*\n"
"sess=`echo \"$sess\" | tr A-Z a-z`\n"
"type \"$sess\" >/dev/null 2>&1 && exec \"$sess\" $*\n"
"type \"start$sess\" >/dev/null 2>&1 && exec \"start$sess\" $*\n"
"type \"$sess-session\" >/dev/null 2>&1 && exec \"$sess-session\" $*\n"
"\n"
"# windowmanager not found, tell user\n"
"exec xmessage -center -buttons OK:0 -default OK \"Sorry, $sess not found.\"\n";

#define F_FILE644	1
#define F_FILE755	2
#define F_PATH		4

typedef struct Ent {
	const char	*key;
	int		flags;
	const char	*param;
	const char	*comment;
} Ent;

static Ent entsDesktop[] = {
{ "BackgroundMode",	0, 0, 0 },
{ "BlendBalance",	0, 0, 0 },
{ "BlendMode",		0, 0, 0 },
{ "ChangeInterval",	0, 0, 0 },
{ "Color1",		0, 0, 0 },
{ "Color2",		0, 0, 0 },
{ "CurrentWallpaper",	0, 0, 0 },
{ "LastChange",		0, 0, 0 },
{ "MultiWallpaperMode",	0, 0, 0 },
{ "Pattern",		0, 0, 0 },
{ "Program",		0, 0, 0 },
{ "ReverseBlending",	0, 0, 0 },
{ "Wallpaper",		0, 0, 0 },
{ "WallpaperList",	0, 0, 0 },
{ "WallpaperMode",	0, 0, 0 },
};

static Ent entsGeneral[] = {
{ "DaemonMode",		0, 0, 
"# If \"false\", KDM won't daemonize after startup. Use this, if you start\n"
"# KDM from inittab with the respawn instruction. Default is true.\n" },
{ "Xservers",		F_FILE644, def_xservers,
"# If the value starts with a slash (/), it specifies the file, where X-servers\n"
"# to be used by KDM are listed; the file is in the usual XDM-Xservers format.\n"
"# Otherwise it's interpreted like one line of the Xservers file, i.e., it\n"
"# specifies exactly one X-server.\n"
"# Default is \"" DEF_SERVER_LINE "\"\n"
"# XXX i'm planning to absorb this file into kdmrc, but i'm not sure how to\n"
"# do this best.\n" },
{ "PidFile",		0, 0,
"# Where KDM should store its PID. Default is \"\" (don't store)\n" },
{"LockPidFile",		0, 0, 
 "# Whether KDM should lock the pid file to prevent having multiple KDM\n"
"# instances running at once. Leave it \"true\", unless you're brave.\n" },
{ "AuthDir",		0, 0, 
"# Where to store authorization files. Default is /var/run/xauth\n" },
{ "AutoRescan",		0, 0, 
"# Whether KDM should automatically re-read configuration files, if it\n"
"# finds them having changed. Just keep it \"true\".\n" },
{ "ExportList",		0, 0, 
"# Additional environment variables KDM should pass on to the Xsetup, Xstartup,\n"
"# Xsession, and Xreset programs. This shouldn't be necessary very often.\n" },
#if !defined(__linux__) && !defined(__OpenBSD__)
{ "RandomFile",		0, 0, 
"# Where KDM should fetch entropy from. Default is /dev/mem.\n" },
#endif
{ "FifoDir",		0, 0, 
"# Where the command FiFos should be created. Make it empty to disable\n"
"# the FiFos. Default is /var/run/xdmctl\n" },
{ "FifoGroup",		0, 0, 
"# To which group the command FiFos should belong.\n"
"# Default is -1 (effectively root)\n" },
};

static Ent entsXdmcp[] = {
{ "Enable",		0, 0, 
"# Whether KDM should listen to XDMCP requests. Default is true.\n" },
{ "Port",		0, 0, 
"# The UDP port KDM should listen on for XDMCP requests. Don't change the 177.\n" },
{ "KeyFile",		F_FILE644, 0, 
"# File with the private keys of X-terminals. Required for XDM authentication.\n"
"# Default is " KDMCONF "/kdmkeys\n" },
{ "Xaccess",		F_FILE644, def_xaccess, 
"# XDMCP access control file in the usual XDM-Xaccess format.\n"
"# Default is " KDMCONF "/Xaccess\n"
"# XXX i'm planning to absorb this file into kdmrc, but i'm not sure how to\n"
"# do this best.\n" },
{ "ChoiceTimeout",	0, 0, 
"# Number of seconds to wait for display to respond after the user has\n"
"# selected a host from the chooser. Default is 15.\n" },
{ "RemoveDomainname",	0, 0, 
"# Strip domain name from remote display names if it is equal to the local\n"
"# domain. Default is true\n" },
{ "SourceAddress",	0, 0, 
"# Use the numeric IP address of the incoming connection instead of the\n"
"# host name. Use this on multihomed hosts. Default is false\n" },
{ "Willing",		F_FILE755, def_willing, 
"# The program which is invoked to dynamically generate replies to XDMCP\n"
"# BroadcastQuery requests.\n"
"# By default no program is invoked and \"Willing to manage\" is sent.\n" },
};

static Ent entsShutdown[] = {
{ "HaltCmd",		0, 0, 
"# The command to run to halt the system. Default is " HALT_CMD "\n" },
{ "RebootCmd",		0, 0, 
"# The command to run to reboot the system. Default is " REBOOT_CMD "\n" },
{ "AllowFifo",		0, 0, 
"# Whether one can shut down the system via the global command FiFo.\n"
"# Default is false\n" },
#if defined(__linux__) && defined(__i386__)
{ "UseLilo",		0, 0, 
"# Offer LiLo boot options in shutdown dialog. Default is false\n" },
{ "LiloCmd",		0, 0, 
"# The location of the LiLo binary. Default is /sbin/lilo\n" },
{ "LiloMap",		0, 0, 
"# The location of the LiLo map file. Default is /boot/map\n" },
#endif
};

static Ent entsCore[] = {
{ "OpenDelay",		0, 0, 
"# How long to wait before retrying to start the display after various\n"
"# errors. Default is 15\n" },
{ "OpenTimeout",	0, 0, 
"# How long to wait before timing out XOpenDisplay. Default is 120\n" },
{ "OpenRepeat",		0, 0, 
"# How often to try the XOpenDisplay. Default is 5\n" },
{ "StartAttempts",	0, 0, 
"# Try at most that many times to start a display. If this fails, the display\n"
"# is disabled. Default is 4\n" },
{ "StartInterval",	0, 0, 
"# The StartAttempt counter is reset after that many seconds. Default is 30\n" },
{ "PingInterval",	0, 0, 
"# Ping remote display every that many minutes. Default is 5\n" },
{ "PingTimeout",	0, 0, 
"# Wait for a Pong that many minutes. Default is 5\n" },
{ "TerminateServer",	0, 0, 
"# Restart instead of resetting the local X-server after session exit.\n"
"# Use it if the server leaks memory, etc. Default is false\n" },
{ "ResetSignal",	0, 0, 
"# The signal needed to reset the local X-server. Default is 1 (SIGHUP)\n" },
{ "TermSignal",		0, 0, 
"# The signal needed to terminate the local X-server. Default is 15 (SIGTERM)\n" },
{ "ResetForAuth",	0, 0, 
"# Need to reset the X-server to make it read initial Xauth file.\n"
"# Default is false\n" },
{ "Authorize",		0, 0, 
"# Create X-authorizations for local displays. Default is true\n" },
{ "AuthNames",		0, 0, 
"# Which X-authorization mechanisms should be used.\n"
"# Default is " DEF_AUTH_NAME "\n" },
{ "AuthFile",		0, 0, 
"# The name of this X-server's Xauth file. Default is \"\", which means, that\n"
"# a random name in the AuthDir directory will be used.\n" },
{ "Resources",		F_FILE644, 0, 
"# Specify a file with X-resources for the greeter, chooser and background.\n"
"# The KDE frontend doesn't care for this, so you don't need it unless you\n"
"# use an alternative chooser or another background generator than kdmdesktop.\n"
"# Default is \"\"\n" },
{ "Xrdb",		0, 0, 
"# The xrdb program to use to read the above specified recources.\n"
"# Default is " XBINDIR "/xrdb\n" },
{ "Setup",		F_FILE755, def_setup, 
"# A program to run before the greeter is shown. You should start kdmdesktop\n"
"# there. Also, xconsole can be started by this script.\n"
"# Default is \"\"\n" },
{ "Startup",		F_FILE755, def_startup, 
"# A program to run before a user session starts. You should invoke sessreg\n"
"# there and optionally change the ownership of the console, etc.\n"
"# Default is \"\"\n" },
{ "Reset",		F_FILE755, def_reset, 
"# A program to run after a user session exits. You should invoke sessreg\n"
"# there and optionally change the ownership of the console, etc.\n"
"# Default is \"\"\n" },
{ "Session",		F_FILE755, def_session, 
"# The program which is run as the user which logs in. It is supposed to\n"
"# interpret the session argument (see SessionTypes) and start an appropriate\n"
"# session according to it.\n"
"# Default is " XBINDIR "/xterm -ls -T\n" },
{ "FailsafeClient",	0, 0, 
"# The program to run if Session fails.\n"
"# Default is " XBINDIR "/xterm\n" },
{ "UserPath",		F_PATH, DEF_USER_PATH, 
"# The PATH for the Session program. Default is\n"
"# " DEF_USER_PATH "\n" },
{ "SystemPath",		F_PATH, DEF_SYSTEM_PATH, 
"# The PATH for Setup, Startup and Reset, etc. Default is\n"
"# " DEF_SYSTEM_PATH "\n" },
{ "SystemShell",	0, 0, 
"# The default system shell. Default is /bin/sh\n" },
{ "UserAuthDir",	0, 0, 
"# Where to put the user's X-server authorization file if ~/.Xauthority\n"
"# cannot be created. Default is /tmp\n" },
{ "Chooser",		0, 0, 
"# The host chooser program to use.\n"
"# Default is " KDE_BINDIR "/chooser\n"
"# XXX this is going to be integrated into the greeter (probably).\n" },
{ "AutoReLogin",	0, 0, 
"# If \"true\", KDM will automatically restart a session after an X-server\n"
"# crash (or if it is killed by Alt-Ctrl-BackSpace). Note, that enabling\n"
"# this opens a security hole: a secured display lock can be circumvented.\n"
"# Default is false\n" },
{ "AllowRootLogin",	0, 0, 
"# Allow root logins? Default is true\n" },
{ "AllowNullPasswd",	0, 0, 
"# Allow to log in, when user has set an empty password? Default is true\n" },
{ "AllowShutdown",	0, 0, 
"# Who is allowed to shut down the system. This applies both to the\n"
"# greeter and to the command FiFo. Default is All\n"
"# \"None\" - no \"Shutdown...\" button is shown at all\n"
"# \"Root\" - the root password must be entered to shut down\n"
"# \"All\" - everybody can shut down the machine (Default)\n" },
{ "AllowSdForceNow",	0, 0, 
"# Who is allowed to abort all still running sessions when shutting down.\n"
"# Same options as for AllowShutdown. Default is All\n" },
{ "DefaultSdMode",	0, 0, 
"# The default choice for the shutdown condition/timing.\n"
"# \"Schedule\" - shutdown after all sessions exit (possibly at once) (Default)\n"
"# \"TryNow\" - shutdown, if no sessions are open, otherwise do nothing\n"
"# \"ForceNow\" - shutdown unconditionally\n" }, 
{ "SessSaveFile",	0, 0, 
"# Where (relatively to the user's home directory) to store the last\n"
"# selected session. Default is .wmrc\n" },
{ "ServerAttempts",	0, 0, 
"# How often to try to run the X-server. Running includes executing it and\n"
"# waiting for it to come up. Default is 1\n" },
{ "ServerTimeout",	0, 0, 
"# How long to wait for a local X-server to come up. Default is 15\n" },
{ "NoPassEnable",	0, 0, 
"# Enable password-less logins on this display. USE WITH EXTREME CARE!\n"
"# Default is false\n" },
{ "NoPassUsers",	0, 0, 
"# The users that don't need to provide a password to log in. NEVER list root!\n" },
{ "AutoLoginEnable",	0, 0, 
"# Enable automatic login on this display. USE WITH EXTREME CARE!\n"
"# Default is false\n" },
{ "AutoLoginUser",	0, 0, 
"# The user to log in automatically. NEVER specify root!\n" },
{ "AutoLoginPass",	0, 0, 
"# The password for the user to log in automatically. This is NOT required\n"
"# unless the user is to be logged into a NIS or Kerberos domain. If you use\n"
"# it, you must \"chmod 600 kdmrc\" for obvious reasons.\n" },
{ "AutoLoginSession",	0, 0, 
"# The session for the user to log in automatically. This becomes useless after\n"
"# the user's first login, as the last used session will take precedence.\n" },
{ "AutoLogin1st",	0, 0, 
"# If \"true\", the auto-login is truly automatic, i.e., the user is logged in\n"
"# when KDM comes up. If \"false\", the auto-login must be initiated by crashing\n"
"# the X-server with Alt-Ctrl-BackSpace. Default is true\n" },
};

static Ent entsGreeter[] = {
{ "SessionTypes",	0, 0, 
"# Session types the users can select. It is advisable to have \"default\" and\n"
"# \"failsafe\" listed herein, which is also the default.\n"
"# Note, that the meaning of this value is entirely up to your Session program.\n" },
{ "GUIStyle",		0, 0, 
"# Widget Style of the greeter:\n"
"# KDE, Windows, Platinum, Motif, MotifPlus, CDE, SGI; Default is KDE\n" },
{ "LogoArea",		0, 0, 
"# What should be shown righthand of the input lines:\n"
"# \"Logo\" - the image specified by LogoPixmap (Default)\n"
"# \"Clock\" - a neat analog clock\n"
"# \"None\" - nothing\n" },
{ "LogoPixmap",		0, 0, 
"# The image to show when LogoArea=Logo. Default is kdelogo.png\n" },
{ "GreeterPosFixed",	0, 0, 
"# Normally, the greeter is centered on the screen. Use this, if you want\n"
"# it to appear elsewhere on the screen. Default is false\n" },
{ "GreeterPosX",	0, 0, 0 },
{ "GreeterPosY",	0, 0, 0 },
{ "GreeterScreen",	0, 0,
"# The screen the greeter should be displayed on in multi-headed setups.\n"
"# Default is 0\n" },
{ "GreetString",	0, 0, 
"# The headline in the greeter.\n"
"# The following character pairs are replaced:\n"
"# - %d -> current display\n"
"# - %h -> host name, possibly with domain name\n"
"# - %n -> node name, most probably the host name without domain name\n"
"# - %s -> the operating system\n"
"# - %r -> the operating system's version\n"
"# - %m -> the machine (hardware) type\n"
"# - %% -> a single %\n"
"# Default is \"Welcome to %s at %n\"\n" },
{ "GreetFont",		0, 0, 
"# The font for the headline. Default is charter,24,bold\n" },
{ "StdFont",		0, 0, 
"# The normal font used in the greeter. Default is helvetica,12\n" },
{ "FailFont",		0, 0, 
"# The font used for the \"Login Failed\" message. Default is helvetica,12,bold\n" },
{ "Language",		0, 0, 
"# Language to use in the greeter.\n"
"# Use the default C or coutry codes like de, en, pl, etc.\n" },
{ "ShowUsers",		0, 0, 
"# Specify, which user names (along with pictures) should be shown in the\n"
"# greeter.\n"
"# \"NotHidden\" - all users except those listed in HiddenUsers (Default)\n"
"# \"Selected\" - only the users listed in SelectedUsers\n"
"# \"None\" - no user list will be shown at all\n" },
{ "SelectedUsers",	0, 0, 
"# For ShowUsers=Selected. Default is \"\"\n" },
{ "HiddenUsers",	0, 0, 
"# For ShowUsers=NotHidden. Default is \"\"\n" },
{ "MinShowUID",		0, 0, 
"# Special case of HiddenUsers: users with a UID less than this number\n"
"# (except root) will not be shown as well. Default is 0\n" },
{ "MaxShowUID",		0, 0, 
"# Complement to MinShowUID: users with a UID greater than this number will\n"
"# not be shown as well. Default is 65535\n" },
{ "SortUsers",		0, 0, 
"# If false, the users are listed in the order they appear in /etc/passwd.\n"
"# If true, they are sorted alphabetically. Default is true\n" },
{ "PreselectUser",	0, 0, 
"# Specify, if/which user should be preselected for log in.\n"
"# Note, that enabling this feature can be considered a security hole,\n"
"# as it presents a valid login name to a potential attacker, so he \"only\"\n"
"# needs to guess the password.\n"
"# \"None\" - don't preselect any user (Default)\n"
"# \"Previous\" - the user which successfully logged in last time\n"
"# \"Default\" - the user specified in the DefaultUser field\n" },
{ "DefaultUser",	0, 0, 
"# The user to preselect if PreselectUser=Default\n" },
{ "FocusPasswd",	0, 0, 
"# If this is true, the password input line is focused automatically if\n"
"# a user is preselected. Default is false\n" },
{ "EchoMode",		0, 0, 
"# The password input fields cloak the typed in text. Specify, how to do it:\n"
"# \"NoEcho\" - nothing is shown at all, the cursor doesn't move\n"
"# \"OneStar\" - \"*\" is shown for every typed letter (Default)\n"
"# \"ThreeStars\" - \"***\" is shown for every typed letter\n" },
{ "GrabServer",		0, 0, 
"# Hold the X-server grabbed the whole time the greeter is visible.\n"
"# This may be more secure, but will disable any background. Default is false\n" },
{ "GrabTimeout",	0, 0, 
"# How many seconds to wait for grab to succeed. Default is 3\n" },
{ "AuthComplain",	0, 0, 
"# Warn, if local X-authorization cannot be created. Default is true\n"
"# XXX this is a dummy currently\n" },
};

typedef struct Sect {
	const char	*name;
	Ent		*ents;
	int		nents;
} Sect;

static Sect allSects[] = { 
{ "Desktop0",		entsDesktop, as(entsDesktop) },
{ "General",		entsGeneral, as(entsGeneral) },
{ "Xdmcp",		entsXdmcp, as(entsXdmcp) },
{ "Shutdown",		entsShutdown, as(entsShutdown) },
{ "-Core",		entsCore, as(entsCore) },
{ "-Greeter",		entsGreeter, as(entsGreeter) },
};

typedef struct DEnt {
	const char	*key;
	const char	*value;
	int		active;
} DEnt;

static DEnt dEntsDesktop[] = {
{ "BackgroundMode",	"Wallpaper", 1 },
{ "BlendBalance",	"100", 1 },
{ "BlendMode",		"NoBlending", 1 },
{ "ChangeInterval",	"60", 1 },
{ "Color1",		"30,114,160", 1 },
{ "Color2",		"192,192,192", 1 },
{ "CurrentWallpaper",	"0", 1 },
{ "LastChange",		"0", 1 },
{ "MultiWallpaperMode",	"NoMulti", 1 },
{ "Pattern",		"", 1 },
{ "Program",		"", 1 },
{ "ReverseBlending",	"false", 1 },
{ "Wallpaper",		"default_blue.jpg", 1 },
{ "WallpaperList",	"", 1 },
{ "WallpaperMode",	"Scaled", 1 },
};

static DEnt dEntsGeneral[] = {
{ "DaemonMode",		"false", 0 },
{ "Xservers",		KDMCONF "/Xservers", 1 },
{ "PidFile",		"/var/run/kdm.pid", 1 },
{ "LockPidFile",	"false", 0 },
{ "AuthDir",		"/tmp", 0 },
{ "AutoRescan",		"false", 0 },
{ "ExportList",		"SOME_VAR,ANOTHER_IMPORTANT_VAR", 0 },
#if !defined(__linux__) && !defined(__OpenBSD__)
{ "RandomFile",		"", 0 },
#endif
{ "FifoDir",		"/tmp", 0 },
{ "FifoGroup",		"xdmctl", 0 },
};

DEnt dEntsXdmcp[] = {
{ "Enable",		"false", 1 },
{ "Port",		"177", 0 },
{ "KeyFile",		"", 0 },
{ "Xaccess",		KDMCONF "/Xaccess", 1 }, /* b/c of inadequate def handling */
{ "ChoiceTimeout",	"10", 0 },
{ "RemoveDomainname",	"false", 0 },
{ "SourceAddress",	"true", 0 },
{ "Willing",		KDMCONF "/Xwilling", 1 },
};

static DEnt dEntsShutdown[] = {
{ "HaltCmd",		"", 0 },
{ "RebootCmd",		"", 0 },
{ "AllowFifo",		"true", 0 },
#if defined(__linux__) && defined(__i386__)
{ "UseLilo",		"true", 0 },
{ "LiloCmd",		"", 0 },
{ "LiloMap",		"", 0 },
#endif
};

static DEnt dEntsAnyCore[] = {
{ "OpenDelay",		"", 0 },
{ "OpenTimeout",	"", 0 },
{ "OpenRepeat",		"", 0 },
{ "StartAttempts",	"", 0 },
{ "StartInterval",	"", 0 },
{ "PingInterval",	"", 0 },
{ "PingTimeout",	"", 0 },
{ "TerminateServer",	"true", 0 },
{ "ResetSignal",	"", 0 },
{ "TermSignal",		"", 0 },
{ "ResetForAuth",	"true", 0 },
{ "Authorize",		"false", 0 },
{ "AuthNames",		"", 0 },
{ "AuthFile",		"", 0 },
{ "Resources",		"", 0 },
{ "Xrdb",		"", 0 },
{ "Setup",		KDMCONF "/Xsetup", 1 },
{ "Startup",		KDMCONF "/Xstartup", 1 },
{ "Reset",		KDMCONF "/Xreset", 1 },
{ "Session",		KDMCONF "/Xsession", 1 },
{ "FailsafeClient",	"", 0 },
{ "UserPath",		"", 0 },
{ "SystemPath",		"", 0 },
{ "SystemShell",	"/bin/bash", 0 },
{ "UserAuthDir",	"", 0 },
{ "Chooser",		"", 0 },
{ "AutoReLogin",	"true", 0 },
{ "AllowRootLogin",	"false", 1 },
{ "AllowNullPasswd",	"false", 1 },
{ "AllowShutdown",	"Root", 1 },
{ "AllowSdForceNow",	"Root", 0 },
{ "DefaultSdMode",	"ForceNow", 0}, 
{ "SessSaveFile",	"", 0 },
};

static DEnt dEntsAnyGreeter[] = {
{ "SessionTypes",	"default,kde,failsafe", 1 },
{ "GUIStyle",		"Windows", 0 },
{ "LogoArea",		"None", 0 },
{ "LogoPixmap",		"", 0 },
{ "GreeterPosFixed",	"true", 0 },
{ "GreeterPosX",	"200", 0 },
{ "GreeterPosY",	"100", 0 },
{ "GreetString",	"K Desktop Environment (%n)", 0 },
{ "GreetFont",		"charter,24,5,0,50,0", 0 },
{ "StdFont",		"helvetica,12,5,0,50,0", 0 },
{ "FailFont",		"helvetica,12,5,0,75,0", 0 },
{ "Language",		"de", 0 },
{ "ShowUsers",		"None", 0 },
{ "SelectedUsers",	"root,johndoe", 0 },
{ "HiddenUsers",	"adm,alias,amanda,apache,bin,bind,daemon,exim,falken,ftp,games,gdm,gopher,halt,httpd,ident,ingres,kmem,lp,mail,mailnull,man,mta,mysql,named,news,nfsnobody,nobody,nscd,ntp,operator,pcap,pop,postfix,postgres,qmaild,qmaill,qmailp,qmailq,qmailr,qmails,radvd,reboot,rpc,rpcuser,rpm,sendmail,shutdown,squid,sympa,sync,tty,uucp,xfs,xten", 1 },
{ "MinShowUID",		"1000", 0 },
{ "MaxShowUID",		"29999", 0 },
{ "SortUsers",		"false", 0 },
{ "PreselectUser",	"Previous", 0 },
{ "DefaultUser",	"ethel", 0 },
{ "FocusPasswd",	"true", 0 },
{ "EchoMode",		"NoEcho", 0 },
{ "GrabServer",		"true", 0 },
{ "GrabTimeout",	"", 0 },
};

static DEnt dEntsLocalCore[] = {
{ "ServerAttempts",	"", 0 },
{ "ServerTimeout",	"", 0 },
{ "AllowShutdown",	"All", 1 },
{ "AllowRootLogin",	"true", 1 },
{ "AllowNullPasswd",	"true", 1 },
{ "NoPassEnable",	"true", 0 },
{ "NoPassUsers",	"fred,ethel", 0 },
};

static DEnt dEntsLocalGreeter[] = {
{ "AuthComplain",	"false", 0 },
{ "GreeterScreen",	"1", 0 },
};

static DEnt dEnts0Core[] = {
{ "AutoLoginEnable",	"true", 0 },
{ "AutoLoginUser",	"fred", 0 },
{ "AutoLoginPass",	"secret!", 0 },
{ "AutoLoginSession",	"kde", 0 },
{ "AutoLogin1st",	"false", 0 },
};

static DEnt dEnts0Greeter[] = {
{ "PreselectUser",	"Default", 0 },
{ "DefaultUser",	"johndoe", 0 },
};

typedef struct DSect {
	const char	*name;
	DEnt		*ents;
	int		nents;
	int		active;
	const char	*comment;
} DSect;

static DSect dAllSects[] = { 
{ "Desktop0",		dEntsDesktop, as(dEntsDesktop), 1, 
"# KDM configuration example.\n"
"# Note, that all comments will be lost if you change this file with\n"
"# the kcontrol frontend.\n"
"#\n"
"# Definition: the greeter is the login dialog, i.e., the part of KDM\n"
"# which the user sees.\n"
"#\n"
"# You can configure every X-display individually.\n"
"# Every display has a display name, which consists of a host name\n"
"# (which is empty for local displays), a colon and a display number.\n"
"# Additionally, a display belongs to a display class (which can be\n"
"# ignored in most cases; the kcontrol config frontend does not support\n"
"# this feature at all).\n"
"# Sections with display-specific settings have the formal syntax\n"
"# \"[X-\" host [\":\" number [ \"_\" class ]] \"-\" sub-section \"]\"\n"
"# You can use the \"*\" wildcard for host, number and class. You may omit\n"
"# trailing components; they are assumed to be \"*\" then.\n"
"# The host part may be a domain specification like \".inf.tu-dresden.de\".\n"
"# From which section a setting is actually taken is determined by these\n"
"# rules:\n"
"# - an exact match takes precedence over a partial match (for the host part),\n"
"#   which in turn takes precedence over a wildcard\n"
"# - precedence decreases from left to right for equally exact matches\n"
"# Example: display name \"myhost:0\", class \"dpy\".\n"
"# [X-myhost:0_dpy] precedes\n"
"# [X-myhost:0_*] (same as [X-myhost:0]) precedes\n"
"# [X-myhost:*_dpy] precedes\n"
"# [X-myhost:*_*] (same as [X-myhost]) precedes\n"
"# [X-*:0_dpy] precedes\n"
"# [X-*:0_*] (same as [X-*:0]) precedes\n"
"# [X-*:*_*] (same as [X-*])\n"
"# These sections do NOT match this display:\n"
"# [X-hishost], [X-myhost:0_dec], [X-*:1], [X-:*]\n"
"# If a setting is not found in any matching section, the default is used.\n"
"#\n"
"# Every comment applies to the following section or key.\n"
"#\n"
"\n"
"# Greeter background\n"
"# XXX do this on a per-display basis: should be [X-*-Desktop]\n"
"# HELPME: I need help with kbackgroundrenderer!  -- ossi@kde.org\n" },
{ "General",		dEntsGeneral, as(dEntsGeneral), 1, 0 },
{ "Xdmcp",		dEntsXdmcp, as(dEntsXdmcp), 1, 0 },
{ "Shutdown",		dEntsShutdown, as(dEntsShutdown), 1, 0 },
{ "X-*-Core",		dEntsAnyCore, as(dEntsAnyCore), 1, 
"# Rough estimations about how many seconds KDM will spend at most on\n"
"# - opening a connection to the X-server (OpenTime):\n"
"#   OpenRepeat * (OpenTimeout + OpenDelay)\n"
"# - starting a local X-server (SeverTime): ServerAttempts * ServerTimeout\n"
"# - starting a display:\n"
"#   - local display: StartAttempts * (ServerTime + OpenTime)\n"
"#   - remote/foreign display: StartAttempts * OpenTime\n\n"
"# Core config for all displays\n" },
{ "X-*-Greeter",	dEntsAnyGreeter, as(dEntsAnyGreeter), 1, 
"# Greeter config for all displays\n" },
{ "X-:*-Core",		dEntsLocalCore, as(dEntsLocalCore), 1, 
"# Core config for local displays\n" },
{ "X-:*-Greeter",	dEntsLocalGreeter, as(dEntsLocalGreeter), 1, 
"# Greeter config for local displays\n" },
{ "X-:0-Core",		dEnts0Core, as(dEnts0Core), 1, 
"# Core config for 1st local display\n" },
{ "X-:0-Greeter",	dEnts0Greeter, as(dEnts0Greeter), 1, 
"# Greeter config for 1st local display\n" },
};


/*
 * target data to be written to kdmrc
 */

typedef struct Entry {
	struct Entry	*next;
	Ent		*spec;
	const char	*value;
	int		active:1;
	int		written:1;
} Entry;

typedef struct Section {
	struct Section	*next;
	Sect		*spec;
	const char	*name;
	const char	*comment;
	Entry		*ents;
	int		active;
} Section;

static Section *config;

static const char *
getfqval (const char *sect, const char *key, const char *defval)
{
    Section *cs;
    Entry *ce;

    for (cs = config; cs; cs = cs->next)
	if (!strcmp (cs->name, sect)) {
	    if (cs->active)
		for (ce = cs->ents; ce; ce = ce->next)
		    if (!strcmp (ce->spec->key, key)) {
			if (ce->active && ce->written)
			    return ce->value;
			break;
		    }
	    break;
	}
    return defval;
}

static Sect *
findSect (const char *name)
{
    const char *p;
    int i;

    p = strrchr (name, '-');
    if (!p)
	p = name;
    for (i = 0; i < as(allSects); i++)
	if (!strcmp (allSects[i].name, p))
	    return allSects + i;
    fprintf (stderr, "Internal error: unknown section %s\n", name);
    exit(1);
}

static Ent *
findEnt (Sect *sect, const char *key)
{
    int i;

    for (i = 0; i < sect->nents; i++)
	if (!strcmp (sect->ents[i].key, key))
	    return sect->ents + i;
    fprintf (stderr, "Internal error: unknown key %s in section %s\n", 
	     key, sect->name);
    exit(1);
}

static void
putfqval (const char *sect, const char *key, const char *value)
{
    Section *cs, **csp;
    Entry *ce, **cep;

    if (!value)
	return;

    for (csp = &config; (cs = *csp); csp = &(cs->next))
	if (!strcmp(sect, cs->name))
	    goto havesec;
    if (!(cs = calloc (1, sizeof(*cs)))) {
	fprintf (stderr, "Out of memory\n");
	return;
    }
    ASPrintf ((char **)&cs->name, "%s", sect);
    if (!cs->name) {
	free (cs);
	fprintf (stderr, "Out of memory\n");
	return;
    }
    cs->spec = findSect (sect);
    *csp = cs;
  havesec:
    cs->active = 1;

    for (cep = &(cs->ents); (ce = *cep); cep = &(ce->next))
	if (!strcmp(key, ce->spec->key))
	    goto haveent;
    if (!(ce = calloc (1, sizeof(*ce)))) {
	fprintf (stderr, "Out of memory\n");
	return;
    }
    ce->spec = findEnt (cs->spec, key);
    *cep = ce;
  haveent:
    ASPrintf ((char **)&ce->value, "%s", value);
    if (!ce->value) {
	*cep = ce->next;
	free (ce);
	fprintf (stderr, "Out of memory\n");
	return;
    }
    ce->written = ce->active = 1;
}

static const char *csect;

#define setsect(se) csect = se

static void
putval (const char *key, const char *value)
{
    putfqval(csect, key, value);
}


static void
mkdefconf (void)
{
    Section *cs, **csp;
    Entry *ce, **cep;
    int sc, ec;

    for (csp = &config, sc = 0; sc < as(dAllSects); csp = &(cs->next), sc++) {
	if (!(cs = calloc (1, sizeof(*cs)))) {
	    fprintf (stderr, "Out of memory\n");
	    return;
	}
	*csp = cs;
	cs->spec = findSect (dAllSects[sc].name);
	cs->name = dAllSects[sc].name;
	cs->comment = dAllSects[sc].comment;
	cs->active = dAllSects[sc].active;
	for (cep = &(cs->ents), ec = 0; ec < dAllSects[sc].nents; 
	     cep = &(ce->next), ec++) {
	    if (!(ce = calloc (1, sizeof(*ce)))) {
		fprintf (stderr, "Out of memory\n");
		return;
	    }
	    *cep = ce;
	    ce->spec = findEnt (cs->spec, dAllSects[sc].ents[ec].key);
	    ce->value = dAllSects[sc].ents[ec].value;
	    ce->active = dAllSects[sc].ents[ec].active;
	}
    }
}


static FILE *
Create (const char *fn, int mode)
{
    FILE *f;
    if (!(f = fopen (fn, "w"))) {
	fprintf (stderr, "Cannot create %s\n", fn);
	exit (1);
    }
    chmod (fn, mode);
    return f;
}


static void 
usedFile (StrList **flist, const char *fn)
{
    StrList **sp;

    for (sp = flist; *sp; sp = &(*sp)->next)
	if (!strcmp ((*sp)->str, fn))
	    return;
    if (!(*sp = calloc (1, sizeof(**sp))))
	fprintf (stderr, "Out of memory\n");
    else {
	ASPrintf ((char **)&(*sp)->str, "%s", fn);
	if (!(*sp)->str) {
	    free (*sp);
	    *sp = 0;
	    fprintf (stderr, "Out of memory\n");
	}
    }
}

StrList *cflist, *lflist;

static void 
copiedFile (const char *fn)
{
    usedFile (&cflist, fn);
}

static void 
linkedFile (const char *fn)
{
    usedFile (&lflist, fn);
}

static void 
handFile (Entry *ce)
{
    char *buf, *bname;
    const char *fname;
    FILE *f;
    File file;
    int mode;
    struct stat st;
    char nname[160];

    if (ce->spec->flags & F_FILE755) {
	if (!(fname = strchr (ce->value, '/')))
	    return;
	mode = 0755;
    } else {
	fname = ce->value;
	mode = 0644;
    }
    if (!(bname = strrchr (fname, '/')))
	return;
    sprintf (nname, "%s/%s", newdir, bname + 1);

    if (stat (fname, &st)) {
	if (ce->written)
	    fprintf (stderr, "Warning: file %s not found\n", fname);
	if (ce->spec->param) {
	    f = Create (nname, mode);
	    fputs (ce->spec->param, f);
	    fclose (f);
	}
    } else if (strcmp (nname, fname)) {
	if (copy_files) {
	    if (!readFile (&file, fname)) {
		fprintf (stderr, "Warning: cannot copy file %s\n", ce->value);
		return;
	    }
	    copiedFile (fname);
	    f = Create (nname, mode);
	    if (mode == 0755 && (file.buf[0] != '#' || file.buf[1] != '!'))
		fwrite (file.buf, file.eof - file.buf, 1, f);
	    else {
		*bname = 0;
		*file.eof = 0;
		buf = sed (file.buf, fname, KDMCONF);
		fputs (buf, f);
		free (buf);
	    }
	    fclose (f);
	} else {
	    linkedFile (fname);
	    return;
	}
    } else
	return;
    ASPrintf ((char **)&ce->value, "%.*s" KDMCONF "/%s", 
	      fname - ce->value, ce->value, bname + 1);
}

static void
addKdePath (Entry *ce)
{
    char *p;
    const char *path;

    path = ce->active ? ce->value : ce->spec->param;
    if (!(p = strstr (path, KDE_BINDIR)) ||
        (p != path && *(p-1) != ':') ||
        (p[sizeof(KDE_BINDIR)-1] && p[sizeof(KDE_BINDIR)-1] != ':'))
	ASPrintf ((char **)&ce->value, KDE_BINDIR ":%s", path);
}

static void
wrconf (FILE *f)
{
    Section *cs;
    Entry *ce;
    StrList *sl = 0, *sp;
    const char *cmt;

    for (cs = config; cs; cs = cs->next) {
	fprintf (f, "%s%s[%s]\n",
		 cs->comment ? cs->comment : "",
		 cs->active ? "" : "#", cs->name);
	for (ce = cs->ents; ce; ce = ce->next) {
	    if (ce->spec->flags & (F_FILE644 | F_FILE755))
		handFile (ce);
	    else if (ce->spec->flags & F_PATH)
		addKdePath (ce);
	    if (ce->spec->comment) {
		cmt = ce->spec->comment;
		for (sp = sl; sp; sp = sp->next)
		    if (sp->str == cmt) {
			cmt = "# See above\n";
			goto havit;
		    }
		if (!(sp = malloc (sizeof(*sp))))
		    fprintf (stderr, "Out of memory\n");
		else {
		    sp->str = cmt;
		    sp->next = sl; sl = sp;
		}
	    } else
		cmt = "";
	  havit:
	    fprintf (f, "%s%s%s=%s\n", 
		     cmt, (cs->active && ce->active) ? "" : "#", ce->spec->key, 
		     ce->value);
	}
	fprintf (f, "\n");
    }
}


/*
static void
writeFile (const char *fsp, int mode, const char *fmt, ...)
{
    va_list args;
    char *fn, *buf;
    FILE *f;
    int len;

    ASPrintf (&fn, fsp, newdir);
    f = Create (fn, mode);
    free (fn);
    va_start(args, fmt);
    len = VASPrintf (&buf, fmt, args);
    va_end(args);
    fwrite (buf, len, 1, f);
    free (buf);
    fclose (f);
}
*/

/*
 * read rc file structure
 */

typedef struct REntry {
	struct REntry	*next;
	char		*key;
	char		*value;
} REntry;

typedef struct RSection {
	struct RSection	*next;
	char		*name;
	REntry		*ents;
} RSection;

static RSection *
ReadConf (const char *fname)
{
    char *nstr;
    char *s, *e, *st, *en, *ek, *sl;
    RSection *rootsec = 0, *cursec;
    REntry *curent;
    int nlen;
    int line, sectmoan;
    File file;

    if (!readFile (&file, fname))
	return 0;
    copiedFile (fname);

    for (s = file.buf, line = 0, cursec = 0, sectmoan = 1; s < file.eof; s++) {
	line++;

	while ((s < file.eof) && isspace (*s) && (*s != '\n'))
	    s++;

	if ((s < file.eof) && ((*s == '\n') || (*s == '#'))) {
	  sktoeol:
	    while ((s < file.eof) && (*s != '\n'))
		s++;
	    continue;
	}
	sl = s;

	if (*s == '[') {
	    while ((s < file.eof) && (*s != '\n'))
		s++;
	    e = s - 1;
	    while ((e > sl) && isspace (*e))
		e--;
	    if (*e != ']') {
		fprintf (stderr, "Invalid section header at %s:%d\n", 
			 fname, line);
		continue;
	    }
	    sectmoan = 0;
	    nstr = sl + 1;
	    nlen = e - nstr;
	    for (cursec = rootsec; cursec; cursec = cursec->next)
		if (!memcmp (nstr, cursec->name, nlen) &&
		    !cursec->name[nlen])
		{
		    fprintf (stderr, "Warning: Multiple occurences of section "
				     "[%.*s] in %s. Consider merging them.\n", 
			     nlen, nstr, fname);
		    goto secfnd;
		}
	    if (!(cursec = malloc (sizeof(*cursec)))) {
		fprintf (stderr, "Out of memory\n");
		return 0;
	    }
	    ASPrintf (&cursec->name, "%.*s", nlen, nstr);
	    cursec->ents = 0;
	    cursec->next = rootsec;
	    rootsec = cursec;
	  secfnd:
	    continue;
	}

	if (!cursec) {
	    if (sectmoan) {
		sectmoan = 0;
		fprintf (stderr, "Entry outside any section at %s:%d",
			 fname, line);
	    }
	    goto sktoeol;
	}

	for (; (s < file.eof) && (*s != '\n'); s++)
	    if (*s == '=')
		goto haveeq;
	fprintf (stderr, "Invalid entry (missing '=') at %s:%d\n", fname, line);
	continue;

      haveeq:
	for (ek = s - 1;; ek--) {
	    if (ek < sl) {
		fprintf (stderr, "Invalid entry (empty key) at %s:%d\n", 
			 fname, line);
		goto sktoeol;
	    }
	    if (!isspace (*ek))
		break;
	}

	s++;
	while ((s < file.eof) && isspace(*s) && (*s != '\n'))
	    s++;
	st = s;
	while ((s < file.eof) && (*s != '\n'))
	    s++;
	for (en = s - 1; en >= st && isspace (*en); en--);

	nstr = sl;
	nlen = ek - sl + 1;
	for (curent = cursec->ents; curent; curent = curent->next)
	    if (!memcmp (nstr, curent->key, nlen) &&
		!curent->key[nlen]) {
		fprintf (stderr, "Multiple occurences of key '%s' in section "
			 "[%s] of %s.\n", curent->key, cursec->name, fname);
		goto keyfnd;
	    }
	if (!(curent = malloc (sizeof (*curent)))) {
	    fprintf (stderr, "Out of memory\n");
	    return 0;
	}
	ASPrintf( &curent->key, "%.*s", nlen, nstr);
	ASPrintf( &curent->value, "%.*s", en - st + 1, st);
	curent->next = cursec->ents;
	cursec->ents = curent;
      keyfnd:
	continue;
    }
    return rootsec;
}


static RSection *rootsect, *cursect;

static int
cfgRead (const char *fn)
{
    rootsect = ReadConf (fn);
    return rootsect != 0;
}

static int
cfgSGroup (const char *name)
{
    for (cursect = rootsect; cursect; cursect = cursect->next)
	if (!strcmp (cursect->name, name))
	    return 1;
    return 0;
}

static char *
cfgEnt (const char *key)
{
    REntry *ce;
    for (ce = cursect->ents; ce; ce = ce->next)
	if (!strcmp (ce->key, key))
	    return ce->value;
    return 0;
}

static void
cpyval (const char *nk, const char *ok)
{
    putval (nk, cfgEnt (ok ? ok : nk));
}

static void
cpyfqval (const char *sect, const char *nk, const char *ok)
{
    putfqval (sect, nk, cfgEnt (ok ? ok : nk));
}

static void
cpygents (Sect *sect)
{
    int i;

    for (i = 0; i < sect->nents; i++)
	cpyval (sect->ents[i].key, 0);
}

static void
cpygroup (Sect *sect)
{
    if (cfgSGroup (sect->name)) {
	setsect(sect->name);
	cpygents (sect);
    }
}

static int
isTrue (const char *val)
{
    return !strcmp(val, "true") || 
	   !strcmp(val, "yes") || 
	   !strcmp(val, "on") || 
	   !strcmp(val, "0");
}

/* the mergeKdmRc* functions should be data-driven */

static int
mergeKdmRcOld (const char *path)
{
    char *p, *p2;

    ASPrintf (&p, "%s/kdmrc", path);
    if (!p)
	return 0;
    if (!cfgRead(p)) {
	free (p);
	return 0;
    }
    printf ("Information: reading old kdmrc %s (from kde < 2.2)\n", p);
    free (p);

    setsect("Desktop0");
    if (cfgSGroup ("KDMDESKTOP")) {
	p = cfgEnt ("BackGroundPictureMode");
	if (!p || !strcmp(p, "None")) {
	    p = cfgEnt ("BackGroundColorMode");
	    if (!p || !strcmp(p, "Plain"))
		putval ("BackgroundMode", "Flat");
	    else if (!strcmp(p, "Vertical"))
		putval ("BackgroundMode", "VerticalGradient");
	    else if (!strcmp(p, "Horizontal"))
		putval ("BackgroundMode", "HorizontalGradient");
	    putval ("WallpaperMode", "NoWallpaper");
	} else {
	    if (!strcmp(p, "Tile"))
		putval ("WallpaperMode", "Tiled");
	    else if (!strcmp(p, "Scale"))
		putval ("WallpaperMode", "Scaled");
	    else
		putval ("WallpaperMode", "Centered");
	    putval ("BackgroundMode", "Wallpaper");
	}
	cpyval ("Wallpaper", "BackGroundPicture");
	cpyval ("Color1", "BackGroundColor1");
	cpyval ("Color2", "BackGroundColor2");
    }
    cpygroup (allSects);	/* Desktop0 */

    setsect ("X-*-Greeter");

    if (cfgSGroup ("Locale"))
	cpyval ("Language", 0);

    if (cfgSGroup ("KDM")) {
	if ((p = cfgEnt("GreetString"))) {
	    if ((p2 = strstr (p, "HOSTNAME"))) {
		strcpy (p2, "%n");
		strcpy (p2 + 2, p2 + 8);
	    }
	    putval ("GreetString", p);
	}
	if ((p = cfgEnt("EchoMode"))) {
	    if (!strcmp (p, "NoStars"))
		putval ("EchoMode", "NoEcho");
	    else
		putval ("EchoMode", p);
	}
	if ((p = cfgEnt("GUIStyle"))) {
	    if (!strcmp (p, "Motif+"))
		putval ("GUIStyle", "MotifPlus");
	    else
		putval ("GUIStyle", p);
	}
	cpyval ("StdFont", 0);
	cpyval ("GreetFont", 0);
	cpyval ("FailFont", 0);
	cpyval ("SessionTypes", 0);
	cpyval ("MinShowUID", "UserIDLow");
	cpyval ("MinShowUID", 0);
	cpyval ("SortUsers", 0);
	cpyval ("SelectedUsers", "Users");
	cpyval ("HiddenUsers", "NoUsers");
	if ((p = cfgEnt ("ShowUsers"))) {
	    if (!strcmp (p, "All"))
		putval ("ShowUsers", "NotHidden");
	    else
		putval ("ShowUsers", p);
	} else
	if ((p = cfgEnt ("UserView"))) {
	    if (isTrue (p)) {
		if (!(p = cfgEnt ("Users")) || !p[0])
		    putval ("ShowUsers", "NotHidden");
		else
		    putval ("ShowUsers", "Selected");
	    } else
		putval ("ShowUsers", "None");
	}
	if ((p = cfgEnt("LogoPixmap"))) {
	    if (!strcmp (p, "/dev/null"))
		putval ("LogoArea", "None");
	    else {
		putval ("LogoPixmap", p);
		putval ("LogoArea", "Logo");
	    }
	}
	if ((p = cfgEnt("LogoArea"))) {
	    if (!strcmp (p, "Logo") || !strcmp (p, "KdmLogo"))
		putval ("LogoArea", "Logo");
	    else if (!strcmp (p, "Clock") || !strcmp (p, "KdmClock"))
		putval ("LogoArea", "Clock");
	    else
		putval ("LogoArea", "None");
	}
	cpyval ("GreeterPosFixed", 0);
	cpyval ("GreeterPosX", 0);
	cpyval ("GreeterPosY", 0);
	if ((p = cfgEnt("ShowPrevious")))
	    putval ("PreselectUser", isTrue(p) ? "Previous" : "None");

	setsect ("Shutdown");
	cpyval ("HaltCmd", "Shutdown");
	cpyval ("RebootCmd", "Restart");

	setsect ("X-*-Core");
	cpyval ("AutoReLogin", 0);
	if (((p = cfgEnt("ShutDownButton"))) || 
	    ((p = cfgEnt("ShutdownButton")))) {
	    if (!strcmp (p, "All")) {
		putval ("AllowShutdown", "All");
		putfqval ("X-:*-Core", "AllowShutdown", "All");
	    } else if (!strcmp (p, "RootOnly")) {
		putval ("AllowShutdown", "Root");
		putfqval ("X-:*-Core", "AllowShutdown", "Root");
	    } else if (!strcmp (p, "ConsoleOnly")) {
		putval ("AllowShutdown", "None");
		putfqval ("X-:*-Core", "AllowShutdown", "All");
	    } else {
		putval ("AllowShutdown", "None");
		putfqval ("X-:*-Core", "AllowShutdown", "None");
	    }
	}

	setsect ("X-:*-Core");
	cpyval ("NoPassEnable", 0);
	cpyval ("NoPassUsers", 0);

	setsect ("X-:0-Core");
	cpyval ("AutoLoginEnable", 0);
	cpyval ("AutoLoginUser", 0);
	cpyval ("AutoLogin1st", 0);
    }

#if defined(__linux__) && defined(__i386__)
    if (cfgSGroup ("Lilo")) {
	setsect("Shutdown");
	cpyval ("UseLilo", "Lilo");
	cpyval ("LiloCmd", "LiloCommand");
        cpyval ("LiloMap", 0);
    }
#endif

    return 1;
}

static struct {
    const char *sect, *key, *def;
} chgdef[] = {
{ "General",	"Xservers",	"%s/kdm/Xservers"	},
{ "General",	"PidFile",	"/var/run/xdm.pid"	},
{ "Xdmcp",	"Xaccess",	"%s/kdm/Xaccess"	}, /* b/c of inadequate def handling */
{ "X-*-Core",	"Setup",	"%s/kdm/Xsetup"		},
{ "X-*-Core",	"Startup",	"%s/kdm/Xstartup"	},
{ "X-*-Core",	"Reset",	"%s/kdm/Xreset"		},
{ "X-*-Core",	"Session",	"%s/kdm/Xsession"	},
};

static int
mergeKdmRcNewer (const char *path)
{
    char *p, *p2;
    int i;

    ASPrintf (&p, "%s/kdm/kdmrc", path);
    if (!p)
	return 0;
    if (!cfgRead(p)) {
	free (p);
	return 0;
    }
    printf ("Information: reading old kdmrc %s (from kde >= 2.2.x)\n", p);
    free (p);

    for (i = 0; i < 4; i++)
	cpygroup (allSects + i);

    if (cfgSGroup ("Xdmcp"))
	cpyfqval ("Xdmcp", "Willing", "Xwilling");

    for (cursect = rootsect; cursect; cursect = cursect->next)
	if (!strncmp (cursect->name, "X-", 2)) {
	    setsect(cursect->name);
	    if ((p = strrchr (cursect->name, '-'))) {
		if (!strcmp (p + 1, "Core")) {
		    cpygents (allSects + 4);
		} else if (!strcmp (p + 1, "Greeter")) {
		    cpygents (allSects + 5);
		    ASPrintf (&p2, "%.*s-Core", 
			      p - cursect->name, cursect->name);
		    if (p2) {
			cpyfqval (p2, "AllowShutdown", 0);
			free (p2);
		    }
		    if ((p = cfgEnt("GUIStyle"))) {
			if (!strcmp (p, "Motif+"))
			    putval ("GUIStyle", "MotifPlus");
			else
			    putval ("GUIStyle", p);
		    }
		    if ((p = cfgEnt("ShowUsers"))) {
			if (!strcmp (p, "All"))
			    putval ("ShowUsers", "NotHidden");
			else
			    putval ("ShowUsers", p);
		    }
		    cpyval ("HiddenUsers", "NoUsers");
		    cpyval ("SelectedUsers", "Users");
		}
	    }
	}

    /*
     * workaround 2.2 default borkedness ...
     * XXX this (incorrectly) overrides 3.x defaults
     */
    for (i = 0; i < as(chgdef); i++)
	if (!getfqval (chgdef[i].sect, chgdef[i].key, 0)) {
	    ASPrintf (&p, chgdef[i].def, path);
	    if (p) {
printf("[%s] %s=%s\n", chgdef[i].sect, chgdef[i].key, p);
		putfqval (chgdef[i].sect, chgdef[i].key, p);
		free (p);
	    }
	}

    return 1;
}

typedef struct XResEnt {
    const char	*xname;
    const char	*ksec, *kname;
    void	(*func)(const char *, const char *, char **);
} XResEnt;

static void
handleXdmVal (const char *dpy, const char *key, char *value,
	      const XResEnt *ents, int nents)
{
    const char *kname;
    int i;
    char knameb[80], sname[80];

    for (i = 0; i < nents; i++)
	if (!strcmp (key, ents[i].xname) ||
	    (key[0] == toupper (ents[i].xname[0]) && 
	     !strcmp (key + 1, ents[i].xname + 1)))
	{
	    sprintf (sname, ents[i].ksec, dpy);
	    if (ents[i].kname)
		kname = ents[i].kname;
	    else {
		kname = knameb;
		sprintf (knameb, "%c%s", 
			 toupper (ents[i].xname[0]), ents[i].xname + 1);
	    }
	    if (ents[i].func)
		ents[i].func (sname, kname, &value);
	    putfqval (sname, kname, value);
	    break;
	}
}

static void 
P_List (const char *sect ATTR_UNUSED, const char *key ATTR_UNUSED, char **value)
{
    int is, d, s;
    char *st;

    for (st = *value, is = d = s = 0; st[s]; s++)
	if (st[s] == ' ' || st[s] == '\t') {
	    if (!is)
		st[d++] = ',';
	    is = 1;
	} else {
	    st[d++] = st[s];
	    is = 0;
	}
    st[d] = 0;
}

static void 
P_authDir (const char *sect ATTR_UNUSED, const char *key ATTR_UNUSED, char **value)
{
    int l;

    l = strlen (*value);
    if (l < 4) {
	*value = 0;
	return;
    }
    if ((*value)[l-1] == '/')
	(*value)[--l] = 0;
    if (!strncmp (*value, "/tmp/", 5) ||
	!strncmp (*value, "/var/tmp/", 9))
    {
	printf ("Warning: Resetting inappropriate value %s for AuthDir to default\n",
	        *value);
	*value = 0;
	return;
    }
    if ((l >= 4 && !strcmp (*value + l - 4, "/tmp")) ||
	(l >= 6 && !strcmp (*value + l - 6, "/xauth")) ||
	(l >= 8 && !strcmp (*value + l - 8, "/authdir")) ||
	(l >= 10 && !strcmp (*value + l - 10, "/authfiles")))
	return;
    ASPrintf (value, "%s/authdir", *value);
}

static void 
P_openDelay (const char *sect, const char *key ATTR_UNUSED, char **value)
{
    putfqval (sect, "ServerTimeout", *value);
}

static void 
P_noPassUsers (const char *sect, const char *key ATTR_UNUSED, char **value ATTR_UNUSED)
{
    putfqval (sect, "NoPassEnable", "true");
}

static void 
P_autoUser (const char *sect, const char *key ATTR_UNUSED, char **value ATTR_UNUSED)
{
    putfqval (sect, "AutoLoginEnable", "true");
}

static void 
P_requestPort (const char *sect, const char *key ATTR_UNUSED, char **value)
{
    if (!strcmp (*value, "0")) {
	*value = 0;
	putfqval (sect, "Enable", "false");
    } else
	putfqval (sect, "Enable", "true");
}

static int kdmrcmode = 0644;

static void 
P_autoPass (const char *sect ATTR_UNUSED, const char *key ATTR_UNUSED, char **value ATTR_UNUSED)
{
    kdmrcmode = 0600;
}

XResEnt globents[] = {
{ "servers", "General", "Xservers", 0 },
{ "requestPort", "Xdmcp", "Port", P_requestPort },
{ "daemonMode", "General", 0, 0 },
{ "pidFile", "General", 0, 0 },
{ "lockPidFile", "General", 0, 0 },
{ "authDir", "General", 0, P_authDir },
{ "autoRescan", "General", 0, 0 },
{ "removeDomainname", "Xdmcp", 0, 0 },
{ "keyFile", "Xdmcp", 0, 0 },
{ "accessFile", "Xdmcp", "Xaccess", 0 },
{ "exportList", "General", 0, P_List },
#if !defined(__linux__) && !defined(__OpenBSD__)
{ "randomFile", "General", 0, 0 },
#endif
{ "choiceTimeout", "Xdmcp", 0, 0 },
{ "sourceAddress", "Xdmcp", 0, 0 },
{ "willing", "Xdmcp", 0, 0 },
{ "autoLogin", "General", 0, 0 },
}, dpyents[] = {
{ "serverAttempts", "X-%s-Core", 0, 0 },
{ "openDelay", "X-%s-Core", 0, P_openDelay },
{ "openRepeat", "X-%s-Core", 0, 0 },
{ "openTimeout", "X-%s-Core", 0, 0 },
{ "startAttempts", "X-%s-Core", 0, 0 },
{ "pingInterval", "X-%s-Core", 0, 0 },
{ "pingTimeout", "X-%s-Core", 0, 0 },
{ "terminateServer", "X-%s-Core", 0, 0 },
{ "grabServer", "X-%s-Core", 0, 0 },
{ "grabTimeout", "X-%s-Core", 0, 0 },
{ "resetSignal", "X-%s-Core", 0, 0 },
{ "termSignal", "X-%s-Core", 0, 0 },
{ "resetForAuth", "X-%s-Core", 0, 0 },
{ "authorize", "X-%s-Core", 0, 0 },
{ "authComplain", "X-%s-Greeter", 0, 0 },
{ "authName", "X-%s-Core", "AuthNames", 0 },
{ "authFile", "X-%s-Core", 0, 0 },
{ "startInterval", "X-%s-Core", 0, 0 },
{ "resources", "X-%s-Core", 0, 0 },
{ "xrdb", "X-%s-Core", 0, 0 },
{ "setup", "X-%s-Core", 0, 0 },
{ "startup", "X-%s-Core", 0, 0 },
{ "reset", "X-%s-Core", 0, 0 },
{ "session", "X-%s-Core", 0, 0 },
{ "userPath", "X-%s-Core", 0, 0 },
{ "systemPath", "X-%s-Core", 0, 0 },
{ "systemShell", "X-%s-Core", 0, 0 },
{ "failsafeClient", "X-%s-Core", 0, 0 },
{ "userAuthDir", "X-%s-Core", 0, 0 },
{ "chooser", "X-%s-Core", 0, 0 },	/* XXX to kill */
{ "noPassUsers", "X-%s-Core", 0, P_noPassUsers },
{ "autoUser", "X-%s-Core", "AutoLoginUser", P_autoUser },
{ "autoPass", "X-%s-Core", "AutoLoginPass", P_autoPass },
{ "autoString", "X-%s-Core", "AutoLoginSession", 0 },
{ "autoLogin1st", "X-%s-Core", 0, 0 },
{ "autoReLogin", "X-%s-Core", 0, 0 },
{ "allowNullPasswd", "X-%s-Core", 0, 0 },
{ "allowRootLogin", "X-%s-Core", 0, 0 },
};

static XrmQuark XrmQString, empty = NULLQUARK;

static Bool 
DumpEntry(
    XrmDatabase         *db ATTR_UNUSED,
    XrmBindingList      bindings,
    XrmQuarkList        quarks,
    XrmRepresentation   *type,
    XrmValuePtr         value,
    XPointer            data ATTR_UNUSED)
{
    const char *dpy, *key;
    int el, hasu;
    char dpybuf[80];

    if (*type != XrmQString)
	return False;
    if (*bindings == XrmBindLoosely || 
	strcmp (XrmQuarkToString (*quarks), "DisplayManager"))
	return False;
    bindings++, quarks++;
    if (!*quarks)
	return False;
    if (*bindings != XrmBindLoosely && !quarks[1]) {	/* DM.foo */
	key = XrmQuarkToString (*quarks);
	handleXdmVal (0, key, value->addr, globents, as(globents));
	return False;
    } else if (*bindings == XrmBindLoosely && !quarks[1]) { /* DM*bar */
	dpy = "*";
	key = XrmQuarkToString (*quarks);
    } else if (*bindings != XrmBindLoosely && quarks[1] &&
	       *bindings != XrmBindLoosely && !quarks[2]) { /* DM.foo.bar */
	dpy = dpybuf + 4;
	strcpy (dpybuf + 4, XrmQuarkToString (*quarks));
	for (hasu = 0, el = 4; dpybuf[el]; el++)
	    if (dpybuf[el] == '_')
		hasu = 1;
	if (!hasu/* && isupper (dpy[0])*/) {
	    dpy = dpybuf;
	    memcpy (dpybuf, "*:*_", 4);
	} else {
	    for (; --el >= 0; )
		if (dpybuf[el] == '_') {
		    dpybuf[el] = ':';
		    for (; --el >= 4; )
			if (dpybuf[el] == '_')
			    dpybuf[el] = '.';
		    break;
		}
	}
	key = XrmQuarkToString (quarks[1]);
    } else
	return False;
    handleXdmVal (dpy, key, value->addr, dpyents, as(dpyents));
    return False;
}

static const char *xdmpath;
static const char *xdmconfs[] = { "%s/kdm-config", "%s/xdm-config" };

static int
mergeXdmCfg (const char *path)
{
    char *p;
    XrmDatabase db;
    int i;

    for (i = 0; i < as(xdmconfs); i++) {
	ASPrintf (&p, xdmconfs[i], path);
	if (!p)
	    return 0;
	if ((db = XrmGetFileDatabase (p))) {
	    printf ("Information: reading old xdm config file %s\n", p);
	    copiedFile (p);
	    free (p);
	    xdmpath = path;
	    XrmEnumerateDatabase(db, &empty, &empty, XrmEnumAllLevels,
				 DumpEntry, (XPointer) 0);
	    return 1;
	}
	free (p);
    }
    return 0;
}

static const char *oldkdes[] = {
    KDE_CONFDIR, 
    "/opt/kde3/share/config",
    "/usr/local/kde3/share/config",

    "/opt/kde/share/config",
    "/usr/local/kde/share/config",
    "/usr/local/share/config",
    "/usr/share/config",

    "/opt/kde2/share/config",
    "/usr/local/kde2/share/config",
    "/opt/kde1/share/config",
    "/usr/local/kde1/share/config",
};

static const char *oldxdms[] = {
    "/etc/X11/kdm", 
    XLIBDIR "/kdm",
    "/etc/X11/xdm", 
    XLIBDIR "/xdm",
};

int main(int argc, char **argv)
{
    const char **where;
    char *newkdmrc;
    FILE *f;
    StrList *fp;
    int i, ap, newer;
    char nname[80];

    for (ap = 1; ap < argc; ap++) {
	if (!strcmp(argv[ap], "--help")) {
	    printf (
"genkdmconf - generate configuration files for kdm\n"
"options:\n"
"  --in /path/to/new/kdm-config-dir\n"
"    In which directory to put the new configuration. You can use this\n"
"    to support a $(DESTDIR), but not to change the final location of\n"
"    the installation - the paths inside the files are not influenced.\n"
"    Default is " KDMCONF ".\n"
"  --old-xdm /path/to/old/xdm-dir\n"
"    Where to look for the config files of an xdm/older kdm.\n"
"    Default is to scan /etc/X11/kdm, $XLIBDIR/kdm, /etc/X11/xdm,\n"
"    $XLIBDIR/xdm; there in turn look for kdm-config and xdm-config.\n"
"  --old-kde /path/to/old/kde-config-dir\n"
"    Where to look for the kdmrc of an older kdm.\n"
"    Default is to scan " KDE_CONFDIR " and\n"
"    {/usr,/usr/local,{/opt,/usr/local}/{kde3,kde,kde2,kde1}}/share/config.\n"
"  --no-old\n"
"    Don't look at other xdm/kdm configurations, just create default config.\n"
"  --copy\n"
"    Don't reference old positions, but copy files.\n"
"    Note that the generated config may be broken.\n");
	    exit (0);
	}
	if (!strcmp(argv[ap], "--no-old")) {
	    no_old = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--copy")) {
	    copy_files = 1;
	    continue;
	}
	where = 0;
	if (!strcmp(argv[ap], "--in"))
	    where = &newdir;
	else if (!strcmp(argv[ap], "--old-xdm"))
	    where = &oldxdm;
	else if (!strcmp(argv[ap], "--old-kde"))
	    where = &oldkde;
	else {
	    fprintf (stderr, "Unknown command line option '%s', try --help\n", argv[ap]);
	    exit (1);
	}
	if (ap + 1 == argc || argv[ap + 1][0] == '-') {
	    fprintf (stderr, "Missing argument to option %s, try --help\n", argv[ap]);
	    exit (1);
	}
	*where = argv[++ap];
    }

    mkdefconf();
    newer = 0;
    if (!no_old) {
	if (oldkde) {
	    if (!(newer = mergeKdmRcNewer (oldkde)) && !mergeKdmRcOld (oldkde))
		fprintf (stderr, 
			 "Cannot read old kdmrc at specified position\n");
	} else
	    for (i = 0; i < as(oldkdes); i++)
		if ((newer = mergeKdmRcNewer (oldkdes[i])) || 
		    mergeKdmRcOld (oldkdes[i]))	{
		    oldkde = oldkdes[i];
		    break;
		}
	if (!newer) {
	    XrmInitialize ();
	    XrmQString = XrmPermStringToQuark("String");
	    if (oldxdm) {
		if (!mergeXdmCfg (oldxdm))
		    fprintf (stderr, 
			     "Cannot read old kdm-config/xdm-config at specified position\n");
	    } else
		for (i = 0; i < as(oldxdms); i++)
		    if (mergeXdmCfg (oldxdms[i])) {
			oldxdm = oldxdms[i];
			break;
		    }
	} else
	    oldxdm = 0;
    }
    ASPrintf (&newkdmrc, "%s/kdmrc", newdir);
    f = Create (newkdmrc, kdmrcmode);
    wrconf (f);
    fclose (f);

    if (oldxdm || oldkde) {
	sprintf (nname, "%s/README", newdir);
	f = Create (nname, 0644);
	fprintf (f, 
"The configuration files in this directory were automatically generated.\n"
"As the used algorithms are pretty dumb, the configuration may be broken.\n");
	if (cflist) {
	    fprintf (f, 
"This configuration is derived from the following files:\n");
	    for (fp = cflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	if (lflist) {
	    fprintf (f, 
"This configuration depends on the following files, which must not be deleted:\n");
	    for (fp = lflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	fprintf (f, 
"Have a look at the program <kdebase-sources>/kdm/kfrontend/genkdmconf\n"
"if you want to generate another configuration.\n");
	fclose (f);
    }

    return 0;
}
