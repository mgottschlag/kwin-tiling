/*

Read options from kdmrc

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

#include <config-workspace.h>

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <grp.h>
#ifdef _POSIX_PRIORITY_SCHEDULING
# include <sched.h>
#endif

#include <X11/X.h>
#ifdef FamilyInternet6
# define IPv6
#endif

#include <greet.h>

#define WANT_CONF_READ
#include <config.ci>

/*
 * Section/Entry definition structs
 */

typedef struct Ent {
    const char *name;
    int id;
    void *ptr;
    const char *def;
} Ent;

typedef struct Sect {
    const char *name;
    Ent *ents;
    int numents;
} Sect;

/*
 * Parsed ini file structs
 */

typedef struct Entry {
    struct Entry *next;
    const char *val;
    Ent *ent;
    int vallen;
    int line;
} Entry;

typedef struct Section {
    struct Section *next;
    Entry *entries;
    Sect *sect;
    const char *name, *dname, *dhost, *dnum, *dclass;
    int nlen, dlen, dhostl, dnuml, dclassl;
} Section;


/*
 * Split up display-name/-class for fast comparison
 */
typedef struct DSpec {
    const char *dhost, *dnum, *dclass;
    int dhostl, dnuml, dclassl;
} DSpec;


/*
 * Config value storage structures
 */

typedef union Value {
    struct {
        const char *ptr;
        int len; /* including 0-terminator */
    } str;
    struct {
        union Value *ptr;
        int totlen; /* summed up length of all contained strings */
    } argv;
    int num;
} Value;

typedef struct Val {
    Value val;
    int id;
} Val;

typedef struct ValArr {
    Val *ents;
    int nents, esiz, nchars, nptrs;
} ValArr;


static void *Malloc(size_t size);
static void *Realloc(void *ptr, size_t size);

#define PRINT_QUOTES
#define LOG_NAME "kdm_config"
#define LOG_DEBUG_MASK DEBUG_CONFIG
#define LOG_PANIC_EXIT 1
#define STATIC static
#include <printf.c>


static void *
Malloc(size_t size)
{
    void *ret;

    if (!(ret = malloc(size)))
        logOutOfMem();
    return ret;
}

static void *
Realloc(void *ptr, size_t size)
{
    void *ret;

    if (!(ret = realloc(ptr, size)) && size)
        logOutOfMem();
    return ret;
}


static void
mkDSpec(DSpec *spec, const char *dname, const char *dclass)
{
    spec->dhost = dname;
    for (spec->dhostl = 0; dname[spec->dhostl] != ':'; spec->dhostl++);
    spec->dnum = dname + spec->dhostl + 1;
    spec->dnuml = strlen(spec->dnum);
    spec->dclass = dclass;
    spec->dclassl = strlen(dclass);
}


static int rfd, wfd;

static int
reader(void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count;) {
      dord:
        ret = read(rfd, (char *)buf + rlen, count - rlen);
        if (ret < 0) {
            if (errno == EINTR)
                goto dord;
            if (errno == EAGAIN)
                break;
            return -1;
        }
        if (!ret)
            break;
        rlen += ret;
    }
    return rlen;
}

static void
gRead(void *buf, int count)
{
    if (reader(buf, count) != count)
        logPanic("Cannot read from core\n");
}

static void
gWrite(const void *buf, int count)
{
    if (write(wfd, buf, count) != count)
        logPanic("Cannot write to core\n");
#ifdef _POSIX_PRIORITY_SCHEDULING
    if ((debugLevel & DEBUG_HLPCON))
        sched_yield();
#endif
}

static void
gSendInt(int val)
{
    gWrite(&val, sizeof(val));
}

static void
gSendStr(const char *buf)
{
    if (buf) {
        int len = strlen(buf) + 1;
        gWrite(&len, sizeof(len));
        gWrite(buf, len);
    } else {
        gWrite(&buf, sizeof(int));
    }
}

static void
gSendNStr(const char *buf, int len)
{
    int tlen = len + 1;
    gWrite(&tlen, sizeof(tlen));
    gWrite(buf, len);
    gWrite("", 1);
}

#ifdef XDMCP
static void
gSendArr(int len, const char *data)
{
    gWrite(&len, sizeof(len));
    gWrite(data, len);
}
#endif

static int
gRecvCmd(int *val)
{
    if (reader(val, sizeof(*val)) != sizeof(*val))
        return False;
    return True;
}

static int
gRecvInt()
{
    int val;

    gRead(&val, sizeof(val));
    return val;
}

static char *
gRecvStr()
{
    int len;
    char *buf;

    len = gRecvInt();
    if (!len)
        return 0;
    if (!(buf = malloc(len)))
        logPanic("No memory for read buffer");
    gRead(buf, len);
    return buf;
}


/* #define WANT_CLOSE 1 */

typedef struct File {
    char *buf, *eof, *cur;
#if defined(HAVE_MMAP) && defined(WANT_CLOSE)
    int ismapped;
#endif
} File;

static int
readFile(File *file, const char *fn, const char *what)
{
    int fd;
    off_t flen;

    if ((fd = open(fn, O_RDONLY)) < 0) {
        logInfo("Cannot open %s file %s\n", what, fn);
        return False;
    }

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
        if (!(file->buf = Malloc(flen + 1))) {
            close(fd);
            return False;
        }
        lseek(fd, 0, SEEK_SET);
        if (read(fd, file->buf, flen) != flen) {
            free(file->buf);
            logError("Cannot read %s file %s\n", what, fn);
            close(fd);
            return False;
        }
    }
    file->eof = (file->cur = file->buf) + flen;
    close(fd);
    return True;
}

#ifdef WANT_CLOSE
static void
freeBuf(File *file)
{
# ifdef HAVE_MMAP
    if (file->ismapped)
        munmap(file->buf, file->eof - file->buf + 1);
    else
# endif
        free(file->buf);
}
#endif

CONF_READ_VARS

#define C_MTYPE_MASK  0x30000000
# define C_PATH          0x10000000     /* C_TYPE_STR is a path spec */
# define C_BOOL          0x10000000     /* C_TYPE_INT is a boolean */
# define C_ENUM          0x20000000     /* C_TYPE_INT is an enum (option) */
# define C_GRP           0x30000000     /* C_TYPE_INT is a group spec */
#define C_INTERNAL    0x40000000        /* don't expose to core */
#define C_CONFIG      0x80000000        /* process only for finding deps */

#ifdef XDMCP
static int
PrequestPort(Value *retval)
{
    if (!VxdmcpEnable.num) {
        retval->num = 0;
        return True;
    }
    return False;
}
#endif

static Value
    emptyStr = { { "", 1 } },
    nullValue = { { 0, 0 } },
    emptyArgv = { { (char *)&nullValue, 0 } };

static int
PnoPassUsers(Value *retval)
{
    if (!VnoPassEnable.num) {
        *retval = emptyArgv;
        return True;
    }
    return False;
}

static int
PautoLoginX(Value *retval)
{
    if (!VautoLoginEnable.num) {
        *retval = emptyStr;
        return True;
    }
    return False;
}

CONF_READ_ENTRIES

static const char *kdmrc = KDMCONF "/kdmrc";

static Section *rootsec;

static void
readConfig()
{
    const char *nstr, *dstr, *cstr, *dhost, *dnum, *dclass;
    char *s, *e, *st, *en, *ek, *sl, *pt;
    Section *cursec;
    Entry *curent;
    Ent *ce;
    int nlen, dlen, clen, dhostl, dnuml, dclassl;
    int i, line, sectmoan, restl;
    File file;
    static int confread;

    if (confread)
        return;
    confread = True;

    debug("reading config %s ...\n", kdmrc);
    if (!readFile(&file, kdmrc, "master configuration"))
        return;

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
            sectmoan = False;
            while ((s < file.eof) && (*s != '\n'))
                s++;
            e = s - 1;
            while ((e > sl) && isspace(*e))
                e--;
            if (*e != ']') {
                cursec = 0;
                logError("Invalid section header at %s:%d\n", kdmrc, line);
                continue;
            }
            nstr = sl + 1;
            nlen = e - nstr;
            for (cursec = rootsec; cursec; cursec = cursec->next)
                if (nlen == cursec->nlen &&
                    !memcmp(nstr, cursec->name, nlen))
                {
                    logInfo("Multiple occurrences of section [%.*s] in %s. "
                            "Consider merging them.\n", nlen, nstr, kdmrc);
                    goto secfnd;
                }
            if (nstr[0] == 'X' && nstr[1] == '-') {
                cstr = nstr + nlen;
                clen = 0;
                while (++clen, *--cstr != '-');
                if (cstr == nstr + 1)
                    goto illsec;
                dstr = nstr + 2;
                dlen = nlen - clen - 2;
                dhost = dstr;
                dhostl = 0;
                for (restl = dlen; restl; restl--) {
                    if (dhost[dhostl] == ':') {
                        dnum = dhost + dhostl + 1;
                        dnuml = 0;
                        for (restl--; restl; restl--) {
                            if (dnum[dnuml] == '_') {
                                dclass = dnum + dnuml + 1;
                                dclassl = restl;
                                goto gotall;
                            }
                            dnuml++;
                        }
                        goto gotnum;
                    }
                    dhostl++;
                }
                dnum = "*";
                dnuml = 1;
              gotnum:
                dclass = "*";
                dclassl = 1;
              gotall: ;
            } else {
                if (nstr[0] == '-')
                    goto illsec;
                dstr = 0;
                dlen = 0;
                dhost = 0;
                dhostl = 0;
                dnum = 0;
                dnuml = 0;
                dclass = 0;
                dclassl = 0;
                cstr = nstr;
                clen = nlen;
            }
            for (i = 0; i < as(allSects); i++)
                if ((int)strlen(allSects[i]->name) == clen &&
                        !memcmp(allSects[i]->name, cstr, clen))
                    goto newsec;
          illsec:
            cursec = 0;
            logError("Unrecognized section name [%.*s] at %s:%d\n",
                     nlen, nstr, kdmrc, line);
            continue;
          newsec:
            if (!(cursec = Malloc(sizeof(*cursec))))
                return;
            cursec->name = nstr;
            cursec->nlen = nlen;
            cursec->dname = dstr;
            cursec->dlen = dlen;
            cursec->dhost = dhost;
            cursec->dhostl = dhostl;
            cursec->dnum = dnum;
            cursec->dnuml = dnuml;
            cursec->dclass = dclass;
            cursec->dclassl = dclassl;
            cursec->sect = allSects[i];
            cursec->entries = 0;
            cursec->next = rootsec;
            rootsec = cursec;
            /*debug("now in section [%.*s], dpy '%.*s', core '%.*s'\n",
                   nlen, nstr, dlen, dstr, clen, cstr);*/
          secfnd:
            continue;
        }

        if (!cursec) {
            if (sectmoan) {
                sectmoan = False;
                logError("Entry outside any section at %s:%d", kdmrc, line);
            }
            goto sktoeol;
        }

        for (; (s < file.eof) && (*s != '\n'); s++)
            if (*s == '=')
                goto haveeq;
        logError("Invalid entry (missing '=') at %s:%d\n", kdmrc, line);
        continue;

      haveeq:
        for (ek = s - 1; ; ek--) {
            if (ek < sl) {
                logError("Invalid entry (empty key) at %s:%d\n", kdmrc, line);
                goto sktoeol;
            }
            if (!isspace(*ek))
                break;
        }

        s++;
        while ((s < file.eof) && isspace(*s) && (*s != '\n'))
            s++;
        for (pt = st = en = s; s < file.eof && *s != '\n'; s++) {
            if (*s == '\\') {
                s++;
                if (s >= file.eof || *s == '\n') {
                    logError("Trailing backslash at %s:%d\n", kdmrc, line);
                    break;
                }
                switch (*s) {
                case 's': *pt++ = ' '; break;
                case 't': *pt++ = '\t'; break;
                case 'n': *pt++ = '\n'; break;
                case 'r': *pt++ = '\r'; break;
                case '\\': *pt++ = '\\'; break;
                default: *pt++ = '\\'; *pt++ = *s; break;
                }
                en = pt;
            } else {
                *pt++ = *s;
                if (*s != ' ' && *s != '\t')
                    en = pt;
            }
        }

        nstr = sl;
        nlen = ek - sl + 1;
        /*debug("read entry '%.*s'='%.*s'\n", nlen, nstr, en - st, st);*/
        for (i = 0; i < cursec->sect->numents; i++) {
            ce = cursec->sect->ents + i;
            if ((int)strlen(ce->name) == nlen &&
                    !memcmp(ce->name, nstr, nlen))
                goto keyok;
        }
        logError("Unrecognized key '%.*s' in section [%.*s] at %s:%d\n",
                 nlen, nstr, cursec->nlen, cursec->name, kdmrc, line);
        continue;
      keyok:
        for (curent = cursec->entries; curent; curent = curent->next)
            if (ce == curent->ent) {
                logError("Multiple occurrences of key '%s' in section [%.*s] of %s\n",
                         ce->name, cursec->nlen, cursec->name, kdmrc);
                goto keyfnd;
            }
        if (!(curent = Malloc(sizeof(*curent))))
            return;
        curent->ent = ce;
        curent->line = line;
        curent->val = st;
        curent->vallen = en - st;
        curent->next = cursec->entries;
        cursec->entries = curent;
      keyfnd:
        continue;
    }
}

static Entry *
findGEnt(int id)
{
    Section *cursec;
    Entry *curent;

    for (cursec = rootsec; cursec; cursec = cursec->next)
        if (!cursec->dname)
            for (curent = cursec->entries; curent; curent = curent->next)
                if (curent->ent->id == id) {
                    debug("line %d: %s = %'.*s\n",
                          curent->line, curent->ent->name,
                          curent->vallen, curent->val);
                    return curent;
                }
    return 0;
}

/* Display name match scoring:
 * - class (any/exact) -> 0/1
 * - number (any/exact) -> 0/2
 * - host (any/nonempty/trail/exact) -> 0/4/8/12
 */
static Entry *
findDEnt(int id, DSpec *dspec)
{
    Section *cursec, *bestsec = 0;
    Entry *curent, *bestent;
    int score, bestscore;

    bestscore = -1, bestent = 0;
    for (cursec = rootsec; cursec; cursec = cursec->next)
        if (cursec->dname) {
            score = 0;
            if (cursec->dclassl != 1 || cursec->dclass[0] != '*') {
                if (cursec->dclassl == dspec->dclassl &&
                        !memcmp(cursec->dclass, dspec->dclass, dspec->dclassl))
                    score = 1;
                else
                    continue;
            }
            if (cursec->dnuml != 1 || cursec->dnum[0] != '*') {
                if (cursec->dnuml == dspec->dnuml &&
                        !memcmp(cursec->dnum, dspec->dnum, dspec->dnuml))
                    score += 2;
                else
                    continue;
            }
            if (cursec->dhostl != 1 || cursec->dhost[0] != '*') {
                if (cursec->dhostl == 1 && cursec->dhost[0] == '+') {
                    if (dspec->dhostl)
                        score += 4;
                    else
                        continue;
                } else if (cursec->dhost[0] == '.') {
                    if (cursec->dhostl < dspec->dhostl &&
                        !memcmp(cursec->dhost,
                                dspec->dhost + dspec->dhostl - cursec->dhostl,
                                cursec->dhostl))
                        score += 8;
                    else
                        continue;
                } else {
                    if (cursec->dhostl == dspec->dhostl &&
                            !memcmp(cursec->dhost, dspec->dhost, dspec->dhostl))
                        score += 12;
                    else
                        continue;
                }
            }
            if (score > bestscore) {
                for (curent = cursec->entries; curent; curent = curent->next)
                    if (curent->ent->id == id) {
                        bestent = curent;
                        bestsec = cursec;
                        bestscore = score;
                        break;
                    }
            }
        }
    if (bestent)
        debug("line %d: %.*s:%.*s_%.*s/%s = %'.*s\n", bestent->line,
              bestsec->dhostl, bestsec->dhost,
              bestsec->dnuml, bestsec->dnum,
              bestsec->dclassl, bestsec->dclass,
              bestent->ent->name, bestent->vallen, bestent->val);
    return bestent;
}

static const char *
convertValue(Ent *et, Value *retval, int vallen, const char *val, char **eopts)
{
    Value *ents;
    int i, b, e, tlen, nents, esiz;
    char buf[80];

    switch (et->id & C_TYPE_MASK) {
    case C_TYPE_INT:
        for (i = 0; i < vallen && i < (int)sizeof(buf) - 1; i++)
            buf[i] = tolower(val[i]);
        buf[i] = 0;
        if ((et->id & C_MTYPE_MASK) == C_BOOL) {
            if (!strcmp(buf, "true") ||
                !strcmp(buf, "on") ||
                !strcmp(buf, "yes") ||
                !strcmp(buf, "1"))
                retval->num = 1;
            else if (!strcmp(buf, "false") ||
                     !strcmp(buf, "off") ||
                     !strcmp(buf, "no") ||
                     !strcmp(buf, "0"))
                retval->num = 0;
            else
                return "boolean";
            return 0;
        } else if ((et->id & C_MTYPE_MASK) == C_ENUM) {
            for (i = 0; eopts[i]; i++)
                if (!memcmp(eopts[i], val, vallen) && !eopts[i][vallen]) {
                    retval->num = i;
                    return 0;
                }
            return "option";
        } else if ((et->id & C_MTYPE_MASK) == C_GRP) {
            struct group *ge;
            if ((ge = getgrnam(buf))) {
                retval->num = ge->gr_gid;
                return 0;
            }
        }
        if (sscanf(buf, "%i", &retval->num) != 1)
            return "integer";
        return 0;
    case C_TYPE_STR:
        retval->str.ptr = val;
        retval->str.len = vallen + 1;
        if ((et->id & C_MTYPE_MASK) == C_PATH)
            if (vallen && val[vallen-1] == '/')
                retval->str.len--;
        return 0;
    case C_TYPE_ARGV:
        if (!(ents = Malloc(sizeof(Value) * (esiz = 10))))
            return 0;
        for (nents = 0, tlen = 0, i = 0; ; i++) {
            for (; i < vallen && isspace(val[i]); i++) ;
            for (b = i; i < vallen && val[i] != ','; i++) ;
            if (b == i)
                break;
            for (e = i; e > b && isspace(val[e - 1]); e--) ;
            if (esiz < nents + 2) {
                Value *entsn = Realloc(ents, sizeof(Value) * (esiz = esiz * 2 + 1));
                if (!nents)
                    break;
                ents = entsn;
            }
            ents[nents].str.ptr = val + b;
            ents[nents].str.len = e - b;
            nents++;
            tlen += e - b + 1;
        }
        ents[nents].str.ptr = 0;
        retval->argv.ptr = ents;
        retval->argv.totlen = tlen;
        return 0;
    default:
        logError("Internal error: unknown value type in id %#x\n", et->id);
        return 0;
    }
}

static void
getValue(Ent *et, DSpec *dspec, Value *retval, char **eopts)
{
    Entry *ent;
    const char *errs;

/*    debug("Getting value %#x\n", et->id);*/
    if (dspec)
        ent = findDEnt(et->id, dspec);
    else
        ent = findGEnt(et->id);
    if (ent) {
        if (!(errs = convertValue(et, retval, ent->vallen, ent->val, eopts)))
            return;
        logError("Invalid %s value '%.*s' at %s:%d\n",
                 errs, ent->vallen, ent->val, kdmrc, ent->line);
    }
    debug("default: %s = %'s\n", et->name, et->def);
    if ((errs = convertValue(et, retval, strlen(et->def), et->def, eopts)))
        logError("Internal error: invalid default %s value '%s' for key %s\n",
                 errs, et->def, et->name);
}

static int
addValue(ValArr *va, int id, Value *val)
{
    int nu;

/*    debug("Addig value %#x\n", id);*/
    if (va->nents == va->esiz) {
        va->ents = Realloc(va->ents, sizeof(Val) * (va->esiz += 50));
        if (!va->ents)
            return False;
    }
    va->ents[va->nents].id = id;
    va->ents[va->nents].val = *val;
    va->nents++;
    switch (id & C_TYPE_MASK) {
    case C_TYPE_INT:
        break;
    case C_TYPE_STR:
        va->nchars += val->str.len;
        break;
    case C_TYPE_ARGV:
        va->nchars += val->argv.totlen;
        for (nu = 0; val->argv.ptr[nu++].str.ptr;);
        va->nptrs += nu;
        break;
    }
    return True;
}

static void
copyValues(ValArr *va, Sect *sec, DSpec *dspec, int isconfig)
{
    Value val;
    int i;

    debug("getting values for section class [%s]\n", sec->name);
    for (i = 0; i < sec->numents; i++) {
/*debug ("value %#x\n", sec->ents[i].id);*/
        if ((sec->ents[i].id & (int)C_CONFIG) != isconfig) {
        } else if (sec->ents[i].id & C_INTERNAL) {
            getValue(sec->ents + i, dspec, ((Value *)sec->ents[i].ptr), 0);
        } else {
            if (((sec->ents[i].id & C_MTYPE_MASK) == C_ENUM) ||
                !sec->ents[i].ptr ||
                !((int (*)(Value *))sec->ents[i].ptr)(&val))
            {
                getValue(sec->ents + i, dspec, &val,
                         (char **)sec->ents[i].ptr);
            }
            if (!addValue(va, sec->ents[i].id, &val))
                break;
        }
    }
    return;
}

static void
sendValues(ValArr *va)
{
    Value *cst;
    int i, nu;

    gSendInt(va->nents);
    gSendInt(va->nptrs);
    gSendInt(0/*va->nints*/);
    gSendInt(va->nchars);
    for (i = 0; i < va->nents; i++) {
        gSendInt(va->ents[i].id & ~C_PRIVATE);
        switch (va->ents[i].id & C_TYPE_MASK) {
        case C_TYPE_INT:
            gSendInt(va->ents[i].val.num);
            break;
        case C_TYPE_STR:
            gSendNStr(va->ents[i].val.str.ptr, va->ents[i].val.str.len - 1);
            break;
        case C_TYPE_ARGV:
            cst = va->ents[i].val.argv.ptr;
            for (nu = 0; cst[nu].str.ptr; nu++);
            gSendInt(nu);
            for (; cst->str.ptr; cst++)
                gSendNStr(cst->str.ptr, cst->str.len);
            break;
        }
    }
}


#ifdef XDMCP
static char *
readWord(File *file, int *len, int EOFatEOL)
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
        *len = wordp - wordBuffer;
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

#define ALIAS_CHARACTER     '%'
#define EQUAL_CHARACTER     '='
#define NEGATE_CHARACTER    '!'
#define CHOOSER_STRING      "CHOOSER"
#define BROADCAST_STRING    "BROADCAST"
#define NOBROADCAST_STRING  "NOBROADCAST"
#define LISTEN_STRING       "LISTEN"
#define WILDCARD_STRING     "*"

typedef struct _HostEntry {
    struct _HostEntry *next;
    int type;
    union _hostOrAlias {
        char *aliasPattern;
        char *hostPattern;
        struct _display {
            int connectionType;
            int hostAddrLen;
            char *hostAddress;
        } displayAddress;
    } entry;
} HostEntry;

typedef struct _ListenEntry {
    struct _ListenEntry *next;
    int iface;
    int mcasts;
    int nmcasts;
} ListenEntry;

typedef struct _AliasEntry {
    struct _AliasEntry *next;
    char *name;
    HostEntry **pHosts;
    int hosts;
    int nhosts;
    int hasBad;
} AliasEntry;

typedef struct _AclEntry {
    struct _AclEntry *next;
    HostEntry **pEntries;
    int entries;
    int nentries;
    HostEntry **pHosts;
    int hosts;
    int nhosts;
    int flags;
} AclEntry;


static int
hasGlobCharacters(char *s)
{
    for (;;)
        switch (*s++) {
        case '?':
        case '*':
            return True;
        case '\0':
            return False;
        }
}

#define PARSE_ALL       0
#define PARSE_NO_BCAST  1
#define PARSE_NO_PAT    2
#define PARSE_NO_ALIAS  4

static int
parseHost(int *nHosts, HostEntry ***hostPtr, int *nChars,
          char *hostOrAlias, int len, int parse)
{
#if defined(IPv6) && defined(AF_INET6)
    struct addrinfo *ai;
#else
    struct hostent *hostent;
#endif
    void *addr;
    int addr_type, addr_len;

    if (!(**hostPtr = Malloc(sizeof(HostEntry))))
        return False;
    if (!(parse & PARSE_NO_BCAST) && !strcmp(hostOrAlias, BROADCAST_STRING)) {
        (**hostPtr)->type = HOST_BROADCAST;
    } else if (!(parse & PARSE_NO_ALIAS) && *hostOrAlias == ALIAS_CHARACTER) {
        (**hostPtr)->type = HOST_ALIAS;
        (**hostPtr)->entry.aliasPattern = hostOrAlias + 1;
        *nChars += len;
    } else if (!(parse & PARSE_NO_PAT) && hasGlobCharacters(hostOrAlias)) {
        (**hostPtr)->type = HOST_PATTERN;
        (**hostPtr)->entry.hostPattern = hostOrAlias;
        *nChars += len + 1;
    } else {
        (**hostPtr)->type = HOST_ADDRESS;
#if defined(IPv6) && defined(AF_INET6)
        if (getaddrinfo(hostOrAlias, 0, 0, &ai))
#else
        if (!(hostent = gethostbyname(hostOrAlias)))
#endif
        {
            logWarn("XDMCP ACL: unresolved host %'s\n", hostOrAlias);
            free(**hostPtr);
            return False;
        }
#if defined(IPv6) && defined(AF_INET6)
        addr_type = ai->ai_addr->sa_family;
        if (ai->ai_family == AF_INET) {
            addr = &((struct sockaddr_in *)ai->ai_addr)->sin_addr;
            addr_len = sizeof(struct in_addr);
        } else /*if (ai->ai_addr->sa_family == AF_INET6)*/ {
            addr = &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr;
            addr_len = sizeof(struct in6_addr);
        }
#else
        addr_type = hostent->h_addrtype;
        addr = hostent->h_addr;
        addr_len = hostent->h_length;
#endif
        if (!((**hostPtr)->entry.displayAddress.hostAddress =
              Malloc(addr_len)))
        {
#if defined(IPv6) && defined(AF_INET6)
            freeaddrinfo(ai);
#endif
            free(**hostPtr);
            return False;
        }
        memcpy((**hostPtr)->entry.displayAddress.hostAddress, addr, addr_len);
        *nChars += addr_len;
        (**hostPtr)->entry.displayAddress.hostAddrLen = addr_len;
        (**hostPtr)->entry.displayAddress.connectionType = addr_type;
#if defined(IPv6) && defined(AF_INET6)
        freeaddrinfo(ai);
#endif
    }
    *hostPtr = &(**hostPtr)->next;
    (*nHosts)++;
    return True;
}

/* Returns True if string is matched by pattern.  Does case folding. */
static int
patternMatch(const char *string, const char *pattern)
{
    int p, s;

    if (!string)
        string = "";

    for (;;) {
        s = *string++;
        switch (p = *pattern++) {
        case '*':
            if (!*pattern)
                return True;
            for (string--; *string; string++)
                if (patternMatch(string, pattern))
                    return True;
            return False;
        case '?':
            if (s == '\0')
                return False;
            break;
        case '\0':
            return s == '\0';
        case '\\':
            p = *pattern++;
            /* fall through */
        default:
            if (tolower(p) != tolower(s))
                return False;
        }
    }
}

#define MAX_DEPTH     32

#define CHECK_NOT      1
#define CHECK_NO_PAT   2

static int
checkHostlist(HostEntry **hosts, int nh, AliasEntry *aliases, int na,
              int depth, int flags)
{
    HostEntry *h;
    AliasEntry *a;
    int hn, an, am;

    for (h = *hosts, hn = 0; hn < nh; hn++, h = h->next)
        if (h->type == HOST_ALIAS) {
            if (depth == MAX_DEPTH) {
                logError("XDMCP ACL: alias recursion involving %%%s\n",
                         h->entry.aliasPattern);
                return True;
            }
            for (a = aliases, an = 0, am = False; an < na; an++, a = a->next)
                if (patternMatch(a->name, h->entry.aliasPattern)) {
                    am = True;
                    if ((flags & CHECK_NOT) && a->hasBad) {
                        logError("XDMCP ACL: alias %%%s with unresolved hosts "
                                 "in denying rule\n", a->name);
                        return True;
                    }
                    if (checkHostlist(a->pHosts, a->nhosts, aliases, na,
                                      depth + 1, flags))
                        return True;
                }
            if (!am) {
                if (flags & CHECK_NOT) {
                    logError("XDMCP ACL: unresolved alias pattern %%%s "
                             "in denying rule\n", h->entry.aliasPattern);
                    return True;
                } else
                    logWarn("XDMCP ACL: unresolved alias pattern %%%s\n",
                            h->entry.aliasPattern);
            }
        } else if (h->type == HOST_PATTERN && (flags & CHECK_NO_PAT)) {
            logWarn("XDMCP ACL: wildcarded pattern %'s in host-only context\n",
                    h->entry.hostPattern);
        }
    return False;
}

static void
readAccessFile(const char *fname)
{
    HostEntry *hostList, **hostPtr = &hostList;
    AliasEntry *aliasList, **aliasPtr = &aliasList;
    AclEntry *acList, **acPtr = &acList, *acl;
    ListenEntry *listenList, **listenPtr = &listenList;
    char *displayOrAlias, *hostOrAlias;
    File file;
    int nHosts, nAliases, nAcls, nListens, nChars, error, bad;
    int i, len;

    nHosts = nAliases = nAcls = nListens = nChars = 0;
    error = False;
    if (!readFile(&file, fname, "XDMCP access control"))
        goto sendacl;
    while ((displayOrAlias = readWord(&file, &len, False))) {
        if (*displayOrAlias == ALIAS_CHARACTER) {
            if (!(*aliasPtr = Malloc(sizeof(AliasEntry)))) {
                error = True;
                break;
            }
            (*aliasPtr)->name = displayOrAlias + 1;
            nChars += len;
            (*aliasPtr)->hosts = nHosts;
            (*aliasPtr)->pHosts = hostPtr;
            (*aliasPtr)->nhosts = 0;
            (*aliasPtr)->hasBad = False;
            while ((hostOrAlias = readWord(&file, &len, True))) {
                if (parseHost(&nHosts, &hostPtr, &nChars, hostOrAlias, len,
                              PARSE_NO_BCAST))
                    (*aliasPtr)->nhosts++;
                else
                    (*aliasPtr)->hasBad = True;
            }
            aliasPtr = &(*aliasPtr)->next;
            nAliases++;
        } else if (!strcmp(displayOrAlias, LISTEN_STRING)) {
            if (!(*listenPtr = Malloc(sizeof(ListenEntry)))) {
                error = True;
                break;
            }
            (*listenPtr)->iface = nHosts;
            if (!(hostOrAlias = readWord(&file, &len, True)) ||
                !strcmp(hostOrAlias, WILDCARD_STRING) ||
                !parseHost(&nHosts, &hostPtr, &nChars, hostOrAlias, len,
                           PARSE_NO_BCAST | PARSE_NO_PAT | PARSE_NO_ALIAS))
            {
                (*listenPtr)->iface = -1;
            }
            (*listenPtr)->mcasts = nHosts;
            (*listenPtr)->nmcasts = 0;
            while ((hostOrAlias = readWord(&file, &len, True)))
                if (parseHost(&nHosts, &hostPtr, &nChars, hostOrAlias, len,
                              PARSE_NO_BCAST | PARSE_NO_PAT | PARSE_NO_ALIAS))
                    (*listenPtr)->nmcasts++;
            listenPtr = &(*listenPtr)->next;
            nListens++;
        } else {
            if (!(*acPtr = Malloc(sizeof(AclEntry)))) {
                error = True;
                break;
            }
            (*acPtr)->flags = 0;
            if (*displayOrAlias == NEGATE_CHARACTER) {
                (*acPtr)->flags |= a_notAllowed;
                displayOrAlias++;
            } else if (*displayOrAlias == EQUAL_CHARACTER) {
                displayOrAlias++;
            }
            (*acPtr)->entries = nHosts;
            (*acPtr)->pEntries = hostPtr;
            (*acPtr)->nentries = 1;
            if (!parseHost(&nHosts, &hostPtr, &nChars, displayOrAlias, len,
                           PARSE_NO_BCAST)) {
                bad = True;
                if ((*acPtr)->flags & a_notAllowed) {
                    logError("XDMCP ACL: unresolved host in denying rule\n");
                    error = True;
                }
            } else {
                bad = False;
            }
            (*acPtr)->hosts = nHosts;
            (*acPtr)->pHosts = hostPtr;
            (*acPtr)->nhosts = 0;
            while ((hostOrAlias = readWord(&file, &len, True))) {
                if (!strcmp(hostOrAlias, CHOOSER_STRING)) {
                    (*acPtr)->flags |= a_useChooser;
                } else if (!strcmp(hostOrAlias, NOBROADCAST_STRING)) {
                    (*acPtr)->flags |= a_notBroadcast;
                } else {
                    if (parseHost(&nHosts, &hostPtr, &nChars,
                                  hostOrAlias, len, PARSE_NO_PAT))
                        (*acPtr)->nhosts++;
                }
            }
            if (!bad) {
                acPtr = &(*acPtr)->next;
                nAcls++;
            }
        }
    }

    if (!nListens) {
        if (!(*listenPtr = Malloc(sizeof(ListenEntry)))) {
            error = True;
        } else {
            (*listenPtr)->iface = -1;
            (*listenPtr)->mcasts = nHosts;
            (*listenPtr)->nmcasts = 0;
#if defined(IPv6) && defined(AF_INET6) && defined(XDM_DEFAULT_MCAST_ADDR6)
            if (parseHost(&nHosts, &hostPtr, &nChars,
                          XDM_DEFAULT_MCAST_ADDR6,
                          sizeof(XDM_DEFAULT_MCAST_ADDR6) - 1,
                          PARSE_ALL))
                (*listenPtr)->nmcasts++;
#endif
            nListens++;
        }
    }

    for (acl = acList, i = 0; i < nAcls; i++, acl = acl->next)
        if (checkHostlist(acl->pEntries, acl->nentries, aliasList, nAliases,
                          0, (acl->flags & a_notAllowed) ? CHECK_NOT : 0) ||
            checkHostlist(acl->pHosts, acl->nhosts, aliasList, nAliases,
                          0, CHECK_NO_PAT))
        {
            error = True;
        }

    if (error) {
        nHosts = nAliases = nAcls = nListens = nChars = 0;
      sendacl:
        logError("No XDMCP requests will be granted\n");
    }
    gSendInt(nHosts);
    gSendInt(nListens);
    gSendInt(nAliases);
    gSendInt(nAcls);
    gSendInt(nChars);
    for (i = 0; i < nHosts; i++, hostList = hostList->next) {
        gSendInt(hostList->type);
        switch (hostList->type) {
        case HOST_ALIAS:
            gSendStr(hostList->entry.aliasPattern);
            break;
        case HOST_PATTERN:
            gSendStr(hostList->entry.hostPattern);
            break;
        case HOST_ADDRESS:
            gSendArr(hostList->entry.displayAddress.hostAddrLen,
                     hostList->entry.displayAddress.hostAddress);
            gSendInt(hostList->entry.displayAddress.connectionType);
            break;
        }
    }
    for (i = 0; i < nListens; i++, listenList = listenList->next) {
        gSendInt(listenList->iface);
        gSendInt(listenList->mcasts);
        gSendInt(listenList->nmcasts);
    }
    for (i = 0; i < nAliases; i++, aliasList = aliasList->next) {
        gSendStr(aliasList->name);
        gSendInt(aliasList->hosts);
        gSendInt(aliasList->nhosts);
    }
    for (i = 0; i < nAcls; i++, acList = acList->next) {
        gSendInt(acList->entries);
        gSendInt(acList->nentries);
        gSendInt(acList->hosts);
        gSendInt(acList->nhosts);
        gSendInt(acList->flags);
    }
}
#endif


int main(int argc ATTR_UNUSED, char **argv)
{
    DSpec dspec;
    ValArr va;
    char *ci, *disp, *dcls, *cfgfile;
    int what;

    if (!(ci = getenv("CONINFO"))) {
        fprintf(stderr, "This program is part of kdm and should not be run manually.\n");
        return 1;
    }
    if (sscanf(ci, "%d %d", &rfd, &wfd) != 2)
        return 1;

    InitLog();

    if ((debugLevel = gRecvInt()) & DEBUG_WCONFIG)
        sleep(100);

/*    debug ("parsing command line\n");*/
    if (**++argv)
        kdmrc = *argv;
/*
    while (*++argv) {
    }
*/

    for (;;) {
/*        debug ("Awaiting command ...\n");*/
        if (!gRecvCmd(&what))
            break;
        switch (what) {
        case GC_Files:
/*            debug ("GC_Files\n");*/
            readConfig();
            copyValues(0, &secGeneral, 0, C_CONFIG);
#ifdef XDMCP
            copyValues(0, &secXdmcp, 0, C_CONFIG);
            gSendInt(2);
#else
            gSendInt(1);
#endif
            gSendStr(kdmrc);
            gSendInt(-1);
#ifdef XDMCP
            gSendNStr(VXaccess.str.ptr, VXaccess.str.len - 1);
            gSendInt(0);
#endif
            for (; (what = gRecvInt()) != -1;)
                switch (what) {
                case GC_gGlobal:
                case GC_gDisplay:
                    gSendInt(0);
                    break;
#ifdef XDMCP
                case GC_gXaccess:
                    gSendInt(1);
                    break;
#endif
                default:
                    gSendInt(-1);
                    break;
                }
            break;
        case GC_GetConf:
/*            debug("GC_GetConf\n");*/
            memset(&va, 0, sizeof(va));
            what = gRecvInt();
            cfgfile = gRecvStr();
            switch (what) {
            case GC_gGlobal:
/*                debug("GC_gGlobal\n");*/
                debug("getting global config\n");
                readConfig();
                copyValues(&va, &secGeneral, 0, 0);
#ifdef XDMCP
                copyValues(&va, &secXdmcp, 0, 0);
#endif
                copyValues(&va, &secShutdown, 0, 0);
                sendValues(&va);
                break;
            case GC_gDisplay:
/*                debug("GC_gDisplay\n");*/
                disp = gRecvStr();
/*                debug(" Display %s\n", disp);*/
                dcls = gRecvStr();
/*                debug(" Class %s\n", dcls);*/
                debug("getting config for display %s, class %s\n", disp, dcls);
                mkDSpec(&dspec, disp, dcls ? dcls : "");
                readConfig();
                copyValues(&va, &sec_Core, &dspec, 0);
                copyValues(&va, &sec_Greeter, &dspec, 0);
                free(disp);
                free(dcls);
                sendValues(&va);
                break;
#ifdef XDMCP
            case GC_gXaccess:
                readAccessFile(cfgfile);
                break;
#endif
            default:
                debug("Unsupported config category %#x\n", what);
            }
            free(cfgfile);
            break;
        default:
            debug("Unknown config command %#x\n", what);
        }
    }

/*    debug("Config reader exiting ...");*/
    return EX_NORMAL;
}
