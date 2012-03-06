/*

Create a suitable configuration for kdm taking previous xdm/kdm
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

#include <greet.h>

#include <X11/Xlib.h>
#include <X11/Xresource.h>

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
#include <sys/wait.h>
#include <sys/param.h>
#ifdef BSD
# ifdef BSD_UTMP
#  include <utmp.h>
# endif
# ifndef _PATH_UTMP
#  if defined(__FreeBSD_version) && __FreeBSD_version >= 900007
#   define _PATH_UTMP "/var/run/utmp"
#  else
#   error "_PATH_UTMP must be defined"
#  endif
# endif
#endif

#define WANT_CONF_GEN
#include <config.ci>

#define RCVERSTR stringify(RCVERMAJOR) "." stringify(RCVERMINOR)

static int old_scripts, no_old_scripts, old_confs, no_old,
    no_backup, no_in_notice, use_destdir, mixed_scripts;
static const char *newdir = KDMCONF, *facesrc = KDMDATA "/pics/users",
    *oldxdm, *oldkde, *oldkdepfx;

static int oldver;


typedef struct StrList {
    struct StrList *next;
    const char *str;
} StrList;

typedef struct StrMap {
    struct StrMap *next;
    const char *key, *value;
} StrMap;


static void *
mmalloc(size_t sz)
{
    void *ptr;

    if (!(ptr = malloc(sz))) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return ptr;
}

static void *
mcalloc(size_t sz)
{
    void *ptr;

    if (!(ptr = calloc(1, sz))) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return ptr;
}

static void *
mrealloc(void *optr, size_t sz)
{
    void *ptr;

    if (!(ptr = realloc(optr, sz))) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    return ptr;
}

static char *
mstrdup(const char *optr)
{
    char *ptr;

    if (!optr)
        return 0;
    if (!(ptr = strdup(optr))) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
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
outCh_OCA(void *bp, char c)
{
    OCABuf *ocabp = (OCABuf *)bp;

    ocabp->tlen++;
    if (ocabp->clen >= ocabp->blen) {
        ocabp->blen = ocabp->blen * 3 / 2 + 100;
        ocabp->buf = mrealloc(ocabp->buf, ocabp->blen);
    }
    ocabp->buf[ocabp->clen++] = c;
}

static int
VASPrintf(char **strp, const char *fmt, va_list args)
{
    OCABuf ocab = { 0, 0, 0, -1 };

    doPrint(outCh_OCA, &ocab, fmt, args);
    outCh_OCA(&ocab, 0);
    *strp = realloc(ocab.buf, ocab.clen);
    if (!*strp)
        *strp = ocab.buf;
    return ocab.tlen;
}

static int
ASPrintf(char **strp, const char *fmt, ...)
{
    va_list args;
    int len;

    va_start(args, fmt);
    len = VASPrintf(strp, fmt, args);
    va_end(args);
    return len;
}

static void
strCat(char **strp, const char *fmt, ...)
{
    char *str, *tstr;
    va_list args;
    int el;

    va_start(args, fmt);
    el = VASPrintf(&str, fmt, args);
    va_end(args);
    if (*strp) {
        int ol = strlen(*strp);
        tstr = mmalloc(el + ol + 1);
        memcpy(tstr, *strp, ol);
        memcpy(tstr + ol, str, el + 1);
        free(*strp);
        free(str);
        *strp = tstr;
    } else {
        *strp = str;
    }
}


#define WANT_CLOSE 1

typedef struct File {
    char *buf, *eof, *cur;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
    int ismapped;
#endif
} File;

static int
readFile(File *file, const char *fn)
{
    off_t flen;
    int fd;

    if ((fd = open(fn, O_RDONLY)) < 0)
        return False;

    flen = lseek(fd, 0, SEEK_END);
#ifdef HAVE_MMAP
# ifdef WANT_CLOSE
    file->ismapped = False;
# endif
    file->buf = mmap(0, flen + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
# ifdef WANT_CLOSE
    if (file->buf)
        file->ismapped = True;
    else
# else
    if (!file->buf)
# endif
#endif
    {
        file->buf = mmalloc(flen + 1);
        lseek(fd, 0, SEEK_SET);
        if (read(fd, file->buf, flen) != flen) {
            free(file->buf);
            close(fd);
            fprintf(stderr, "Cannot read file\n");
            return False; /* maybe better abort? */
        }
    }
    file->eof = file->buf + flen;
    close(fd);
    return True;
}

#ifdef WANT_CLOSE
static void
freeBuf(File *file)
{
# ifdef HAVE_MMAP
    if (file->ismapped)
        munmap(file->buf, file->eof - file->buf);
    else
# endif
        free(file->buf);
}
#endif

static int
isTrue(const char *val)
{
    return !strcmp(val, "true") ||
           !strcmp(val, "yes") ||
           !strcmp(val, "on") ||
           atoi(val);
}


static int
mkpdirs(const char *name, const char *what)
{
    char *mfname = mstrdup(name);
    int i;
    struct stat st;

    for (i = 1; mfname[i]; i++)
        if (mfname[i] == '/') {
            mfname[i] = 0;
            if (stat(mfname, &st)) {
                if (mkdir(mfname, 0755)) {
                    fprintf(stderr, "Cannot create parent %s of %s directory %s: %s\n",
                            mfname, what, name, strerror(errno));
                    free(mfname);
                    return False;
                }
                chmod(mfname, 0755);
            }
            mfname[i] = '/';
        }
    free(mfname);
    return True;
}

static int
mkdirp(const char *name, int mode, const char *what, int existok)
{
    struct stat st;

    if (stat(name, &st)) {
        mkpdirs(name, what);
        if (mkdir(name, mode)) {
            fprintf(stderr, "Cannot create %s directory %s: %s\n",
                    what, name, strerror(errno));
            return False;
        }
        chmod(name, mode);
        return True;
    }
    return existok;
}


static void
displace(const char *fn)
{
    if (!no_backup) {
        char bn[PATH_MAX + 4];
        sprintf(bn, "%s.bak", fn); /* won't overflow if only existing paths are passed */
        rename(fn, bn);
    } else {
        unlink(fn);
    }
}


static char *curName;

/* Create a new file in KDMCONF */
static FILE *
createFile(const char *fn, int mode)
{
    FILE *f;

    free(curName);
    ASPrintf(&curName, "%s/%s", newdir, fn);
    displace(curName);
    if (!(f = fopen(curName, "w"))) {
        fprintf(stderr, "Cannot create %s\n", curName);
        exit(1);
    }
    chmod(curName, mode);
    return f;
}

static void
writeError()
{
    fprintf(stderr, "Warning: cannot write %s (disk full?)\n", curName);
    unlink(curName);
    exit(1);
}

static void
fputs_(const char *str, FILE *f)
{
    if (fputs(str, f) == EOF)
        writeError();
}

static void
ATTR_PRINTFLIKE(2, 3)
fprintf_(FILE *f, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    if (vfprintf(f, fmt, args) < 0)
        writeError();
    va_end(args);
}

static void
fclose_(FILE *f)
{
    if (fclose(f) == EOF)
        writeError();
}


static char *
locate(const char *exe)
{
    int len;
    char *path, *pathe, *name, *thenam, nambuf[PATH_MAX+1];

    if (!(path = getenv("PATH")))
        return 0;
    len = strlen(exe);
    name = nambuf + PATH_MAX - len;
    memcpy(name, exe, len + 1);
    *--name = '/';
    do {
        if (!(pathe = strchr(path, ':')))
            pathe = path + strlen(path);
        len = pathe - path;
        if (len && !(len == 1 && *path == '.')) {
            thenam = name - len;
            if (thenam >= nambuf) {
                memcpy(thenam, path, len);
                if (!access(thenam, X_OK))
                    return mstrdup(thenam);
            }
        }
        path = pathe;
    } while (*path++ != '\0');
    return 0;
}

static int
runAndWait(char **args)
{
    int pid, ret;

    switch ((pid = fork())) {
    case 0:
        execv(args[0], args);
        fprintf(stderr, "Cannot execute %s: %s\n",
                args[0], strerror(errno));
        _exit(127);
    case -1:
        fprintf(stderr, "Cannot fork to execute %s: %s\n",
                args[0], strerror(errno));
        return -1;
    }
    while (waitpid(pid, &ret, 0) < 0)
        if (errno != EINTR)
            return -1;
    return ret;
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
    void (*func)(Entry *ce, Section *cs);
    const char *comment;
} Ent;

typedef struct Sect {
    const char *name;
    Ent *ents;
    int nents;
} Sect;

static Sect *findSect(const char *name);
static Ent *findEnt(Sect *sect, const char *key);

/*
 * Functions to manipulate the current kdmrc data
 */

static const char *
getFqVal(const char *sect, const char *key, const char *defval)
{
    Section *cs;
    Entry *ce;

    for (cs = config; cs; cs = cs->next)
        if (!strcmp(cs->name, sect)) {
            for (ce = cs->ents; ce; ce = ce->next)
                if (!strcmp(ce->spec->key, key)) {
                    if (ce->active && ce->written)
                        return ce->value;
                    break;
                }
            break;
        }
    return defval;
}

static void
putFqVal(const char *sect, const char *key, const char *value)
{
    Section *cs, **csp;
    Entry *ce, **cep;

    if (!value)
        return;

    for (csp = &config; (cs = *csp); csp = &(cs->next))
        if (!strcmp(sect, cs->name))
            goto havesec;
    cs = mcalloc(sizeof(*cs));
    ASPrintf((char **)&cs->name, "%s", sect);
    cs->spec = findSect(sect);
    *csp = cs;
  havesec:

    for (cep = &(cs->ents); (ce = *cep); cep = &(ce->next))
        if (!strcmp(key, ce->spec->key))
            goto haveent;
    ce = mcalloc(sizeof(*ce));
    ce->spec = findEnt(cs->spec, key);
    *cep = ce;
  haveent:
    ASPrintf((char **)&ce->value, "%s", value);
    ce->written = ce->active = True;
}

static const char *csect;

#define setSect(se) csect = se

static void
putVal(const char *key, const char *value)
{
    putFqVal(csect, key, value);
}


static void
writeKdmrc(FILE *f)
{
    Section *cs;
    Entry *ce;
    StrList *sl = 0, *sp;
    const char *cmt;

    putFqVal("General", "ConfigVersion", RCVERSTR);
    for (cs = config; cs; cs = cs->next) {
        fprintf_(f, "%s[%s]\n",
                 cs->comment ? cs->comment : "\n", cs->name);
        for (ce = cs->ents; ce; ce = ce->next) {
            if (ce->spec->comment) {
                cmt = ce->spec->comment;
                for (sp = sl; sp; sp = sp->next)
                    if (sp->str == cmt) {
                        cmt = "# See above\n";
                        goto havit;
                    }
                if (!(sp = malloc(sizeof(*sp)))) {
                    fprintf(stderr, "Warning: Out of memory\n");
                } else {
                    sp->str = cmt;
                    sp->next = sl;
                    sl = sp;
                }
            } else {
                cmt = "";
            }
          havit:
            fprintf_(f, "%s%s%s=%s\n",
                     cmt, ce->active ? "" : "#", ce->spec->key, ce->value);
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
"# Sometimes, however, the chooser cannot figure out how to broadcast,\n"
"# so this may not work in all environments.\n"
"#\n"
"\n"
"*		CHOOSER BROADCAST	#any indirect host can get a chooser\n"
"\n"
"#\n"
"# If you would prefer to configure the set of hosts each terminal sees,\n"
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
"# XDM configurations typically have sessreg here. KDM has it built-in.\n"
"\n"
"# NOTE: The session is aborted if the last command returns non-zero.\n";

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
"# XDM configurations typically have sessreg here. KDM has it built-in.\n";

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
"    $SHELL -c \"if (-f /etc/csh.login) source /etc/csh.login; if (-f ~/.login) source ~/.login; /bin/sh -c 'export -p' >! $xsess_tmp\"\n"
"    . $xsess_tmp\n"
"    rm -f $xsess_tmp\n"
"    ;;\n"
"  *) # Plain sh, ksh, and anything we do not know.\n"
"    [ -f /etc/profile ] && . /etc/profile\n"
"    [ -f $HOME/.profile ] && . $HOME/.profile\n"
"    ;;\n"
"esac\n"
"\n"
"[ -f /etc/xprofile ] && . /etc/xprofile\n"
"[ -f $HOME/.xprofile ] && . $HOME/.xprofile\n"
"\n"
"if [ -d /etc/X11/Xresources ]; then\n"
"  for i in /etc/X11/Xresources/*; do\n"
"    [ -f $i ] && xrdb -merge $i\n"
"  done\n"
"elif [ -f /etc/X11/Xresources ]; then\n"
"  xrdb -merge /etc/X11/Xresources\n"
"fi\n"
"[ -f $HOME/.Xresources ] && xrdb -merge $HOME/.Xresources\n"
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
"Wallpaper=stripes.png\n"
"WallpaperList=\n"
"WallpaperMode=Scaled\n";

/* Create a copy of a file under KDMCONF and fill it */
static void
writeCopy(const char *fn, int mode, time_t stamp, const char *buf, size_t len)
{
    char *nname;
    int fd;
    struct utimbuf utim;

    ASPrintf(&nname, "%s/%s", newdir, fn);
    displace(nname);
    mkpdirs(nname, "target");
    if ((fd = creat(nname, mode)) < 0) {
        fprintf(stderr, "Cannot create %s\n", nname);
        exit(1);
    }
    if (write(fd, buf, len) != (ssize_t)len || close(fd) < 0) {
        fprintf(stderr, "Cannot write %s (disk full?)\n", nname);
        unlink(nname);
        exit(1);
    }
    if (stamp) {
        utim.actime = utim.modtime = stamp;
        utime(nname, &utim);
    }
    free(nname);
}


/* returns static array! */
static const char *
reSect(const char *sec, const char *name)
{
    static char sname[64];
    char *p;

    if ((p = strrchr(sec, '-'))) {
        sprintf(sname, "%.*s-%s", (int)(p - sec), sec, name);
        return sname;
    } else {
        return name;
    }
}

static int
inNewDir(const char *name)
{
    return !memcmp(name, KDMCONF "/", sizeof(KDMCONF));
}

static const char *
getMapping(StrMap *sm, const char *k)
{
    for (; sm; sm = sm->next)
        if (!strcmp(sm->key, k))
            return sm->value;
    return 0;
}

static void
addMapping(StrMap **sm, const char *k, const char *v)
{
    for (; *sm; sm = &(*sm)->next)
        if (!strcmp((*sm)->key, k))
            return;
    *sm = mcalloc(sizeof(**sm));
    ASPrintf((char **)&(*sm)->key, "%s", k);
    ASPrintf((char **)&(*sm)->value, "%s", v);
}

static int
inList(StrList *sp, const char *s)
{
    for (; sp; sp = sp->next)
        if (!strcmp(sp->str, s))
            return True;
    return False;
}

static void
addStr(StrList **sp, const char *s)
{
    for (; *sp; sp = &(*sp)->next)
        if (!strcmp((*sp)->str, s))
            return;
    *sp = mcalloc(sizeof(**sp));
    ASPrintf((char **)&(*sp)->str, "%s", s);
}

static StrList *
splitList(const char *str)
{
    StrList *sp, **spp = &sp;
    const char *e;
    if (!*str)
        return 0;
    for (;;) {
        *spp = mcalloc(sizeof(**spp));
        if (!(e = strchr(str, ',')))
            break;
        ASPrintf((char **)&(*spp)->str, "%.*s", (int)(e - str), str);
        str = e + 1;
        spp = &(*spp)->next;
    }
    (*spp)->str = mstrdup(str);
    (*spp)->next = 0;
    return sp;
}

static char *
joinList(StrList *sp)
{
    char *s = 0;
    if (!sp)
        return mstrdup("");
    s = mstrdup(sp->str);
    for (;;) {
        sp = sp->next;
        if (!sp)
            return s;
        strCat(&s, ",%s", sp->str);
    }
}

StrMap *cfmap;
StrList *aflist, *uflist, *eflist, *cflist, *lflist;

/* file is part of new config */
static void
addedFile(const char *fn)
{
    addStr(&aflist, fn);
}

/* file from old config was parsed */
static void
usedFile(const char *fn)
{
    addStr(&uflist, fn);
}

/* file from old config was copied with slight modifications */
static void
editedFile(const char *fn)
{
    addStr(&eflist, fn);
}

/* file from old config was copied verbatim */
static void
copiedFile(const char *fn)
{
    addStr(&cflist, fn);
}

/* file from old config is still being used */
static void
linkedFile(const char *fn)
{
    addStr(&lflist, fn);
}

/*
 * NOTE: This code will not correctly deal with default files colliding
 * with pre-existing files. This should be OK, as for each class of files
 * (scripts, configs) only one origin is used, and conflicts between classes
 * are rather unlikely.
 */

/* Make a possibly modified copy of a file under KDMCONF */
static int
copyFile(Entry *ce, int mode, int (*proc)(File *))
{
    const char *tptr;
    char *nname;
    File file;
    int rt;

    if (!*ce->value)
        return True;

    if ((nname = (char *)getMapping(cfmap, ce->value))) {
        rt = inList(aflist, nname);
        goto doret;
    }
    if (oldkde) {
        int olen = strlen(oldkde);
        if (!memcmp(ce->value, oldkde, olen)) {
            if (!memcmp(ce->value + olen, "/kdm/", 5)) {
                tptr = ce->value + olen + 4;
                goto gotn;
            }
            if (ce->value[olen] == '/') {
                tptr = ce->value + olen;
                goto gotn;
            }
        }
    }
    if (oldxdm) {
        int olen = strlen(oldxdm);
        if (!memcmp(ce->value, oldxdm, olen) && ce->value[olen] == '/') {
            tptr = ce->value + olen;
            goto gotn;
        }
    }
    if (!(tptr = strrchr(ce->value, '/'))) {
        fprintf(stderr, "Warning: cannot cope with relative path %s\n", ce->value);
        return False;
    }
  gotn:
    ASPrintf(&nname, KDMCONF "%s", tptr);
    if (inList(aflist, nname)) {
        int cnt = 1;
        do {
            free(nname);
            ASPrintf(&nname, KDMCONF "%s-%d", tptr , ++cnt);
        } while (inList(aflist, nname));
    }
    addMapping(&cfmap, ce->value, nname);
    if (!readFile(&file, ce->value)) {
        fprintf(stderr, "Warning: cannot copy file %s\n", ce->value);
        rt = False;
    } else {
        if (!proc || !proc(&file)) {
            if (!use_destdir && !strcmp(ce->value, nname)) {
                linkedFile(nname);
            } else {
                struct stat st;
                stat(ce->value, &st);
                writeCopy(nname + sizeof(KDMCONF), mode, st.st_mtime,
                          file.buf, file.eof - file.buf);
                copiedFile(ce->value);
            }
        } else {
            writeCopy(nname + sizeof(KDMCONF), mode, 0,
                      file.buf, file.eof - file.buf);
            editedFile(ce->value);
        }
        if (strcmp(ce->value, nname) && inNewDir(ce->value) && !use_destdir)
            displace(ce->value);
        addedFile(nname);
        rt = True;
    }
  doret:
    ce->value = nname;
    return rt;
}

static void
doLinkFile(const char *name)
{
    File file;

    if (inList(aflist, name))
        return;
    if (!readFile(&file, name)) {
        fprintf(stderr, "Warning: cannot read file %s\n", name);
        return;
    }
    if (inNewDir(name) && use_destdir) {
        struct stat st;
        stat(name, &st);
        writeCopy(name + sizeof(KDMCONF), st.st_mode, st.st_mtime,
                  file.buf, file.eof - file.buf);
        copiedFile(name);
    } else {
        linkedFile(name);
    }
    addedFile(name);
}

/* Incorporate an existing file */
static void
linkFile(Entry *ce)
{
    if (ce->written && *ce->value)
        doLinkFile(ce->value);
}

/* Create a new file in KDMCONF and fill it */
static void
writeFile(const char *tname, int mode, const char *cont)
{
    FILE *f = createFile(tname + sizeof(KDMCONF), mode);
    fputs_(cont, f);
    fclose_(f);
    addedFile(tname);
}


static void
handleBgCfg(Entry *ce, Section *cs)
{
    if (!ce->active) { /* can be only the X-*-Greeter one */
        writeFile(def_BackgroundCfg, 0644, def_background);
#if 0 /* risk of kcontrol clobbering the original file */
    } else if (old_confs) {
        linkFile(ce);
#endif
    } else {
        if (!copyFile(ce, 0644, 0)) {
            if (!strcmp(cs->name, "X-*-Greeter"))
                writeFile(def_BackgroundCfg, 0644, def_background);
            ce->active = False;
        }
    }
}


#ifdef HAVE_VTS
static char *
memMem(char *mem, int lmem, const char *smem, int lsmem)
{
    for (; lmem >= lsmem; mem++, lmem--)
        if (!memcmp(mem, smem, lsmem))
            return mem + lsmem;
    return 0;
}

static int maxTTY, TTYmask;

static void
getInitTab(void)
{
    File it;
    char *p, *eol, *ep;
    int tty;

    if (maxTTY)
        return;
    if (readFile(&it, "/etc/inittab")) {
        usedFile("/etc/inittab");
        for (p = it.buf; p < it.eof; p = eol + 1) {
            for (eol = p; eol < it.eof && *eol != '\n'; eol++);
            if (*p != '#') {
                if ((ep = memMem(p, eol - p, " tty", 4)) &&
                    ep < eol && isdigit(*ep))
                {
                    if (ep + 1 == eol || isspace(*(ep + 1)))
                        tty = *ep - '0';
                    else if (isdigit(*(ep + 1)) &&
                             (ep + 2 == eol || isspace(*(ep + 2))))
                        tty = (*ep - '0') * 10 + (*(ep + 1) - '0');
                    else
                        continue;
                    TTYmask |= 1 << (tty - 1);
                    if (tty > maxTTY)
                        maxTTY = tty;
                }
            }
        }
        freeBuf(&it);
    }
    if (!maxTTY) {
        maxTTY = 6;
        TTYmask = 0x3f;
    }
}
#endif


static char *
readWord(File *file, int EOFatEOL)
{
    char *wordp, *wordBuffer;
    int quoted;
    char c;

  rest:
    wordp = wordBuffer = file->cur;
  mloop:
    quoted = False;
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
            quoted = True;
            goto qloop;
        }
        break;
    }
    *wordp++ = c;
    goto mloop;
}

/* backslashes are double-escaped - first parseArgs, then KConfig */

static StrList *
splitArgs(const char *string)
{
    const char *word;
    char *str;
    int wlen;
    StrList *args, **argp = &args;

    while (*string) {
        if (isspace(*string)) {
            string++;
            continue;
        }
        word = string;
        wlen = 0;
        do {
            if (*string == '\\') {
                if (*++string != '\\')
                    string--;
                if (*++string != '\\')
                    string--;
                if (!*++string)
                    string--;
                wlen++;
            } else if (*string == '\'') {
                while (*++string != '\'' && *string) {
                    if (*string == '\\' && *++string != '\\')
                        string--;
                    wlen++;
                }
            } else if (*string == '"') {
                while (*++string != '"' && *string) {
                    if (*string == '\\') {
                        if (*++string != '\\')
                            string--;
                        if (*++string != '\\')
                            string--;
                        if (!*++string)
                            string--;
                    }
                    wlen++;
                }
            } else {
                wlen++;
            }
        } while (*++string && !isspace(*string));
        *argp = mmalloc(sizeof(**argp));
        (*argp)->str = str = mmalloc(wlen + 1);
        do {
            if (*word == '\\') {
                if (*++word != '\\')
                    word--;
                if (*++word != '\\')
                    word--;
                if (!*++word)
                    word--;
                *str++ = *word;
            } else if (*word == '\'') {
                while (*++word != '\'' && *word) {
                    if (*word == '\\' && *++word != '\\')
                        word--;
                    *str++ = *word;
                }
            } else if (*word == '"') {
                while (*++word != '"' && *word) {
                    if (*word == '\\') {
                        if (*++word != '\\')
                            word--;
                        if (*++word != '\\')
                            word--;
                        if (!*++word)
                            word--;
                    }
                    *str++ = *word;
                }
            } else {
                *str++ = *word;
            }
        } while (*++word && !isspace(*word));
        *str = 0;
        argp = &(*argp)->next;
    }
    *argp = 0;
    return args;
}

static const char *
joinArgs(StrList *argv)
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
            if (isspace(*s) || *s == '\'')
                nq = 2;
            else if (*s == '"')
                slen += 2;
            else if (*s == '\\')
                slen += 3;
        slen += nq;
    }
    rs = str = mmalloc(slen);
    for (av = argv; av; av = av->next) {
        int nq = 0;
        for (s = av->str; *s; s++)
            if (isspace(*s) || *s == '\'')
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

typedef enum { InvalidDpy, LocalDpy, LocalUidDpy, ForeignDpy } DisplayMatchType;

static struct displayMatch {
    const char *name;
    int len;
    DisplayMatchType type;
} displayTypes[] = {
    { "local", 5, LocalDpy },
    { "local_uid", 9, LocalUidDpy },
    { "foreign", 7, ForeignDpy },
};

static DisplayMatchType
parseDisplayType(const char *string, const char **atPos)
{
    struct displayMatch *d;

    *atPos = 0;
    for (d = displayTypes; d < displayTypes + as(displayTypes); d++) {
        if (!memcmp(d->name, string, d->len) &&
            (!string[d->len] || string[d->len] == '@'))
        {
            if (string[d->len] == '@' && string[d->len + 1])
                *atPos = string + d->len + 1;
            return d->type;
        }
    }
    return InvalidDpy;
}

typedef struct serverEntry {
    struct serverEntry *next;
    const char *name, *class2, *console, *owner, *argvs, *arglvs;
    StrList *argv, *arglv;
    DisplayMatchType type;
    int reserve, vt;
} ServerEntry;

static int
mstrcmp(const char *s1, const char *s2)
{
    if (s1 == s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;
    return strcmp(s1, s2);
}

static void
absorbXservers(const char *sect ATTR_UNUSED, char **value)
{
    ServerEntry *se, *se1, *serverList, **serverPtr;
    const char *word, *word2;
    char *sdpys, *rdpys;
    StrList **argp, **arglp, *ap, *ap2;
    File file;
    int nldpys = 0, nrdpys = 0, dpymask = 0;
    int cpuid, cpcmd, cpcmdl;
#ifdef HAVE_VTS
    int dn, cpvt, mtty;
#endif

    if (**value == '/') {
        if (!readFile(&file, *value))
            return;
        usedFile(*value);
    } else {
        file.buf = *value;
        file.eof = *value + strlen(*value);
    }
    file.cur = file.buf;

    serverPtr = &serverList;
#ifdef HAVE_VTS
  bustd:
#endif
    while ((word = readWord(&file, 0))) {
        se = mcalloc(sizeof(*se));
        se->name = word;
        if (!(word = readWord(&file, 1)))
            continue;
        se->type = parseDisplayType(word, &se->console);
        if (se->type == InvalidDpy) {
            se->class2 = word;
            if (!(word = readWord(&file, 1)))
                continue;
            se->type = parseDisplayType(word, &se->console);
            if (se->type == InvalidDpy) {
                while (readWord(&file, 1));
                continue;
            }
        }
        if (se->type == LocalUidDpy)
            if (!(se->owner = readWord(&file, 1)))
                continue;
        word = readWord(&file, 1);
        if (word && !strcmp(word, "reserve")) {
            se->reserve = True;
            word = readWord(&file, 1);
        }
        if ((se->type != ForeignDpy) != (word != 0))
            continue;
        argp = &se->argv;
        arglp = &se->arglv;
        while (word) {
#ifdef HAVE_VTS
            if (word[0] == 'v' && word[1] == 't') {
                se->vt = atoi(word + 2);
            } else if (!strcmp(word, "-crt")) { /* SCO style */
                if (!(word = readWord(&file, 1)) || memcmp(word, "/dev/tty", 8))
                    goto bustd;
                se->vt = atoi(word + 8);
            } else
#endif
            if (strcmp(word, se->name)) {
                ap = mmalloc(sizeof(*ap));
                ap->str = word;
                if (!strcmp(word, "-nolisten")) {
                    if (!(word2 = readWord(&file, 1)))
                        break;
                    ap2 = mmalloc(sizeof(*ap2));
                    ap2->str = word2;
                    ap->next = ap2;
                    if (!strcmp(word2, "unix")) {
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
            word = readWord(&file, 1);
        }
        *argp = *arglp = 0;
        if (se->type != ForeignDpy) {
            nldpys++;
            dpymask |= 1 << atoi(se->name + 1);
            if (se->reserve)
                nrdpys++;
        }
        *serverPtr = se;
        serverPtr = &se->next;
    }
    *serverPtr = 0;

#ifdef HAVE_VTS
    /* don't copy only if all local displays are ordered and have a vt */
    cpvt = False;
    getInitTab();
    for (se = serverList, mtty = maxTTY; se; se = se->next)
        if (se->type != ForeignDpy) {
            mtty++;
            if (se->vt != mtty) {
                cpvt = True;
                break;
            }
        }
#endif

    for (se = serverList; se; se = se->next) {
        se->argvs = joinArgs(se->argv);
        se->arglvs = joinArgs(se->arglv);
    }

    se1 = 0, cpuid = cpcmd = cpcmdl = False;
    for (se = serverList; se; se = se->next)
        if (se->type != ForeignDpy) {
            if (!se1) {
                se1 = se;
            } else {
                if (strcmp(se1->argvs, se->argvs))
                    cpcmd = True;
                if (strcmp(se1->arglvs, se->arglvs))
                    cpcmdl = True;
                if (mstrcmp(se1->owner, se->owner))
                    cpuid = True;
            }
        }
    if (se1) {
        putFqVal("X-:*-Core", "ServerCmd", se1->argvs);
        if (se1->owner)
            putFqVal("X-:*-Core", "ServerUID", se1->owner);
        putFqVal("X-:*-Core", "ServerArgsLocal", se1->arglvs);
        for (se = serverList; se; se = se->next)
            if (se->type != ForeignDpy) {
                char sec[32];
                sprintf(sec, "X-%s-Core", se->name);
                if (cpcmd)
                    putFqVal(sec, "ServerCmd", se->argvs);
                if (cpcmdl)
                    putFqVal(sec, "ServerArgsLocal", se->arglvs);
#ifdef HAVE_VTS
                if (cpvt && se->vt) {
                    char vt[8];
                    sprintf(vt, "%d", se->vt);
                    putFqVal(sec, "ServerVT", vt);
                }
#else
                if (se->console)
                    putFqVal(sec, "ServerTTY", se->console);
#endif
                if (cpuid && se->owner)
                    putFqVal(sec, "ServerUID", se->owner);
            }
    }

    sdpys = rdpys = 0;
    for (se = serverList; se; se = se->next)
        strCat(se->reserve ? &rdpys : &sdpys,
               se->class2 ? ",%s_%s" : ",%s", se->name, se->class2);

#ifdef HAVE_VTS
    /* add reserve dpys */
    if (nldpys < 4 && nldpys && !nrdpys)
        for (; nldpys < 4; nldpys++) {
            for (dn = 0; dpymask & (1 << dn); dn++);
            dpymask |= (1 << dn);
            strCat(&rdpys, ",:%d", dn);
        }
#endif

    putFqVal("General", "StaticServers", sdpys ? sdpys + 1 : "");
    putFqVal("General", "ReserveServers", rdpys ? rdpys + 1 : "");

    if (**value == '/' && inNewDir(*value) && !use_destdir)
        displace(*value);
}

#ifdef HAVE_VTS
static void
upd_servervts(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) { /* there is only the Global one */
#ifdef __linux__ /* XXX actually, sysvinit */
        getInitTab();
        ASPrintf((char **)&ce->value, "-%d", maxTTY + 1);
        ce->active = ce->written = True;
#endif
    }
}

static void
upd_consolettys(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) { /* there is only the Global one */
#ifdef __linux__ /* XXX actually, sysvinit */
        char *buf;
        int i;

        getInitTab();
        for (i = 0, buf = 0; i < 16; i++)
            if (TTYmask & (1 << i))
                strCat(&buf, ",tty%d", i + 1);
        if (buf) {
            ce->value = buf + 1;
            ce->active = ce->written = True;
        }
#endif
    }
}
#endif

static void
upd_servercmd(Entry *ce, Section *cs ATTR_UNUSED)
{
    StrList *sa;
    FILE *fp;
    char *svr;
    char buf[20000];

    if (!ce->active || oldver >= 0x0204)
        return;
    if (!(sa = splitArgs(ce->value)))
        return;
    ASPrintf(&svr, "%s -help 2>&1", sa->str);
    if (!(fp = popen(svr, "r")))
        return;
    buf[fread(buf, 1, sizeof(buf) - 1, fp)] = 0;
    pclose(fp);
    if (strstr(buf, "\n-br "))
        addStr(&sa, "-br");
    if (strstr(buf, "\n-novtswitch "))
        addStr(&sa, "-novtswitch");
    if (strstr(buf, "\n-quiet "))
        addStr(&sa, "-quiet");
    ce->value = joinArgs(sa);
    ce->written = True;
}

#ifdef XDMCP
static void
cp_keyfile(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) /* there is only the Global one */
        return;
    if (old_confs)
        linkFile(ce);
    else
        if (!copyFile(ce, 0600, 0))
            ce->active = False;
}

static void
mk_xaccess(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) /* there is only the Global one */
        writeFile(def_Xaccess, 0644, def_xaccess);
    else if (old_confs)
        linkFile(ce);
    else
        copyFile(ce, 0644, 0); /* don't handle error, it will disable Xdmcp automatically */
}

static void
mk_willing(Entry *ce, Section *cs ATTR_UNUSED)
{
    const char *fname;

    if (!ce->active) { /* there is only the Global one */
        goto dflt;
    } else {
        if (!(fname = strchr(ce->value, '/')))
            return; /* obviously in-line (or empty) */
        if (old_scripts || inNewDir(fname)) {
            doLinkFile(fname);
        } else {
          dflt:
            ce->value = KDMCONF "/Xwilling";
            ce->active = ce->written = True;
            writeFile(ce->value, 0755, def_willing);
        }
    }
}
#endif

/*
static int
edit_resources(File *file)
{
    // XXX remove any login*, chooser*, ... resources
    return False;
}
*/

static void
cp_resources(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) /* the X-*-Greeter one */
        return;
    if (old_confs)
        linkFile(ce);
    else
        if (!copyFile(ce, 0644, 0/*edit_resources*/))
            ce->active = False;
}

static int
delstr(File *fil, const char *pat)
{
    char *p, *pp, *bpp;
    const char *pap, *paap;

    *fil->eof = 0;
    for (p = fil->buf; *p; p++) {
        for (pp = p, pap = pat; ;) {
            if (!*pap) {
                *p = '\n';
                memcpy(p + 1, pp, fil->eof - pp + 1);
                fil->eof -= pp - p - 1;
                return True;
            } else if (!memcmp(pap, "*/", 2)) {
                paap = pap += 2;
                while (!isspace(*pap))
                    pap++;
                if (*pp != '/')
                    break;
                for (;;)
                    for (bpp = ++pp; *pp != '/'; pp++)
                        if (!*pp || isspace(*pp))
                            goto wbrk;
              wbrk:
                if ((pp - bpp != pap - paap) || memcmp(bpp, paap, pap - paap))
                    break;
            } else if (*pap == '\t') {
                pap++;
                while (*pp == ' ' || *pp == '\t')
                    pp++;
            } else if (*pap == '[') {
                pap++;
                for (;;) {
                    if (!*pap) {
                        fprintf(stderr, "Internal error: unterminated char set\n");
                        exit(1);
                    }
                    if (*pap == *pp) {
                        while (*++pap != ']')
                            if (!*pap) {
                                fprintf(stderr, "Internal error: unterminated char set\n");
                                exit(1);
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
    return False;
}

/* XXX
   the UseBackground voodoo will horribly fail, if multiple sections link
   to the same Xsetup file
*/

static int mod_usebg;

static int
edit_setup(File *file)
{
    int chg =
        delstr(file, "\n"
               "(\n"
               "  PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
               "  */kdmdesktop\t&\n"
               "  echo $! >$PIDFILE\n"
               "  wait $!\n"
               "  rm $PIDFILE\n"
               ")\t&\n") |
        delstr(file, "\n"
               "*/kdmdesktop\t&\n") |
        delstr(file, "\n"
               "kdmdesktop\t&\n") |
        delstr(file, "\n"
               "kdmdesktop\n");
    putVal("UseBackground", chg ? "true" : "false");
    return chg;
}

static void
mk_setup(Entry *ce, Section *cs)
{
    setSect(reSect(cs->name, "Greeter"));
    if (old_scripts || mixed_scripts) {
        if (mod_usebg && *ce->value)
            putVal("UseBackground", "false");
        linkFile(ce);
    } else {
        if (ce->active && inNewDir(ce->value)) {
            if (mod_usebg)
                copyFile(ce, 0755, edit_setup);
            else
                linkFile(ce);
        } else {
            ce->value = KDMCONF "/Xsetup";
            ce->active = ce->written = True;
            writeFile(ce->value, 0755, def_setup);
        }
    }
}

static int
edit_startup(File *file)
{
    int chg1 = False, chg2 = False;

    if (mod_usebg &&
        (delstr(file, "\n"
                "PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
                "if [[] -f $PIDFILE ] ; then\n"
                "	   kill `cat $PIDFILE`\n"
                "fi\n") ||
         delstr(file, "\n"
                "PIDFILE=/var/run/kdmdesktop-$DISPLAY.pid\n"
                "test -f $PIDFILE && kill `cat $PIDFILE`\n")))
        chg1 = True;
    if (oldver < 0x0203) {
        chg2 =
#ifdef _AIX
            delstr(file, "\n"
"# We create a pseudodevice for finger.  (host:0 becomes [kx]dm/host_0)\n");
"# Without it, finger errors out with \"Cannot stat /dev/host:0\".\n"
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
            delstr(file, "\n"
"exec sessreg -a -l $DISPLAY -x */Xservers -u " _PATH_UTMP " $USER\n") |
# endif
#endif /* _AIX */
            delstr(file, "\n"
"exec sessreg -a -l $DISPLAY"
#ifdef BSD
" -x */Xservers"
#endif
" $USER\n") |
            delstr(file, "\n"
"exec sessreg -a -l $DISPLAY -u /var/run/utmp -x */Xservers $USER\n");
        putVal("UseSessReg", chg2 ? "true" : "false");
    }
    return chg1 | chg2;
}

static void
mk_startup(Entry *ce, Section *cs)
{
    setSect(cs->name);
    if (old_scripts || mixed_scripts) {
        linkFile(ce);
    } else {
        if (ce->active && inNewDir(ce->value)) {
            if (mod_usebg || oldver < 0x0203)
                copyFile(ce, 0755, edit_startup);
            else
                linkFile(ce);
        } else {
            ce->value = KDMCONF "/Xstartup";
            ce->active = ce->written = True;
            writeFile(ce->value, 0755, def_startup);
        }
    }
}

static int
edit_reset(File *file)
{
    return
#ifdef _AIX
        delstr(file, "\n"
"if [[] -f /usr/lib/X11/xdm/sessreg ]; then\n"
"  devname=`echo $DISPLAY | /usr/bin/sed -e 's/[[]:\\.]/_/g' | /usr/bin/cut -c1-8`\n"
"  exec /usr/lib/X11/xdm/sessreg -d -l [kx]dm/$devname $USER\n"
"fi\n") |
#else
# ifdef BSD
        delstr(file, "\n"
"exec sessreg -d -l $DISPLAY -x */Xservers -u " _PATH_UTMP " $USER\n") |
# endif
#endif /* _AIX */
        delstr(file, "\n"
"exec sessreg -d -l $DISPLAY"
# ifdef BSD
" -x */Xservers"
# endif
" $USER\n") |
        delstr(file, "\n"
"exec sessreg -d -l $DISPLAY -u /var/run/utmp -x */Xservers $USER\n");
}

static void
mk_reset(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (old_scripts || mixed_scripts) {
        linkFile(ce);
    } else {
        if (ce->active && inNewDir(ce->value)) {
            if (oldver < 0x0203)
                copyFile(ce, 0755, edit_reset);
            else
                linkFile(ce);
        } else {
            ce->value = KDMCONF "/Xreset";
            ce->active = ce->written = True;
            writeFile(ce->value, 0755, def_reset);
        }
    }
}

static void
mk_session(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *def_session;
    const char *tmpf;

    if ((old_scripts || (ce->active && inNewDir(ce->value))) &&
            oldver >= 0x202) {
        linkFile(ce);
    } else {
        tmpf = locate("mktemp") ?
                   "`mktemp /tmp/xsess-env-XXXXXX`" :
                   locate("tempfile") ?
                       "`tempfile`" :
                       "$HOME/.xsession-env-$DISPLAY";
        ASPrintf(&def_session, "%s%s%s", def_session1, tmpf, def_session2);
        ce->value = KDMCONF "/Xsession";
        ce->active = ce->written = True;
        writeFile(ce->value, 0755, def_session);
    }
}

static void
upd_language(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp(ce->value, "C"))
        ce->value = (char *)"en_US";
}

static void
upd_guistyle(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!strcmp(ce->value, "Motif+"))
        ce->value = (char *)"MotifPlus";
    else if (!strcmp(ce->value, "KDE"))
        ce->value = (char *)"Default";
}

static void
upd_showusers(Entry *ce, Section *cs)
{
    if (!strcmp(ce->value, "All")) {
        ce->value = (char *)"NotHidden";
    } else if (!strcmp(ce->value, "None")) {
        if (ce->active)
            putFqVal(cs->name, "UserList", "false");
        ce->value = (char *)"Selected";
        ce->active = False;
        ce->written = True;
    }
}

static const char *defminuid, *defmaxuid;

static void
upd_minshowuid(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {
        ce->value = defminuid;
        ce->active = ce->written = True;
    }
}

static void
upd_maxshowuid(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (!ce->active) {
        ce->value = defmaxuid;
        ce->active = ce->written = True;
    }
}

static void
upd_hiddenusers(Entry *ce, Section *cs)
{
    char *nv;
    const char *msu, *pt, *et;
    struct passwd *pw;
    unsigned minuid, maxuid;
    char nbuf[128];

    if (!ce->active)
        return;

    msu = getFqVal(cs->name, "MinShowUID", "0");
    sscanf(msu, "%u", &minuid);
    msu = getFqVal(cs->name, "MaxShowUID", "65535");
    sscanf(msu, "%u", &maxuid);

    nv = 0;
    pt = ce->value;
    for (;;) {
        et = strpbrk(pt, ";,");
        if (et) {
            memcpy(nbuf, pt, et - pt);
            nbuf[et - pt] = 0;
        } else {
            strcpy(nbuf, pt);
        }
        if ((pw = getpwnam(nbuf))) {
            if (!pw->pw_uid ||
                (pw->pw_uid >= minuid && pw->pw_uid <= maxuid))
            {
                if (nv)
                    strCat(&nv, ",%s", nbuf);
                else
                    nv = mstrdup(nbuf);
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
        ASPrintf((char **)&ce->value, "%d", time(0));
        ce->active = ce->written = True;
    }
}

static void
upd_fifodir(Entry *ce, Section *cs ATTR_UNUSED)
{
    const char *dir;
    struct stat st;

    if (use_destdir)
        return;
    dir = ce->active ? ce->value : def_FifoDir;
    stat(dir, &st);
    chmod(dir, st.st_mode | 0755);
}

static gid_t greeter_gid;
static uid_t greeter_uid;

static void
upd_greeteruid(Entry *ce, Section *cs ATTR_UNUSED)
{
    struct passwd *pw;
    char *ok, *adduser;
    int uid;

    if (use_destdir || !ce->active)
        return;
    if (!(pw = getpwnam(ce->value))) {
        uid = strtol(ce->value, &ok, 10);
        if (*ok || !(pw = getpwuid(uid))) {
            if (!access("/etc/debian_version", R_OK)
                && (adduser = locate("adduser"))) {
                const char *args[] = {
                    adduser, "--system", "--group",
                    "--home", "/var", "--no-create-home",
                    ce->value, 0
                };
                if (runAndWait((char **)args)) {
                    fprintf(stderr, "Warning: Creation of missing GreeterUID"
                                    " user %s failed\n", ce->value);
                    ce->active = False;
                    return;
                }
            } else {
                fprintf(stderr, "Warning: Do not know how to create missing"
                                " GreeterUID user %s\n", ce->value);
                ce->active = False;
                return;
            }
            if (!(pw = getpwnam(ce->value))) {
                fprintf(stderr, "Warning: Newly created GreeterUID user %s"
                                " still missing!?\n", ce->value);
                ce->active = False;
                return;
            }
        }
    }
    greeter_uid = pw->pw_uid;
    greeter_gid = pw->pw_gid;
}

static void
upd_datadir(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *oldsts, *newsts;
    const char *dir;
    struct stat st;

    if (use_destdir)
        return;
    dir = ce->active ? ce->value : def_DataDir;
    ASPrintf(&newsts, "%s/kdmsts", dir);
    if (mkdirp(dir, 0755, "data", 0) && oldkde) {
        ASPrintf(&oldsts, "%s/kdm/kdmsts", oldkde);
        rename(oldsts, newsts);
    }
    if (stat(dir, &st))
        return;
    if ((st.st_uid != greeter_uid || st.st_gid != greeter_gid) &&
            chown(dir, greeter_uid, greeter_gid))
        fprintf(stderr, "Warning: Cannot assign ownership of data directory"
                        " %s: %s\n", dir, strerror(errno));
    if (stat(newsts, &st))
        return;
    if ((st.st_uid != greeter_uid || st.st_gid != greeter_gid) &&
            chown(newsts, greeter_uid, greeter_gid))
        fprintf(stderr, "Warning: Cannot assign ownership of status file"
                        " %s: %s\n", newsts, strerror(errno));
}

static void
upd_userlogfile(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *p;

    if ((p = strstr(ce->value, "%s")))
        ASPrintf((char **)&ce->value, "%.*s%%d%s", p - ce->value, ce->value, p + 2);
}

/*
 * Copy single file.
 * Do not overwrite existing target.
 * Do not complain if source cannot be read.
 */
static void
copyPlainFile(const char *from, const char *to)
{
    File file;
    int fd;

    if (readFile(&file, from)) {
        if ((fd = open(to, O_WRONLY | O_CREAT | O_EXCL, 0644)) >= 0) {
            size_t len = file.eof - file.buf;
            if (write(fd, file.buf, len) != (ssize_t)len) {
                fprintf(stderr, "Warning: cannot write %s (disk full?)\n", to);
                unlink(to);
            }
            if (close(fd) < 0) {
                fprintf(stderr, "Warning: cannot write %s (disk full?)\n", to);
                unlink(to);
            }
        } else if (errno != EEXIST) {
            fprintf(stderr, "Warning: cannot create %s\n", to);
        }
        freeBuf(&file);
    }
}

static int
copyDir(const char *from, const char *to)
{
    DIR *dir;
    struct dirent *ent;
    struct stat st;
    char bn[PATH_MAX], bo[PATH_MAX];

    if (!(dir = opendir(from)))
        return False;
    while ((ent = readdir(dir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
            continue;
        sprintf(bo, "%s/%s", from, ent->d_name);
        if (stat(bo, &st) || !S_ISREG(st.st_mode))
            continue;
        sprintf(bn, "%s/%s", to, ent->d_name);
        copyPlainFile(bo, bn);
    }
    closedir(dir);
    return True;
}

static void
upd_facedir(Entry *ce, Section *cs ATTR_UNUSED)
{
    char *oldpic, *newpic, *olddir;
    struct passwd *pw;

    if (use_destdir)
        return;
    if (oldkdepfx) { /* Do we have a previous install? */
        /* This would be the prev install's default location */
        ASPrintf(&olddir, "%s/share/apps/kdm/faces", oldkdepfx);
        if (ce->active && strcmp(olddir, ce->value))
            /* Not default location, so don't touch the setting. */
            return;
        /* Default location, so absorb it. */
        ce->active = False;
        /* Don't copy if old dir == new new. */
        if (!strcmp(olddir, def_FaceDir))
            olddir = 0;
    } else {
        olddir = 0;
    }
    if (mkdirp(def_FaceDir, 0755, "user face", True)) {
        const char *defpic = def_FaceDir "/.default.face.icon";
        const char *rootpic = def_FaceDir "/root.face.icon";
        if (oldkde && (!olddir || !copyDir(olddir, def_FaceDir)) &&
            oldver < 0x0201) /* This isn't exact - didn't inc version. */
        {
            setpwent();
            while ((pw = getpwent()))
                if (strcmp(pw->pw_name, "root")) {
                    ASPrintf(&oldpic, "%s/share/apps/kdm/pics/users/%s.png",
                             oldkdepfx, pw->pw_name);
                    ASPrintf(&newpic, def_FaceDir "/%s.face.icon", pw->pw_name);
                    rename(oldpic, newpic);
                    free(newpic);
                    free(oldpic);
                }
            endpwent();
            ASPrintf(&oldpic, "%s/share/apps/kdm/pics/users/default.png", oldkdepfx);
            if (!rename(oldpic, defpic))
                defpic = 0;
            ASPrintf(&oldpic, "%s/share/apps/kdm/pics/users/root.png", oldkdepfx);
            if (!rename(oldpic, rootpic))
                rootpic = 0;
        }
        if (defpic) {
            ASPrintf(&oldpic, "%s/default1.png", facesrc);
            copyPlainFile(oldpic, defpic);
        }
        if (rootpic) {
            ASPrintf(&oldpic, "%s/root1.png", facesrc);
            copyPlainFile(oldpic, rootpic);
        }
    }
}

static void
upd_sessionsdirs(Entry *ce, Section *cs ATTR_UNUSED)
{
    StrList *sl, *sp;
    int olen;
    char olddir[PATH_MAX];

    if (ce->written) {
        sprintf(olddir, "%s/share/apps/kdm/sessions", oldkdepfx);
        olen = strlen(oldkde);
        sl = splitList(ce->value);
        for (sp = sl; sp; sp = sp->next) {
            if (!strcmp(sp->str, olddir)) {
                sp->str = def_SessionsDirs;
            } else if (!memcmp(sp->str, oldkde, olen) &&
                       !memcmp(sp->str + olen, "/kdm/", 5)) {
                char nd[PATH_MAX];
                sprintf(nd, "%s%s", newdir, sp->str + olen + 4);
                mkdirp(nd, 0755, "sessions", False);
                copyDir(sp->str, nd);
                ASPrintf((char **)&sp->str, KDMCONF "%s", sp->str + olen + 4);
            }
        }
        ce->value = joinList(sl);
    } else {
        char nd[PATH_MAX];
        sprintf(nd, "%s/sessions", newdir);
        mkdirp(nd, 0755, "sessions", False);
    }
}

static void
upd_preloader(Entry *ce, Section *cs ATTR_UNUSED)
{
    if (ce->written) { /* implies oldkde != 0 */
        char *oldpl;
        ASPrintf(&oldpl, "%s/bin/preloadkde", oldkdepfx);
        if (!strcmp(ce->value, oldpl))
            ce->value = (char *)KDE_BINDIR "/preloadkde";
        free(oldpl);
    }
}


CONF_GEN_ENTRIES

static Sect *
findSect(const char *name)
{
    const char *p;
    int i;

    p = strrchr(name, '-');
    if (!p)
        p = name;
    for (i = 0; i < as(allSects); i++)
        if (!strcmp(allSects[i]->name, p))
            return allSects[i];
    fprintf(stderr, "Internal error: unknown section %s\n", name);
    exit(1);
}

static Ent *
findEnt(Sect *sect, const char *key)
{
    int i;

    for (i = 0; i < sect->nents; i++)
        if (!strcmp(sect->ents[i].key, key))
            return sect->ents + i;
    fprintf(stderr, "Internal error: unknown key %s in section %s\n",
            key, sect->name);
    exit(1);
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
makeDefaultConfig(void)
{
    Section *cs, **csp;
    Entry *ce, **cep;
    int sc, ec;

    for (csp = &config, sc = 0; sc < as(dAllSects); csp = &(cs->next), sc++) {
        cs = mcalloc(sizeof(*cs));
        *csp = cs;
        cs->spec = findSect(dAllSects[sc].name);
        cs->name = dAllSects[sc].name;
        cs->comment = dAllSects[sc].comment;
        for (cep = &(cs->ents), ec = 0; ec < dAllSects[sc].nents;
             cep = &(ce->next), ec++)
        {
            ce = mcalloc(sizeof(*ce));
            *cep = ce;
            ce->spec = findEnt(cs->spec, dAllSects[sc].ents[ec].key);
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
readConfig(const char *fname)
{
    char *nstr;
    char *s, *e, *st, *en, *ek, *sl;
    RSection *rootsec = 0, *cursec;
    REntry *curent;
    int nlen;
    int line, sectmoan;
    File file;

    if (!readFile(&file, fname))
        return 0;
    usedFile(fname);

    for (s = file.buf, line = 0, cursec = 0, sectmoan = 1; s < file.eof; s++) {
        line++;

        while ((s < file.eof) && isspace(*s) && (*s != '\n'))
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
            while ((e > sl) && isspace(*e))
                e--;
            if (*e != ']') {
                fprintf(stderr, "Invalid section header at %s:%d\n",
                        fname, line);
                continue;
            }
            sectmoan = False;
            nstr = sl + 1;
            nlen = e - nstr;
            for (cursec = rootsec; cursec; cursec = cursec->next)
                if (!memcmp(nstr, cursec->name, nlen) &&
                    !cursec->name[nlen])
                {
#if 0 /* not our business ... */
                    fprintf(stderr, "Warning: Multiple occurrences of section "
                            "[%.*s] in %s. Consider merging them.\n",
                            nlen, nstr, fname);
#endif
                    goto secfnd;
                }
            cursec = mmalloc(sizeof(*cursec));
            ASPrintf((char **)&cursec->name, "%.*s", nlen, nstr);
            cursec->ents = 0;
            cursec->next = rootsec;
            rootsec = cursec;
          secfnd:
            continue;
        }

        if (!cursec) {
            if (sectmoan) {
                sectmoan = False;
                fprintf(stderr, "Entry outside any section at %s:%d",
                        fname, line);
            }
            goto sktoeol;
        }

        for (; (s < file.eof) && (*s != '\n'); s++)
            if (*s == '=')
                goto haveeq;
        fprintf(stderr, "Invalid entry (missing '=') at %s:%d\n", fname, line);
        continue;

      haveeq:
        for (ek = s - 1;; ek--) {
            if (ek < sl) {
                fprintf(stderr, "Invalid entry (empty key) at %s:%d\n",
                        fname, line);
                goto sktoeol;
            }
            if (!isspace(*ek))
                break;
        }

        s++;
        while ((s < file.eof) && isspace(*s) && (*s != '\n'))
            s++;
        st = s;
        while ((s < file.eof) && (*s != '\n'))
            s++;
        for (en = s - 1; en >= st && isspace(*en); en--);

        nstr = sl;
        nlen = ek - sl + 1;
        for (curent = cursec->ents; curent; curent = curent->next)
            if (!memcmp(nstr, curent->key, nlen) && !curent->key[nlen]) {
                fprintf(stderr, "Multiple occurrences of key '%s' in section "
                        "[%s] of %s.\n", curent->key, cursec->name, fname);
                goto keyfnd;
            }
        curent = mmalloc(sizeof(*curent));
        ASPrintf((char **)&curent->key, "%.*s", nlen, nstr);
        ASPrintf((char **)&curent->value, "%.*s", en - st + 1, st);
        curent->next = cursec->ents;
        cursec->ents = curent;
      keyfnd:
        continue;
    }
    return rootsec;
}


static int
mergeKdmRcOld(const char *path)
{
    char *p;
    struct stat st;

    ASPrintf(&p, "%s/kdmrc", path);
    if (stat(p, &st)) {
        free(p);
        return False;
    }
    printf("Information: ignoring pre-existing kdmrc %s from kde < 2.2\n", p);
    free(p);
    return True;
}

typedef struct {
    const char *sect, *key, *def;
    int (*cond)(void);
} FDefs;

/*
 * The idea is to determine how exactly the pre-existing config would
 * have been interpreted, so no default configs are created where builtin
 * defaults were used so far.
 */
static void
applyDefs(FDefs *chgdef, int ndefs, const char *path)
{
    char *p;
    int i;

    for (i = 0; i < ndefs; i++)
        if (!getFqVal(chgdef[i].sect, chgdef[i].key, 0) &&
            (!chgdef[i].cond || chgdef[i].cond()))
        {
            ASPrintf(&p, chgdef[i].def, path);
            putFqVal(chgdef[i].sect, chgdef[i].key, p);
            free(p);
        }
}

static int
if_usebg (void)
{
    return isTrue(getFqVal("X-*-Greeter", "UseBackground", "true"));
}

static FDefs kdmdefs_all[] = {
#ifdef XDMCP
{ "Xdmcp", "Xaccess", "%s/kdm/Xaccess", 0 },
{ "Xdmcp", "Willing", "", 0 },
#endif
{ "X-*-Core", "Setup", "", 0 },
{ "X-*-Core", "Startup", "", 0 },
{ "X-*-Core", "Reset", "", 0 },
{ "X-*-Core", "Session", XBINDIR "/xterm -ls -T", 0 },
{ "X-*-Greeter", "BackgroundCfg", "%s/kdm/backgroundrc", if_usebg },
};

typedef struct KUpdEnt {
    const char *okey, *nsec, *nkey;
    void (*func)(const char *sect, char **value);
} KUpdEnt;

typedef struct KUpdSec {
    const char *osec;
    KUpdEnt *ents;
    int nents;
} KUpdSec;

#ifdef XDMCP
static void
P_EnableChooser(const char *sect ATTR_UNUSED, char **value)
{
    *value = (char *)(isTrue(*value) ? "DefaultLocal" : "LocalOnly");
}
#endif

static void
P_UseLilo(const char *sect ATTR_UNUSED, char **value)
{
    *value = (char *)(isTrue(*value) ? "Lilo" : "None");
}

static void
P_EchoMode(const char *sect ATTR_UNUSED, char **value)
{
    *value = (char *)(!strcmp(*value, "NoEcho") ? "false" : "true");
}

CONF_GEN_KMERGE

static int
mergeKdmRcNewer(const char *path, int obsRet)
{
    char *p;
    const char *cp, *sec, *key;
    RSection *rootsect, *cs;
    REntry *ce;
    int i, j, ma, mi;
    static char sname[64];

    ASPrintf(&p, "%s/kdm/kdmrc", path);
    if (!(rootsect = readConfig(p))) {
        free(p);
        return False;
    }
    for (cs = rootsect; cs; cs = cs->next)
        if (!strcmp(cs->name, "General"))
            for (ce = cs->ents; ce; ce = ce->next)
                if (!strcmp(ce->key, "ConfigVersion"))
                    goto gotcfgv;
    printf("Information: ignoring pre-existing kdmrc %s from kde < 3.1\n", p);
    free(p);
    return obsRet;
  gotcfgv:
    sscanf(ce->value, "%d.%d", &ma, &mi);
    oldver = (ma << 8) | mi;
    printf("Information: reading pre-existing kdmrc %s (config version %d.%d)\n",
           p, ma, mi);
    free(p);

    for (cs = rootsect; cs; cs = cs->next) {
        cp = strrchr(cs->name, '-');
        if (!cp)
            cp = cs->name;
        else if (cs->name[0] != 'X' || cs->name[1] != '-')
            goto dropsec;
        for (i = 0; i < as(kupsects); i++)
            if (!strcmp(cp, kupsects[i].osec)) {
                for (ce = cs->ents; ce; ce = ce->next) {
                    for (j = 0; j < kupsects[i].nents; j++)
                        if (!strcmp(ce->key, kupsects[i].ents[j].okey)) {
                            if (kupsects[i].ents[j].nsec == (char *) - 1) {
                                kupsects[i].ents[j].func(0, &ce->value);
                                goto gotkey;
                            }
                            if (!kupsects[i].ents[j].nsec) {
                                sec = cs->name;
                            } else {
                                sec = sname;
                                sprintf(sname, "%.*s-%s",
                                        (int)(cp - cs->name), cs->name,
                                        kupsects[i].ents[j].nsec);
                            }
                            if (!kupsects[i].ents[j].nkey)
                                key = ce->key;
                            else
                                key = kupsects[i].ents[j].nkey;
                            if (kupsects[i].ents[j].func)
                                kupsects[i].ents[j].func(sec, &ce->value);
                            putFqVal(sec, key, ce->value);
                            goto gotkey;
                        }
                    printf("Information: dropping key %s from section [%s]\n",
                           ce->key, cs->name);
                  gotkey:
                    ;
                }
                goto gotsec;
            }
      dropsec:
        printf("Information: dropping section [%s]\n", cs->name);
      gotsec:
        ;
    }

    applyDefs(kdmdefs_all, as(kdmdefs_all), path);

    return True;
}


typedef struct XResEnt {
    const char *xname;
    const char *ksec, *kname;
    void (*func)(const char *sect, char **value);
} XResEnt;

static void
handleXdmVal(const char *dpy, const char *key, char *value,
             const XResEnt *ents, int nents)
{
    const char *kname;
    int i;
    char knameb[80], sname[80];

    for (i = 0; i < nents; i++)
        if (!strcmp(key, ents[i].xname) ||
            (key[0] == toupper(ents[i].xname[0]) &&
             !strcmp(key + 1, ents[i].xname + 1)))
        {
            if (ents[i].ksec == (char *)-1) {
                ents[i].func(0, &value);
                break;
            }
            sprintf(sname, ents[i].ksec, dpy);
            if (ents[i].kname) {
                kname = ents[i].kname;
            } else {
                kname = knameb;
                sprintf(knameb, "%c%s",
                        toupper(ents[i].xname[0]), ents[i].xname + 1);
            }
            if (ents[i].func)
                ents[i].func(sname, &value);
            putFqVal(sname, kname, value);
            break;
        }
}

static void
P_list(const char *sect ATTR_UNUSED, char **value)
{
    int is, d, s;
    char *st;

    for (st = *value, is = False, d = s = 0; st[s]; s++)
        if (st[s] == ' ' || st[s] == '\t') {
            if (!is)
                st[d++] = ',';
            is = True;
        } else {
            st[d++] = st[s];
            is = False;
        }
    st[d] = 0;
}

static void
P_authDir(const char *sect ATTR_UNUSED, char **value)
{
    int l;

    l = strlen(*value);
    if (l < 4) {
        *value = 0;
        return;
    }
    if ((*value)[l-1] == '/')
        (*value)[--l] = 0;
    if (!strncmp(*value, "/tmp/", 5) ||
        !strncmp(*value, "/var/tmp/", 9))
    {
        printf("Warning: Resetting inappropriate value %s for AuthDir to default\n",
               *value);
        *value = 0;
        return;
    }
    if ((l >= 4 && !strcmp(*value + l - 4, "/tmp")) ||
        (l >= 6 && !strcmp(*value + l - 6, "/xauth")) ||
        (l >= 8 && !strcmp(*value + l - 8, "/authdir")) ||
        (l >= 10 && !strcmp(*value + l - 10, "/authfiles")))
        return;
    ASPrintf(value, "%s/authdir", *value);
}

static void
P_openDelay(const char *sect, char **value)
{
    putFqVal(sect, "ServerTimeout", *value);
}

static void
P_noPassUsers(const char *sect, char **value ATTR_UNUSED)
{
    putFqVal(sect, "NoPassEnable", "true");
}

static void
P_autoUser(const char *sect, char **value ATTR_UNUSED)
{
    putFqVal(sect, "AutoLoginEnable", "true");
}

#ifdef XDMCP
static void
P_requestPort(const char *sect, char **value)
{
    if (!strcmp(*value, "0")) {
        *value = 0;
        putFqVal(sect, "Enable", "false");
    } else {
        putFqVal(sect, "Enable", "true");
    }
}
#endif

static int kdmrcmode = 0644;

static void
P_autoPass(const char *sect ATTR_UNUSED, char **value ATTR_UNUSED)
{
    kdmrcmode = 0600;
}

CONF_GEN_XMERGE

static XrmQuark XrmQString, empty = NULLQUARK;

static Bool
dumpEntry(XrmDatabase *db ATTR_UNUSED,
          XrmBindingList bindings,
          XrmQuarkList quarks,
          XrmRepresentation *type,
          XrmValuePtr value,
          XPointer data ATTR_UNUSED)
{
    const char *dpy, *key;
    int el, hasu;
    char dpybuf[80];

    if (*type != XrmQString)
        return False;
    if (*bindings == XrmBindLoosely ||
            strcmp(XrmQuarkToString (*quarks), "DisplayManager"))
        return False;
    bindings++, quarks++;
    if (!*quarks)
        return False;
    if (*bindings != XrmBindLoosely && !quarks[1]) { /* DM.foo */
        key = XrmQuarkToString(*quarks);
        handleXdmVal(0, key, value->addr, globents, as(globents));
        return False;
    } else if (*bindings == XrmBindLoosely && !quarks[1]) { /* DM*bar */
        dpy = "*";
        key = XrmQuarkToString(*quarks);
    } else if (*bindings != XrmBindLoosely && quarks[1] &&
               *bindings != XrmBindLoosely && !quarks[2])
    { /* DM.foo.bar */
        dpy = dpybuf + 4;
        strcpy(dpybuf + 4, XrmQuarkToString(*quarks));
        for (hasu = False, el = 4; dpybuf[el]; el++)
            if (dpybuf[el] == '_')
                hasu = True;
        if (!hasu/* && isupper (dpy[0])*/) {
            dpy = dpybuf;
            memcpy(dpybuf, "*:*_", 4);
        } else {
            for (; --el >= 0;)
                if (dpybuf[el] == '_') {
                    dpybuf[el] = ':';
                    for (; --el >= 4;)
                        if (dpybuf[el] == '_')
                            dpybuf[el] = '.';
                    break;
                }
        }
        key = XrmQuarkToString(quarks[1]);
    } else {
        return False;
    }
    handleXdmVal(dpy, key, value->addr, dpyents, as(dpyents));
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
mergeXdmCfg(const char *path)
{
    char *p;
    XrmDatabase db;

    ASPrintf(&p, "%s/xdm-config", path);
    if ((db = XrmGetFileDatabase(p))) {
        printf("Information: reading xdm config file %s\n", p);
        usedFile(p);
        free(p);
        XrmEnumerateDatabase(db, &empty, &empty, XrmEnumAllLevels,
                             dumpEntry, (XPointer)0);
        applyDefs(xdmdefs, as(xdmdefs), path);
        mod_usebg = True;
        return True;
    }
    free(p);
    return False;
}

static void
fprintfLineWrap(FILE *f, const char *msg, ...)
{
    char *txt, *ftxt, *line;
    va_list ap;
    int col, lword, fspace;

    va_start(ap, msg);
    VASPrintf(&txt, msg, ap);
    va_end(ap);
    ftxt = 0;
    for (line = txt, col = 0, lword = fspace = -1; line[col];) {
        if (line[col] == '\n') {
            strCat(&ftxt, "%.*s", ++col, line);
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
                strCat(&ftxt, "%.*s\n", fspace, line);
                line += lword;
                col -= lword;
                lword = 0;
                fspace = -1;
            }
        }
        col++;
    }
    free(txt);
    if (ftxt) {
        fputs_(ftxt, f);
        free(ftxt);
    }
}


static const char * const oldkdes[] = {
    KDE_CONFDIR,
    "/opt/kde4/share/config",
    "/usr/local/kde4/share/config",

    "/opt/kde/share/config",
    "/usr/local/kde/share/config",
    "/usr/local/share/config",
    "/usr/share/config",

    "/opt/kde3/share/config",
    "/usr/local/kde3/share/config",
};

static const char * const oldxdms[] = {
    "/etc/X11/xdm",
    XLIBDIR "/xdm",
};

int main(int argc, char **argv)
{
    const char **where;
    FILE *f;
    StrList *fp;
    Section *cs;
    Entry *ce, **cep;
    int i, ap, locals, foreigns;
    int no_old_xdm = 0, no_old_kde = 0;
    struct stat st;

    for (ap = 1; ap < argc; ap++) {
        if (!strcmp(argv[ap], "--help")) {
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
"    Where to look for the config files of an xdm.\n"
"    Default is to scan /etc/X11/xdm & $XLIBDIR/xdm.\n"
"    Note that you possibly need to use --no-old-kde to make this take effect.\n"
"  --old-kde /path/to/old/kde-config-dir\n"
"    Where to look for the kdmrc of a previously installed kdm.\n"
"    Default is to scan " KDE_CONFDIR " and\n"
"    {/usr,/usr/local,{/opt,/usr/local}/{kde4,kde,kde3}}/share/config.\n"
"  --no-old\n"
"    Do not look at older xdm/kdm configurations, just create default config.\n"
"  --no-old-xdm\n"
"    Do not look at older xdm configurations.\n"
"  --no-old-kde\n"
"    Do not look at older kdm configurations.\n"
"  --old-scripts\n"
"    Directly use all scripts from the older xdm/kdm configuration.\n"
"  --no-old-scripts\n"
"    Do not use scripts from the older xdm/kdm configuration even if it lives\n"
"    in the new target directory.\n"
"  --old-confs\n"
"    Directly use all ancillary config files from the older xdm/kdm\n"
"    configuration. This is usually a bad idea.\n"
"  --no-backup\n"
"    Overwrite/delete old config files instead of backing them up.\n"
"  --no-in-notice\n"
"    Do not put the notice about --in being used into the generated README.\n"
);
            exit(0);
        }
        if (!strcmp(argv[ap], "--no-old")) {
            no_old = True;
            continue;
        }
        if (!strcmp(argv[ap], "--old-scripts")) {
            old_scripts = True;
            continue;
        }
        if (!strcmp(argv[ap], "--no-old-scripts")) {
            no_old_scripts = True;
            continue;
        }
        if (!strcmp(argv[ap], "--old-confs")) {
            old_confs = True;
            continue;
        }
        if (!strcmp(argv[ap], "--no-old-xdm")) {
            no_old_xdm = True;
            continue;
        }
        if (!strcmp(argv[ap], "--no-old-kde")) {
            no_old_kde = True;
            continue;
        }
        if (!strcmp(argv[ap], "--no-backup")) {
            no_backup = True;
            continue;
        }
        if (!strcmp(argv[ap], "--no-in-notice")) {
            no_in_notice = True;
            continue;
        }
        where = 0;
        if (!strcmp(argv[ap], "--in")) {
            where = &newdir;
        } else if (!strcmp(argv[ap], "--old-xdm")) {
            where = &oldxdm;
        } else if (!strcmp(argv[ap], "--old-kde")) {
            where = &oldkde;
        } else if (!strcmp(argv[ap], "--face-src")) {
            where = &facesrc;
        } else {
            fprintf(stderr, "Unknown command line option '%s', try --help\n", argv[ap]);
            exit(1);
        }
        if (ap + 1 == argc || argv[ap + 1][0] == '-') {
            fprintf(stderr, "Missing argument to option '%s', try --help\n", argv[ap]);
            exit(1);
        }
        *where = argv[++ap];
    }
    if (memcmp(newdir, KDMCONF, sizeof(KDMCONF)))
        use_destdir = True;

    if (!mkdirp(newdir, 0755, "target", True))
        exit(1);

    makeDefaultConfig();
    if (no_old) {
        DIR *dir;
        StrList *bfl = 0;
        if ((dir = opendir(newdir))) {
            struct dirent *ent;
            char bn[PATH_MAX];
            while ((ent = readdir(dir))) {
                int l;
                if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                    continue;
                l = sprintf(bn, "%s/%s", newdir, ent->d_name); /* cannot overflow (kernel would not allow the creation of a longer path) */
                if (!stat(bn, &st) && !S_ISREG(st.st_mode))
                    continue;
                if (no_backup || !memcmp(bn + l - 4, ".bak", 5))
                    unlink(bn);
                else
                    addStr(&bfl, bn);
            }
            closedir(dir);
            for (; bfl; bfl = bfl->next)
                displace(bfl->str);
        }
    } else {
        if (oldkde) {
            if (!mergeKdmRcNewer(oldkde, True) && !mergeKdmRcOld(oldkde)) {
                fprintf(stderr,
                        "Cannot read pre-existing kdmrc at specified location\n");
                oldkde = 0;
            }
        } else if (!no_old_kde) {
            for (i = 0; i < as(oldkdes); i++) {
                if (i && !strcmp(oldkdes[0], oldkdes[i]))
                    continue;
                if (mergeKdmRcNewer(oldkdes[i], 0)) {
                    oldkde = oldkdes[i];
                    break;
                }
                mergeKdmRcOld(oldkdes[i]); /* only prints a message */
            }
        }
        if (oldkde) {
#define SHR_CONF "/share/config"
            int olen = strlen(oldkde);
            if (olen < (int)sizeof(SHR_CONF) ||
                memcmp(oldkde + olen - sizeof(SHR_CONF) + 1,
                       SHR_CONF, sizeof(SHR_CONF)))
            {
                fprintf(stderr,
                        "Warning: --old-kde does not end with " SHR_CONF ". "
                        "Might wreak havoc.\n");
                oldkdepfx = oldkde;
            } else
                ASPrintf((char **)&oldkdepfx,
                         "%.*s", olen - sizeof(SHR_CONF) + 1, oldkde);
            oldxdm = 0;
        } else if (!no_old_xdm) {
            XrmInitialize();
            XrmQString = XrmPermStringToQuark("String");
            if (oldxdm) {
                if (!mergeXdmCfg(oldxdm)) {
                    fprintf(stderr,
                            "Cannot read xdm-config at specified location\n");
                    oldxdm = 0;
                }
            } else
                for (i = 0; i < as(oldxdms); i++)
                    if (mergeXdmCfg(oldxdms[i])) {
                        oldxdm = oldxdms[i];
                        break;
                    }
        }
    }
    /*
     * How to proceed with pre-existing scripts (which are named in the config):
     * - old_scripts set or some scripts in new target already => keep 'em
     * - no_old_scripts set or all scripts outside new target => pretend that
     *   the old config did not reference them in the first place
     */
    if (no_old_scripts)
        goto no_old_s;
    if (!old_scripts) {
        locals = foreigns = False;
        for (cs = config; cs; cs = cs->next)
            if (!strcmp(cs->spec->name, "-Core")) {
                for (ce = cs->ents; ce; ce = ce->next)
                    if (ce->active &&
                        (!strcmp(ce->spec->key, "Setup") ||
                         !strcmp(ce->spec->key, "Startup") ||
                         !strcmp(ce->spec->key, "Reset")))
                    {
                        if (inNewDir(ce->value))
                            locals = True;
                        else
                            foreigns = True;
                    }
            }
        if (foreigns) {
            if (locals) {
                fprintf(stderr,
                        "Warning: both local and foreign scripts referenced. "
                        "Will not touch any.\n");
                mixed_scripts = True;
            } else {
              no_old_s:
                for (cs = config; cs; cs = cs->next) {
                    if (!strcmp(cs->spec->name, "Xdmcp")) {
                        for (ce = cs->ents; ce; ce = ce->next)
                            if (!strcmp(ce->spec->key, "Willing"))
                                ce->active = ce->written = False;
                    } else if (!strcmp(cs->spec->name, "-Core")) {
                        for (cep = &cs->ents; (ce = *cep);) {
                            if (ce->active &&
                                (!strcmp(ce->spec->key, "Setup") ||
                                 !strcmp(ce->spec->key, "Startup") ||
                                 !strcmp(ce->spec->key, "Reset") ||
                                 !strcmp(ce->spec->key, "Session")))
                            {
                                if (!memcmp(cs->name, "X-*-", 4)) {
                                    ce->active = ce->written = False;
                                } else {
                                    *cep = ce->next;
                                    free(ce);
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
    if (!stat("/etc/debian_version", &st)) { /* debian */
        defminuid = "1000";
        defmaxuid = "29999";
    } else if (!stat("/usr/portage", &st)) { /* gentoo */
        defminuid = "1000";
        defmaxuid = "65000";
    } else if (!stat("/etc/mandrake-release", &st)) { /* mandrake - check before redhat! */
        defminuid = "500";
        defmaxuid = "65000";
    } else if (!stat("/etc/redhat-release", &st)) { /* redhat */
        defminuid = "100";
        defmaxuid = "65000";
    } else /* if (!stat("/etc/SuSE-release", &st)) */ { /* suse */
        defminuid = "500";
        defmaxuid = "65000";
    }
#else
    defminuid = "1000";
    defmaxuid = "65000";
#endif
    for (i = 0; i <= CONF_MAX_PRIO; i++)
        for (cs = config; cs; cs = cs->next)
            for (ce = cs->ents; ce; ce = ce->next)
                if (ce->spec->func && i == ce->spec->prio)
                    ce->spec->func(ce, cs);
    f = createFile("kdmrc", kdmrcmode);
    writeKdmrc(f);
    fclose_(f);

    f = createFile("README", 0644);
    fprintf_(f,
"This automatically generated configuration consists of the following files:\n");
    fprintf_(f, "- " KDMCONF "/kdmrc\n");
    for (fp = aflist; fp; fp = fp->next)
        fprintf_(f, "- %s\n", fp->str);
    if (use_destdir && !no_in_notice)
        fprintfLineWrap(f,
"All files destined for " KDMCONF " were actually saved in %s; "
"this config will not be workable until moved in place.\n", newdir);
    if (uflist || eflist || cflist || lflist) {
        fprintf_(f,
"\n"
"This config was derived from existing files. As the used algorithms are\n"
"pretty dumb, it may be broken.\n");
        if (uflist) {
            fprintf_(f,
"Information from these files was extracted:\n");
            for (fp = uflist; fp; fp = fp->next)
                fprintf_(f, "- %s\n", fp->str);
        }
        if (lflist) {
            fprintf_(f,
"These files were directly incorporated:\n");
            for (fp = lflist; fp; fp = fp->next)
                fprintf_(f, "- %s\n", fp->str);
        }
        if (cflist) {
            fprintf_(f,
"These files were copied verbatim:\n");
            for (fp = cflist; fp; fp = fp->next)
                fprintf_(f, "- %s\n", fp->str);
        }
        if (eflist) {
            fprintf_(f,
"These files were copied with modifications:\n");
            for (fp = eflist; fp; fp = fp->next)
                fprintf_(f, "- %s\n", fp->str);
        }
        if (!no_backup && !use_destdir)
            fprintf_(f,
"Old files that would have been overwritten were renamed to <oldname>.bak.\n");
    }
    fprintf_(f,
"\nTry 'genkdmconf --help' if you want to generate another configuration.\n"
"\nYou may delete this README.\n");
    fclose_(f);

    return 0;
}
