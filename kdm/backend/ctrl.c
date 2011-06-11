/*

Copyright 1988, 1998  The Open Group
Copyright 2001-2005 Oswald Buddenhagen <ossi@kde.org>

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
 * display manager
 */

#include "dm.h"
#include "dm_auth.h"
#include "dm_socket.h"
#include "dm_error.h"

#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <sys/stat.h>

static void
acceptSock(CtrlRec *cr)
{
    struct cmdsock *cs;
    int fd;

    if ((fd = accept(cr->fd, 0, 0)) < 0) {
      bust:
        logError("Error accepting command connection\n");
        return;
    }
    if (!(cs = Malloc(sizeof(*cs)))) {
        close(fd);
        goto bust;
    }
    cs->sock.fd = fd;
    cs->sock.buffer = 0;
    cs->sock.buflen = 0;
    cs->next = cr->css;
    cr->css = cs;
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
    registerCloseOnFork(fd);
    registerInput(fd);
}

static void
nukeSock(struct cmdsock *cs)
{
    unregisterInput(cs->sock.fd);
    closeNclearCloseOnFork(cs->sock.fd);
    free(cs->sock.buffer);
    free(cs);
}


#ifdef HONORS_SOCKET_PERMS
static CtrlRec ctrl = { 0, 0, -1, 0 };
#else
static CtrlRec ctrl = { 0, 0, 0, -1, 0 };

static int mkTempDir(char *dir)
{
    int i, l = strlen(dir) - 6;

    for (i = 0; i < 100; i++) {
        randomStr(dir + l);
        if (!mkdir(dir, 0700))
            return True;
        if (errno != EEXIST)
            break;
    }
    return False;
}
#endif

void
openCtrl(struct display *d)
{
    CtrlRec *cr;
    const char *dname;
    char *sockdir;
    struct sockaddr_un sa;

    if (!*fifoDir)
        return;
    if (d)
        cr = &d->ctrl, dname = displayName(d);
    else
        cr = &ctrl, dname = 0;
    if (cr->fd < 0) {
        if (mkdir(fifoDir, 0755)) {
            if (errno != EEXIST) {
                logError("mkdir %\"s failed: %m; no control sockets will be available\n",
                         fifoDir);
                return;
            }
        } else {
            chmod(fifoDir, 0755); /* override umask */
        }
        sockdir = 0;
        strApp(&sockdir, fifoDir, dname ? "/dmctl-" : "/dmctl",
               dname, (char *)0);
        if (sockdir) {
            strApp(&cr->path, sockdir, "/socket", (char *)0);
            if (cr->path) {
                if (strlen(cr->path) >= sizeof(sa.sun_path)) {
                    logError("path %\"s too long; control socket will not be available\n",
                             cr->path);
#ifdef HONORS_SOCKET_PERMS
                } else if (mkdir(sockdir, 0700) && errno != EEXIST) {
                    logError("mkdir %\"s failed: %m; control socket will not be available\n",
                             sockdir);
                } else if (unlink(cr->path) && errno != ENOENT) {
                    logError("unlink %\"s failed: %m; control socket will not be available\n",
                             cr->path);
                } else {
#else
                } else if (unlink(sockdir) && errno != ENOENT) {
                    logError("unlink %\"s failed: %m; control socket will not be available\n",
                             sockdir);
                } else if (!strApp(&cr->realdir, sockdir, "-XXXXXX", (char *)0)) {
                } else if (!mkTempDir(cr->realdir)) {
                    logError("mkdir %\"s failed: %m; control socket will not be available\n",
                             cr->realdir);
                    free(cr->realdir);
                    cr->realdir = 0;
                } else if (symlink(cr->realdir, sockdir)) {
                    logError("symlink %\"s => %\"s failed: %m; control socket will not be available\n",
                             sockdir, cr->realdir);
                    rmdir(cr->realdir);
                    free(cr->realdir);
                    cr->realdir = 0;
                } else {
                    chown(sockdir, 0, d ? 0 : fifoGroup);
                    chmod(sockdir, 0750);
#endif
                    if ((cr->fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
                        logError("Cannot create control socket: %m\n");
                    } else {
                        sa.sun_family = AF_UNIX;
                        strcpy(sa.sun_path, cr->path);
                        if (!bind(cr->fd, (struct sockaddr *)&sa, sizeof(sa))) {
                            if (!listen(cr->fd, 5)) {
#ifdef HONORS_SOCKET_PERMS
                                chmod(cr->path, 0660);
                                if (!d)
                                    chown(cr->path, -1, fifoGroup);
                                chmod(sockdir, 0755);
#else
                                chmod(cr->path, 0666);
#endif
                                registerCloseOnFork(cr->fd);
                                registerInput(cr->fd);
                                free(sockdir);
                                return;
                            }
                            unlink(cr->path);
                            logError("Cannot listen on control socket %\"s: %m\n",
                                     cr->path);
                        } else {
                            logError("Cannot bind control socket %\"s: %m\n",
                                     cr->path);
                        }
                        close(cr->fd);
                        cr->fd = -1;
                    }
#ifdef HONORS_SOCKET_PERMS
                    rmdir(sockdir);
#else
                    unlink(sockdir);
                    rmdir(cr->realdir);
                    free(cr->realdir);
                    cr->realdir = 0;
#endif
                }
                free(cr->path);
                cr->path = 0;
            }
            free(sockdir);
        }
    }
}

void
closeCtrl(struct display *d)
{
    CtrlRec *cr = d ? &d->ctrl : &ctrl;

    if (cr->fd >= 0) {
        unregisterInput(cr->fd);
        closeNclearCloseOnFork(cr->fd);
        cr->fd = -1;
        unlink(cr->path);
        *strrchr(cr->path, '/') = 0;
#ifdef HONORS_SOCKET_PERMS
        rmdir(cr->path);
#else
        rmdir(cr->realdir);
        free(cr->realdir);
        cr->realdir = 0;
        unlink(cr->path);
#endif
        free(cr->path);
        cr->path = 0;
        while (cr->css) {
            struct cmdsock *cs = cr->css;
            cr->css = cs->next;
            nukeSock(cs);
        }
    }
}

void
chownCtrl(CtrlRec *cr, int uid)
{
    if (cr->path) {
#ifdef HONORS_SOCKET_PERMS
        chown(cr->path, uid, -1);
#else
        chown(cr->realdir, uid, -1);
#endif
    }
}

void
updateCtrl(void)
{
    unsigned ffl, slc;

    ffl = 0;
    if (ctrl.path)
        for (ffl = strlen(ctrl.path), slc = 2; ;)
            if (ctrl.path[--ffl] == '/')
                if (!--slc)
                    break;
    if (ffl != strlen(fifoDir) || memcmp(fifoDir, ctrl.path, ffl) ||
        ctrl.gid != fifoGroup)
    {
        closeCtrl(0);
        openCtrl(0);
    }
}


static void
fLog(struct display *d, int fd, const char *sts, const char *msg, ...)
{
    char *fmsg, *otxt;
    int olen;
    va_list va;

    va_start(va, msg);
    VASPrintf(&fmsg, msg, va);
    va_end(va);
    if (!fmsg)
        return;
    olen = ASPrintf(&otxt, "%s\t%\\s\n", sts, fmsg);
    if (otxt) {
        writer(fd, otxt, olen);
        free(otxt);
    }
    if (d)
        debug("control socket for %s: %s - %s", d->name, sts, fmsg);
    else
        debug("global control socket: %s - %s", sts, fmsg);
    free(fmsg);
}


static char *
unQuote(const char *str)
{
    char *ret, *adp;

    if (!(ret = Malloc(strlen(str) + 1)))
        return 0;
    for (adp = ret; *str; str++, adp++)
        if (*str == '\\')
            switch (*++str) {
            case 0: str--; /* fallthrough */
            case '\\': *adp = '\\'; break;
            case 'n': *adp = '\n'; break;
            case 't': *adp = '\t'; break;
            default: *adp++ = '\\'; *adp = *str; break;
            }
        else
            *adp = *str;
    *adp = 0;
    return ret;
}

static void
strCatL(char **bp, const char *str, int max)
{
    int dnl = strnlen(str, max);
    memcpy(*bp, str, dnl);
    *bp += dnl;
}

static void
strCat(char **bp, const char *str)
{
    int dnl = strlen(str);
    memcpy(*bp, str, dnl);
    *bp += dnl;
}

static void
sdCat(char **bp, SdRec *sdr)
{
    if (sdr->how == SHUT_HALT)
        strCat(bp, "halt,");
    else
        strCat(bp, "reboot,");
    if (sdr->start == TO_INF)
        strCat(bp, "0,");
    else
        *bp += sprintf(*bp, "%d,", sdr->start);
    if (sdr->timeout == TO_INF)
        strCat(bp, "-1,");
    else
        *bp += sprintf(*bp, "%d,", sdr->timeout);
    if (sdr->force == SHUT_ASK)
        strCat(bp, "ask");
    else if (sdr->force == SHUT_FORCE)
        strCat(bp, "force");
    else if (sdr->force == SHUT_FORCEMY)
        strCat(bp, "forcemy");
    else
        strCat(bp, "cancel");
    *bp += sprintf(*bp, ",%d,%s", sdr->uid, sdr->osname ? sdr->osname : "-");
}

static void
emitXSessC(struct display *di, struct display *d, void *ctx)
{
    const char *dname;
    char *bp;
    char cbuf[1024];

    bp = cbuf;
    *bp++ = '\t';
    dname = displayName(di);
    strCatL(&bp, dname, sizeof(cbuf) / 2);
    *bp++ = ',';
#ifdef HAVE_VTS
    if (di->serverVT)
        bp += sprintf(bp, "vt%d", di->serverVT);
#endif
    *bp++ = ',';
#ifdef XDMCP
    if (di->status == remoteLogin) {
        *bp++ = ',';
        strCatL(&bp, di->remoteHost, sizeof(cbuf) / 3);
    } else
#endif
    {
        if (di->userName)
            strCatL(&bp, di->userName, sizeof(cbuf) / 5);
        *bp++ = ',';
        if (di->sessName)
            strCatL(&bp, di->sessName, sizeof(cbuf) / 5);
    }
    *bp++ = ',';
    if (di == d)
        *bp++ = '*';
    if (di->userSess >= 0 &&
        (d ? (d->userSess != di->userSess &&
              (d->allowNuke == SHUT_NONE ||
               (d->allowNuke == SHUT_ROOT && d->userSess))) :
             !fifoAllowNuke))
        *bp++ = '!';
    writer((int)(long)ctx, cbuf, bp - cbuf);
}

static void
emitTTYSessC(STRUCTUTMP *ut, struct display *d, void *ctx)
{
    struct passwd *pw;
    char *bp;
    int vt, l;
    char cbuf[sizeof(ut->ut_line) + sizeof(ut->ut_user) + sizeof(ut->ut_host) + 16];
    char user[sizeof(ut->ut_user) + 1];

#ifndef BSD_UTMP
    if (ut->ut_type != USER_PROCESS)
        l = 0;
    else
#endif
    {
        l = strnlen(ut->ut_user, sizeof(ut->ut_user));
        memcpy(user, ut->ut_user, l);
    }
    user[l] = 0;
    bp = cbuf;
    *bp++ = '\t';
    strCatL(&bp, ut->ut_line, sizeof(ut->ut_line));
    *bp++ = ',';
    if (*ut->ut_host) {
        *bp++ = '@';
        strCatL(&bp, ut->ut_host, sizeof(ut->ut_host));
    }
#ifdef HAVE_VTS
    else if ((vt = TTYtoVT(ut->ut_line)))
        bp += sprintf(bp, "vt%d", vt);
#endif
    *bp++ = ',';
    strCat(&bp, user);
    *bp++ = ',';
    /* blank: session type unknown */
    *bp++ = ',';
    /* blank: certainly not querying display */
    *bp++ = 't';
    if (*user &&
        (d ? ((d->allowNuke == SHUT_NONE ||
               (d->allowNuke == SHUT_ROOT && d->userSess)) &&
              (!(pw = getpwnam(user)) || d->userSess != (int)pw->pw_uid)) :
             !fifoAllowNuke))
        *bp++ = '!';
    writer((int)(long)ctx, cbuf, bp - cbuf);
}

static void
processCtrl(const char *string, int len, int fd, struct display *d)
{
#define Reply(t) writer(fd, t, strlen(t))
#ifdef HAVE_VTS
# define CMD_ACTIVATE "activate\t"
#else
# define CMD_ACTIVATE
#endif

    struct display *di;
    const char *word;
    char **ar, **ap, *args, *bp;
    SdRec sdr;
    char cbuf[1024];

    if (!(ar = initStrArr(0)))
        return;
    for (word = string; ; string++, len--)
        if (!len || *string == '\t') {
            if (!(ar = addStrArr(ar, word, string - word)))
                return;
            if (!len)
                break;
            word = string + 1;
        }
    if (d)
        debug("control socket for %s received %'[s\n", d->name, ar);
    else
        debug("global control socket received %'[s\n", ar);
    if (ar[0]) {
        if (!strcmp(ar[0], "caps")) {
            if (ar[1])
                goto exce;
            Reply("ok\tkdm\tlist\t");
            if (bootManager != BO_NONE)
                Reply("bootoptions\t");
            if (d) {
                if ((d->displayType & d_location) == dLocal)
                    Reply("local\t" CMD_ACTIVATE);
                if (d->allowShutdown != SHUT_NONE) {
                    if (d->allowShutdown == SHUT_ROOT && d->userSess)
                        Reply("shutdown root\t");
                    else
                        Reply("shutdown\t");
                    Reply("shutdown ask\t");
                    if (d->allowNuke != SHUT_NONE) {
                        if (d->allowNuke == SHUT_ROOT && d->userSess)
                            Reply("nuke root\t");
                        else
                            Reply("nuke\t");
                    }
                }
                if ((d->displayType & d_location) == dLocal &&
                    anyReserveDisplays())
                {
                    writer(fd, cbuf, sprintf(cbuf, "reserve %d\t",
                                             idleReserveDisplays()));
                }
                Reply("lock\tsuicide\n");
            } else {
                if (fifoAllowShutdown) {
                    Reply("shutdown\t");
                    if (fifoAllowNuke)
                        Reply("nuke\t");
                }
                if (anyReserveDisplays())
                    writer(fd, cbuf, sprintf(cbuf, "reserve %d\t",
                                             idleReserveDisplays()));
                Reply(CMD_ACTIVATE "resume\tmanage\tlogin\n");
            }
            goto bust;
        } else if (!strcmp(ar[0], "list")) {
            int flags = lstRemote | lstTTY;
            if (ar[1]) {
                if (!strcmp(ar[1], "all")) {
                    flags = lstRemote | lstPassive | lstTTY;
                } else if (!strcmp(ar[1], "alllocal")) {
                    flags = lstPassive | lstTTY;
                } else {
                    fLog(d, fd, "bad", "invalid list scope %\"s", ar[1]);
                    goto bust;
                }
                if (ar[2])
                    goto exce;
            }
            Reply("ok");
            listSessions(flags, d, (void *)(long)fd, emitXSessC, emitTTYSessC);
            Reply("\n");
            goto bust;
        } else if (!strcmp(ar[0], "reserve")) {
            if (ar[1]) { /* Formerly the timeout. Just ignore it. */
                if (ar[2])
                    goto exce;
            }
            if (d && (d->displayType & d_location) != dLocal) {
                fLog(d, fd, "perm", "display is not local");
                goto bust;
            }
            if (!startReserveDisplay()) {
                fLog(d, fd, "noent", "no reserve display available");
                goto bust;
            }
#ifdef HAVE_VTS
        } else if (!strcmp(ar[0], "activate")) {
            int vt;
            if (!ar[1])
                goto miss;
            if (ar[2])
                goto exce;
            if (d && (d->displayType & d_location) != dLocal) {
                fLog(d, fd, "perm", "display is not local");
                goto bust;
            }
            if (ar[1][0] != 'v' || ar[1][1] != 't' ||
                (vt = atoi(ar[1] + 2)) <= 0)
            {
                if (!(di = findDisplayByName(ar[1]))) {
                    fLog(d, fd, "noent", "display not found");
                    goto bust;
                }
                if ((di->displayType & d_location) != dLocal) {
                    fLog(d, fd, "inval", "target display is not local");
                    goto bust;
                }
                if (!di->serverVT) {
                    fLog(d, fd, "noent", "target display has no VT assigned");
                    goto bust;
                }
                vt = di->serverVT;
            }
            if (!activateVT(vt)) {
                fLog(d, fd, "inval", "VT switch failed");
                goto bust;
            }
#endif
        } else if (!strcmp(ar[0], "shutdown")) {
            ap = ar;
            if (!*++ap)
                goto miss;
            sdr.force = SHUT_CANCEL;
            sdr.osname = 0;
            if (!strcmp(*ap, "status")) {
                if (*++ap)
                    goto exce;
                bp = cbuf;
                *bp++ = 'o';
                *bp++ = 'k';
                if (sdRec.how) {
                    strCat(&bp, "\tglobal,");
                    sdCat(&bp, &sdRec);
                }
                if (d && d->sdRec.how) {
                    strCat(&bp, "\tlocal,");
                    sdCat(&bp, &d->sdRec);
                }
                *bp++ = '\n';
                writer(fd, cbuf, bp - cbuf);
                goto bust;
            } else if (!strcmp(*ap, "cancel")) {
                sdr.how = 0;
                sdr.start = 0;
                if (ap[1]) {
                    if (!d)
                        goto exce;
                    if (!strcmp(*++ap, "global")) {
                        sdr.start = TO_INF;
                    } else if (strcmp(*ap, "local")) {
                        fLog(d, fd, "bad", "invalid cancel scope %\"s", *ap);
                        goto bust;
                    }
                }
            } else {
                if (!strcmp(*ap, "reboot")) {
                    sdr.how = SHUT_REBOOT;
                } else if (!strcmp(*ap, "halt")) {
                    sdr.how = SHUT_HALT;
                } else {
                    fLog(d, fd, "bad", "invalid type %\"s", *ap);
                    goto bust;
                }
                sdr.uid = -1;
                if (!*++ap)
                    goto miss;
                if (**ap == '=') {
                    switch (setBootOption(*ap + 1, &sdr)) {
                    case BO_NOMAN:
                        fLog(d, fd, "notsup", "boot options unavailable");
                        goto bust;
                    case BO_NOENT:
                        fLog(d, fd, "noent", "no such boot option");
                        goto bust;
                    case BO_IO:
                        fLog(d, fd, "io", "io error");
                        goto bust;
                    }
                    if (!*++ap)
                        goto miss;
                }
                sdr.start = strtol(*ap, &bp, 10);
                if (bp != *ap && !*bp) {
                    if (**ap == '+')
                        sdr.start += now;
                    if (!*++ap)
                        goto miss;
                    sdr.timeout = strtol(*ap, &bp, 10);
                    if (bp == *ap || *bp) {
                        fLog(d, fd, "bad", "invalid timeout %\"s", ar[3]);
                        goto bust;
                    }
                    if (sdr.timeout < 0) {
                        sdr.timeout = TO_INF;
                    } else {
                        if (**ap == '+')
                            sdr.timeout += sdr.start ? sdr.start : now;
                        if (!*++ap)
                            goto miss;
                        if (!strcmp(*ap, "force")) {
                            sdr.force = SHUT_FORCE;
                        } else if (d && !strcmp(*ap, "forcemy")) {
                            sdr.force = SHUT_FORCEMY;
                        } else if (strcmp(*ap, "cancel")) {
                            fLog(d, fd, "bad", "invalid timeout action %\"s", *ap);
                            goto bust;
                        }
                    }
                } else {
                    sdr.timeout = 0;
                    if (d && !strcmp(*ap, "ask")) {
                        sdr.force = SHUT_ASK;
                    } else if (!strcmp(*ap, "forcenow")) {
                        sdr.force = SHUT_FORCE;
                    } else if (!strcmp(*ap, "schedule")) {
                        sdr.timeout = TO_INF;
                    } else if (strcmp(*ap, "trynow")) {
                        fLog(d, fd, "bad", "invalid mode %\"s", *ap);
                        goto bust;
                    }
                }
            }
            if (*++ap)
                goto exce;
            if (d) {
                sdr.uid = d->userSess >= 0 ? d->userSess : 0;
                if (d->allowShutdown == SHUT_NONE ||
                    (d->allowShutdown == SHUT_ROOT && sdr.uid &&
                     sdr.force != SHUT_ASK))
                {
                    fLog(d, fd, "perm", "shutdown forbidden");
                    goto bust;
                }
                if (!sdr.how && !sdr.start) {
                    free(d->sdRec.osname);
                    d->sdRec = sdr;
                } else {
                    if (sdRec.how && sdRec.force == SHUT_FORCE &&
                        ((d->allowNuke == SHUT_NONE && sdRec.uid != sdr.uid) ||
                         (d->allowNuke == SHUT_ROOT && sdr.uid)))
                    {
                        fLog(d, fd, "perm", "overriding forced shutdown forbidden");
                        goto bust;
                    }
                    if (sdr.force == SHUT_FORCE &&
                        (d->allowNuke == SHUT_NONE ||
                         (d->allowNuke == SHUT_ROOT && sdr.uid)))
                    {
                        fLog(d, fd, "perm", "forced shutdown forbidden");
                        goto bust;
                    }
                    if (!sdr.start) {
                        free(d->sdRec.osname);
                        d->sdRec = sdr;
                    } else {
                        if (!sdr.how) {
                            cancelShutdown();
                        } else {
                            free(sdRec.osname);
                            sdRec = sdr;
                        }
                    }
                }
            } else {
                if (!fifoAllowShutdown) {
                    fLog(d, fd, "perm", "shutdown forbidden");
                    goto bust;
                }
                if (sdRec.how && sdRec.force == SHUT_FORCE &&
                    sdRec.uid != -1 && !fifoAllowNuke)
                {
                    fLog(d, fd, "perm", "overriding forced shutdown forbidden");
                    goto bust;
                }
                if (!sdr.how) {
                    cancelShutdown();
                } else {
                    if (sdr.force != SHUT_CANCEL) {
                        if (!fifoAllowNuke) {
                            fLog(d, fd, "perm", "forced shutdown forbidden");
                            goto bust;
                        }
                    } else {
                        if (!sdr.start && !sdr.timeout && anyUserLogins(-1)) {
                            fLog(d, fd, "busy", "user sessions running");
                            goto bust;
                        }
                    }
                    sdr.uid = -1;
                    free(sdRec.osname);
                    sdRec = sdr;
                }
            }
        } else if (!strcmp(ar[0], "listbootoptions")) {
            char **opts;
            int def, cur, i, j;

            if (ar[1])
                goto exce;
            switch (getBootOptions(&opts, &def, &cur)) {
            case BO_NOMAN:
                fLog(d, fd, "notsup", "boot options unavailable");
                goto bust;
            case BO_IO:
                fLog(d, fd, "io", "io error");
                goto bust;
            }
            Reply("ok\t");
            for (i = 0; opts[i]; i++) {
                bp = cbuf;
                if (i)
                    *bp++ = ' ';
                for (j = 0; opts[i][j]; j++)
                    if (opts[i][j] == ' ') {
                        *bp++ = '\\';
                        *bp++ = 's';
                    } else {
                        *bp++ = opts[i][j];
                    }
                writer(fd, cbuf, bp - cbuf);
            }
            freeStrArr(opts);
            writer(fd, cbuf, sprintf(cbuf, "\t%d\t%d\n", def, cur));
            goto bust;
        } else if (d) {
            if (!strcmp(ar[0], "lock")) {
                if (ar[1])
                    goto exce;
                d->hstent->lock = True;
            } else if (!strcmp(ar[0], "unlock")) {
                if (ar[1])
                    goto exce;
                d->hstent->lock = False;
            } else if (!strcmp(ar[0], "suicide")) {
                if (ar[1])
                    goto exce;
                if (d->status == running && d->pid != -1) {
                    terminateProcess(d->pid, SIGTERM);
                    d->status = raiser;
                }
            } else {
                fLog(d, fd, "nosys", "unknown command");
                goto bust;
            }
        } else {
            if (!strcmp(ar[0], "login")) {
                int nuke;
                if (arrLen(ar) < 5) {
                  miss:
                    fLog(d, fd, "bad", "missing argument(s)");
                    goto bust;
                }
                if (!(di = findDisplayByName(ar[1]))) {
                    fLog(d, fd, "noent", "display %s not found", ar[1]);
                    goto bust;
                }
                if (ar[5]) {
                    if (!(args = unQuote(ar[5]))) {
                      oom:
                        fLog(d, fd, "nomem", "out of memory");
                        goto bust;
                    }
                    if (ar[6]) {
                        free(args);
                      exce:
                        fLog(d, fd, "bad", "excess argument(s)");
                        goto bust;
                    }
                    setNLogin(di, ar[3], ar[4], args, 2);
                    free(args);
                } else {
                    setNLogin(di, ar[3], ar[4], 0, 2);
                }
                nuke = !strcmp(ar[2], "now");
                switch (di->status) {
                case running:
                    if (di->pid != -1 && (di->userSess < 0 || nuke)) {
                        terminateProcess(di->pid, SIGTERM);
                        di->status = raiser;
                    }
                    break;
                case remoteLogin:
                    if (di->serverPid != -1 && nuke)
                        terminateProcess(di->serverPid, SIGTERM);
                    break;
                case reserve:
                    di->status = notRunning;
                    break;
                case textMode:
#ifndef HAVE_VTS
                    switchToX(di);
#endif
                    break;
                default:
                    break;
                }
            } else if (!strcmp(ar[0], "resume")) {
                if (ar[1])
                    goto exce;
                wakeDisplays();
            } else if (!strcmp(ar[0], "manage")) {
                if (!ar[1])
                    goto miss;
                if (*ar[1] == ':') {
                    fLog(d, fd, "bad", "display needs host (try localhost?)");
                    goto bust;
                }
                if (findDisplayByName(ar[1])) {
                    fLog(d, fd, "exists", "display is already being managed");
                    goto bust;
                }
                if (!(di = newDisplay(ar[1])))
                    goto oom;
                di->displayType = dForeign | dPermanent | dFromCommand;
                if (ar[2]) {
                    if (*ar[2] && !strDup(&di->class2, ar[2])) {
                        removeDisplay(di);
                        goto oom;
                    }
                    if (ar[3]) {
                        if (!ar[4])
                            goto miss;
                        if (ar[5]) {
                            removeDisplay(di);
                            goto exce;
                        }
                        switch (setDynamicDisplayAuthorization(di, ar[3], ar[4])) {
                        case SetAuthOOM:
                            removeDisplay(di);
                            goto oom;
                        case SetAuthBad:
                            removeDisplay(di);
                            fLog(d, fd, "bad", "invalid authorization data");
                            goto bust;
                        case SetAuthOk:
                            break;
                        }
                    }
                }
            } else if (!strcmp(ar[0], "unmanage")) {
                if (!ar[1])
                    goto miss;
                if (ar[2])
                    goto exce;
                if (!(di = findDisplayByName(ar[1]))) {
                    fLog(d, fd, "noent", "display %s not found", ar[1]);
                    goto bust;
                }
                if ((di->displayType & d_origin) != dFromCommand) {
                    fLog(d, fd, "perm", "not an on-demand display");
                    goto bust;
                }
                stopDisplay(di);
            } else {
                fLog(d, fd, "nosys", "unknown command");
                goto bust;
            }
        }
        Reply("ok\n");
    }
  bust:
    freeStrArr(ar);
}

static int
handleChan(struct display *d, struct bsock *cs, fd_set *reads)
{
    char *bufp, *nbuf, *obuf, *eol;
    int len, bl, llen;
    char buf[1024];

    bl = cs->buflen;
    obuf = cs->buffer;
    if (bl <= 0 && FD_ISSET(cs->fd, reads)) {
        FD_CLR(cs->fd, reads);
        bl = -bl;
        memcpy(buf, obuf, bl);
        if ((len = reader(cs->fd, buf + bl, sizeof(buf) - bl)) <= 0)
            return -1;
        bl += len;
        bufp = buf;
    } else {
        len = 0;
        bufp = obuf;
    }
    if (bl > 0) {
        if ((eol = memchr(bufp, '\n', bl))) {
            llen = eol - bufp + 1;
            bl -= llen;
            if (bl) {
                if (!(nbuf = Malloc(bl)))
                    return -1;
                memcpy(nbuf, bufp + llen, bl);
            } else {
                nbuf = 0;
            }
            cs->buffer = nbuf;
            cs->buflen = bl;
            processCtrl(bufp, llen - 1, cs->fd, d);
            free(obuf);
            return 1;
        } else if (!len) {
            cs->buflen = -bl;
        }
    }
    return 0;
}

int
handleCtrl(fd_set *reads, struct display *d)
{
    CtrlRec *cr = d ? &d->ctrl : &ctrl;
    struct cmdsock *cs, **csp;

    if (cr->fd >= 0 && FD_ISSET(cr->fd, reads)) {
        acceptSock(cr);
    } else {
        for (csp = &cr->css; (cs = *csp);) {
            switch (handleChan(d, &cs->sock, reads)) {
            case -1:
                *csp = cs->next;
                nukeSock(cs);
                continue;
            case 1:
                return True;
            default:
                break;
            }
            csp = &cs->next;
        }
    }
    return False;
}
