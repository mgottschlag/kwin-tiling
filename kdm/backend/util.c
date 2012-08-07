/*

Copyright 1989, 1998  The Open Group
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
 * various utility routines
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_error.h"

#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>

#if 0 /*def USG; this was hpux once upon a time */
# define NEED_UTSNAME
#endif

#ifdef NEED_UTSNAME
# include <sys/utsname.h>
#endif

void *
Calloc(size_t nmemb, size_t size)
{
    void *ret;

    if (!(ret = calloc(nmemb, size)))
        logOutOfMem();
    return ret;
}

void *
Malloc(size_t size)
{
    void *ret;

    if (!(ret = malloc(size)))
        logOutOfMem();
    return ret;
}

void *
Realloc(void *ptr, size_t size)
{
    void *ret;

    if (!(ret = realloc(ptr, size)) && size)
        logOutOfMem();
    return ret;
}

void
strCatL(char **bp, const char *str, int max)
{
    int dnl = strnlen(str, max);
    memcpy(*bp, str, dnl);
    *bp += dnl;
}

void
strCat(char **bp, const char *str)
{
    int dnl = strlen(str);
    memcpy(*bp, str, dnl);
    *bp += dnl;
}

int
strCmp(const char *s1, const char *s2)
{
    if (s1 == s2)
        return 0;
    if (!s1)
        return -1;
    if (!s2)
        return 1;
    return strcmp(s1, s2);
}

void
wipeStr(char *str)
{
    if (str) {
        bzero(str, strlen(str));
        free(str);
    }
}

#ifndef HAVE_STRNLEN
int
strnlen(const char *s, int max)
{
    unsigned l;

    for (l = 0; l < (unsigned)max && s[l]; l++);
    return l;
}
#endif

/* duplicate src; wipe & free old dst string */
int
reStrN(char **dst, const char *src, int len)
{
    char *ndst = 0;

    if (src) {
        if (len < 0)
            len = strlen(src);
        if (*dst && !memcmp(*dst, src, len) && !(*dst)[len])
            return 1;
        if (!(ndst = Malloc(len + 1))) {
            wipeStr(*dst);
            *dst = 0;
            return 0;
        }
        memcpy(ndst, src, len);
        ndst[len] = 0;
    }
    wipeStr(*dst); /* make an option, if we should become heavily used */
    *dst = ndst;
    return 2;
}

int
reStr(char **dst, const char *src)
{
    return reStrN(dst, src, -1);
}

/* duplicate src */
int
strNDup(char **dst, const char *src, int len)
{
    if (src) {
        if (len < 0)
            len = strlen(src);
        if (!(*dst = Malloc(len + 1)))
            return False;
        memcpy(*dst, src, len);
        (*dst)[len] = 0;
    } else {
        *dst = 0;
    }
    return True;
}

int
strDup(char **dst, const char *src)
{
    return strNDup(dst, src, -1);
}

char *
replaceInString(const char *str, const char *before, const char *after)
{
    int len;
    size_t beforeLen = strlen(before), afterLen = strlen(after);
    char *buf, *ret;
    const char *ptr;

    for (ptr = str, len = 0; (ptr = strstr(ptr, before)); len++)
        ptr += beforeLen;
    len = strlen(str) + (afterLen - beforeLen) * len;

    if (!(buf = ret = Malloc(len + 1)))
        return 0;

    for (ptr = str; (str = strstr(str, before)); str += beforeLen, ptr = str) {
        strCatL(&buf, ptr, str - ptr);
        strCatL(&buf, after, afterLen);
    }
    strcpy(buf, ptr);
    return ret;
}

/* append any number of strings to dst */
int
strApp(char **dst, ...)
{
    int len;
    char *bk, *pt, *dp;
    va_list va;

    len = 1;
    if (*dst)
        len += strlen(*dst);
    va_start(va, dst);
    for (;;) {
        pt = va_arg(va, char *);
        if (!pt)
            break;
        len += strlen(pt);
    }
    va_end(va);
    if (!(bk = Malloc(len))) {
        free(*dst);
        *dst = 0;
        return False;
    }
    dp = bk;
    if (*dst) {
        len = strlen(*dst);
        memcpy(dp, *dst, len);
        dp += len;
        free(*dst);
    }
    va_start(va, dst);
    for (;;) {
        pt = va_arg(va, char *);
        if (!pt)
            break;
        len = strlen(pt);
        memcpy(dp, pt, len);
        dp += len;
    }
    va_end(va);
    *dp = '\0';
    *dst = bk;
    return True;
}

char *
expandMacros(const char *str, struct expando *expandos)
{
    struct expando *ex;
    int cnt = 0, rcnt, vl;
    const char *sp = str;
    char *ret, *cp;
    char c;

  oke1:
    while ((c = *sp++)) {
        if (c == '%') {
            rcnt = 0;
            for (;;) {
                c = *sp++;
                if (!c)
                    goto murks1;
                if (c == '%')
                    break;
                for (ex = expandos; ex->key; ex++)
                    if (c == ex->key) {
                        if (ex->val)
                            cnt += strlen(ex->val) + rcnt;
                        goto oke1;
                    }
                rcnt++;
            }
        }
        cnt++;
    }
  murks1:
    if (!(cp = ret = Malloc(cnt + 1)))
        return 0;
  oke2:
    while ((c = *str++)) {
        if (c == '%') {
            rcnt = 0;
            for (;;) {
                c = *str++;
                if (!c)
                    goto murks2;
                if (c == '%')
                    break;
                for (ex = expandos; ex->key; ex++)
                    if (c == ex->key) {
                        if (ex->val) {
                            vl = strlen(ex->val);
                            memcpy(cp, str - 1 - rcnt, rcnt);
                            memcpy(cp + rcnt, ex->val, vl);
                            cp += vl + rcnt;
                        }
                        ex->uses++;
                        goto oke2;
                    }
                rcnt++;
            }
        }
        *cp++ = c;
    }
  murks2:
    *cp = 0;
    return ret;
}


char **
initStrArr(char **arr)
{
    if (!arr && (arr = Malloc(sizeof(char *))))
        arr[0] = 0;
    return arr;
}

int
arrLen(char **arr)
{
    int nu = 0;
    if (arr)
        for (; arr[nu]; nu++);
    return nu;
}

char **
extStrArr(char **arr, char ***strp)
{
    char **rarr;
    int nu;

    nu = arrLen(arr);
    if ((rarr = Realloc(arr, sizeof(char *) * (nu + 2)))) {
        rarr[nu + 1] = 0;
        *strp = rarr + nu;
        return rarr;
    }
    freeStrArr(arr);
    return 0;
}

char **
addStrArr(char **arr, const char *str, int len)
{
    char **strp;

    if ((arr = extStrArr(arr, &strp))) {
        if (strNDup(strp, str, len))
            return arr;
        freeStrArr(arr);
    }
    return 0;
}

char **
xCopyStrArr(int rn, char **arr)
{
    char **rarr;
    int nu;

    nu = arrLen(arr);
    if ((rarr = Calloc(sizeof(char *), nu + rn + 1)))
        memcpy(rarr + rn, arr, sizeof(char *) * nu);
    return rarr;
}

void
freeStrArr(char **arr)
{
    char **a;

    if (arr) {
        for (a = arr; *a; a++)
            free(*a);
        free(arr);
    }
}


char **
parseArgs(char **argv, const char *string)
{
    const char *word;
    char **strp, *str;
    int wlen;

    if (!(argv = initStrArr(argv)))
        return 0;
    while (*string) {
        if (isspace(*string)) {
            string++;
            continue;
        }
        word = string;
        wlen = 0;
        do {
            if (*string == '\\') {
                if (!*++string)
                    string--;
                wlen++;
            } else if (*string == '\'') {
                while (*++string != '\'' && *string)
                    wlen++;
            } else if (*string == '"') {
                while (*++string != '"' && *string) {
                    if (*string == '\\') {
                        if (!*++string)
                            string--;
                    }
                    wlen++;
                }
            } else {
                wlen++;
            }
        } while (*++string && !isspace(*string));
        if (!(argv = extStrArr(argv, &strp)))
            return 0;
        if (!(*strp = str = Malloc(wlen + 1))) {
            freeStrArr(argv);
            return 0;
        }
        do {
            if (*word == '\\') {
                if (!*++word)
                    word--;
                *str++ = *word;
            } else if (*word == '\'') {
                while (*++word != '\'' && *word)
                    *str++ = *word;
            } else if (*word == '"') {
                while (*++word != '"' && *word) {
                    if (*word == '\\') {
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
    }
    return argv;
}


const char *
getEnv(char **e, const char *name)
{
    if (e) {
        int l = strlen(name);
        for (; *e; e++)
            if (!memcmp(*e, name, l) && (*e)[l] == '=')
                return (*e) + l + 1;
    }
    return 0;
}

char **
setEnv(char **e, const char *name, const char *value)
{
    char **new, **old;
    char *newe;
    int envsize;
    int l;

    newe = 0;
    if (!strApp(&newe, name, "=", value, (char *)0))
        return e;
    envsize = 0;
    if (e) {
        l = strlen(name);
        for (old = e; *old; old++)
            if (!memcmp(*old, name, l) && ((*old)[l] == '=' || !(*old)[l])) {
                free(*old);
                *old = newe;
                return e;
            }
        envsize = old - e;
    }
    if (!(new = Realloc(e, (unsigned)((envsize + 2) * sizeof(char *))))) {
        free(newe);
        return e;
    }
    new[envsize] = newe;
    new[envsize + 1] = 0;
    return new;
}

char **
putEnv(const char *string, char **env)
{
    char *b, *n;

    if (!(b = strchr(string, '=')))
        return 0;
    if (!strNDup(&n, string, b - string))
        return 0;
    env = setEnv(env, n, b + 1);
    free(n);
    return env;
}

static int
getHostname(char *buf, int maxlen)
{
    int len;

#ifdef NEED_UTSNAME
    /*
     * same host name crock as in server and xinit.
     */
    struct utsname name;

    uname(&name);
    len = strlen(name.nodename);
    if (len >= maxlen)
        len = maxlen - 1;
    memcpy(buf, name.nodename, len);
    buf[len] = '\0';
#else
    buf[0] = '\0';
    (void)gethostname(buf, maxlen);
    buf[maxlen - 1] = '\0';
    len = strlen(buf);
#endif /* NEED_UTSNAME */
    return len;
}

static char localHostbuf[256];
static int gotLocalHostname;

const char *
localHostname(void)
{
    if (!gotLocalHostname) {
        getHostname(localHostbuf, sizeof(localHostbuf) - 1);
        gotLocalHostname = True;
    }
    return localHostbuf;
}

static int
atomicIO(ssize_t (*f)(int, void *, size_t), int fd, void *buf, int count)
{
    int ret, rlen;

    for (rlen = 0; rlen < count;) {
      dord:
        ret = f(fd, (char *)buf + rlen, count - rlen);
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

int
reader(int fd, void *buf, int count)
{
    return atomicIO(read, fd, buf, count);
}

int
writer(int fd, const void *buf, int count)
{
    return atomicIO((ssize_t(*)(int, void *, size_t))write,
                    fd, (void *)buf, count);
}

int
fGets(char *buf, int max, FILE *f)
{
    int len;

    if (!fgets(buf, max, f))
        return -1;
    len = strlen(buf);
    if (len && buf[len - 1] == '\n')
        buf[--len] = 0;
    return len;
}

time_t
mTime(const char *fn)
{
    struct stat st;

    if (stat(fn, &st))
        return -1;
    else
        return st.st_mtime;
}

void
randomStr(char *s)
{
    static const char letters[] =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    unsigned i, rn = secureRandom();

    for (i = 0; i < 6; i++) {
        *s++ = letters[rn % 62];
        rn /= 62;
    }
    *s = 0;
}

static int
strNChrCnt(const char *s, int slen, char c)
{
    int i, cnt;

    for (i = cnt = 0; i < slen && s[i]; i++)
        if (s[i] == c)
            cnt++;
    return cnt;
}

#define atox(c) ('0' <= c && c <= '9' ? c - '0' : \
                 'a' <= c && c <= 'f' ? c - 'a' + 10 : \
                 'A' <= c && c <= 'F' ? c - 'A' + 10 : -1)

int
hexToBinary(char *out, const char *in)
{
    int top, bottom;
    char c;
    char *oout;

    for (oout = out; (c = *in++);) {
        if ((top = atox(c)) < 0)
            return 0;
        if (!(c = *in++))
            return 0;
        if ((bottom = atox(c)) < 0)
            return 0;
        *out++ = (top << 4) | bottom;
    }
    return out - oout;
}

#undef atox

/* X -from ip6-addr does not work here, so i don't know whether this is needed.
#define IP6_MAGIC
*/

void
listSessions(int flags, struct display *d, void *ctx,
             void (*emitXSess)(struct display *, struct display *, void *),
             void (*emitTTYSess)(STRUCTUTMP *, struct display *, void *))
{
    struct display *di;
#ifdef IP6_MAGIC
    int le, dot;
#endif
#ifdef BSD_UTMP
    int fd;
    struct utmp ut[1];
#else
    STRUCTUTMP *ut;
#endif

    for (di = displays; di; di = di->next)
        if (((flags & lstRemote) || (di->displayType & d_location) == dLocal) &&
            (di->status == remoteLogin ||
             ((flags & lstPassive) ? di->status == running : di->userSess >= 0)))
            emitXSess(di, d, ctx);

    if (!(flags & lstTTY))
        return;

#ifdef BSD_UTMP
    if ((fd = open(UTMP_FILE, O_RDONLY)) < 0)
        return;
    while (reader(fd, ut, sizeof(ut[0])) == sizeof(ut[0])) {
        if (*ut->ut_user) { /* no idea how to list passive TTYs on BSD */
#else
    SETUTENT();
    while ((ut = GETUTENT())) {
        if (ut->ut_type == USER_PROCESS
# if 0 /* list passive TTYs at all? not too sensible, i think. */
            || ((flags & lstPassive) && ut->ut_type == LOGIN_PROCESS)
# endif
          )
        {
            if (ut->ut_pid <= 0 || (kill(ut->ut_pid, 0) < 0 && errno == ESRCH))
                continue; /* ignore stale utmp entries */
#endif
            if (*ut->ut_host) { /* from remote or x */
                if (!(flags & lstRemote))
                    continue;
            } else {
                /* hack around broken konsole which does not set ut_host. */
                /* this check is probably linux-specific. */
                /* alternatively we could open the device and try VT_OPENQRY. */
                if (memcmp(ut->ut_line, "tty", 3) || !isdigit(ut->ut_line[3]))
                    continue;
            }
            if (strNChrCnt(ut->ut_line, sizeof(ut->ut_line), ':'))
                continue; /* x login */
            switch (strNChrCnt(ut->ut_host, sizeof(ut->ut_host), ':')) {
            case 1: /* x terminal */
                continue;
            default:
#ifdef IP6_MAGIC
                /* unknown - IPv6 makes things complicated */
                le = strnlen(ut->ut_host, sizeof(ut->ut_host));
                /* cut off screen number */
                for (dot = le; ut->ut_host[--dot] != ':';)
                    if (ut->ut_host[dot] == '.') {
                        le = dot;
                        break;
                    }
                for (di = displays; di; di = di->next)
                    if (!memcmp(di->name, ut->ut_host, le) && !di->name[le])
                        goto cont; /* x terminal */
                break;
              cont:
                continue;
            case 0: /* no x terminal */
#endif
                break;
            }
            emitTTYSess(ut, d, ctx);
        }
    }
#ifdef BSD_UTMP
    close(fd);
#else
    ENDUTENT();
#endif
    endpwent(); /* The TTY callbacks use getpwnam(). */
}

typedef struct {
    int any;
    int uid;
} AULData;

static void
noteXSession(struct display *di, struct display *d, void *ctx)
{
    AULData *dt = (AULData *)ctx;
    (void)d;

    if (di->status == remoteLogin || di->userSess != dt->uid)
        dt->any = True;
}

static void
noteTTYSession(STRUCTUTMP *ut, struct display *d, void *ctx)
{
    AULData *dt = (AULData *)ctx;
    struct passwd *pw;
    (void)d;

    if (dt->uid < 0 || !(pw = getpwnam(ut->ut_user)) || (int)pw->pw_uid != dt->uid)
        dt->any = True;
}

int
anyUserLogins(int uid)
{
    AULData dt;

    dt.any = False;
    dt.uid = uid;
    listSessions(lstRemote | lstTTY, 0, &dt, noteXSession, noteTTYSession);
    return dt.any;
}

