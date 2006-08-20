/*

Create a suitable configuration for kdm taking old xdm/kdm
installations into account

Copyright (C) 2001-2005 Oswald Buddenhagen <ossi@kde.org>


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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <X11/Xlib.h>
#include <X11/Xresource.h>

#include <config.h>
#include <config-unix.h>
#include <config-kdm.h>

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
#include <limits.h>
#include <sys/stat.h>
#include <sys/param.h>
#ifdef BSD
# include <utmp.h>
#endif

#include "config.ci"

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

#define RCVERSTR stringify(RCVERMAJOR) "." stringify(RCVERMINOR)

static int old_scripts, no_old_scripts, old_confs, no_old,
	no_backup, no_in_notice, use_destdir, mixed_scripts;
static const char *newdir = KDMCONF, *facesrc = KDMDATA "/pics/users",
	*oldxdm, *oldkde;

static int oldver;


typedef struct StrList {
	struct StrList *next;
	const char *str;
} StrList;


static void *
mmalloc( size_t sz )
{
	void *ptr;

	if (!(ptr = malloc( sz ))) {
		fprintf( stderr, "Out of memory\n" );
		exit( 1 );
	}
	return ptr;
}

static void *
mcalloc( size_t sz )
{
	void *ptr;

	if (!(ptr = calloc( 1, sz ))) {
		fprintf( stderr, "Out of memory\n" );
		exit( 1 );
	}
	return ptr;
}

static void *
mrealloc( void *optr, size_t sz )
{
	void *ptr;

	if (!(ptr = realloc( optr, sz ))) {
		fprintf( stderr, "Out of memory\n" );
		exit( 1 );
	}
	return ptr;
}

static char *
mstrdup( const char *optr )
{
	char *ptr;

	if (!optr)
		return 0;
	if (!(ptr = strdup( optr ))) {
		fprintf( stderr, "Out of memory\n" );
		exit( 1 );
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
OutCh_OCA( void *bp, char c )
{
	OCABuf *ocabp = (OCABuf *)bp;

	ocabp->tlen++;
	if (ocabp->clen >= ocabp->blen) {
		ocabp->blen = ocabp->blen * 3 / 2 + 100;
		ocabp->buf = mrealloc( ocabp->buf, ocabp->blen );
	}
	ocabp->buf[ocabp->clen++] = c;
}

static int
VASPrintf( char **strp, const char *fmt, va_list args )
{
	OCABuf ocab = { 0, 0, 0, -1 };

	DoPr( OutCh_OCA, &ocab, fmt, args );
	OutCh_OCA( &ocab, 0 );
	*strp = realloc( ocab.buf, ocab.clen );
	if (!*strp)
		*strp = ocab.buf;
	return ocab.tlen;
}

static int
ASPrintf( char **strp, const char *fmt, ... )
{
	va_list args;
	int len;

	va_start( args, fmt );
	len = VASPrintf( strp, fmt, args );
	va_end( args );
	return len;
}

static void
StrCat( char **strp, const char *fmt, ... )
{
	char *str, *tstr;
	va_list args;
	int el;

	va_start( args, fmt );
	el = VASPrintf( &str, fmt, args );
	va_end( args );
	if (*strp) {
		int ol = strlen( *strp );
		tstr = mmalloc( el + ol + 1 );
		memcpy( tstr, *strp, ol );
		memcpy( tstr + ol, str, el + 1 );
		free( *strp );
		free( str );
		*strp = tstr;
	} else
		*strp = str;
}


#define WANT_CLOSE 1

typedef struct File {
	char *buf, *eof, *cur;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
	int ismapped;
#endif
} File;

static int
readFile( File *file, const char *fn )
{
	off_t flen;
	int fd;

	if ((fd = open( fn, O_RDONLY )) < 0)
		return 0;

	flen = lseek( fd, 0, SEEK_END );
#ifdef HAVE_MMAP
# ifdef WANT_CLOSE
	file->ismapped = 0;
# endif
	file->buf = mmap( 0, flen + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0 );
# ifdef WANT_CLOSE
	if (file->buf)
		file->ismapped = 1;
	else
# else
	if (!file->buf)
# endif
#endif
	{
		file->buf = mmalloc( flen + 1 );
		lseek( fd, 0, SEEK_SET );
		if (read( fd, file->buf, flen ) != flen) {
			free( file->buf );
			close( fd );
			fprintf( stderr, "Cannot read file\n" );
			return 0; /* maybe better abort? */
		}
	}
	file->eof = file->buf + flen;
	close( fd );
	return 1;
}

#ifdef WANT_CLOSE
static void
freeBuf( File *file )
{
# ifdef HAVE_MMAP
	if (file->ismapped)
		munmap( file->buf, file->eof - file->buf );
	else
# endif
		free( file->buf );
}
#endif

static int
isTrue( const char *val )
{
	return !strcmp( val, "true" ) ||
	       !strcmp( val, "yes" ) ||
	       !strcmp( val, "on" ) ||
	       atoi( val );
}

static int
mkdirp( const char *name, int mode, const char *what, int existok )
{
	char *mfname = mstrdup( name );
	int i;
	struct stat st;

	for (i = 1; mfname[i]; i++)
		if (mfname[i] == '/') {
			mfname[i] = 0;
			if (stat( mfname, &st )) {
				if (mkdir( mfname, 0755 )) {
					fprintf( stderr, "Cannot create parent %s of %s directory %s: %s\n",
					         mfname, what, name, strerror( errno ) );
					free( mfname );
					return 0;
				}
				chmod( mfname, 0755 );
			}
			mfname[i] = '/';
		}
	free( mfname );
	if (stat( name, &st )) {
		if (mkdir( name, mode )) {
			fprintf( stderr, "Cannot create %s directory %s: %s\n",
			         what, name, strerror( errno ) );
			return 0;
		}
		chmod( name, mode );
		return 1;
	}
	return existok;
}


static void
displace( const char *fn )
{
	if (!no_backup) {
		char bn[PATH_MAX + 4];
		sprintf( bn, "%s.bak", fn ); /* won't overflow if only existing paths are passed */
		rename( fn, bn );
	} else
		unlink( fn );
}


static char *
locate( const char *exe )
{
	int len;
	char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

	if (!(path = getenv( "PATH" )))
		return 0;
	len = strlen( exe );
	name = nambuf + PATH_MAX - len;
	memcpy( name, exe, len + 1 );
	*--name = '/';
	do {
		if (!(pathe = strchr( path, ':' )))
			pathe = path + strlen( path );
		len = pathe - path;
		if (len && !(len == 1 && *path == '.')) {
			thenam = name - len;
			if (thenam >= nambuf) {
				memcpy( thenam, path, len );
				if (!access( thenam, X_OK ))
					return mstrdup( thenam );
			}
		}
		path = pathe;
	} while (*path++ != '\0');
	return 0;
}


/*
 * target data to be written to kdmrc
 */

typedef struct Entry {
	struct Entry *next;
	struct Ent *spec;
	const char *value;
	int active:1;
	int written:1;
} Entry;

typedef struct Section {
	struct Section *next;
	struct Sect *spec;
	const char *name;
	const char *comment;
	Entry *ents;
} Section;

static Section *config; /* the kdmrc data to be written */

/*
 * Specification of the (currently possible) kdmrc entries
 */

typedef struct Ent {
	const char *key;
	int prio;
	void (*func)( Entry *ce, Section *cs );
	const char *comment;
} Ent;

typedef struct Sect {
	const char *name;
	Ent *ents;
	int nents;
} Sect;

static Sect *findSect( const char *name );
static Ent *findEnt( Sect *sect, const char *key );

/*
 * Functions to manipulate the current kdmrc data
 */

static const char *
getfqval( const char *sect, const char *key, const char *defval )
{
	Section *cs;
	Entry *ce;

	for (cs = config; cs; cs = cs->next)
		if (!strcmp( cs->name, sect )) {
			for (ce = cs->ents; ce; ce = ce->next)
				if (!strcmp( ce->spec->key, key )) {
					if (ce->active && ce->written)
						return ce->value;
					break;
				}
			break;
		}
	return defval;
}

static void
putfqval( const char *sect, const char *key, const char *value )
{
	Section *cs, **csp;
	Entry *ce, **cep;

	if (!value)
		return;

	for (csp = &config; (cs = *csp); csp = &(cs->next))
		if (!strcmp( sect, cs->name ))
			goto havesec;
	cs = mcalloc( sizeof(*cs) );
	ASPrintf( (char **)&cs->name, "%s", sect );
	cs->spec = findSect( sect );
	*csp = cs;
  havesec:

	for (cep = &(cs->ents); (ce = *cep); cep = &(ce->next))
		if (!strcmp( key, ce->spec->key ))
			goto haveent;
	ce = mcalloc( sizeof(*ce) );
	ce->spec = findEnt( cs->spec, key );
	*cep = ce;
  haveent:
	ASPrintf( (char **)&ce->value, "%s", value );
	ce->written = ce->active = 1;
}

static const char *csect;

#define setsect(se) csect = se

static void
putval( const char *key, const char *value )
{
	putfqval( csect, key, value );
}


static void
wrconf( FILE *f )
{
	Section *cs;
	Entry *ce;
	StrList *sl = 0, *sp;
	const char *cmt;

	putfqval( "General", "ConfigVersion", RCVERSTR );
	for (cs = config; cs; cs = cs->next) {
		fprintf( f, "%s[%s]\n",
		         cs->comment ? cs->comment : "\n", cs->name );
		for (ce = cs->ents; ce; ce = ce->next) {
			if (ce->spec->comment) {
				cmt = ce->spec->comment;
				for (sp = sl; sp; sp = sp->next)
					if (sp->str == cmt) {
						cmt = "# See above\n";
						goto havit;
					}
				if (!(sp = malloc( sizeof(*sp) )))
					fprintf( stderr, "Warning: Out of memory\n" );
				else {
					sp->str = cmt;
					sp->next = sl; sl = sp;
				}
			} else
				cmt = "";
		  havit:
			fprintf( f, "%s%s%s=%s\n",
			         cmt, ce->active ? "" : "#", ce->spec->key, ce->value );
		}
	}
}


/*
 * defaults
 */
#ifdef XDMCP
static const char def_xaccess[] =
"# Xaccess - Access control file for XDMCP connections\n"
"#\n"
"# To control Direct and Broadcast access:\n"
"#\n"
"#	pattern\n"
"#\n"
"# To control Indirect queries:\n"
"#\n"
"#	pattern		list of hostnames and/or macros ...\n"
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
"#	%name		list of hosts ...\n"
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
#endif

#ifdef XDMCP
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
#endif

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
"#devname=`echo $DISPLAY | cut -c1-8`\n"
"#if [ ! -d /dev/xdm ]; then\n"
"#  mkdir /dev/xdm\n"
"#  chmod 755 /dev/xdm\n"
"#fi\n"
"#touch /dev/xdm/$devname\n"
"#chmod 644 /dev/xdm/$devname\n"
"#exec sessreg -a -l xdm/$devname -h \"`echo $DISPLAY | cut -d: -f1`\""
#else
"#exec sessreg -a -l $DISPLAY -h \"`echo $DISPLAY | cut -d: -f1`\""
# ifdef BSD
" -x " KDMCONF "/Xservers"
# endif
#endif /* _AIX */
" $USER\n"
"\n# NOTE: The session is aborted if the last command returns non-zero.\n";

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
"#devname=`echo $DISPLAY | cut -c1-8`\n"
"#exec sessreg -d -l xdm/$devname -h \"`echo $DISPLAY | cut -d: -f1`\""
#else
"#exec sessreg -d -l $DISPLAY -h \"`echo $DISPLAY | cut -d: -f1`\""
# ifdef BSD
" -x " KDMCONF "/Xservers"
# endif
#endif /* _AIX */
" $USER\n";

static const char def_session1[] =
"#! /bin/sh\n"
"# Xsession - run as user\n"
"\n"
"session=$1\n"
"\n"
"# Note that the respective logout scripts are not sourced.\n"
"case $SHELL in\n"
"  */bash)\n"
"    [ -z \"$BASH\" ] && exec $SHELL $0 \"$@\"\n"
"    set +o posix\n"
"    [ -f /etc/profile ] && . /etc/profile\n"
"    if [ -f $HOME/.bash_profile ]; then\n"
"      . $HOME/.bash_profile\n"
"    elif [ -f $HOME/.bash_login ]; then\n"
"      . $HOME/.bash_login\n"
"    elif [ -f $HOME/.profile ]; then\n"
"      . $HOME/.profile\n"
"    fi\n"
"    ;;\n"
"  */zsh)\n"
"    [ -z \"$ZSH_NAME\" ] && exec $SHELL $0 \"$@\"\n"
"    emulate -R zsh\n"
"    [ -d /etc/zsh ] && zdir=/etc/zsh || zdir=/etc\n"
"    zhome=${ZDOTDIR:-$HOME}\n"
"    # zshenv is always sourced automatically.\n"
"    [ -f $zdir/zprofile ] && . $zdir/zprofile\n"
"    [ -f $zhome/.zprofile ] && . $zhome/.zprofile\n"
"    [ -f $zdir/zlogin ] && . $zdir/zlogin\n"
"    [ -f $zhome/.zlogin ] && . $zhome/.zlogin\n"
"    ;;\n"
"  */csh|*/tcsh)\n"
"    # [t]cshrc is always sourced automatically.\n"
"    # Note that sourcing csh.login after .cshrc is non-standard.\n"
"    xsess_tmp=";
static const char def_session2[] =
"\n"
"    $SHELL -c \"if (-f /etc/csh.login) source /etc/csh.login; if (-f ~/.login) source ~/.login; /bin/sh -c export > $xsess_tmp\"\n"
"    . $xsess_tmp\n"
"    rm -f $xsess_tmp\n"
"    ;;\n"
"  *) # Plain sh, ksh, and anything we don't know.\n"
"    [ -f /etc/profile ] && . /etc/profile\n"
"    [ -f $HOME/.profile ] && . $HOME/.profile\n"
"    ;;\n"
"esac\n"
"\n"
"[ -f /etc/xprofile ] && . /etc/xprofile\n"
"[ -f $HOME/.xprofile ] && . $HOME/.xprofile\n"
"\n"
"case $session in\n"
"  \"\")\n"
"    exec xmessage -center -buttons OK:0 -default OK \"Sorry, $DESKTOP_SESSION is no valid session.\"\n"
"    ;;\n"
"  failsafe)\n"
"    exec xterm -geometry 80x24-0-0\n"
"    ;;\n"
"  custom)\n"
"    exec $HOME/.xsession\n"
"    ;;\n"
"  default)\n"
"    exec " KDE_BINDIR "/startkde\n"
"    ;;\n"
"  *)\n"
"    eval exec \"$session\"\n"
"    ;;\n"
"esac\n"
"exec xmessage -center -buttons OK:0 -default OK \"Sorry, cannot execute $session. Check $DESKTOP_SESSION.desktop.\"\n";

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
prepName( const char *fn )
{
	const char *tname;
	char *nname;

	tname = strrchr( fn, '/' );
	ASPrintf( &nname, "%s/%s", newdir, tname ? tname + 1 : fn );
	displace( nname );
	return nname;
}

static FILE *
Create( const char *fn, int mode )
{
	char *nname;
	FILE *f;

	nname = prepName( fn );
	if (!(f = fopen( nname, "w" ))) {
		fprintf( stderr, "Cannot create %s\n", nname );
		exit( 1 );
	}
	chmod( nname, mode );
	free( nname );
	return f;
}

static void
WriteOut( const char *fn, int mode, time_t stamp, const char *buf, size_t len )
{
	char *nname;
	int fd;
	struct utimbuf utim;

	nname = prepName( fn );
	if ((fd = creat( nname, mode )) < 0) {
		fprintf( stderr, "Cannot create %s\n", nname );
		exit( 1 );
	}
	write( fd, buf, len );
	close( fd );
	if (stamp) {
		utim.actime = utim.modtime = stamp;
		utime( nname, &utim );
	}
	free( nname );
}


/* returns static array! */
static const char *
resect( const char *sec, const char *name )
{
	static char sname[64];
	char *p;

	if ((p = strrchr( sec, '-' ))) {
		sprintf( sname, "%.*s-%s", p - sec, sec, name );
		return sname;
	} else
		return name;
}

static int
inNewDir( const char *name )
{
	return !memcmp( name, KDMCONF "/", sizeof(KDMCONF) );
}

static int
inList( StrList *sp, const char *s )
{
	for (; sp; sp = sp->next)
		if (!strcmp( sp->str, s ))
			return 1;
	return 0;
}

static void
addStr( StrList **sp, const char *s )
{
	for (; *sp; sp = &(*sp)->next)
		if (!strcmp( (*sp)->str, s ))
			return;
	*sp = mcalloc( sizeof(**sp) );
	ASPrintf( (char **)&(*sp)->str, "%s", s );
}

StrList *aflist, *uflist, *eflist, *cflist, *lflist;

/* file is part of new config */
static void
addedFile( const char *fn )
{
	addStr( &aflist, fn );
}

/* file from old config was parsed */
static void
usedFile( const char *fn )
{
	addStr( &uflist, fn );
}

/* file from old config was copied with slight modifications */
static void
editedFile( const char *fn )
{
	addStr( &eflist, fn );
}

/* file from old config was copied verbatim */
static void
copiedFile( const char *fn )
{
	addStr( &cflist, fn );
}

/* file from old config is still being used */
static void
linkedFile( const char *fn )
{
	addStr( &lflist, fn );
}

/*
 * XXX this stuff is highly borked. it does not handle collisions at all.
 */
static int
copyfile( Entry *ce, const char *tname, int mode, int (*proc)( File * ) )
{
	const char *tptr;
	char *nname;
	File file;
	int rt;

	if (!*ce->value)
		return 1;

	tptr = strrchr( tname, '/' );
	ASPrintf( &nname, KDMCONF "/%s", tptr ? tptr + 1 : tname );
	if (inList( cflist, ce->value ) ||
	    inList( eflist, ce->value ) ||
	    inList( lflist, ce->value ))
	{
		rt = 1;
		goto doret;
	}
	if (!readFile( &file, ce->value )) {
		fprintf( stderr, "Warning: cannot copy file %s\n", ce->value );
		rt = 0;
	} else {
		if (!proc || !proc( &file )) {
			if (!use_destdir && !strcmp( ce->value, nname ))
				linkedFile( nname );
			else {
				struct stat st;
				stat( ce->value, &st );
				WriteOut( nname, mode, st.st_mtime, file.buf, file.eof - file.buf );
				copiedFile( ce->value );
			}
		} else {
			WriteOut( nname, mode, 0, file.buf, file.eof - file.buf );
			editedFile( ce->value );
		}
		if (strcmp( ce->value, nname ) && inNewDir( ce->value ) && !use_destdir)
			displace( ce->value );
		addedFile( nname );
		rt = 1;
	}
  doret:
	ce->value = nname;
	return rt;
}

static void
dlinkfile( const char *name )
{
	File file;

	if (!readFile( &file, name )) {
		fprintf( stderr, "Warning: cannot read file %s\n", name );
		return;
	}
	if (inNewDir( name ) && use_destdir) {
		struct stat st;
		stat( name, &st );
		WriteOut( name, st.st_mode, st.st_mtime, file.buf, file.eof - file.buf );
		copiedFile( name );
	} else
		linkedFile( name );
	addedFile( name );
}

static void
linkfile( Entry *ce )
{
	if (ce->written && *ce->value)
		dlinkfile( ce->value );
}

static void
writefile( const char *tname, int mode, const char *cont )
{
	WriteOut( tname, mode, 0, cont, strlen( cont ) );
	addedFile( tname );
}


char *background;

static void
handBgCfg( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) /* can be only the X-*-Greeter one */
		writefile( def_BackgroundCfg, 0644,
		           background ? background : def_background );
#if 0 /* risk of kcontrol clobbering the original file */
	else if (old_confs)
		linkfile( ce );
#endif
	else {
		if (!copyfile( ce, ce->value, 0644, 0 )) {
			if (!strcmp( cs->name, "X-*-Greeter" ))
				writefile( def_BackgroundCfg, 0644, def_background );
			ce->active = 0;
		}
	}
}


#ifdef HAVE_VTS
static char *
mem_mem( char *mem, int lmem, const char *smem, int lsmem )
{
	for (; lmem >= lsmem; mem++, lmem--)
		if (!memcmp( mem, smem, lsmem ))
			return mem + lsmem;
	return 0;
}

static int maxTTY, TTYmask;

static void
getInitTab( void )
{
	File it;
	char *p, *eol, *ep;
	int tty;

	if (maxTTY)
		return;
	if (readFile( &it, "/etc/inittab" )) {
		usedFile( "/etc/inittab" );
		for (p = it.buf; p < it.eof; p = eol + 1) {
			for (eol = p; eol < it.eof && *eol != '\n'; eol++);
			if (*p != '#') {
				if ((ep = mem_mem( p, eol - p, " tty", 4 )) &&
				    ep < eol && isdigit( *ep ))
				{
					if (ep + 1 == eol || isspace( *(ep + 1) ))
						tty = *ep - '0';
					else if (isdigit( *(ep + 1) ) &&
					         (ep + 2 == eol || isspace( *(ep + 2) )))
						tty = (*ep - '0') * 10 + (*(ep + 1) - '0');
					else
						continue;
					TTYmask |= 1 << (tty - 1);
					if (tty > maxTTY)
						maxTTY = tty;
				}
			}
		}
		freeBuf( &it );
	}
	if (!maxTTY) {
		maxTTY = 6;
		TTYmask = 0x3f;
	}
}
#endif


/* TODO: handle solaris' local_uid specs */

static char *
ReadWord( File *file, int EOFatEOL )
{
	char *wordp, *wordBuffer;
	int quoted;
	char c;

  rest:
	wordp = wordBuffer = file->cur;
  mloop:
	quoted = 0;
  qloop:
	if (file->cur == file->eof) {
	  doeow:
		if (wordp == wordBuffer)
			return 0;
	  retw:
		*wordp = '\0';
		return wordBuffer;
	}
	c = *file->cur++;
	switch (c) {
	case '#':
		if (quoted)
			break;
		do {
			if (file->cur == file->eof)
				goto doeow;
			c = *file->cur++;
		} while (c != '\n');
	case '\0':
	case '\n':
		if (EOFatEOL && !quoted) {
			file->cur--;
			goto doeow;
		}
		if (wordp != wordBuffer) {
			file->cur--;
			goto retw;
		}
		goto rest;
	case ' ':
	case '\t':
		if (wordp != wordBuffer)
			goto retw;
		goto rest;
	case '\\':
		if (!quoted) {
			quoted = 1;
			goto qloop;
		}
		break;
	}
	*wordp++ = c;
	goto mloop;
}

/* backslashes are double-escaped - for KConfig and for parseArgs */
static const char *
joinArgs( StrList *argv )
{
	StrList *av;
	const char *s, *rs;
	char *str;
	int slen;

	if (!argv)
		return "";
	for (slen = 0, av = argv; slen++, av; av = av->next) {
		int nq = 0;
		for (s = av->str; *s; s++, slen++)
			if (isspace( *s ) || *s == '\'')
				nq = 2;
			else if (*s == '"')
				slen += 2;
			else if (*s == '\\')
				slen += 3;
		slen += nq;
	}
	rs = str = mmalloc( slen );
	for (av = argv; av; av = av->next) {
		int nq = 0;
		for (s = av->str; *s; s++)
			if (isspace( *s ) || *s == '\'')
				nq = 2;
		if (av != argv)
			*str++ = ' ';
		if (nq)
			*str++ = '"';
		for (s = av->str; *s; s++) {
			if (*s == '\\')
				*str++ = '\\';
			if (*s == '"' || *s == '\\') {
				*str++ = '\\';
				*str++ = '\\';
			}
			*str++ = *s;
		}
		if (nq)
			*str++ = '"';
	}
	*str = 0;
	return rs;
}

# define dLocation  1
# define dLocal     0
# define dForeign   1

static struct displayMatch {
	const char *name;
	int len, type;
} displayTypes[] = {
	{ "local", 5, dLocal },
	{ "foreign", 7, dForeign },
};

static int
parseDisplayType( const char *string, const char **atPos )
{
	struct displayMatch *d;

	*atPos = 0;
	for (d = displayTypes; d < displayTypes + as(displayTypes); d++) {
		if (!memcmp( d->name, string, d->len ) &&
		    (!string[d->len] || string[d->len] == '@'))
		{
			if (string[d->len] == '@' && string[d->len + 1])
				*atPos = string + d->len + 1;
			return d->type;
		}
	}
	return -1;
}

typedef struct serverEntry {
	struct serverEntry *next;
	const char *name, *class2, *console, *argvs, *arglvs;
	StrList *argv, *arglv;
	int type, reserve, vt;
} ServerEntry;

static void
absorb_xservers( const char *sect ATTR_UNUSED, char **value )
{
	ServerEntry *se, *se1, *serverList, **serverPtr;
	const char *word, *word2;
	char *sdpys, *rdpys;
	StrList **argp, **arglp, *ap, *ap2;
	File file;
	int nldpys = 0, nrdpys = 0, dpymask = 0;
	int cpcmd, cpcmdl;
#ifdef HAVE_VTS
	int dn, cpvt, mtty;
#endif

	if (**value == '/') {
		if (!readFile( &file, *value ))
			return;
		usedFile( *value );
	} else {
		file.buf = *value;
		file.eof = *value + strlen( *value );
	}
	file.cur = file.buf;

	serverPtr = &serverList;
#ifdef HAVE_VTS
  bustd:
#endif
	while ((word = ReadWord( &file, 0 ))) {
		se = mcalloc( sizeof(*se) );
		se->name = word;
		if (!(word = ReadWord( &file, 1 )))
			continue;
		se->type = parseDisplayType( word, &se->console );
		if (se->type < 0) {
			se->class2 = word;
			if (!(word = ReadWord( &file, 1 )))
				continue;
			se->type = parseDisplayType( word, &se->console );
			if (se->type < 0) {
				while (ReadWord( &file, 1 ));
				continue;
			}
		}
		word = ReadWord( &file, 1 );
		if (word && !strcmp( word, "reserve" )) {
			se->reserve = 1;
			word = ReadWord( &file, 1 );
		}
		if (((se->type & dLocation) == dLocal) != (word != 0))
			continue;
		argp = &se->argv;
		arglp = &se->arglv;
		while (word) {
#ifdef HAVE_VTS
			if (word[0] == 'v' && word[1] == 't')
				se->vt = atoi( word + 2 );
			else if (!strcmp( word, "-crt" )) { /* SCO style */
				if (!(word = ReadWord( &file, 1 )) ||
				    memcmp( word, "/dev/tty", 8 ))
					goto bustd;
				se->vt = atoi( word + 8 );
			} else
#endif
			if (strcmp( word, se->name )) {
				ap = mmalloc( sizeof(*ap) );
				ap->str = word;
				if (!strcmp( word, "-nolisten" )) {
					if (!(word2 = ReadWord( &file, 1 )))
						break;
					ap2 = mmalloc( sizeof(*ap2) );
					ap2->str = word2;
					ap->next = ap2;
					if (!strcmp( word2, "unix" )) {
						*argp = ap;
						argp = &ap2->next;
					} else {
						*arglp = ap;
						arglp = &ap2->next;
					}
				} else {
					*argp = ap;
					argp = &ap->next;
				}
			}
			word = ReadWord( &file, 1 );
		}
		*argp = *arglp = 0;
		if ((se->type & dLocation) == dLocal) {
			nldpys++;
			dpymask |= 1 << atoi( se->name + 1 );
			if (se->reserve)
				nrdpys++;
		}
		*serverPtr = se;
		serverPtr = &se->next;
	}
	*serverPtr = 0;

#ifdef HAVE_VTS
	/* don't copy only if all local displays are ordered and have a vt */
	cpvt = 0;
	getInitTab();
	for (se = serverList, mtty = maxTTY; se; se = se->next)
		if ((se->type & dLocation) == dLocal) {
			mtty++;
			if (se->vt != mtty) {
				cpvt = 1;
				break;
			}
		}
#endif

	for (se = serverList; se; se = se->next) {
		se->argvs = joinArgs( se->argv );
		se->arglvs = joinArgs( se->arglv );
	}

	se1 = 0, cpcmd = cpcmdl = 0;
	for (se = serverList; se; se = se->next)
		if ((se->type & dLocation) == dLocal) {
			if (!se1)
				se1 = se;
			else {
				if (strcmp( se1->argvs, se->argvs ))
					cpcmd = 1;
				if (strcmp( se1->arglvs, se->arglvs ))
					cpcmdl = 1;
			}
		}
	if (se1) {
		putfqval( "X-:*-Core", "ServerCmd", se1->argvs );
		putfqval( "X-:*-Core", "ServerArgsLocal", se1->arglvs );
		for (se = serverList; se; se = se->next)
			if ((se->type & dLocation) == dLocal) {
				char sec[32];
				sprintf( sec, "X-%s-Core", se->name );
				if (cpcmd)
					putfqval( sec, "ServerCmd", se->argvs );
				if (cpcmdl)
					putfqval( sec, "ServerArgsLocal", se->arglvs );
#ifdef HAVE_VTS
				if (cpvt && se->vt) {
					char vt[8];
					sprintf( vt, "%d", se->vt );
					putfqval( sec, "ServerVT", vt );
				}
#else
				if (se->console)
					putfqval( sec, "ServerTTY", se->console );
#endif
			}
	}

	sdpys = rdpys = 0;
	for (se = serverList; se; se = se->next)
		StrCat( se->reserve ? &rdpys : &sdpys,
		        se->class2 ? ",%s_%s" : ",%s", se->name, se->class2 );

#ifdef HAVE_VTS
	/* add reserve dpys */
	if (nldpys < 4 && nldpys && !nrdpys)
		for (; nldpys < 4; nldpys++) {
			for (dn = 0; dpymask & (1 << dn); dn++);
			dpymask |= (1 << dn);
			StrCat( &rdpys, ",:%d", dn );
		}
#endif

	putfqval( "General", "StaticServers", sdpys ? sdpys + 1 : "" );
	putfqval( "General", "ReserveServers", rdpys ? rdpys + 1 : "" );

	if (**value == '/' && inNewDir( *value ) && !use_destdir)
		displace( *value );
}

#ifdef HAVE_VTS
static void
upd_servervts( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) { /* there is only the Global one */
#ifdef __linux__ /* XXX actually, sysvinit */
		getInitTab();
		ASPrintf( (char **)&ce->value, "-%d", maxTTY + 1 );
		ce->active = ce->written = 1;
#endif
	}
}

static void
upd_consolettys( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) { /* there is only the Global one */
#ifdef __linux__ /* XXX actually, sysvinit */
		char *buf;
		int i;

		getInitTab();
		for (i = 0, buf = 0; i < 16; i++)
			if (TTYmask & (1 << i))
				StrCat( &buf, ",tty%d", i + 1 );
		if (buf) {
			ce->value = buf + 1;
			ce->active = ce->written = 1;
		}
#endif
	}
}
#endif

#ifdef XDMCP
static void
cp_keyfile( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) /* there is only the Global one */
		return;
	if (old_confs)
		linkfile( ce );
	else
		if (!copyfile( ce, "kdmkeys", 0600, 0 ))
			ce->active = 0;
}

static void
mk_xaccess( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) /* there is only the Global one */
		writefile( def_Xaccess, 0644, def_xaccess );
	else if (old_confs)
		linkfile( ce );
	else
		copyfile( ce, "Xaccess", 0644, 0 ); /* don't handle error, it will disable Xdmcp automatically */
}

static void
mk_willing( Entry *ce, Section *cs ATTR_UNUSED )
{
	const char *fname;

	if (!ce->active) /* there is only the Global one */
		goto dflt;
	else {
		if (!(fname = strchr( ce->value, '/' )))
			return; /* obviously in-line (or empty) */
		if (old_scripts || inNewDir( fname ))
			dlinkfile( fname );
		else {
		  dflt:
			ce->value = KDMCONF "/Xwilling";
			ce->active = ce->written = 1;
			writefile( ce->value, 0755, def_willing );
		}
	}
}
#endif

/*
static int
edit_resources( File *file )
{
	// XXX remove any login*, chooser*, ... resources
	return 0;
}
*/

static void
cp_resources( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) /* the X-*-Greeter one */
		return;
	if (old_confs)
		linkfile( ce );
	else
		if (!copyfile( ce, ce->value, 0644, 0/*edit_resources*/ ))
			ce->active = 0;
}

static int
delstr( File *fil, const char *pat )
{
	char *p, *pp, *bpp;
	const char *pap, *paap;

	*fil->eof = 0;
	for (p = fil->buf; *p; p++) {
		for (pp = p, pap = pat; ; ) {
			if (!*pap) {
				*p = '\n';
				memcpy( p + 1, pp, fil->eof - pp + 1 );
				fil->eof -= pp - p - 1;
				return 1;
			} else if (!memcmp( pap, "*/", 2 )) {
				paap = pap += 2;
				while (!isspace( *pap ))
					pap++;
				if (*pp != '/')
					break;
				for (;;)
					for (bpp = ++pp; *pp != '/'; pp++)
						if (!*pp || isspace( *pp ))
							goto wbrk;
			  wbrk:
				if ((pp - bpp != pap - paap) || memcmp( bpp, paap, pap - paap ))
					break;
			} else if (*pap == '\t') {
				pap++;
				while (*pp == ' ' || *pp == '\t')
					pp++;
			} else if (*pap == '[') {
				pap++;
				for (;;) {
					if (!*pap) {
						fprintf( stderr, "Internal error: unterminated char set\n" );
						exit( 1 );
					}
					if (*pap == *pp) {
						while (*++pap != ']')
							if (!*pap) {
								fprintf( stderr, "Internal error: unterminated char set\n" );
								exit( 1 );
							}
						pap++;
						pp++;
						break;
					}
					if (*++pap == ']')
						goto no;
				}
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
edit_setup( File *file )
{
	int chg =
		delstr( file, "\n"
		        "(\n"
		        "  PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
		        "  */kdmdesktop\t&\n"
		        "  echo $! >$PIDFILE\n"
		        "  wait $!\n"
		        "  rm $PIDFILE\n"
		        ")\t&\n" ) |
		delstr( file, "\n"
		        "*/kdmdesktop\t&\n" ) |
		delstr( file, "\n"
		        "kdmdesktop\t&\n" ) |
		delstr( file, "\n"
		        "kdmdesktop\n" );
	putval( "UseBackground", chg ? "true" : "false" );
	return chg;
}

static void
mk_setup( Entry *ce, Section *cs )
{
	setsect( resect( cs->name, "Greeter" ) );
	if (old_scripts || mixed_scripts) {
		if (mod_usebg && *ce->value)
			putval( "UseBackground", "false" );
		linkfile( ce );
	} else {
		if (ce->active && inNewDir( ce->value )) {
			if (mod_usebg)
				copyfile( ce, ce->value, 0755, edit_setup );
			else
				linkfile( ce );
		} else {
			ce->value = KDMCONF "/Xsetup";
			ce->active = ce->written = 1;
			writefile( ce->value, 0755, def_setup );
		}
	}
}

static int
edit_startup( File *file )
{
	int chg1 = 0, chg2 = 0;

	if (mod_usebg &&
	    (delstr( file, "\n"
	             "PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
	             "if [[] -f $PIDFILE ] ; then\n"
	             "	   kill `cat $PIDFILE`\n"
	             "fi\n" ) ||
	     delstr( file, "\n"
	             "PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
	             "test -f $PIDFILE && kill `cat $PIDFILE`\n" )))
		chg1 = 1;
	if (oldver < 0x0203) {
		chg2 = 
#ifdef _AIX
			delstr( file, "\n"
"# We create a pseudodevice for finger.  (host:0 becomes [kx]dm/host_0)\n" );
"# Without it, finger errors out with \"Can't stat /dev/host:0\".\n"
"#\n"
"if [[] -f /usr/lib/X11/xdm/sessreg ]; then\n"
"  devname=`echo $DISPLAY | /usr/bin/sed -e 's/[[]:\\.]/_/g' | /usr/bin/cut -c1-8`\n"
"  hostname=`echo $DISPLAY | /usr/bin/cut -d':' -f1`\n"
"\n"
"  if [[] -z \"$devname\" ]; then\n"
"    devname=\"unknown\"\n"
"  fi\n"
"  if [[] ! -d /dev/[kx]dm ]; then\n"
"    /usr/bin/mkdir /dev/[kx]dm\n"
"    /usr/bin/chmod 755 /dev/[kx]dm\n"
"  fi\n"
"  /usr/bin/touch /dev/[kx]dm/$devname\n"
"  /usr/bin/chmod 644 /dev/[kx]dm/$devname\n"
"\n"
"  if [[] -z \"$hostname\" ]; then\n"
"    exec /usr/lib/X11/xdm/sessreg -a -l [kx]dm/$devname $USER\n"
"  else\n"
"    exec /usr/lib/X11/xdm/sessreg -a -l [kx]dm/$devname -h $hostname $USER\n"
"  fi\n"
"fi\n") |
#else
# ifdef BSD
			delstr( file, "\n"
"exec sessreg -a -l $DISPLAY -x */Xservers -u " _PATH_UTMP " $USER\n" ) |
# endif
#endif /* _AIX */
			delstr( file, "\n"
"exec sessreg -a -l $DISPLAY"
#ifdef BSD
" -x */Xservers"
#endif
" $USER\n" ) |
			delstr( file, "\n"
"exec sessreg -a -l $DISPLAY -u /var/run/utmp -x */Xservers $USER\n" );
		putval( "UseSessReg", chg2 ? "true" : "false");
	}
	return chg1 | chg2;
}

static void
mk_startup( Entry *ce, Section *cs )
{
	setsect( cs->name );
	if (old_scripts || mixed_scripts)
		linkfile( ce );
	else {
		if (ce->active && inNewDir( ce->value )) {
			if (mod_usebg || oldver < 0x0203)
				copyfile( ce, ce->value, 0755, edit_startup );
			else
				linkfile( ce );
		} else {
			ce->value = KDMCONF "/Xstartup";
			ce->active = ce->written = 1;
			writefile( ce->value, 0755, def_startup );
		}
	}
}

static int
edit_reset( File *file )
{
	return
#ifdef _AIX
		delstr( file, "\n"
"if [[] -f /usr/lib/X11/xdm/sessreg ]; then\n"
"  devname=`echo $DISPLAY | /usr/bin/sed -e 's/[[]:\\.]/_/g' | /usr/bin/cut -c1-8`\n"
"  exec /usr/lib/X11/xdm/sessreg -d -l [kx]dm/$devname $USER\n"
"fi\n" ) |
#else
# ifdef BSD
		delstr( file, "\n"
"exec sessreg -d -l $DISPLAY -x */Xservers -u " _PATH_UTMP " $USER\n" ) |
# endif
#endif /* _AIX */
		delstr( file, "\n"
"exec sessreg -d -l $DISPLAY"
# ifdef BSD
" -x */Xservers"
# endif
" $USER\n" ) |
		delstr( file, "\n"
"exec sessreg -d -l $DISPLAY -u /var/run/utmp -x */Xservers $USER\n" );
}

static void
mk_reset( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (old_scripts || mixed_scripts)
		linkfile( ce );
	else {
		if (ce->active && inNewDir( ce->value )) {
			if (oldver < 0x0203)
				copyfile( ce, ce->value, 0755, edit_reset );
			else
				linkfile( ce );
		} else {
			ce->value = KDMCONF "/Xreset";
			ce->active = ce->written = 1;
			writefile( ce->value, 0755, def_reset );
		}
	}
}

static void
mk_session( Entry *ce, Section *cs ATTR_UNUSED )
{
	char *def_session;
	const char *tmpf;

	if ((old_scripts || (ce->active && inNewDir( ce->value ))) &&
	    oldver >= 0x202)
		linkfile( ce );
	else {
		tmpf = locate( "mktemp" ) ?
		           "`mktemp`" :
		           locate( "tempfile" ) ?
		               "`tempfile`" :
		               "$HOME/.xsession-env-$DISPLAY";
		ASPrintf( &def_session, "%s%s%s", def_session1, tmpf, def_session2 );
		ce->value = KDMCONF "/Xsession";
		ce->active = ce->written = 1;
		writefile( ce->value, 0755, def_session );
	}
}

static void
upd_language( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!strcmp( ce->value, "C" ))
		ce->value = (char *)"en_US";
}

static void
upd_guistyle( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!strcmp( ce->value, "Motif+" ))
		ce->value = (char *)"MotifPlus";
	else if (!strcmp( ce->value, "KDE" ))
		ce->value = (char *)"Default";
}

static void
upd_showusers( Entry *ce, Section *cs )
{
	if (!strcmp( ce->value, "All" ))
		ce->value = (char *)"NotHidden";
	else if (!strcmp( ce->value, "None" )) {
		if (ce->active)
			putfqval( cs->name, "UserList", "false" );
		ce->value = (char *)"Selected";
		ce->active = 0;
		ce->written = 1;
	}
}

static const char *defminuid, *defmaxuid;

static void
upd_minshowuid( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) {
		ce->value = defminuid;
		ce->active = ce->written = 1;
	}
}

static void
upd_maxshowuid( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) {
		ce->value = defmaxuid;
		ce->active = ce->written = 1;
	}
}

static void
upd_hiddenusers( Entry *ce, Section *cs ATTR_UNUSED )
{
	char *nv;
	const char *msu, *pt, *et;
	struct passwd *pw;
	unsigned minuid, maxuid;
	char nbuf[128];

	if (!ce->active)
		return;

	msu = getfqval( cs->name, "MinShowUID", "0" );
	sscanf( msu, "%u", &minuid );
	msu = getfqval( cs->name, "MaxShowUID", "65535" );
	sscanf( msu, "%u", &maxuid );

	nv = 0;
	pt = ce->value;
	for (;;) {
		et = strpbrk( pt, ";," );
		if (et) {
			memcpy( nbuf, pt, et - pt );
			nbuf[et - pt] = 0;
		} else
			strcpy( nbuf, pt );
		if ((pw = getpwnam( nbuf ))) {
			if (!pw->pw_uid ||
			    (pw->pw_uid >= minuid && pw->pw_uid <= maxuid))
			{
				if (nv)
					StrCat( &nv, ",%s", nbuf );
				else
					nv = mstrdup( nbuf );
			}
		}
		if (!et)
			break;
		pt = et + 1;
	}
	ce->value = nv ? nv : "";
}

static void
upd_forgingseed( Entry *ce, Section *cs ATTR_UNUSED )
{
	if (!ce->active) {
		ASPrintf( (char **)&ce->value, "%d", time( 0 ) );
		ce->active = ce->written = 1;
	}
}

static void
upd_fifodir( Entry *ce, Section *cs ATTR_UNUSED )
{
	const char *dir;
	struct stat st;

	if (use_destdir)
		return;
	dir = ce->active ? ce->value : def_FifoDir;
	stat( dir, &st );
	chmod( dir, st.st_mode | 0755 );
}

static void
upd_datadir( Entry *ce, Section *cs ATTR_UNUSED )
{
	char *oldsts, *newsts;
	const char *dir;

	if (use_destdir)
		return;
	dir = ce->active ? ce->value : def_DataDir;
	if (mkdirp( dir, 0755, "data", 0 ) && oldkde) {
		ASPrintf( &oldsts, "%s/kdm/kdmsts", oldkde );
		ASPrintf( &newsts, "%s/kdmsts", dir );
		rename( oldsts, newsts );
	}
}

static void
CopyFile( const char *from, const char *to )
{
	File file;
	int fd;

	if (readFile( &file, from )) {
		if ((fd = creat( to, 0644 )) >= 0) {
			write( fd, file.buf, file.eof - file.buf );
			close( fd );
		}
		freeBuf( &file );
	}
}

static void
upd_facedir( Entry *ce, Section *cs ATTR_UNUSED )
{
	char *oldpic, *newpic, *defpic, *rootpic;
	const char *dir;
	struct passwd *pw;

	if (use_destdir)
		return;
	dir = ce->active ? ce->value : def_FaceDir;
	if (mkdirp( dir, 0755, "user face", 0 )) {
		ASPrintf( &defpic, "%s/.default.face.icon", dir );
		ASPrintf( &rootpic, "%s/root.face.icon", dir );
		if (oldkde) {
			setpwent();
			while ((pw = getpwent()))
				if (strcmp( pw->pw_name, "root" )) {
					ASPrintf( &oldpic, "%s/../apps/kdm/pics/users/%s.png",
					          oldkde, pw->pw_name );
					ASPrintf( &newpic, "%s/%s.face.icon", dir, pw->pw_name );
					rename( oldpic, newpic );
					free( newpic );
					free( oldpic );
				}
			endpwent();
			ASPrintf( &oldpic, "%s/../apps/kdm/pics/users/default.png", oldkde );
			if (!rename( oldpic, defpic ))
				defpic = 0;
			ASPrintf( &oldpic, "%s/../apps/kdm/pics/users/root.png", oldkde );
			if (!rename( oldpic, rootpic ))
				rootpic = 0;
		}
		if (defpic) {
			ASPrintf( &oldpic, "%s/default1.png", facesrc );
			CopyFile( oldpic, defpic );
		}
		if (rootpic) {
			ASPrintf( &oldpic, "%s/root1.png", facesrc );
			CopyFile( oldpic, rootpic );
		}
	}
}

CONF_GEN_ENTRIES

static Sect *
findSect( const char *name )
{
	const char *p;
	int i;

	p = strrchr( name, '-' );
	if (!p)
		p = name;
	for (i = 0; i < as(allSects); i++)
		if (!strcmp( allSects[i]->name, p ))
			return allSects[i];
	fprintf( stderr, "Internal error: unknown section %s\n", name );
	exit( 1 );
}

static Ent *
findEnt( Sect *sect, const char *key )
{
	int i;

	for (i = 0; i < sect->nents; i++)
		if (!strcmp( sect->ents[i].key, key ))
			return sect->ents + i;
	fprintf( stderr, "Internal error: unknown key %s in section %s\n",
	         key, sect->name );
	exit( 1 );
}


/*
 * defaults
 */

typedef struct DEnt {
	const char *key;
	const char *value;
	int active;
} DEnt;

typedef struct DSect {
	const char *name;
	DEnt *ents;
	int nents;
	const char *comment;
} DSect;

CONF_GEN_EXAMPLE

static void
mkdefconf( void )
{
	Section *cs, **csp;
	Entry *ce, **cep;
	int sc, ec;

	for (csp = &config, sc = 0; sc < as(dAllSects); csp = &(cs->next), sc++) {
		cs = mcalloc( sizeof(*cs) );
		*csp = cs;
		cs->spec = findSect( dAllSects[sc].name );
		cs->name = dAllSects[sc].name;
		cs->comment = dAllSects[sc].comment;
		for (cep = &(cs->ents), ec = 0; ec < dAllSects[sc].nents;
		     cep = &(ce->next), ec++)
		{
			ce = mcalloc( sizeof(*ce) );
			*cep = ce;
			ce->spec = findEnt( cs->spec, dAllSects[sc].ents[ec].key );
			ce->value = dAllSects[sc].ents[ec].value;
			ce->active = dAllSects[sc].ents[ec].active;
		}
	}
}


/*
 * read rc file structure
 */

typedef struct REntry {
	struct REntry *next;
	const char *key;
	char *value;
} REntry;

typedef struct RSection {
	struct RSection *next;
	const char *name;
	REntry *ents;
} RSection;

static RSection *
ReadConf( const char *fname )
{
	char *nstr;
	char *s, *e, *st, *en, *ek, *sl;
	RSection *rootsec = 0, *cursec;
	REntry *curent;
	int nlen;
	int line, sectmoan;
	File file;

	if (!readFile( &file, fname ))
		return 0;
	usedFile( fname );

	for (s = file.buf, line = 0, cursec = 0, sectmoan = 1; s < file.eof; s++) {
		line++;

		while ((s < file.eof) && isspace( *s ) && (*s != '\n'))
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
			while ((e > sl) && isspace( *e ))
				e--;
			if (*e != ']') {
				fprintf( stderr, "Invalid section header at %s:%d\n",
				         fname, line );
				continue;
			}
			sectmoan = 0;
			nstr = sl + 1;
			nlen = e - nstr;
			for (cursec = rootsec; cursec; cursec = cursec->next)
				if (!memcmp( nstr, cursec->name, nlen ) &&
				    !cursec->name[nlen])
				{
#if 0 /* not our business ... */
					fprintf( stderr, "Warning: Multiple occurrences of section "
					         "[%.*s] in %s. Consider merging them.\n",
					         nlen, nstr, fname );
#endif
					goto secfnd;
				}
			cursec = mmalloc( sizeof(*cursec) );
			ASPrintf( (char **)&cursec->name, "%.*s", nlen, nstr );
			cursec->ents = 0;
			cursec->next = rootsec;
			rootsec = cursec;
		  secfnd:
			continue;
		}

		if (!cursec) {
			if (sectmoan) {
				sectmoan = 0;
				fprintf( stderr, "Entry outside any section at %s:%d",
				         fname, line );
			}
			goto sktoeol;
		}

		for (; (s < file.eof) && (*s != '\n'); s++)
			if (*s == '=')
				goto haveeq;
		fprintf( stderr, "Invalid entry (missing '=') at %s:%d\n", fname, line );
		continue;

	  haveeq:
		for (ek = s - 1;; ek--) {
			if (ek < sl) {
				fprintf( stderr, "Invalid entry (empty key) at %s:%d\n",
				         fname, line );
				goto sktoeol;
			}
			if (!isspace( *ek ))
				break;
		}

		s++;
		while ((s < file.eof) && isspace( *s ) && (*s != '\n'))
			s++;
		st = s;
		while ((s < file.eof) && (*s != '\n'))
			s++;
		for (en = s - 1; en >= st && isspace( *en ); en--);

		nstr = sl;
		nlen = ek - sl + 1;
		for (curent = cursec->ents; curent; curent = curent->next)
			if (!memcmp( nstr, curent->key, nlen ) &&
			    !curent->key[nlen]) {
				fprintf( stderr, "Multiple occurrences of key '%s' in section "
				         "[%s] of %s.\n", curent->key, cursec->name, fname );
				goto keyfnd;
			}
		curent = mmalloc( sizeof(*curent) );
		ASPrintf( (char **)&curent->key, "%.*s", nlen, nstr );
		ASPrintf( (char **)&curent->value, "%.*s", en - st + 1, st );
		curent->next = cursec->ents;
		cursec->ents = curent;
	  keyfnd:
		continue;
	}
	return rootsec;
}


static int
mergeKdmRcOld( const char *path )
{
	char *p;
	struct stat st;

	ASPrintf( &p, "%s/kdmrc", path );
	if (stat( p, &st )) {
		free( p );
		return 0;
	}
	printf( "Information: ignoring old kdmrc %s from kde < 2.2\n", p );
	free( p );
	return 1;
}

typedef struct {
	const char *sect, *key, *def;
	int (*cond)( void );
} FDefs;

static void
applydefs( FDefs *chgdef, int ndefs, const char *path )
{
	char *p;
	int i;

	for (i = 0; i < ndefs; i++)
		if (!getfqval( chgdef[i].sect, chgdef[i].key, 0 ) &&
		    (!chgdef[i].cond || chgdef[i].cond()))
		{
			ASPrintf( &p, chgdef[i].def, path );
			putfqval( chgdef[i].sect, chgdef[i].key, p );
			free( p );
		}
}

#ifdef XDMCP
static FDefs kdmdefs_all[] = {
{ "Xdmcp", "Xaccess", "%s/kdm/Xaccess", 0 },
{ "Xdmcp", "Willing", "", 0 },
};
#endif

static FDefs kdmdefs_eq_22[] = {
{ "General", "PidFile", "/var/run/xdm.pid",  0 },
{ "X-*-Core", "Setup", "%s/kdm/Xsetup",  0 },
{ "X-*-Core", "Startup", "%s/kdm/Xstartup",  0 },
{ "X-*-Core", "Reset", "%s/kdm/Xreset",  0 },
{ "X-*-Core", "Session", "%s/kdm/Xsession",  0 },
};

#ifdef XDMCP
static int
if_xdmcp (void)
{
	return isTrue( getfqval( "Xdmcp", "Enable", "true" ) );
}

static FDefs kdmdefs_le_30[] = {
{ "Xdmcp", "KeyFile", "%s/kdm/kdmkeys", if_xdmcp },
};
#endif

/* HACK: misused by is22conf() below */
static FDefs kdmdefs_ge_30[] = {
{ "X-*-Core", "Setup", "", 0 },
{ "X-*-Core", "Startup", "", 0 },
{ "X-*-Core", "Reset", "", 0 },
{ "X-*-Core", "Session", XBINDIR "/xterm -ls -T", 0 },
};

static int
if_usebg (void)
{
	return isTrue( getfqval( "X-*-Greeter", "UseBackground", "true" ) );
}

static FDefs kdmdefs_ge_31[] = {
{ "X-*-Greeter","BackgroundCfg","%s/kdm/backgroundrc", if_usebg },
};

static int
is22conf( const char *path )
{
	char *p;
	const char *val;
	int i, sl;

	sl = ASPrintf( &p, "%s/kdm/", path );
	/* safe bet, i guess ... */
	for (i = 0; i < 4; i++) {
		val = getfqval( "X-*-Core", kdmdefs_ge_30[i].key, 0 );
		if (val && !memcmp( val, p, sl )) {
			free( p );
			return 0;
		}
	}
	free( p );
	return 1;
}

typedef struct KUpdEnt {
	const char *okey, *nsec, *nkey;
	void (*func)( const char *sect, char **value );
} KUpdEnt;

typedef struct KUpdSec {
	const char *osec;
	KUpdEnt *ents;
	int nents;
} KUpdSec;

#ifdef XDMCP
static void
P_EnableChooser( const char *sect ATTR_UNUSED, char **value )
{
	*value = (char *)(isTrue( *value ) ? "DefaultLocal" : "LocalOnly");
}
#endif

static void
P_UseLilo( const char *sect ATTR_UNUSED, char **value )
{
	*value = (char *)(isTrue( *value ) ? "Lilo" : "None");
}

CONF_GEN_KMERGE

static int
mergeKdmRcNewer( const char *path )
{
	char *p;
	const char *cp, *sec, *key;
	RSection *rootsect, *cs;
	REntry *ce;
	int i, j;
	static char sname[64];

	ASPrintf( &p, "%s/kdm/kdmrc", path );
	if (!(rootsect = ReadConf( p ))) {
		free( p );
		return 0;
	}
	printf( "Information: reading old kdmrc %s (from kde >= 2.2.x)\n", p );
	free( p );

	for (cs = rootsect; cs; cs = cs->next) {
		if (!strcmp( cs->name, "Desktop0" )) {
			background = mstrdup( "[Desktop0]\n" );
			for (ce = cs->ents; ce; ce = ce->next)
				StrCat( &background, "%s=%s\n", ce->key, ce->value );
		} else {
			cp = strrchr( cs->name, '-' );
			if (!cp)
				cp = cs->name;
			else if (cs->name[0] != 'X' || cs->name[1] != '-')
				goto dropsec;
			for (i = 0; i < as(kupsects); i++)
				if (!strcmp( cp, kupsects[i].osec )) {
					for (ce = cs->ents; ce; ce = ce->next) {
						for (j = 0; j < kupsects[i].nents; j++)
							if (!strcmp( ce->key, kupsects[i].ents[j].okey )) {
								if (kupsects[i].ents[j].nsec == (char *)-1) {
									kupsects[i].ents[j].func( 0, &ce->value );
									goto gotkey;
								}
								if (!kupsects[i].ents[j].nsec)
									sec = cs->name;
								else {
									sec = sname;
									sprintf( sname, "%.*s-%s", cp - cs->name, cs->name,
									         kupsects[i].ents[j].nsec );
								}
								if (!kupsects[i].ents[j].nkey)
									key = ce->key;
								else
									key = kupsects[i].ents[j].nkey;
								if (kupsects[i].ents[j].func)
									kupsects[i].ents[j].func( sec, &ce->value );
								putfqval( sec, key, ce->value );
								goto gotkey;
							}
						printf( "Information: dropping key %s from section [%s]\n",
						        ce->key, cs->name );
					  gotkey: ;
					}
					goto gotsec;
				}
		  dropsec:
			printf( "Information: dropping section [%s]\n", cs->name );
		  gotsec: ;
		}
	}

#ifdef XDMCP
	applydefs( kdmdefs_all, as(kdmdefs_all), path );
#endif
	if (!*(cp = getfqval( "General", "ConfigVersion", "" ))) { /* < 3.1 */
		mod_usebg = 1;
		if (is22conf( path )) {
			/* work around 2.2.x defaults borkedness */
			applydefs( kdmdefs_eq_22, as(kdmdefs_eq_22), path );
			printf( "Information: old kdmrc is from kde 2.2\n" );
		} else {
			applydefs( kdmdefs_ge_30, as(kdmdefs_ge_30), path );
			printf( "Information: old kdmrc is from kde 3.0\n" );
		}
#ifdef XDMCP
		/* work around minor <= 3.0.x defaults borkedness */
		applydefs( kdmdefs_le_30, as(kdmdefs_le_30), path );
#endif
	} else {
		int ma, mi;
		sscanf( cp, "%d.%d", &ma, &mi );
		oldver = (ma << 8) | mi;
		printf( "Information: old kdmrc is from kde >= 3.1 (config version %d.%d)\n", ma, mi );
		applydefs( kdmdefs_ge_30, as(kdmdefs_ge_30), path );
		applydefs( kdmdefs_ge_31, as(kdmdefs_ge_31), path );
	}

	return 1;
}


typedef struct XResEnt {
	const char *xname;
	const char *ksec, *kname;
	void (*func)( const char *sect, char **value );
} XResEnt;

static void
handleXdmVal( const char *dpy, const char *key, char *value,
              const XResEnt *ents, int nents )
{
	const char *kname;
	int i;
	char knameb[80], sname[80];

	for (i = 0; i < nents; i++)
		if (!strcmp( key, ents[i].xname ) ||
		    (key[0] == toupper( ents[i].xname[0] ) &&
		     !strcmp( key + 1, ents[i].xname + 1 )))
		{
			if (ents[i].ksec == (char *)-1) {
				ents[i].func (0, &value);
				break;
			}
			sprintf( sname, ents[i].ksec, dpy );
			if (ents[i].kname)
				kname = ents[i].kname;
			else {
				kname = knameb;
				sprintf( knameb, "%c%s",
				         toupper( ents[i].xname[0] ), ents[i].xname + 1 );
			}
			if (ents[i].func)
				ents[i].func( sname, &value );
			putfqval( sname, kname, value );
			break;
		}
}

static void
P_List( const char *sect ATTR_UNUSED, char **value )
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
P_authDir( const char *sect ATTR_UNUSED, char **value )
{
	int l;

	l = strlen( *value );
	if (l < 4) {
		*value = 0;
		return;
	}
	if ((*value)[l-1] == '/')
		(*value)[--l] = 0;
	if (!strncmp( *value, "/tmp/", 5 ) ||
	    !strncmp( *value, "/var/tmp/", 9 ))
	{
		printf( "Warning: Resetting inappropriate value %s for AuthDir to default\n",
		        *value );
		*value = 0;
		return;
	}
	if ((l >= 4 && !strcmp( *value + l - 4, "/tmp" )) ||
	    (l >= 6 && !strcmp( *value + l - 6, "/xauth" )) ||
	    (l >= 8 && !strcmp( *value + l - 8, "/authdir" )) ||
	    (l >= 10 && !strcmp( *value + l - 10, "/authfiles" )))
		return;
	ASPrintf( value, "%s/authdir", *value );
}

static void
P_openDelay( const char *sect, char **value )
{
	putfqval( sect, "ServerTimeout", *value );
}

static void
P_noPassUsers( const char *sect, char **value ATTR_UNUSED )
{
	putfqval( sect, "NoPassEnable", "true" );
}

static void
P_autoUser( const char *sect, char **value ATTR_UNUSED )
{
	putfqval( sect, "AutoLoginEnable", "true" );
}

#ifdef XDMCP
static void
P_requestPort( const char *sect, char **value )
{
	if (!strcmp( *value, "0" )) {
		*value = 0;
		putfqval( sect, "Enable", "false" );
	} else
		putfqval( sect, "Enable", "true" );
}
#endif

static int kdmrcmode = 0644;

static void
P_autoPass( const char *sect ATTR_UNUSED, char **value ATTR_UNUSED )
{
	kdmrcmode = 0600;
}

CONF_GEN_XMERGE

static XrmQuark XrmQString, empty = NULLQUARK;

static Bool
DumpEntry( XrmDatabase *db ATTR_UNUSED,
           XrmBindingList bindings,
           XrmQuarkList quarks,
           XrmRepresentation *type,
           XrmValuePtr value,
           XPointer data ATTR_UNUSED )
{
	const char *dpy, *key;
	int el, hasu;
	char dpybuf[80];

	if (*type != XrmQString)
		return False;
	if (*bindings == XrmBindLoosely ||
	    strcmp( XrmQuarkToString (*quarks), "DisplayManager" ))
		return False;
	bindings++, quarks++;
	if (!*quarks)
		return False;
	if (*bindings != XrmBindLoosely && !quarks[1]) { /* DM.foo */
		key = XrmQuarkToString (*quarks);
		handleXdmVal( 0, key, value->addr, globents, as(globents) );
		return False;
	} else if (*bindings == XrmBindLoosely && !quarks[1]) { /* DM*bar */
		dpy = "*";
		key = XrmQuarkToString (*quarks);
	} else if (*bindings != XrmBindLoosely && quarks[1] &&
	           *bindings != XrmBindLoosely && !quarks[2])
	{ /* DM.foo.bar */
		dpy = dpybuf + 4;
		strcpy( dpybuf + 4, XrmQuarkToString (*quarks) );
		for (hasu = 0, el = 4; dpybuf[el]; el++)
			if (dpybuf[el] == '_')
				hasu = 1;
		if (!hasu/* && isupper (dpy[0])*/) {
			dpy = dpybuf;
			memcpy( dpybuf, "*:*_", 4 );
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
	handleXdmVal( dpy, key, value->addr, dpyents, as(dpyents) );
	return False;
}

static FDefs xdmdefs[] = {
#ifdef XDMCP
{ "Xdmcp", "Xaccess", "%s/Xaccess", 0 },
{ "Xdmcp", "Willing", "", 0 },
#endif
{ "X-*-Core", "Setup", "", 0 },
{ "X-*-Core", "Startup", "", 0 },
{ "X-*-Core", "Reset", "", 0 },
{ "X-*-Core", "Session", "", 0 },
};

static int
mergeXdmCfg( const char *path )
{
	char *p;
	XrmDatabase db;

	ASPrintf( &p, "%s/xdm-config", path );
	if ((db = XrmGetFileDatabase( p ))) {
		printf( "Information: reading old xdm config file %s\n", p );
		usedFile( p );
		free( p );
		XrmEnumerateDatabase( db, &empty, &empty, XrmEnumAllLevels,
		                      DumpEntry, (XPointer)0 );
		applydefs( xdmdefs, as(xdmdefs), path );
		mod_usebg = 1;
		return 1;
	}
	free( p );
	return 0;
}

static void
fwrapprintf( FILE *f, const char *msg, ... )
{
	char *txt, *ftxt, *line;
	va_list ap;
	int col, lword, fspace;

	va_start( ap, msg );
	VASPrintf( &txt, msg, ap );
	va_end( ap );
	ftxt = 0;
	for (line = txt, col = 0, lword = fspace = -1; line[col]; ) {
		if (line[col] == '\n') {
			StrCat( &ftxt, "%.*s", ++col, line );
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
				StrCat( &ftxt, "%.*s\n", fspace, line );
				line += lword;
				col -= lword;
				lword = 0;
				fspace = -1;
			}
		}
		col++;
	}
	free( txt );
	fputs( ftxt, f );
	free( ftxt );
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
};

static const char *oldxdms[] = {
	"/etc/X11/xdm",
	XLIBDIR "/xdm",
};

int main( int argc, char **argv )
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
	char *nname;

	for (ap = 1; ap < argc; ap++) {
		if (!strcmp( argv[ap], "--help" )) {
			printf(
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
			exit( 0 );
		}
		if (!strcmp( argv[ap], "--no-old" )) {
			no_old = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--old-scripts" )) {
			old_scripts = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--no-old-scripts" )) {
			no_old_scripts = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--old-confs" )) {
			old_confs = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--no-old-xdm" )) {
			no_old_xdm = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--no-old-kde" )) {
			no_old_kde = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--no-backup" )) {
			no_backup = 1;
			continue;
		}
		if (!strcmp( argv[ap], "--no-in-notice" )) {
			no_in_notice = 1;
			continue;
		}
		where = 0;
		if (!strcmp( argv[ap], "--in" ))
			where = &newdir;
		else if (!strcmp( argv[ap], "--old-xdm" ))
			where = &oldxdm;
		else if (!strcmp( argv[ap], "--old-kde" ))
			where = &oldkde;
		else if (!strcmp( argv[ap], "--face-src" ))
			where = &facesrc;
		else {
			fprintf( stderr, "Unknown command line option '%s', try --help\n", argv[ap] );
			exit( 1 );
		}
		if (ap + 1 == argc || argv[ap + 1][0] == '-') {
			fprintf( stderr, "Missing argument to option '%s', try --help\n", argv[ap] );
			exit( 1 );
		}
		*where = argv[++ap];
	}
	if (memcmp( newdir, KDMCONF, sizeof(KDMCONF) ))
		use_destdir = 1;

	if (!mkdirp( newdir, 0755, "target", 1 ))
		exit( 1 );

	mkdefconf();
	newer = 0;
	if (no_old) {
		DIR *dir;
		if ((dir = opendir( newdir ))) {
			struct dirent *ent;
			char bn[PATH_MAX];
			while ((ent = readdir( dir ))) {
				int l;
			 if (!strcmp( ent->d_name, "." ) || !strcmp( ent->d_name, ".." ))
					continue;
				l = sprintf( bn, "%s/%s", newdir, ent->d_name ); /* cannot overflow (kernel would not allow the creation of a longer path) */
				if (!stat( bn, &st ) && !S_ISREG( st.st_mode ))
					continue;
				if (no_backup || !memcmp( bn + l - 4, ".bak", 5 ))
					unlink( bn );
				else
					displace( bn );
			}
			closedir( dir );
		}
	} else {
		if (oldkde) {
			if (!(newer = mergeKdmRcNewer( oldkde )) && !mergeKdmRcOld( oldkde ))
				fprintf( stderr,
				         "Cannot read old kdmrc at specified location\n" );
		} else if (!no_old_kde) {
			for (i = 0; i < as(oldkdes); i++) {
				if ((newer = mergeKdmRcNewer( oldkdes[i] )) ||
				    mergeKdmRcOld( oldkdes[i] )) {
					oldkde = oldkdes[i];
					break;
				}
			}
		}
		if (!newer && !no_old_xdm) {
			XrmInitialize();
			XrmQString = XrmPermStringToQuark( "String" );
			if (oldxdm) {
				if (!mergeXdmCfg( oldxdm ))
					fprintf( stderr,
					         "Cannot read old kdm-config/xdm-config at specified location\n" );
			} else
				for (i = 0; i < as(oldxdms); i++)
					if (mergeXdmCfg( oldxdms[i] )) {
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
			if (!strcmp( cs->spec->name, "-Core" )) {
				for (ce = cs->ents; ce; ce = ce->next)
					if (ce->active &&
					    (!strcmp( ce->spec->key, "Setup" ) ||
					     !strcmp( ce->spec->key, "Startup" ) ||
					     !strcmp( ce->spec->key, "Reset" )))
					{
						if (inNewDir( ce->value ))
							locals = 1;
						else
							foreigns = 1;
					}
			}
		if (foreigns) {
			if (locals) {
				fprintf( stderr,
				         "Warning: both local and foreign scripts referenced. "
				         "Won't touch any.\n" );
				mixed_scripts = 1;
			} else {
			  no_old_s:
				for (cs = config; cs; cs = cs->next) {
					if (!strcmp( cs->spec->name, "Xdmcp" )) {
						for (ce = cs->ents; ce; ce = ce->next)
							if (!strcmp( ce->spec->key, "Willing" ))
								ce->active = ce->written = 0;
					} else if (!strcmp( cs->spec->name, "-Core" )) {
						for (cep = &cs->ents; (ce = *cep); ) {
							if (ce->active &&
							    (!strcmp( ce->spec->key, "Setup" ) ||
							     !strcmp( ce->spec->key, "Startup" ) ||
							     !strcmp( ce->spec->key, "Reset" ) ||
							     !strcmp( ce->spec->key, "Session" )))
							{
								if (!memcmp( cs->name, "X-*-", 4 ))
									ce->active = ce->written = 0;
								else {
									*cep = ce->next;
									free( ce );
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
#ifdef __linux__
	if (!stat( "/etc/debian_version", &st )) { /* debian */
		defminuid = "1000";
		defmaxuid = "29999";
	} else if (!stat( "/usr/portage", &st )) { /* gentoo */
		defminuid = "1000";
		defmaxuid = "65000";
	} else if (!stat( "/etc/mandrake-release", &st )) { /* mandrake - check before redhat! */
		defminuid = "500";
		defmaxuid = "65000";
	} else if (!stat( "/etc/redhat-release", &st )) { /* redhat */
		defminuid = "100";
		defmaxuid = "65000";
	} else /* if (!stat( "/etc/SuSE-release", &st )) */ { /* suse */
		defminuid = "500";
		defmaxuid = "65000";
	}
#else
	defminuid = "1000";
	defmaxuid = "65000";
#endif
	for (i = 0; i < CONF_MAX_PRIO; i++)
		for (cs = config; cs; cs = cs->next)
			for (ce = cs->ents; ce; ce = ce->next)
				if (ce->spec->func && i == ce->spec->prio)
					ce->spec->func( ce, cs );
	ASPrintf( &newkdmrc, "%s/kdmrc", newdir );
	f = Create( newkdmrc, kdmrcmode );
	wrconf( f );
	fclose( f );

	ASPrintf( &nname, "%s/README", newdir );
	f = Create( nname, 0644 );
	fprintf( f,
"This automatically generated configuration consists of the following files:\n" );
	fprintf( f, "- " KDMCONF "/kdmrc\n" );
	for (fp = aflist; fp; fp = fp->next)
		fprintf( f, "- %s\n", fp->str );
	if (use_destdir && !no_in_notice)
		fwrapprintf( f,
"All files destined for " KDMCONF " were actually saved in %s; "
"this config won't be workable until moved in place.\n", newdir );
	if (uflist || eflist || cflist || lflist) {
		fprintf( f,
"\n"
"This config was derived from existing files. As the used algorithms are\n"
"pretty dumb, it may be broken.\n" );
		if (uflist) {
			fprintf( f,
"Information from these files was extracted:\n" );
			for (fp = uflist; fp; fp = fp->next)
				fprintf( f, "- %s\n", fp->str );
		}
		if (lflist) {
			fprintf( f,
"These files were directly incorporated:\n" );
			for (fp = lflist; fp; fp = fp->next)
				fprintf( f, "- %s\n", fp->str );
		}
		if (cflist) {
			fprintf( f,
"These files were copied verbatim:\n" );
			for (fp = cflist; fp; fp = fp->next)
				fprintf( f, "- %s\n", fp->str );
		}
		if (eflist) {
			fprintf( f,
"These files were copied with modifications:\n" );
			for (fp = eflist; fp; fp = fp->next)
				fprintf( f, "- %s\n", fp->str );
		}
		if (!no_backup && !use_destdir)
			fprintf( f,
"Old files that would have been overwritten were renamed to <oldname>.bak.\n" );
	}
	fprintf( f,
"\nTry 'genkdmconf --help' if you want to generate another configuration.\n"
"\nYou may delete this README.\n" );
	fclose( f );

	return 0;
}
