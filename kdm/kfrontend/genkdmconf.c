    /*

    Create a suitable configuration for kdm taking old xdm/kdm 
    installations into account

    Copyright (C) 2001-2003 Oswald Buddenhagen <ossi@kde.org>


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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    */

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include <config.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <utime.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/param.h>
#ifdef BSD
# include <utmp.h>
#endif

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
# define ATTR_UNUSED __attribute__((unused))
#else
# define ATTR_UNUSED
#endif

#if defined(__sun) && !defined(__sun__)
# define __sun__
#endif

#define as(ar) ((int)(sizeof(ar)/sizeof(ar[0])))

#define __stringify(x) #x
#define stringify(x) __stringify(x)

#define KDMCONF KDE_CONFDIR "/kdm"
#define KDMDATA KDE_DATADIR "/kdm"

#define RCVERMAJOR 2
#define RCVERMINOR 1
#define RCVERSTR stringify(RCVERMAJOR) "." stringify(RCVERMINOR)

static int old_scripts, no_old_scripts, old_confs, no_old,
    no_backup, no_in_notice, use_destdir, mixed_scripts;
static const char *newdir = KDMCONF, *facesrc = KDMDATA "/pics/users",
    *oldxdm, *oldkde;

static int oldver;


typedef struct StrList {
    struct StrList	*next;
    const char		*str;
} StrList;


static void *
mmalloc (size_t sz)
{
    void *ptr;

    if (!(ptr = malloc (sz))) {
	fprintf (stderr, "Out of memory\n");
	exit (1);
    }
    return ptr;
}

static void *
mcalloc (size_t sz)
{
    void *ptr;

    if (!(ptr = calloc (1, sz))) {
	fprintf (stderr, "Out of memory\n");
	exit (1);
    }
    return ptr;
}

static void *
mrealloc (void *optr, size_t sz)
{
    void *ptr;

    if (!(ptr = realloc (optr, sz))) {
	fprintf (stderr, "Out of memory\n");
	exit (1);
    }
    return ptr;
}

static char *
mstrdup (const char *optr)
{
    char *ptr;

    if (!optr)
	return 0;
    if (!(ptr = strdup (optr))) {
	fprintf (stderr, "Out of memory\n");
	exit (1);
    }
    return ptr;
}


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

    ocabp->tlen++;
    if (ocabp->clen >= ocabp->blen) {
	ocabp->blen = ocabp->blen * 3 / 2 + 100;
	ocabp->buf = mrealloc (ocabp->buf, ocabp->blen);
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

static void
StrCat (char **strp, const char *fmt, ...)
{
    char *str, *tstr;
    va_list args;
    int el;

    va_start(args, fmt);
    el = VASPrintf (&str, fmt, args);
    va_end(args);
    if (*strp) {
	int ol = strlen (*strp);
	tstr = mmalloc (el + ol + 1);
	memcpy (tstr, *strp, ol);
	memcpy (tstr + ol, str, el + 1);
	free (*strp);
	free (str);
	*strp = tstr;
    } else
	*strp = str;
}

/*
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
*/


#define WANT_CLOSE 1

typedef struct File {
	char *buf, *eof;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
	int ismapped;
#endif
} File;

static int
readFile (File *file, const char *fn)
{
    off_t flen;
    int fd;

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
	file->buf = mmalloc (flen + 1);
	lseek (fd, 0, SEEK_SET);
	if (read (fd, file->buf, flen) != flen) {
	    free (file->buf);
	    close (fd);
	    fprintf (stderr, "Cannot read file\n");
	    return 0;	/* maybe better abort? */
	}
    }
    file->eof = file->buf + flen;
    close (fd);
    return 1;
}

#ifdef WANT_CLOSE
static void
freeBuf (File *file)
{
# ifdef HAVE_MMAP
    if (file->ismapped)
	munmap(file->buf, file->eof - file->buf);
    else
# endif
	free (file->buf);
}
#endif

static int
mkdirp (const char *name, int mode, const char *what, int existok)
{
    char *mfname = mstrdup (name);
    int i;

    for (i = 1; mfname[i]; i++)
	if (mfname[i] == '/') {
	    mfname[i] = 0;
	    if (mkdir (mfname, 0755)) {
		if (errno != EEXIST) {
		    fprintf (stderr, "Cannot create parent %s of %s directory %s: %s\n",
			     mfname, what, name, strerror (errno));
		    free (mfname);
		    return 0;
		}
	    } else
		chmod (mfname, 0755);
	    mfname[i] = '/';
	}
    free (mfname);
    if (mkdir (name, mode)) {
	if (errno != EEXIST) {
	    fprintf (stderr, "Cannot create %s directory %s: %s\n",
		     what, name, strerror (errno));
	    return 0;
	}
	return existok;
    }
    chmod (name, mode);
    return 1;
}


static void
displace (const char *fn)
{
    if (!no_backup) {
	char bn[PATH_MAX + 4];
	sprintf(bn, "%s.bak", fn); /* won't overflow if only existing paths are passed */
	rename(fn, bn);
    } else
	unlink(fn);
}


/*
 * target data to be written to kdmrc
 */

typedef struct Entry {
	struct Entry	*next;
	struct Ent	*spec;
	const char	*value;
	int		active:1;
	int		written:1;
} Entry;

typedef struct Section {
	struct Section	*next;
	struct Sect	*spec;
	const char	*name;
	const char	*comment;
	Entry		*ents;
	int		active;
} Section;

static Section *config; /* the kdmrc data to be written */

/*
 * Specification of the (currently possible) kdmrc entries
 */

typedef struct Ent {
	const char	*key;
	int		prio;
	void (*func)(Entry *ce, Section *cs);
	const char	*comment;
} Ent;

typedef struct Sect {
	const char	*name;
	Ent		*ents;
	int		nents;
} Sect;

static Sect *findSect (const char *name);
static Ent *findEnt (Sect *sect, const char *key);

/*
 * Functions to manipulate the current kdmrc data
 */

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
    cs = mcalloc (sizeof(*cs));
    ASPrintf ((char **)&cs->name, "%s", sect);
    cs->spec = findSect (sect);
    *csp = cs;
  havesec:
    cs->active = 1;

    for (cep = &(cs->ents); (ce = *cep); cep = &(ce->next))
	if (!strcmp(key, ce->spec->key))
	    goto haveent;
    ce = mcalloc (sizeof(*ce));
    ce->spec = findEnt (cs->spec, key);
    *cep = ce;
  haveent:
    ASPrintf ((char **)&ce->value, "%s", value);
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
wrconf (FILE *f)
{
    Section *cs;
    Entry *ce;
    StrList *sl = 0, *sp;
    const char *cmt;

    putfqval ("General", "ConfigVersion", RCVERSTR);
    for (cs = config; cs; cs = cs->next) {
	fprintf (f, "%s%s[%s]\n",
		 cs->comment ? cs->comment : "",
		 cs->active ? "" : "#", cs->name);
	for (ce = cs->ents; ce; ce = ce->next) {
	    if (ce->spec->comment) {
		cmt = ce->spec->comment;
		for (sp = sl; sp; sp = sp->next)
		    if (sp->str == cmt) {
			cmt = "# See above\n";
			goto havit;
		    }
		if (!(sp = malloc (sizeof(*sp))))
		    fprintf (stderr, "Warning: Out of memory\n");
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
 * defaults
 */

#ifndef HALT_CMD
# ifdef _AIX
#  define HALT_CMD	"/usr/sbin/shutdown -h now"
#  define REBOOT_CMD	"/usr/sbin/shutdown -r now"
# elif defined(BSD)
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
# if 0 /*def HASXDMAUTH*/
#  define DEF_AUTH_NAME	"XDM-AUTHORIZATION-1,MIT-MAGIC-COOKIE-1"
# else
#  define DEF_AUTH_NAME	"MIT-MAGIC-COOKIE-1"
# endif
#endif

#ifdef __linux__
# define DEF_SERVER_LINE ":0 local@tty1 " XBINDIR "/X vt7"
/* ###
# define DEF_SERVER_LINES DEF_SERVER_LINE "\n" \
			 ":1 local@tty2 reserve " XBINDIR "/X :1 vt8\n" \
			 ":2 local@tty3 reserve " XBINDIR "/X :2 vt9\n" \
			 "#:3 local@tty4 reserve " XBINDIR "/X :3 vt10\n" \
			 "#:4 local@tty5 reserve " XBINDIR "/X :4 vt11\n"
*/
# define DEF_SERVER_LINES DEF_SERVER_LINE "\n" \
			 "#:1 local@tty2 reserve " XBINDIR "/X :1 vt8\n" \
			 "#:2 local@tty3 reserve " XBINDIR "/X :2 vt9\n" \
			 "#:3 local@tty4 reserve " XBINDIR "/X :3 vt10\n" \
			 "#:4 local@tty5 reserve " XBINDIR "/X :4 vt11\n"
#elif defined(__sun__)
# define DEF_SERVER_LINE ":0 local@console " XBINDIR "/X"
#elif defined(_AIX)
# define DEF_SERVER_LINE ":0 local@lft0 " XBINDIR "/X -T -force"
#else
# define DEF_SERVER_LINE ":0 local " XBINDIR "/X"
#endif
#ifndef DEF_SERVER_LINES
# define DEF_SERVER_LINES DEF_SERVER_LINE "\n"
#endif

static const char def_xaccess[] = 
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

/* XXX
#define XSERVERS_MAJOR 2
#define XSERVERS_MINOR 0
*/
#define XSERVERS_MAJOR 1
#define XSERVERS_MINOR 99

#define VERSION_WARNING "### Don't change these two lines; they are hints for genkdmconf. ###\n"
#define XSERVERS_VERSION VERSION_WARNING "### Version " stringify(XSERVERS_MAJOR) "." stringify(XSERVERS_MINOR) " ###\n"

static const char def_xservers[] = 
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
"\n" DEF_SERVER_LINES "\n"
"\n" XSERVERS_VERSION;

static const char def_willing[] = 
"#! /bin/sh\n"
"# The output of this script is displayed in the chooser window\n"
"# (instead of \"Willing to manage\").\n"
"\n"
"load=`uptime|sed -e 's/^.*load[^0-9]*//'`\n"
"nrusers=`who|cut -c 1-8|sort -u|wc -l|sed 's/^[ \t]*//'`\n"
"s=\"\"; [ \"$nrusers\" != 1 ] && s=s\n"
"\n"
"echo \"${nrusers} user${s}, load: ${load}\"\n";

static const char def_setup[] = 
"#! /bin/sh\n"
"# Xsetup - run as root before the login dialog appears\n"
"\n"
"#xconsole -geometry 480x130-0-0 -notify -verbose -fn fixed -exitOnFail -file /dev/xconsole &\n";

static const char def_startup[] = 
"#! /bin/sh\n"
"# Xstartup - run as root before session starts\n"
"\n"
"# By convention, both xconsole and xterm -C check that the\n"
"# console is owned by the invoking user and is readable before attaching\n"
"# the console output.  This way a random user can invoke xterm -C without\n"
"# causing serious grief; still, it can cause havoc, so xconsole is started\n"
"# by Xsetup usually.\n"
"# This is not required if you use PAM with the pam_console module.\n"
"#\n"
"#chown $USER /dev/console\n"
"\n"
#ifdef _AIX
"# We create a pseudodevice for finger.  (host:0 becomes xdm/host_0)\n"
"# Without it, finger errors out with \"Can't stat /dev/host:0\".\n"
"#\n"
"if [ -f /usr/lib/X11/xdm/sessreg ]; then\n"
"  devname=`echo $DISPLAY | /usr/bin/sed -e 's/[:\\.]/_/g' | /usr/bin/cut -c1-8`\n"
"  hostname=`echo $DISPLAY | /usr/bin/cut -d':' -f1`\n"
"\n"
"  if [ -z \"$devname\" ]; then\n"
"    devname=\"unknown\"\n"
"  fi\n"
"  if [ ! -d /dev/xdm ]; then\n"
"    /usr/bin/mkdir /dev/xdm\n"
"    /usr/bin/chmod 755 /dev/xdm\n"
"  fi\n"
"  /usr/bin/touch /dev/xdm/$devname\n"
"  /usr/bin/chmod 644 /dev/xdm/$devname\n"
"\n"
"  if [ -z \"$hostname\" ]; then\n"
"    exec /usr/lib/X11/xdm/sessreg -a -l xdm/$devname $USER\n"
"  else\n"
"    exec /usr/lib/X11/xdm/sessreg -a -l xdm/$devname -h $hostname $USER\n"
"  fi\n"
"fi\n";
#else
"exec sessreg -a -l $DISPLAY"
# ifdef BSD
" -x " KDMCONF "/Xservers -u " _PATH_UTMP
# endif
" $USER\n";
#endif /* _AIX */

static const char def_reset[] = 
"#! /bin/sh\n"
"# Xreset - run as root after session exits\n"
"\n"
"# Reassign ownership of the console to root, this should disallow\n"
"# assignment of console output to any random users's xterm. See Xstartup.\n"
"#\n"
"#chown root /dev/console\n"
"#chmod 622 /dev/console\n"
"\n"
#ifdef _AIX
"if [ -f /usr/lib/X11/xdm/sessreg ]; then\n"
"  devname=`echo $DISPLAY | /usr/bin/sed -e 's/[:\\.]/_/g' | /usr/bin/cut -c1-8`\n"
"  exec /usr/lib/X11/xdm/sessreg -d -l xdm/$devname $USER\n"
"fi\n";
#else
"exec sessreg -d -l $DISPLAY"
# ifdef BSD
" -x " KDMCONF "/Xservers -u " _PATH_UTMP
# endif
" $USER\n";
#endif /* _AIX */

static const char def_session[] = 
"#! /bin/sh\n"
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
"DM_PATH=$PATH\n"
"test -f /etc/profile && . /etc/profile\n"
"test -f $HOME/.profile && . $HOME/.profile\n"
"IFS_SAVE=$IFS\n"
"IFS=:\n"
"for i in $PATH; do\n"
"    case :$DM_PATH: in\n"
"      *:$i:*) ;;\n"
"      ::) DM_PATH=$i;;\n"
"      *) DM_PATH=$DM_PATH:$i;;\n"
"    esac\n"
"done\n"
"IFS=$IFS_SAVE\n"
"PATH=$DM_PATH\n"
"export PATH\n"
"\n"
"test -f /etc/xprofile && . /etc/xprofile\n"
"test -f $HOME/.xprofile && . $HOME/.xprofile\n"
"\n"
"case $1 in\n"
"    \"\")\n"
"	exec xmessage -center -buttons OK:0 -default OK \"Sorry, $DESKTOP_SESSION is no valid session.\"\n"
"	;;\n"
"    failsafe)\n"
"	exec xterm -geometry 80x24-0-0\n"
"	;;\n"
"    custom)\n"
"	exec $HOME/.xsession\n"
"	;;\n"
"    default)\n"
"	exec startkde\n"
"	;;\n"
"    *)\n"
"	eval exec \"$1\"\n"
"	;;\n"
"esac\n"
"exec xmessage -center -buttons OK:0 -default OK \"Sorry, cannot execute $1. Check $DESKTOP_SESSION.desktop.\"\n";

static const char def_background[] =
"[Desktop0]\n"
"BackgroundMode=Flat\n"
"BlendBalance=100\n"
"BlendMode=NoBlending\n"
"ChangeInterval=60\n"
"Color1=0,0,200\n"
"Color2=192,192,192\n"
"CurrentWallpaper=0\n"
"LastChange=0\n"
"MinOptimizationDepth=1\n"
"MultiWallpaperMode=NoMulti\n"
"Pattern=fish\n"
"Program=\n"
"ReverseBlending=false\n"
"UseSHM=false\n"
"Wallpaper=default_blue.jpg\n"
"WallpaperList=\n"
"WallpaperMode=Scaled\n";

static char *
prepName (const char *fn)
{
    const char *tname;
    char *nname;

    tname = strrchr (fn, '/');
    ASPrintf (&nname, "%s/%s", newdir, tname ? tname + 1 : fn);
    displace (nname);
    return nname;
}

static FILE *
Create (const char *fn, int mode)
{
    char *nname;
    FILE *f;

    nname = prepName (fn);
    if (!(f = fopen (nname, "w"))) {
	fprintf (stderr, "Cannot create %s\n", nname);
	exit (1);
    }
    chmod (nname, mode);
    free (nname);
    return f;
}

static void
WriteOut (const char *fn, int mode, time_t stamp, const char *buf, size_t len)
{
    char *nname;
    int fd;
    struct utimbuf utim;

    nname = prepName (fn);
    if ((fd = creat (nname, mode)) < 0) {
	fprintf (stderr, "Cannot create %s\n", nname);
	exit (1);
    }
    write (fd, buf, len);
    close (fd);
    if (stamp) {
	utim.actime = utim.modtime = stamp;
	utime (nname, &utim);
    }
    free (nname);
}


/* returns static array! */
static const char *
resect (const char *sec, const char *name)
{
    static char sname[64];
    char *p;

    if ((p = strrchr (sec, '-'))) {
	sprintf (sname, "%.*s-%s", p - sec, sec, name);
	return sname;
    } else
	return name;
}

static int
inNewDir (const char *name)
{
    return !memcmp (name, KDMCONF "/", sizeof(KDMCONF));
}

static int
inList (StrList *sp, const char *s)
{
    for (; sp; sp = sp->next)
	if (!strcmp (sp->str, s))
	    return 1;
    return 0;
}

static void 
addStr (StrList **sp, const char *s)
{
    for (; *sp; sp = &(*sp)->next)
	if (!strcmp ((*sp)->str, s))
	    return;
    *sp = mcalloc (sizeof(**sp));
    ASPrintf ((char **)&(*sp)->str, "%s", s);
}

StrList *aflist, *uflist, *eflist, *cflist, *lflist;

/* file is part of new config */
static void 
addedFile (const char *fn)
{
    addStr (&aflist, fn);
}

/* file from old config was parsed */
static void 
usedFile (const char *fn)
{
    addStr (&uflist, fn);
}

/* file from old config was copied with slight modifications */
static void 
editedFile (const char *fn)
{
    addStr (&eflist, fn);
}

/* file from old config was copied verbatim */
static void 
copiedFile (const char *fn)
{
    addStr (&cflist, fn);
}

/* file from old config is still being used */
static void 
linkedFile (const char *fn)
{
    addStr (&lflist, fn);
}

/*
 * XXX this stuff is highly borked. it does not handle collisions at all.
 */
static int
copyfile (Entry *ce, const char *tname, int mode, int (*proc)(File *, char **, int *))
{
    const char *tptr;
    char *nname;
    File file;
    int rt;

    if (!*ce->value)
	return 1;

    tptr = strrchr (tname, '/');
    ASPrintf (&nname, KDMCONF "/%s", tptr ? tptr + 1 : tname);
    if (inList (cflist, ce->value) ||
	inList (eflist, ce->value) || 
	inList (lflist, ce->value))
    {
	rt = 1;
	goto doret;
    }
    if (!readFile (&file, ce->value)) {
	fprintf (stderr, "Warning: cannot copy file %s\n", ce->value);
	rt = 0;
    } else {
	char *nbuf;
	int nlen;

	if (!proc || !proc (&file, &nbuf, &nlen) ||
	    (nlen == file.eof - file.buf && !memcmp (nbuf, file.buf, nlen)))
	{
	    if (!use_destdir && !strcmp (ce->value, nname))
		linkedFile (nname);
	    else {
		struct stat st;
		stat (ce->value, &st);
		WriteOut (nname, mode, st.st_mtime, file.buf, file.eof - file.buf);
		copiedFile (ce->value);
	    }
	} else {
	    WriteOut (nname, mode, 0, nbuf, nlen);
	    editedFile (ce->value);
	}
	if (strcmp (ce->value, nname) && inNewDir (ce->value) && !use_destdir)
	    displace (ce->value);
	addedFile (nname);
	rt = 1;
    }
  doret:
    ce->value = nname;
    return rt;
}

static void
dlinkfile (const char *name)
{
    File file;

    if (!readFile (&file, name)) {
	fprintf (stderr, "Warning: cannot read file %s\n", name);
	return;
    }
    if (inNewDir (name) && use_destdir) {
	struct stat st;
	stat (name, &st);
	WriteOut (name, st.st_mode, st.st_mtime, file.buf, file.eof - file.buf);
	copiedFile (name);
    } else
	linkedFile (name);
    addedFile (name);
}

static void
linkfile (Entry *ce)
{
    if (ce->written && *ce->value)
	dlinkfile (ce->value);
}

static void
writefile (const char *tname, int mode, const char *cont)
{
    WriteOut (tname, mode, 0, cont, strlen (cont));
    addedFile (tname);
}


char *background;

static void 
handBgCfg (Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active)	/* can be only the X-*-Greeter one */
	writefile (KDMCONF "/backgroundrc", 0644,
		   background ? background : def_background);
#if 0 /* risk of kcontrol clobbering the original file */
    else if (old_confs)
	linkfile (ce);
#endif
    else {
	if (!copyfile (ce, ce->value, 0644, 0)) {
	    if (!strcmp (cs->name, "X-*-Greeter"))
		writefile (KDMCONF "/backgroundrc", 0644, def_background);
	    ce->active = 0;
	}
    }
}


/* TODO: handle solaris' local_uid specs */

#ifdef __linux__
# define RDPYS
#endif

#if defined(__linux__) || defined(__sun__)
# define CONS
#endif

#if defined(RDPYS) || defined(CONS)

# define dLocal		0
# define dForeign	1

static struct displayMatch {
	const char	*name;
	int		len, type;
} displayTypes[] = {
	{ "local", 5,	dLocal },
	{ "foreign", 7,	dForeign },
};

static int
parseDisplayType (const char *string
#ifdef CONS
		  , const char **atPos
#endif
	)
{
    struct displayMatch	*d;

#ifdef CONS
    *atPos = 0;
#endif
    for (d = displayTypes; d < displayTypes + as(displayTypes); d++) {
#ifdef CONS
	if (!memcmp (d->name, string, d->len) &&
	    (!string[d->len] || string[d->len] == '@')) {
	    if (string[d->len] == '@')
		*atPos = string + d->len + 1;
#else
	if (!memcmp (d->name, string, d->len + 1)) {
#endif
	    return d->type;
	}
    }
    return -1;
}

typedef struct Line {
    struct Line *next;
    StrList *words;
    char *comment;
} Line;
#endif /* RDPYS || CONS */

static int
edit_xservers(File *file, char **nbuf, int *nlen)
{
#if defined(RDPYS) || defined(CONS)
    char *cur, *nword, *wordp, *lstrt, *buf, *p, *rp;
    const char *word, *dname, *dclass, *dclassp, *atPos;
    Line *lin, *lines = 0, **lptr = &lines;
    StrList *wrd, **wptr, *xswords = 0, **xswordp;
    int ndpys = 0, nldpys = 0, nrdpys = 0, dpymask = 0, vtmask = 63;
    int ttymask = 0;
    int type, tty, vt, dn, maj, min;
    int quoted, i;
    char c;

    /* parse it */
    *file->eof = 0;
    cur = file->buf;
  nline:
    if (cur >= file->eof)
	goto done;
    if (!memcmp (cur, VERSION_WARNING, sizeof(VERSION_WARNING) - 1)) {
	cur += sizeof(VERSION_WARNING) - 1;
	goto nline;	/* *chomp* */
    }
    i = 0;
    sscanf (cur, "### Version %d.%d ###\n%n", &maj, &min, &i);
    if (i) {
	if (maj > XSERVERS_MAJOR ||
	    (maj == XSERVERS_MAJOR && min >= XSERVERS_MINOR))
	    return 0;	/* up to date */
	cur += i;
	goto nline;	/* *chomp* */
    }
    *lptr = lin = mcalloc (sizeof(*lin));
    lptr = &(*lptr)->next;
    wptr = &lin->words;
  nword:
    lstrt = cur;
    do {
	c = *cur++;
    } while (c == ' ' || c == '\t');
    if (!c || c == '\n')
	goto nline;
    if (c == '#') {
	do {
	    c = *cur++;
	} while (c && c != '\n');
	lin->comment = nword = mmalloc (cur - lstrt);
	memcpy (nword, lstrt, cur - lstrt - 1);
	nword[cur - lstrt - 1] = 0;
	goto nline;
    }
    wordp = cur - 1;
  mloop:
    quoted = 0;
  qloop:
    switch (c) {
    case '#':
	if (quoted)
	    break;
	cur--;
	goto deol;
    case '\n':
	if (quoted)
	    break;
      deol:
    case ' ':
    case '\t':
    case 0:
	*wptr = wrd = mcalloc (sizeof(*wrd));
	wptr = &(*wptr)->next;
	wrd->str = nword = mmalloc (cur - wordp);
	cur--;
	memcpy (nword, wordp, cur - wordp);
	nword[cur - wordp] = 0;
	goto nword;
    case '\\':
	if (!quoted) {
	    quoted = 1;
	    c = *cur++;
	    goto qloop;
	}
	break;
    }
    c = *cur++;
    goto mloop;
  done:

    /* analyze it */
    for (lin = lines; lin; lin = lin->next) {
	if (!(wrd = lin->words))
	    continue;	/* no display */
	dname = wrd->str;
	if (!(wrd = wrd->next))
	    continue;	/* no type */
	type = parseDisplayType (wrd->str
#ifdef CONS
				, &atPos
#endif
		);
	if (type < 0) {
	    if (!(wrd = wrd->next))
		continue;	/* no type */
	    type = parseDisplayType (wrd->str
#ifdef CONS
				    , &atPos
#endif
		);
	    if (type < 0)	/* invalid type */
		continue;
	}
	if (!(p = strchr (dname, ':')))
	    continue;	/* invalid display name */
	dn = strtol (++p, &rp, 10);
	if (*rp)
	    continue;	/* same here ... */
#ifdef RDPYS
	ndpys++;
#endif
	if (type != dLocal)
	    continue;	/* foreign doesn't count */
#ifdef RDPYS
	nldpys++;
	dpymask |= 1 << dn;
#endif
#ifdef CONS
	if (atPos) {
#ifdef __linux__
	    if (!memcmp (atPos, "tty", 3) && (tty = atoi (atPos + 3)))
		ttymask |= 1 << (tty - 1);
#elif defined(__sun__)
	    if (!memcmp (atPos, "console", 8))
		ttymask |= 1;
#endif
	}
#endif
#ifdef RDPYS
	wrd = wrd->next;
	if (wrd && !strcmp (wrd->str, "reserve")) {
	    nrdpys++;
	    wrd = wrd->next;
	}
	xswordp = 0;
	if (!xswords)
	    xswordp = &xswords;
	while (wrd) {
	    word = wrd->str;
	    if (word[0] == 'v' && word[1] == 't') {
		vt = strtol (word + 2, &rp, 10);
		if (!*rp)
		    vtmask |= 1 << (vt - 1);
	    } else if (xswordp) {
		/* this will break on -display :<n>, but in such a situation
		   (Xnest) we have already lost anyway */
		if (word[0] != ':') {
		    *xswordp = mcalloc (sizeof(*xswordp));
		    (*xswordp)->str = word;
		    xswordp = &(*xswordp)->next;
		}
	    }
	    wrd = wrd->next;
	}
#endif
    }

    /* edit it */
    buf = 0;
    for (lin = lines; lin; lin = lin->next) {
	if (lin->words) {
	    /* NOTE: we throw away b0rked display specs */
	    wrd = lin->words;
	    dname = wrd->str;
	    if (!(wrd = wrd->next))
		continue;	/* no type */
	    type = parseDisplayType (wrd->str
#ifdef CONS
				     , &atPos
#endif
		    );
	    if (type < 0) {
		dclass = wrd->str;		
		dclassp = " ";
		if (!(wrd = wrd->next))
		    continue;	/* no type */
		type = parseDisplayType (wrd->str
#ifdef CONS
					, &atPos
#endif
			);
		if (type < 0)	/* invalid type */
		    continue;
	    } else
		dclass = dclassp = "";
	    if (!(p = strchr (dname, ':')))
		continue;	/* invalid display name */
	    dn = strtol (++p, &rp, 10);
	    if (*rp)
		continue;	/* same here ... */
	    if (type != dLocal) {
		StrCat (&buf, "%s%s%s foreign", dname, dclassp, dclass);	/* foreign doesn't count */
		goto elin;
	    }
#ifdef CONS
	    if (!atPos) {
# ifdef __linux__
		for (tty = 0; ttymask & (1 << tty); tty++);
		ttymask |= (1 << tty);
		StrCat (&buf, "%s%s%s local@tty%d", dname, dclassp, dclass, tty + 1);
# elif defined(__sun__)
		if (!(ttymask & 1)) {
		    ttymask |= 1;
		    StrCat (&buf, "%s%s%s local@console", dname, dclassp, dclass);
		} else
		    StrCat (&buf, "%s%s%s local", dname, dclassp, dclass);
# endif
	    } else
		StrCat (&buf, "%s%s%s local@%s", dname, dclassp, dclass, atPos);
#else
	    StrCat (&buf, "%s%s%s local", dname, dclassp, dclass);
#endif
#ifdef RDPYS
	    vt = 0;
#endif
	    while ((wrd = wrd->next)) {
		word = wrd->str;
#ifdef RDPYS
		if (word[0] == 'v' && word[1] == 't')
		    vt = 1;
#endif
		StrCat (&buf, " %s", word);
	    }
#ifdef RDPYS
	    if (!vt) {
		for (vt = 6; vtmask & (1 << vt); vt++);
		vtmask |= (1 << vt);
		StrCat (&buf, " vt%d", vt + 1);
	    }
#endif
	}
      elin:
	if (lin->comment)
	    StrCat (&buf, "%s\n", lin->comment);
	else
	    StrCat (&buf, "\n");
    }

#ifdef RDPYS
    /* add reserve dpys */
/* ###
    if (nldpys < 3 && nldpys && !nrdpys) {
	for (; nldpys < 3; nldpys++) {
	    for (dn = 0; dpymask & (1 << dn); dn++);
	    dpymask |= (1 << dn);
	    for (vt = 6; vtmask & (1 << vt); vt++);
	    vtmask |= (1 << vt);
# ifdef CONS
#  ifdef __linux__
	    for (tty = 0; ttymask & (1 << tty); tty++);
	    ttymask |= (1 << tty);
	    StrCat (&buf, ":%d local@tty%d reserve", dn, tty + 1);
#  elif defined(__sun__)
	    if (!(ttymask & 1)) {
		ttymask |= 1;
		StrCat (&buf, "%s local@console", dname);
	    } else
		StrCat (&buf, "%s local", dname);
#  endif
# else
	    StrCat (&buf, ":%d local reserve", dn);
# endif
	    for (wrd = xswords; wrd; wrd = wrd->next)
		StrCat (&buf, " %s", wrd->str);
	    StrCat (&buf, " :%d vt%d\n", dn, vt + 1);
	}
    }
*/
#endif

    StrCat (&buf, "\n" XSERVERS_VERSION, 0);

    *nbuf = buf;
    *nlen = strlen(buf);
    return 1;
#else /* RDPYS || CONS */
    return 0;
#endif
}

static void
mk_xservers(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {	/* there is only the Global one */
      mkdef:
	ce->value = KDMCONF "/Xservers";
	ce->active = ce->written = 1;
	writefile (ce->value, 0644, def_xservers);
    } else if (old_confs)
	linkfile (ce);
    else
	if (!copyfile (ce, "Xservers", 0644, edit_xservers))
	    goto mkdef;
}

static void
cp_keyfile(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active)	/* there is only the Global one */
	return;
    if (old_confs)
	linkfile (ce);
    else
	if (!copyfile (ce, "kdmkeys", 0600, 0))
	    ce->active = 0;
}

static void
mk_xaccess(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active)	/* there is only the Global one */
	writefile (KDMCONF "/Xaccess", 0644, def_xaccess);
    else if (old_confs)
	linkfile (ce);
    else
	copyfile (ce, "Xaccess", 0644, 0);	/* don't handle error, it will disable Xdmcp automatically */
}

static void
mk_willing(Entry *ce, Section *cs ATTR_UNUSED)
{
    const char *fname;

    if (!ce->active)	/* there is only the Global one */
	goto dflt;
    else {
	if (!(fname = strchr (ce->value, '/')))
	    return;	/* obviously in-line (or empty) */
	if (old_scripts || inNewDir (fname))
	    dlinkfile (fname);
	else {
	  dflt:
	    ce->value = KDMCONF "/Xwilling";
	    ce->active = ce->written = 1;
	    writefile (ce->value, 0755, def_willing);
	}
    }
}

/*
static int
edit_resources(File *file, char **nbuf, int *nlen)
{
    // XXX remove any login*, chooser*, ... resources
    return 0;
}
*/

static void
cp_resources(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active)	/* the X-*-Greeter one */
	return;
    if (old_confs)
	linkfile (ce);
    else
	if (!copyfile (ce, ce->value, 0644, 0/*edit_resources*/))
	    ce->active = 0;
}

static int
delstr (File *fil, const char *pat)
{
    char *p, *pp, *bpp;
    const char *pap, *paap;

    *fil->eof = 0;
    for (p = fil->buf; *p; p++) {
	for (pp = p, pap = pat; ; ) {
	    if (!*pap) {
		*p = '\n';
		memcpy (p + 1, pp, fil->eof - pp);
		fil->eof -= pp - p - 1;
		return 1;
	    } else if (!memcmp (pap, "*/", 2)) {
		paap = pap += 2;
		while (!isspace (*pap))
		    pap++;
		for (;;) {
		    if (*pp++ != '/')
			goto no;
		    for (bpp = pp; *pp != '/'; pp++)
			if (isspace(*pp))
			    goto wbrk;
		}
	      wbrk:
		if (memcmp (bpp, paap, pap - paap))
		    break;
	    } else if (*pap == '\t') {
		pap++;
		while (*pp == ' ' || *pp == '\t')
		    pp++;
	    } else {
		 if (*pap == '\n')
		    while (*pp == ' ' || *pp == '\t')
			pp++;
		if (*pap != *pp)
		    break;
		pap++;
		pp++;
	    }
	}
      no: ;
    }
    return 0;
}

/* XXX
   the UseBackground voodoo will horribly fail, if multiple sections link
   to the same Xsetup file
*/

static int mod_usebg;

static int
edit_setup(File *file, char **nbuf ATTR_UNUSED, int *nlen ATTR_UNUSED)
{
    putval ("UseBackground", 
		(delstr (file, "\n"
			"(\n"
			"  PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
			"  */kdmdesktop\t&\n"
			"  echo $! >$PIDFILE\n"
			"  wait $!\n"
			"  rm $PIDFILE\n"
			")\t&\n") ||
		 delstr (file, "\n"
			"*/kdmdesktop\t&\n") ||
		 delstr (file, "\n"
			"kdmdesktop\t&\n") ||
		 delstr (file, "\n"
			"kdmdesktop\n")) ? "true" : "false");
    return 0;
}

static void
mk_setup(Entry *ce, Section *cs)
{
    setsect (resect (cs->name, "Greeter"));
    if (old_scripts || mixed_scripts) {
	if (mod_usebg && *ce->value)
	    putval ("UseBackground", "false");
	linkfile (ce);
    } else {
	if (ce->active && inNewDir (ce->value)) {
	    if (mod_usebg)
		copyfile (ce, ce->value, 0755, edit_setup);
	    else
		linkfile (ce);
	} else {
	    ce->value = KDMCONF "/Xsetup";
	    ce->active = ce->written = 1;
	    writefile (ce->value, 0755, def_setup);
	}
    }
}

static int
edit_startup(File *file, char **nbuf ATTR_UNUSED, int *nlen ATTR_UNUSED)
{
    if (!delstr (file, "\n"
		"PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
		"if [ -f $PIDFILE ] ; then\n"
		"     kill `cat $PIDFILE`\n"
		"fi\n"))
	delstr (file, "\n"
		"PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
		"test -f $PIDFILE && kill `cat $PIDFILE`\n");
    return 0;
}

static void
mk_startup(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (old_scripts || mixed_scripts)
	linkfile (ce);
    else {
	if (ce->active && inNewDir (ce->value)) {
	    if (mod_usebg)
		copyfile (ce, ce->value, 0755, edit_startup);
	    else
		linkfile (ce);
	} else {
	    ce->value = KDMCONF "/Xstartup";
	    ce->active = ce->written = 1;
	    writefile (ce->value, 0755, def_startup);
	}
    }
}

static void
mk_reset(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (old_scripts || mixed_scripts || (ce->active && inNewDir (ce->value)))
	linkfile (ce);
    else {
	ce->value = KDMCONF "/Xreset";
	ce->active = ce->written = 1;
	writefile (ce->value, 0755, def_reset);
    }
}

static void
mk_session(Entry *ce, Section *cs ATTR_UNUSED)
{
    if ((old_scripts || (ce->active && inNewDir (ce->value))) &&
	oldver >= 0x201)
	linkfile (ce);
    else {
	ce->value = KDMCONF "/Xsession";
	ce->active = ce->written = 1;
	writefile (ce->value, 0755, def_session);
    }
}

static void
addKdePath (Entry *ce, const char *defpath)
{
    char *p;
    const char *path;

    path = ce->active ? ce->value : defpath;
    if (!(p = strstr (path, KDE_BINDIR)) ||
	(p != path && *(p-1) != ':') ||
	(p[sizeof(KDE_BINDIR)-1] && p[sizeof(KDE_BINDIR)-1] != ':'))
    {
	ASPrintf ((char **)&ce->value, KDE_BINDIR ":%s", path);
	ce->written = ce->active = 1;
    }
}

static void
ck_userpath(Entry *ce, Section *cs ATTR_UNUSED)
{
    addKdePath(ce, DEF_USER_PATH);
}

static void
ck_systempath(Entry *ce, Section *cs ATTR_UNUSED)
{
    addKdePath(ce, DEF_SYSTEM_PATH);
}

static void
upd_language(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp (ce->value, "C"))
	ce->value = (char *)"en_US";
}

static void
upd_greetstring(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *p2;
    if ((p2 = strstr (ce->value, "HOSTNAME"))) {
	strcpy (p2, "%n");
	strcpy (p2 + 2, p2 + 8);
    }
}

static void
upd_echomode(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp (ce->value, "NoStars"))
	ce->value = (char *)"NoEcho";
}

static void
upd_guistyle(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp (ce->value, "Motif+"))
	ce->value = (char *)"MotifPlus";
    else if (!strcmp (ce->value, "KDE"))
	ce->value = (char *)"Default";
}

static void
upd_showusers(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp (ce->value, "All"))
	ce->value = (char *)"NotHidden";
}

static void
upd_logoarea(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp (ce->value, "KdmLogo"))
	ce->value = (char *)"Logo";
    else if (!strcmp (ce->value, "KdmClock"))
	ce->value = (char *)"Clock";
    else if (strcmp (ce->value, "Logo") && strcmp (ce->value, "Clock"))
	ce->value = (char *)"None";
}

static const char *defminuid, *defmaxuid;

static void
upd_minshowuid(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {
	ce->value = defminuid;
	ce->active = ce->written = 1;
    }
}

static void
upd_maxshowuid(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {
	ce->value = defmaxuid;
	ce->active = ce->written = 1;
    }
}

static void
upd_hiddenusers(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *nv;
    const char *msu, *pt, *et;
    struct passwd *pw;
    unsigned minuid, maxuid;
    char nbuf[128];

    if (!ce->active)
	return;

    msu = getfqval (cs->name, "MinShowUID", "0");
    sscanf (msu, "%u", &minuid);
    msu = getfqval (cs->name, "MaxShowUID", "65535");
    sscanf (msu, "%u", &maxuid);

    nv = 0;
    pt = ce->value;
    for (;;) {
	et = strpbrk (pt, ";,");
	if (et) {
	    memcpy (nbuf, pt, et - pt);
	    nbuf[et - pt] = 0;
	} else
	    strcpy (nbuf, pt);
	if ((pw = getpwnam (nbuf))) {
	    if (!pw->pw_uid ||
		(pw->pw_uid >= minuid && pw->pw_uid <= maxuid))
	    {
		if (nv)
		    StrCat (&nv, ",%s", nbuf);
		else
		    nv = mstrdup (nbuf);
	    }
	}
	if (!et)
	    break;
	pt = et + 1;
    }
    ce->value = nv ? nv : "";
}

static void
upd_forgingseed(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {
	ASPrintf ((char **)&ce->value, "%d", time(0));
	ce->active = ce->written = 1;
    }
}

static void
upd_datadir(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *oldsts, *newsts;
    const char *dir;

    if (use_destdir)
	return;
    dir = ce->active ? ce->value : "/var/lib/kdm";
    if (mkdirp (dir, 0755, "data", 0) && oldkde) {
	ASPrintf (&oldsts, "%s/kdm/kdmsts", oldkde);
	ASPrintf (&newsts, "%s/kdmsts", dir);
	rename (oldsts, newsts);
    }
}

static void
CopyFile (const char *from, const char *to)
{
    File file;
    int fd;

    if (readFile (&file, from)) {
	if ((fd = creat (to, 0644)) >= 0) {
	    write (fd, file.buf, file.eof - file.buf);
	    close (fd);
	}
	freeBuf (&file);
    }
}

static void
upd_facedir(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *oldpic, *newpic, *defpic, *rootpic;
    const char *dir;
    struct passwd *pw;

    if (use_destdir)
	return;
    dir = ce->active ? ce->value : KDMDATA "/faces";
    if (mkdirp (dir, 0755, "user face", 0)) {
	ASPrintf (&defpic, "%s/.default.face.icon", dir);
	ASPrintf (&rootpic, "%s/root.face.icon", dir);
	if (oldkde) {
	    setpwent ();
	    while ((pw = getpwent()))
		if (strcmp (pw->pw_name, "root")) {
		    ASPrintf (&oldpic, "%s/../apps/kdm/pics/users/%s.png",
					oldkde, pw->pw_name);
		    ASPrintf (&newpic, "%s/%s.face.icon", dir, pw->pw_name);
		    rename (oldpic, newpic);
		    free (newpic);
		    free (oldpic);
		}
	    endpwent ();
	    ASPrintf (&oldpic, "%s/../apps/kdm/pics/users/default.png", oldkde);
	    if (!rename (oldpic, defpic))
		defpic = 0;
	    ASPrintf (&oldpic, "%s/../apps/kdm/pics/users/root.png", oldkde);
	    if (!rename (oldpic, rootpic))
		rootpic = 0;
	}
	if (defpic) {
	    ASPrintf (&oldpic, "%s/default1.png", facesrc);
	    CopyFile (oldpic, defpic);
	}
	if (rootpic) {
	    ASPrintf (&oldpic, "%s/root1.png", facesrc);
	    CopyFile (oldpic, rootpic);
	}
    }
}

static Ent entsGeneral[] = {
{ "ConfigVersion",	0, 0, 
"# This option exists solely for the purpose of a clean automatic upgrade.\n"
"# Don't even think about changing it!\n" },
{ "Xservers",		0, mk_xservers,
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
"# Additional environment variables KDM should pass on to kdm_config, kdm_greet,\n"
"# Xsetup, Xstartup, Xsession, and Xreset. LD_LIBRARY_PATH and XCURSOR_THEME are\n"
"# good candidates; otherwise it shouldn't be necessary very often.\n" },
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
{ "DataDir",		0, upd_datadir,
"# The directory kdm should store persistent working data in.\n"
"# Default is /var/lib/kdm\n" },
{ "DmrcDir",		0, 0,
"# The directory kdm should store users' .dmrc files in. This is only needed\n"
"# if the home directories are not readable before actually logging in (like\n"
"# with AFS). Default is \"\"\n" },
};

static Ent entsXdmcp[] = {
{ "Enable",		0, 0, 
"# Whether KDM should listen to XDMCP requests. Default is true.\n" },
{ "Port",		0, 0, 
"# The UDP port KDM should listen on for XDMCP requests. Don't change the 177.\n" },
{ "KeyFile",		0, cp_keyfile, 
"# File with the private keys of X-terminals. Required for XDM authentication.\n"
"# Default is \"\"\n" },
{ "Xaccess",		0, mk_xaccess, 
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
{ "Willing",		0, mk_willing, 
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
{ "AllowFifoNow",	0, 0, 
"# Whether one can abort still running sessions when shutting down the system\n"
"# via the global command FiFo. Default is true\n" },
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
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
{ "Resources",		0, cp_resources, 
"# Specify a file with X-resources for the greeter, chooser and background.\n"
"# The KDE frontend doesn't care for this, so you don't need it unless you\n"
"# use an alternative chooser or another background generator than kdmdesktop.\n"
"# Default is \"\"\n" },
{ "Xrdb",		0, 0, 
"# The xrdb program to use to read the above specified recources.\n"
"# Default is " XBINDIR "/xrdb\n" },
{ "Setup",		0, mk_setup, 
"# A program to run before the greeter is shown. You should start kdmdesktop\n"
"# there. Also, xconsole can be started by this script.\n"
"# Default is \"\"\n" },
{ "Startup",		0, mk_startup, 
"# A program to run before a user session starts. You should invoke sessreg\n"
"# there and optionally change the ownership of the console, etc.\n"
"# Default is \"\"\n" },
{ "Reset",		0, mk_reset, 
"# A program to run after a user session exits. You should invoke sessreg\n"
"# there and optionally change the ownership of the console, etc.\n"
"# Default is \"\"\n" },
{ "Session",		0, mk_session, 
"# The program which is run as the user which logs in. It is supposed to\n"
"# interpret the session argument (see SessionsDirs) and start an appropriate\n"
"# session according to it.\n"
"# Default is " XBINDIR "/xterm -ls -T\n" },
{ "FailsafeClient",	0, 0, 
"# The program to run if Session fails.\n"
"# Default is " XBINDIR "/xterm\n" },
{ "UserPath",		0, ck_userpath, 
"# The PATH for the Session program. Default is\n"
"# " DEF_USER_PATH "\n" },
{ "SystemPath",		0, ck_systempath, 
"# The PATH for Setup, Startup and Reset, etc. Default is\n"
"# " DEF_SYSTEM_PATH "\n" },
{ "SystemShell",	0, 0, 
"# The default system shell. Default is /bin/sh\n" },
{ "UserAuthDir",	0, 0, 
"# Where to put the user's X-server authorization file if ~/.Xauthority\n"
"# cannot be created. Default is /tmp\n" },
{ "AutoReLogin",	0, 0, 
"# If \"true\", KDM will automatically restart a session after an X-server\n"
"# crash (or if it is killed by Alt-Ctrl-BackSpace). Note, that enabling\n"
"# this opens a security hole: a secured display lock can be circumvented\n"
"# (unless you use KDE's built-in screen lock). Default is false\n" },
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
{ "InteractiveSd",	0, 0, 
"# If this is false the user must select the shutdown condition/timing already\n"
"# in the shutdown dialog. If this is true he won't be bothered with the options,\n"
"# but will be asked what to do if sessions are actually open. Default is true\n"
"# NOTE: the interaction is currently not implemented. If this is set to true,\n"
"# a normal forced shutdown will happen (without caring for the AllowSdForceNow\n"
"# option!), i.e., KDM will behave exactly as before KDE 3.0.\n" }, 
{ "ServerAttempts",	0, 0, 
"# How often to try to run the X-server. Running includes executing it and\n"
"# waiting for it to come up. Default is 1\n" },
{ "ServerTimeout",	0, 0, 
"# How long to wait for a local X-server to come up. Default is 15\n" },
{ "NoPassEnable",	0, 0, 
"# Enable password-less logins on this display. USE WITH EXTREME CARE!\n"
"# Default is false\n" },
{ "NoPassUsers",	0, 0, 
"# The users that don't need to provide a password to log in. NEVER list root!\n"
"# Default is \"\"\n" },
{ "AutoLoginEnable",	0, 0, 
"# Enable automatic login on this display. USE WITH EXTREME CARE!\n"
"# Default is false\n" },
{ "AutoLoginUser",	0, 0, 
"# The user to log in automatically. NEVER specify root! Default is \"\"\n" },
{ "AutoLoginPass",	0, 0, 
"# The password for the user to log in automatically. This is NOT required\n"
"# unless the user is to be logged into a NIS or Kerberos domain. If you use\n"
"# it, you should \"chmod 600 kdmrc\" for obvious reasons. Default is \"\"\n" },
{ "SessionsDirs",	0, 0, 
"# The directories containing session type definitions in .desktop format.\n"
"# Default is " KDMDATA "/sessions\n" },
};

static Ent entsGreeter[] = {
{ "GUIStyle",		0, upd_guistyle, 
"# Widget style of the greeter. \"\" means the built-in default which currently\n"
"# is \"Keramik\". Default is \"\"\n" },
{ "ColorScheme",		0, 0, 
"# Widget color scheme of the greeter. \"\" means the built-in default which\n"
"# currently is quite greyish. Default is \"\"\n" },
{ "LogoArea",		0, upd_logoarea, 
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
"# The numbering starts with 0 and corresponds to the listing order in the\n"
"# active ServerLayout section of XF86Config. -1 means to use the upper-left\n"
"# screen, -2 means to use the upper-right screen. Default is 0\n" },
{ "GreetString",	0, upd_greetstring, 
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
"# The font for the headline. Default is charter,20,bold\n" },
{ "StdFont",		0, 0, 
"# The normal font used in the greeter. Default is helvetica,10\n" },
{ "FailFont",		0, 0, 
"# The font used for the \"Login Failed\" message. Default is helvetica,10,bold\n" },
{ "AntiAliasing",	0, 0,
"# Whether the fonts shown in the greeter should be antialiased. Default is false\n" },
{ "NumLock",		0, 0,
"# What to do with the Num Lock modifier for the time the greeter is running:\n"
"# \"On\" -> - turn on\n"
"# \"Off\" -> - turn off\n"
"# \"Keep\" -> - don't change the state (Default)\n" },
{ "Language",		0, upd_language, 
"# Language to use in the greeter. Default is en_US\n" },
{ "ShowUsers",		0, upd_showusers, 
"# Specify, which user names (along with pictures) should be shown in the\n"
"# greeter.\n"
"# \"NotHidden\" - all users except those listed in HiddenUsers (Default)\n"
"# \"Selected\" - only the users listed in SelectedUsers\n"
"# \"None\" - no user list will be shown at all\n" },
{ "SelectedUsers",	0, 0, 
"# For ShowUsers=Selected. Default is \"\"\n" },
{ "HiddenUsers",	1, upd_hiddenusers,
"# For ShowUsers=NotHidden. Default is \"\"\n" },
{ "MinShowUID",		0, upd_minshowuid, 
"# Special case of HiddenUsers: users with a UID less than this number\n"
"# (except root) will not be shown as well. Default is 0\n" },
{ "MaxShowUID",		0, upd_maxshowuid, 
"# Complement to MinShowUID: users with a UID greater than this number will\n"
"# not be shown as well. Default is 65535\n" },
{ "SortUsers",		0, 0, 
"# If false, the users are listed in the order they appear in /etc/passwd.\n"
"# If true, they are sorted alphabetically. Default is true\n" },
{ "FaceSource",		0, 0, 
"# Specify, where the users' pictures should be taken from.\n"
"# \"AdminOnly\" - from <FaceDir>/$USER.face[.icon] (Default)\n"
"# \"UserOnly\" - from the user's $HOME/.face[.icon]\n"
"# \"PreferAdmin\" - prefer <FaceDir>, fallback on $HOME\n"
"# \"PreferUser\" - ... and the other way round\n" },
{ "FaceDir",		0, upd_facedir, 
"# The directory containing the user images if FaceSource is not UserOnly.\n"
"# Default is " KDE_DATADIR "/kdm/faces\n" },
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
{ "EchoMode",		0, upd_echomode, 
"# The password input fields cloak the typed in text. Specify, how to do it:\n"
"# \"NoEcho\" - nothing is shown at all, the cursor doesn't move\n"
"# \"OneStar\" - \"*\" is shown for every typed letter (Default)\n"
"# \"ThreeStars\" - \"***\" is shown for every typed letter\n" },
{ "UseBackground",	0, 0,
"# If true, krootimage will be automatically started by KDM. Otherwise, the\n"
"# Setup script should be used to setup the background. Default is true\n" },
{ "BackgroundCfg",	0, handBgCfg,
"# The configuration file to be used by krootimage.\n"
"# Default is " KDMCONF "/backgroundrc\n" },
{ "GrabServer",		0, 0, 
"# Hold the X-server grabbed the whole time the greeter is visible. This\n"
"# may be more secure, but it will disable any background and other\n"
"# X-clients started from the Setup script. Default is false\n" },
{ "GrabTimeout",	0, 0, 
"# How many seconds to wait for grab to succeed. Default is 3\n" },
{ "AuthComplain",	0, 0, 
"# Warn, if local X-authorization cannot be created. Default is true\n"
"# XXX this is a dummy currently\n" },
{ "LoginMode",	0, 0, 
"# Specify whether the greeter of local displays should start up in host chooser\n"
"# (remote) or login (local) mode and whether it is allowed to switch to the\n"
"# other mode.\n"
"# \"LocalOnly\" - only local login possible (Default)\n"
"# \"RemoteOnly\" - only choice of remote host possible\n"
"# \"DefaultLocal\" - start up in local mode, but allow switch to remote mode\n"
"# \"DefaultRemote\" - ... and the other way round\n" },
{ "ChooserHosts",	0, 0, 
"# A list of hosts to be automatically added to the remote login menu. The\n"
"# special name \"*\" means broadcast. Default is \"*\"\n" },
{ "ForgingSeed",	0, upd_forgingseed,
"# Use this number as a random seed when forging saved session types, etc. of\n"
"# unknown users. This is used to avoid telling an attacker about existing users\n"
"# by reverse conclusion. This value should be random but constant across the\n"
"# login domain. Default is 0\n" },
#ifdef WITH_KDM_XCONSOLE
{ "ShowLog",		0, 0, 
"# Enable KDM's built-in xconsole. Note, that this can be enabled for only\n"
"# one display at a time. Default is false\n" },
{ "LogSource",		0, 0, 
"# The data source for KDM's built-in xconsole. The default \"\" means that\n"
"# a console log redirection should be requested from /dev/console.\n" },
#endif
{ "PluginsLogin",	0, 0,
"# Specify greeter plugins that can be used to obtain authentication data.\n"
"# This can be a plugin's base name (expands to $kde_modulesdir/kgreet_$base)\n"
"# or a full pathname. Default is classic\n" },
{ "PluginsShutdown",	0, 0,
"# Same as PluginsLogin, but for the shutdown dialog.\n" },
{ "PluginOptions",	0, 0,
"# A list of options of the form Key=Value. The conversation plugins can query\n"
"# these settings; it's up to them what possible keys are.\n" },
{ "AllowConsole",	0, 0,
"# Show the \"Console Login\" action in the greeter (when the respective @tty\n"
"# entry exists in Xservers). Default is true\n" },
{ "AllowClose",		0, 0,
"# Show the \"Restart X Server\"/\"Close Connection\" action in the greeter.\n"
"# Default is false\n" },
};

static Sect
    secGeneral	= { "General",	entsGeneral, as(entsGeneral) },
    secXdmcp	= { "Xdmcp",	entsXdmcp, as(entsXdmcp) },
    secShutdown	= { "Shutdown",	entsShutdown, as(entsShutdown) },
    sec_Core	= { "-Core",	entsCore, as(entsCore) },
    sec_Greeter	= { "-Greeter",	entsGreeter, as(entsGreeter) },
    *allSects[] = { &secGeneral, &secXdmcp, &secShutdown,
		    &sec_Core, &sec_Greeter };

static Sect *
findSect (const char *name)
{
    const char *p;
    int i;

    p = strrchr (name, '-');
    if (!p)
	p = name;
    for (i = 0; i < as(allSects); i++)
	if (!strcmp (allSects[i]->name, p))
	    return allSects[i];
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


/*
 * defaults
 */

typedef struct DEnt {
	const char	*key;
	const char	*value;
	int		active;
} DEnt;

static DEnt dEntsGeneral[] = {
{ "ConfigVersion",	"", 0 },	/* will be overridden on writeout */
{ "Xservers",		"", 0 },
{ "PidFile",		"/var/run/kdm.pid", 1 },
{ "LockPidFile",	"false", 0 },
{ "AuthDir",		"/tmp", 0 },
{ "AutoRescan",		"false", 0 },
#ifdef sun
{ "ExportList",		"LD_LIBRARY_PATH", 1 },
#else
{ "ExportList",		"SOME_VAR,ANOTHER_IMPORTANT_VAR", 0 },
#endif
#if !defined(__linux__) && !defined(__OpenBSD__)
{ "RandomFile",		"", 0 },
#endif
{ "FifoDir",		"/tmp", 0 },
{ "FifoGroup",		"xdmctl", 0 },
{ "DataDir",		"/var/lib/kdm", 0 },
{ "DmrcDir",		"/nfs-shared/var/dmrcs", 0 },
};

DEnt dEntsXdmcp[] = {
{ "Enable",		"false", 1 },
{ "Port",		"177", 0 },
{ "KeyFile",		KDMCONF "/kdmkeys", 0 },
{ "Xaccess",		"", 0 },
{ "ChoiceTimeout",	"10", 0 },
{ "RemoveDomainname",	"false", 0 },
{ "SourceAddress",	"true", 0 },
{ "Willing",		"", 0 },
};

static DEnt dEntsShutdown[] = {
{ "HaltCmd",		"", 0 },
{ "RebootCmd",		"", 0 },
{ "AllowFifo",		"true", 0 },
{ "AllowFifoNow",	"false", 0 },
#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
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
{ "Setup",		"", 0 },
{ "Startup",		"", 0 },
{ "Reset",		"", 0 },
{ "Session",		"", 0 },
{ "FailsafeClient",	"", 0 },
{ "UserPath",		"", 0 },
{ "SystemPath",		"", 0 },
{ "SystemShell",	"/bin/bash", 0 },
{ "UserAuthDir",	"", 0 },
{ "AutoReLogin",	"true", 0 },
{ "AllowRootLogin",	"false", 1 },
{ "AllowNullPasswd",	"false", 1 },
{ "AllowShutdown",	"Root", 1 },
{ "AllowSdForceNow",	"Root", 0 },
{ "DefaultSdMode",	"ForceNow", 0}, 
{ "InteractiveSd",	"false", 0}, 
{ "SessionsDirs",	"/etc/X11/sessions,/usr/share/xsessions", 0 },
};

static DEnt dEntsAnyGreeter[] = {
{ "GUIStyle",		"Windows", 0 },
{ "ColorScheme",	"Pumpkin", 0 },
{ "LogoArea",		"None", 0 },
{ "LogoPixmap",		"", 0 },
{ "GreeterPosFixed",	"true", 0 },
{ "GreeterPosX",	"200", 0 },
{ "GreeterPosY",	"100", 0 },
{ "GreetString",	"K Desktop Environment (%n)", 0 },
{ "GreetFont",		"charter,20,5,0,50,0", 0 },
{ "StdFont",		"helvetica,10,5,0,50,0", 0 },
{ "FailFont",		"helvetica,10,5,0,75,0", 0 },
{ "AntiAliasing",	"true", 0 },
{ "NumLock",		"Off", 0 },
{ "Language",		"de_DE", 0 },
{ "ShowUsers",		"None", 0 },
{ "SelectedUsers",	"root,johndoe", 0 },
{ "HiddenUsers",	"root", 0 },
{ "MinShowUID",		"", 0 },
{ "MaxShowUID",		"", 0 },
{ "SortUsers",		"false", 0 },
{ "FaceSource",		"PreferUser", 0 },
{ "FaceDir",		"/usr/share/faces", 0 },
{ "PreselectUser",	"Previous", 0 },
{ "DefaultUser",	"ethel", 0 },
{ "FocusPasswd",	"true", 0 },
{ "EchoMode",		"NoEcho", 0 },
{ "UseBackground",	"false", 0 },
{ "BackgroundCfg",	"", 0 },
{ "GrabServer",		"true", 0 },
{ "GrabTimeout",	"", 0 },
{ "ForgingSeed",	"", 0 },
{ "PluginsLogin",	"sign", 0 },
{ "PluginsShutdown",	"modern", 0 },
{ "PluginOptions",	"SomeKey=randomvalue,Foo=bar", 0 },
{ "AllowConsole",	"false", 0 },
{ "AllowClose",	        "true", 0 },
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
{ "GreeterScreen",	"-1", 0 },
{ "LoginMode",		"DefaultLocal", 1 },
{ "ChooserHosts",	"*,ugly,sky,dino,kiste.local,login.crap.com", 0 },
};

static DEnt dEnts0Core[] = {
{ "AutoLoginEnable",	"true", 0 },
{ "AutoLoginUser",	"fred", 0 },
{ "AutoLoginPass",	"secret!", 0 },
};

static DEnt dEnts0Greeter[] = {
{ "PreselectUser",	"Default", 0 },
{ "DefaultUser",	"johndoe", 0 },
#ifdef WITH_KDM_XCONSOLE
{ "ShowLog",		"true", 1 },
{ "LogSource",		"/dev/xconsole", 1 },
#endif
};

typedef struct DSect {
	const char	*name;
	DEnt		*ents;
	int		nents;
	int		active;
	const char	*comment;
} DSect;

static DSect dAllSects[] = { 
{ "General",		dEntsGeneral, as(dEntsGeneral), 1,
"# KDM master configuration file\n"
"#\n"
"# Definition: the greeter is the login dialog, i.e., the part of KDM\n"
"# which the user sees.\n"
"#\n"
"# You can configure every X-display individually.\n"
"# Every display has a display name, which consists of a host name\n"
"# (which is empty for local displays specified in the Xservers file),\n"
"# a colon and a display number. Additionally, a display belongs to a\n"
"# display class (which can be ignored in most cases; the control center\n"
"# does not support this feature at all).\n"
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
"# Every comment applies to the following section or key. Note, that all\n"
"# comments will be lost if you change this file with the kcontrol frontend.\n"
"# The defaults refer to KDM's built-in values, not anything set in this file.\n"
"#\n"
"\n" },
{ "Xdmcp",		dEntsXdmcp, as(dEntsXdmcp), 1, 0 },
{ "Shutdown",		dEntsShutdown, as(dEntsShutdown), 1, 0 },
{ "X-*-Core",		dEntsAnyCore, as(dEntsAnyCore), 1, 
"# Rough estimations about how many seconds KDM will spend at most on\n"
"# - opening a connection to the X-server (OpenTime):\n"
"#   OpenRepeat * (OpenTimeout + OpenDelay)\n"
"# - starting a local X-server (ServerTime): ServerAttempts * ServerTimeout\n"
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

static void
mkdefconf (void)
{
    Section *cs, **csp;
    Entry *ce, **cep;
    int sc, ec;

    for (csp = &config, sc = 0; sc < as(dAllSects); csp = &(cs->next), sc++) {
	cs = mcalloc (sizeof(*cs));
	*csp = cs;
	cs->spec = findSect (dAllSects[sc].name);
	cs->name = dAllSects[sc].name;
	cs->comment = dAllSects[sc].comment;
	cs->active = dAllSects[sc].active;
	for (cep = &(cs->ents), ec = 0; ec < dAllSects[sc].nents; 
	     cep = &(ce->next), ec++) {
	    ce = mcalloc (sizeof(*ce));
	    *cep = ce;
	    ce->spec = findEnt (cs->spec, dAllSects[sc].ents[ec].key);
	    ce->value = dAllSects[sc].ents[ec].value;
	    ce->active = dAllSects[sc].ents[ec].active;
	}
    }
}


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
    usedFile (fname);

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
#if 0	/* not our business ... */
		    fprintf (stderr, "Warning: Multiple occurrences of section "
				     "[%.*s] in %s. Consider merging them.\n", 
			     nlen, nstr, fname);
#endif
		    goto secfnd;
		}
	    cursec = mmalloc (sizeof(*cursec));
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
		fprintf (stderr, "Multiple occurrences of key '%s' in section "
			 "[%s] of %s.\n", curent->key, cursec->name, fname);
		goto keyfnd;
	    }
	curent = mmalloc (sizeof (*curent));
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

/* the mergeKdmRc* functions should be data-driven; upd_* should go here */

static int
mergeKdmRcOld (const char *path)
{
    char *p;

    ASPrintf (&p, "%s/kdmrc", path);
    if (!cfgRead(p)) {
	free (p);
	return 0;
    }
    printf ("Information: reading old kdmrc %s (from kde < 2.2)\n", p);
    free (p);

    if (cfgSGroup ("Desktop0")) {
	REntry *ce;
	background = mstrdup ("[Desktop0]\n");
	for (ce = cursect->ents; ce; ce = ce->next)
	    StrCat (&background, "%s=%s\n", ce->key, ce->value);
    } else if (cfgSGroup ("KDMDESKTOP")) {
	background = mstrdup ("[Desktop0]\n");
	p = cfgEnt ("BackGroundPictureMode");
	if (!p || !strcmp(p, "None")) {
	    p = cfgEnt ("BackGroundColorMode");
	    if (!p || !strcmp(p, "Plain"))
		StrCat (&background, "BackgroundMode=Flat\n");
	    else if (!strcmp(p, "Vertical"))
		StrCat (&background, "BackgroundMode=VerticalGradient\n");
	    else if (!strcmp(p, "Horizontal"))
		StrCat (&background, "BackgroundMode=HorizontalGradient\n");
	    StrCat (&background, "WallpaperMode=NoWallpaper\n");
	} else {
	    if (!strcmp(p, "Tile"))
		StrCat (&background, "WallpaperMode=Tiled\n");
	    else if (!strcmp(p, "Scale"))
		StrCat (&background, "WallpaperMode=Scaled\n");
	    else
		StrCat (&background, "WallpaperMode=Centered\n");
	    StrCat (&background, "BackgroundMode=Wallpaper\n");
	}
	if ((p = cfgEnt ("BackGroundPicture")))
	    StrCat (&background, "Wallpaper=%s\n", p);
	if ((p = cfgEnt ("BackGroundColor1")))
	    StrCat (&background, "Color1=%s\n", p);
	if ((p = cfgEnt ("BackGroundColor2")))
	    StrCat (&background, "Color2=%s\n", p);
    }

    setsect ("X-*-Greeter");

    if (cfgSGroup ("Locale"))
	cpyval ("Language", 0);

    if (cfgSGroup ("KDM")) {
	cpyval ("GreetString", 0);
	cpyval ("EchoMode", 0);
	cpyval ("GUIStyle", 0);
	cpyval ("StdFont", 0);
	cpyval ("GreetFont", 0);
	cpyval ("FailFont", 0);
	cpyval ("MinShowUID", "UserIDLow");
	cpyval ("MinShowUID", 0);
	cpyval ("SortUsers", 0);
	cpyval ("SelectedUsers", "Users");
	cpyval ("HiddenUsers", "NoUsers");
	cpyval ("ShowUsers", 0);
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
	cpyval ("LogoArea", 0);
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
    }

#if defined(__linux__) && ( defined(__i386__) || defined(__amd64__) )
    if (cfgSGroup ("Lilo")) {
	setsect("Shutdown");
	cpyval ("UseLilo", "Lilo");
	cpyval ("LiloCmd", "LiloCommand");
	cpyval ("LiloMap", 0);
    }
#endif

    return 1;
}

typedef struct {
    const char *sect, *key, *def;
    int (*cond)(void);
} FDefs;

static void
applydefs (FDefs *chgdef, int ndefs, const char *path)
{
    char *p;
    int i;

    for (i = 0; i < ndefs; i++)
	if (!getfqval (chgdef[i].sect, chgdef[i].key, 0) &&
	    (!chgdef[i].cond || chgdef[i].cond()))
	{
	    ASPrintf (&p, chgdef[i].def, path);
	    putfqval (chgdef[i].sect, chgdef[i].key, p);
	    free (p);
	}
}

static FDefs kdmdefs_all[] = {
{ "Xdmcp",	"Xaccess",	"%s/kdm/Xaccess",	0		},
{ "Xdmcp",	"Willing",	"",			0		},
};

static FDefs kdmdefs_eq_22[] = {
{ "General",	"Xservers",	"%s/kdm/Xservers", 	0		},
{ "General",	"PidFile",	"/var/run/xdm.pid", 	0		},
{ "X-*-Core",	"Setup",	"%s/kdm/Xsetup", 	0		},
{ "X-*-Core",	"Startup",	"%s/kdm/Xstartup", 	0		},
{ "X-*-Core",	"Reset",	"%s/kdm/Xreset", 	0		},
{ "X-*-Core",	"Session",	"%s/kdm/Xsession", 	0		},
};

static int
if_xdmcp (void)
{
    return isTrue (getfqval ("Xdmcp", "Enable", "true"));
}

static FDefs kdmdefs_le_30[] = {
{ "Xdmcp",	"KeyFile",	"%s/kdm/kdmkeys",	if_xdmcp	},
};

/* HACK: misused by is22conf() below */
static FDefs kdmdefs_ge_30[] = {
{ "X-*-Core",	"Setup",	"",			0		},
{ "X-*-Core",	"Startup",	"",			0		},
{ "X-*-Core",	"Reset",	"",			0		},
{ "X-*-Core",	"Session",	XBINDIR "/xterm -ls -T", 0		},
};

static int
if_usebg (void)
{
    return isTrue (getfqval ("X-*-Greeter", "UseBackground", "true"));
}

static FDefs kdmdefs_ge_31[] = {
{ "X-*-Greeter","BackgroundCfg","%s/kdm/backgroundrc",	if_usebg	},
};

static int
is22conf (const char *path)
{
    char *p;
    const char *val;
    int i, sl;

    sl = ASPrintf (&p, "%s/kdm/", path);
    /* safe bet, i guess ... */
    for (i = 0; i < 4; i++) {
	val = getfqval ("X-*-Core", kdmdefs_ge_30[i].key, 0);
	if (val && !memcmp (val, p, sl)) {
	    free (p);
	    return 0;
	}
    }
    free (p);
    return 1;
}

static int
mergeKdmRcNewer (const char *path)
{
    char *p, *p2;
    const char *cp;

    ASPrintf (&p, "%s/kdm/kdmrc", path);
    if (!cfgRead(p)) {
	free (p);
	return 0;
    }
    printf ("Information: reading old kdmrc %s (from kde >= 2.2.x)\n", p);
    free (p);

    if (cfgSGroup ("Desktop0")) {
	REntry *cre;
	background = mstrdup ("[Desktop0]\n");
	for (cre = cursect->ents; cre; cre = cre->next)
	    StrCat (&background, "%s=%s\n", cre->key, cre->value);
    }

    cpygroup (&secGeneral);
    cpygroup (&secXdmcp);
    cpygroup (&secShutdown);

    if (cfgSGroup ("Xdmcp"))
	cpyfqval ("Xdmcp", "Willing", "Xwilling");

    for (cursect = rootsect; cursect; cursect = cursect->next)
	if (!strncmp (cursect->name, "X-", 2)) {
	    setsect(cursect->name);
	    p = strrchr (cursect->name, '-');
	    if (!strcmp (p + 1, "Core")) {
		cpygents (&sec_Core);
	    } else if (!strcmp (p + 1, "Greeter")) {
		cpygents (&sec_Greeter);
		ASPrintf (&p2, "%.*s-Core", 
			  p - cursect->name, cursect->name);
		cpyfqval (p2, "AllowShutdown", 0);
		free (p2);
		cpyval ("HiddenUsers", "NoUsers");
		cpyval ("SelectedUsers", "Users");
		if ((p = cfgEnt ("EnableChooser")))	/* from make_it_cool branch and SuSE 8.1 */
		    putval ("LoginMode",
			    isTrue (p) ? "DefaultLocal" : "LocalOnly");
	    }
	}

    applydefs (kdmdefs_all, as(kdmdefs_all), path);
    if (!*(cp = getfqval ("General", "ConfigVersion", ""))) {	/* < 3.1 */
	mod_usebg = 1;
	if (is22conf (path)) {
	    /* work around 2.2.x defaults borkedness */
	    applydefs (kdmdefs_eq_22, as(kdmdefs_eq_22), path);
	    printf ("Information: old kdmrc is from kde 2.2\n");
	} else {
	    applydefs (kdmdefs_ge_30, as(kdmdefs_ge_30), path);
	    printf ("Information: old kdmrc is from kde 3.0\n");
	}
	/* work around minor <= 3.0.x defaults borkedness */
	applydefs (kdmdefs_le_30, as(kdmdefs_le_30), path);
    } else {
	int ma, mi;
	sscanf (cp, "%d.%d", &ma, &mi);
	oldver = (ma << 8) | mi;
	printf ("Information: old kdmrc is from kde >= 3.1 (config version %d.%d)\n", ma, mi);
	applydefs (kdmdefs_ge_30, as(kdmdefs_ge_30), path);
	applydefs (kdmdefs_ge_31, as(kdmdefs_ge_31), path);
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
{ "noPassUsers", "X-%s-Core", 0, P_noPassUsers },
{ "autoUser", "X-%s-Core", "AutoLoginUser", P_autoUser },
{ "autoPass", "X-%s-Core", "AutoLoginPass", P_autoPass },
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

static FDefs xdmdefs[] = {
{ "Xdmcp",	"Xaccess",	"%s/Xaccess",		0	},
{ "Xdmcp",	"Willing",	"",			0	},
{ "X-*-Core",	"Setup",	"",			0	},
{ "X-*-Core",	"Startup",	"",			0	},
{ "X-*-Core",	"Reset",	"",			0	},
{ "X-*-Core",	"Session",	"",			0	},
};

static int
mergeXdmCfg (const char *path)
{
    char *p;
    XrmDatabase db;
    int i;

    for (i = 0; i < as(xdmconfs); i++) {
	ASPrintf (&p, xdmconfs[i], path);
	if ((db = XrmGetFileDatabase (p))) {
	    printf ("Information: reading old xdm config file %s\n", p);
	    usedFile (p);
	    free (p);
	    xdmpath = path;
	    XrmEnumerateDatabase(db, &empty, &empty, XrmEnumAllLevels,
				 DumpEntry, (XPointer) 0);
	    applydefs (xdmdefs, as(xdmdefs), path);
	    mod_usebg = 1;
	    return 1;
	}
	free (p);
    }
    return 0;
}

static void
fwrapprintf (FILE *f, const char *msg, ...)
{
    char *txt, *ftxt, *line;
    va_list ap;
    int col, lword, fspace;

    va_start (ap, msg);
    VASPrintf (&txt, msg, ap);
    va_end (ap);
    ftxt = 0;
    for (line = txt, col = 0, lword = fspace = -1; line[col]; ) {
	if (line[col] == '\n') {
	    StrCat (&ftxt, "%.*s", ++col, line);
	    line += col;
	    col = 0;
	    lword = fspace = -1;
	    continue;
	} else if (line[col] == ' ') {
	    if (lword >= 0) {
		fspace = col;
		lword = -1;
	    }
	} else {
	    if (lword < 0)
		lword = col;
	    if (col >= 78 && fspace >= 0) {
		StrCat (&ftxt, "%.*s\n", fspace, line);
		line += lword;
		col -= lword;
		lword = 0;
		fspace = -1;
	    }
	}
	col++;
    }
    free (txt);
    fputs (ftxt, f);
    free (ftxt);
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
    Section *cs;
    Entry *ce, **cep;
    int i, ap, newer, locals, foreigns;
    int no_old_xdm = 0, no_old_kde = 0;
    struct stat st;
    char nname[80];

    for (ap = 1; ap < argc; ap++) {
	if (!strcmp(argv[ap], "--help")) {
	    printf (
"genkdmconf - generate configuration files for kdm\n"
"\n"
"If an older xdm/kdm configuration is found, its config files are \"absorbed\";\n"
"if it lives in the new target directory, its scripts are reused (and possibly\n"
"modified) as well, otherwise the scripts are ignored and default scripts are\n"
"installed.\n"
"\n"
"options:\n"
"  --in /path/to/new/kdm-config-dir\n"
"    In which directory to put the new configuration. You can use this\n"
"    to support a $(DESTDIR), but not to change the final location of\n"
"    the installation - the paths inside the files are not affected.\n"
"    Default is " KDMCONF ".\n"
"  --old-xdm /path/to/old/xdm-dir\n"
"    Where to look for the config files of an xdm/older kdm.\n"
"    Default is to scan /etc/X11/kdm, $XLIBDIR/kdm, /etc/X11/xdm,\n"
"    $XLIBDIR/xdm; there in turn look for kdm-config and xdm-config.\n"
"    Note that you possibly need to use --no-old-kde to make this take effect.\n"
"  --old-kde /path/to/old/kde-config-dir\n"
"    Where to look for the kdmrc of an older kdm.\n"
"    Default is to scan " KDE_CONFDIR " and\n"
"    {/usr,/usr/local,{/opt,/usr/local}/{kde3,kde,kde2,kde1}}/share/config.\n"
"  --no-old\n"
"    Don't look at older xdm/kdm configurations, just create default config.\n"
"  --no-old-xdm\n"
"    Don't look at older xdm configurations.\n"
"  --no-old-kde\n"
"    Don't look at older kdm configurations.\n"
"  --old-scripts\n"
"    Directly use all scripts from the older xdm/kdm configuration.\n"
"  --no-old-scripts\n"
"    Don't use scripts from the older xdm/kdm configuration even if it lives\n"
"    in the new target directory.\n"
"  --old-confs\n"
"    Directly use all ancillary config files from the older xdm/kdm\n"
"    configuration. This is usually a bad idea.\n"
"  --no-backup\n"
"    Overwrite/delete old config files instead of backing them up.\n"
"  --no-in-notice\n"
"    Don't put the notice about --in being used into the generated README.\n"
);
	    exit (0);
	}
	if (!strcmp(argv[ap], "--no-old")) {
	    no_old = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--old-scripts")) {
	    old_scripts = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--no-old-scripts")) {
	    no_old_scripts = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--old-confs")) {
	    old_confs = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--no-old-xdm")) {
	    no_old_xdm = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--no-old-kde")) {
	    no_old_kde = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--no-backup")) {
	    no_backup = 1;
	    continue;
	}
	if (!strcmp(argv[ap], "--no-in-notice")) {
	    no_in_notice = 1;
	    continue;
	}
	where = 0;
	if (!strcmp(argv[ap], "--in"))
	    where = &newdir;
	else if (!strcmp(argv[ap], "--old-xdm"))
	    where = &oldxdm;
	else if (!strcmp(argv[ap], "--old-kde"))
	    where = &oldkde;
	else if (!strcmp(argv[ap], "--face-src"))
	    where = &facesrc;
	else {
	    fprintf (stderr, "Unknown command line option '%s', try --help\n", argv[ap]);
	    exit (1);
	}
	if (ap + 1 == argc || argv[ap + 1][0] == '-') {
	    fprintf (stderr, "Missing argument to option '%s', try --help\n", argv[ap]);
	    exit (1);
	}
	*where = argv[++ap];
    }
    if (memcmp (newdir, KDMCONF, sizeof(KDMCONF)))
	use_destdir = 1;

    if (!mkdirp (newdir, 0755, "target", 1))
	exit (1);

    mkdefconf();
    newer = 0;
    if (no_old) {
	DIR *dir;
	if ((dir = opendir (newdir))) {
	    struct dirent *ent;
	    char bn[PATH_MAX];
	    readdir (dir); /* . */
	    readdir (dir); /* .. */
	    while ((ent = readdir (dir))) {
		int l = sprintf (bn, "%s/%s", newdir, ent->d_name); /* cannot overflow (kernel would not allow the creation of a longer path) */
		if (!stat (bn, &st) && !S_ISREG (st.st_mode))
		    continue;
		if (no_backup || !memcmp (bn + l - 4, ".bak", 5))
		    unlink (bn);
		else
		    displace (bn);
	    }
	    closedir (dir);
	}
    } else {
	if (oldkde) {
	    if (!(newer = mergeKdmRcNewer (oldkde)) && !mergeKdmRcOld (oldkde))
		fprintf (stderr, 
			 "Cannot read old kdmrc at specified location\n");
	} else if (!no_old_kde) {
	    for (i = 0; i < as(oldkdes); i++) {
		if ((newer = mergeKdmRcNewer (oldkdes[i])) ||
		    mergeKdmRcOld (oldkdes[i])) {
		    oldkde = oldkdes[i];
		    break;
		}
	    }
	}
	if (!newer && !no_old_xdm) {
	    XrmInitialize ();
	    XrmQString = XrmPermStringToQuark("String");
	    if (oldxdm) {
		if (!mergeXdmCfg (oldxdm))
		    fprintf (stderr, 
			     "Cannot read old kdm-config/xdm-config at specified location\n");
	    } else
		for (i = 0; i < as(oldxdms); i++)
		    if (mergeXdmCfg (oldxdms[i])) {
			oldxdm = oldxdms[i];
			break;
		    }
	} else
	    oldxdm = 0;
    }
    if (no_old_scripts)
	goto no_old_s;
    if (!old_scripts) {
	locals = foreigns = 0;
	for (cs = config; cs; cs = cs->next)
	    if (cs->active && !strcmp (cs->spec->name, "-Core")) {
		for (ce = cs->ents; ce; ce = ce->next)
		    if (ce->active &&
			(!strcmp (ce->spec->key, "Setup") ||
			 !strcmp (ce->spec->key, "Startup") ||
			 !strcmp (ce->spec->key, "Reset")))
		    {
			if (inNewDir (ce->value))
			    locals = 1;
			else
			    foreigns = 1;
		    }
	    }
	if (foreigns) {
	    if (locals) {
		fprintf (stderr, 
			 "Warning: both local and foreign scripts referenced. "
			 "Won't touch any.\n");
		mixed_scripts = 1;
	    } else {
	      no_old_s:
		for (cs = config; cs; cs = cs->next) {
		    if (cs->active) {
			if (!strcmp (cs->spec->name, "Xdmcp")) {
			    for (ce = cs->ents; ce; ce = ce->next)
				if (!strcmp (ce->spec->key, "Willing"))
				    ce->active = ce->written = 0;
			} else if (!strcmp (cs->spec->name, "-Core")) {
			    for (cep = &cs->ents; (ce = *cep); ) {
				if (ce->active &&
				    (!strcmp (ce->spec->key, "Setup") ||
				     !strcmp (ce->spec->key, "Startup") ||
				     !strcmp (ce->spec->key, "Reset") ||
				     !strcmp (ce->spec->key, "Session")))
				{
				    if (!memcmp (cs->name, "X-*-", 4))
					ce->active = ce->written = 0;
				    else {
					*cep = ce->next;
					free (ce);
					continue;				    
				    }
				}
				cep = &ce->next;
			    }
			}
		    }
		}
	    }
	}
    }
#ifdef __linux__
    if (!stat( "/etc/debian_version", &st )) {	/* debian */
	defminuid = "1000";
	defmaxuid = "29999";
    } else if (!stat( "/usr/portage", &st )) {	/* gentoo */
	defminuid = "1000";
	defmaxuid = "65000";
    } else if (!stat( "/etc/mandrake-release", &st )) {	/* mandrake - check before redhat! */
	defminuid = "500";
	defmaxuid = "65000";
    } else if (!stat( "/etc/redhat-release", &st )) {	/* redhat */
	defminuid = "100";
	defmaxuid = "65000";
    } else /* if (!stat( "/etc/SuSE-release", &st )) */ {	/* suse */
	defminuid = "500";
	defmaxuid = "65000";
    }
#else
    defminuid = "1000";
    defmaxuid = "65000";
#endif
    for (i = 0; ; i++) {
	int got = 0;
	for (cs = config; cs; cs = cs->next)
	    for (ce = cs->ents; ce; ce = ce->next)
		if (i == ce->spec->prio) {
		    got = 1;
		    if (ce->spec->func)
			ce->spec->func (ce, cs);
		}
	if (!got)
	    break;
    }
    ASPrintf (&newkdmrc, "%s/kdmrc", newdir);
    f = Create (newkdmrc, kdmrcmode);
    wrconf (f);
    fclose (f);

    sprintf (nname, "%s/README", newdir);
    f = Create (nname, 0644);
    fprintf (f, 
"This automatically generated configuration consists of the following files:\n");
    fprintf (f, "- " KDMCONF "/kdmrc\n");
    for (fp = aflist; fp; fp = fp->next)
	fprintf (f, "- %s\n", fp->str);
    if (use_destdir && !no_in_notice)
	fwrapprintf (f,
"All files destined for " KDMCONF " were actually saved in %s; "
"this config won't be workable until moved in place.\n", newdir);
    if (uflist || eflist || cflist || lflist) {
	fprintf (f, 
"\n"
"This config was derived from existing files. As the used algorithms are\n"
"pretty dumb, it may be broken.\n");
	if (uflist) {
	    fprintf (f, 
"Information from these files was extracted:\n");
	    for (fp = uflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	if (lflist) {
	    fprintf (f, 
"These files were directly incorporated:\n");
	    for (fp = lflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	if (cflist) {
	    fprintf (f, 
"These files were copied verbatim:\n");
	    for (fp = cflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	if (eflist) {
	    fprintf (f, 
"These files were copied with modifications:\n");
	    for (fp = eflist; fp; fp = fp->next)
		fprintf (f, "- %s\n", fp->str);
	}
	if (!no_backup && !use_destdir)
	    fprintf (f, 
"Old files that would have been overwritten were renamed to <oldname>.bak.\n");
    }
    fprintf (f, 
"\nTry 'genkdmconf --help' if you want to generate another configuration.\n"
"\nYou may delete this README.\n");
    fclose (f);

    return 0;
}
