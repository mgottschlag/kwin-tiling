/*
   Copyright (C) 2004 Oswald Buddenhagen <ossi@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the Lesser GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the Lesser GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kdisplaymanager.h"

#ifdef Q_WS_X11

#include <kapplication.h>
#include <klocale.h>
#include <kuser.h>

#include <QtDBus/QtDBus>
#include <QRegExp>

#include <X11/Xauth.h>
#include <X11/Xlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

class CKManager : public QDBusInterface
{
public:
    CKManager() :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
                QLatin1String("/org/freedesktop/ConsoleKit/Manager"),
                QLatin1String("org.freedesktop.ConsoleKit.Manager"),
                QDBusConnection::systemBus()) {}
};

class CKSeat : public QDBusInterface
{
public:
    CKSeat(const QDBusObjectPath &path) :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
                path.path(),
                QLatin1String("org.freedesktop.ConsoleKit.Seat"),
                QDBusConnection::systemBus()) {}
};

class CKSession : public QDBusInterface
{
public:
    CKSession(const QDBusObjectPath &path) :
        QDBusInterface(
                QLatin1String("org.freedesktop.ConsoleKit"),
            path.path(),
                QLatin1String("org.freedesktop.ConsoleKit.Session"),
                QDBusConnection::systemBus()) {}
};

class GDMFactory : public QDBusInterface
{
public:
    GDMFactory() :
        QDBusInterface(
                QLatin1String("org.gnome.DisplayManager"),
                QLatin1String("/org/gnome/DisplayManager/LocalDisplayFactory"),
                QLatin1String("org.gnome.DisplayManager.LocalDisplayFactory"),
                QDBusConnection::systemBus()) {}
};

class LightDMDBus : public QDBusInterface
{
public:
    LightDMDBus() :
        QDBusInterface(
                QLatin1String("org.freedesktop.DisplayManager"),
                qgetenv("XDG_SEAT_PATH"),
                QLatin1String("org.freedesktop.DisplayManager.Seat"),
                QDBusConnection::systemBus()) {}
};

static enum { Dunno, NoDM, NewKDM, OldKDM, NewGDM, OldGDM, LightDM } DMType = Dunno;
static const char *ctl, *dpy;

class KDisplayManager::Private
{
public:
    Private() : fd(-1) {}
    ~Private() {
        if (fd >= 0)
            close(fd);
    }

    int fd;
};

KDisplayManager::KDisplayManager() : d(new Private)
{
    const char *ptr;
    struct sockaddr_un sa;

    if (DMType == Dunno) {
        if (!(dpy = ::getenv("DISPLAY")))
            DMType = NoDM;
        else if ((ctl = ::getenv("DM_CONTROL")))
            DMType = NewKDM;
        else if ((ctl = ::getenv("XDM_MANAGED")) && ctl[0] == '/')
            DMType = OldKDM;
        else if (LightDMDBus().isValid())
            DMType = LightDM;
        else if (::getenv("GDMSESSION"))
            DMType = GDMFactory().isValid() ? NewGDM : OldGDM;
        else
            DMType = NoDM;
    }
    switch (DMType) {
    default:
        return;
    case NewKDM:
    case OldGDM:
        if ((d->fd = ::socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
            return;
        sa.sun_family = AF_UNIX;
        if (DMType == OldGDM) {
            strcpy(sa.sun_path, "/var/run/gdm_socket");
            if (::connect(d->fd, (struct sockaddr *)&sa, sizeof(sa))) {
                strcpy(sa.sun_path, "/tmp/.gdm_socket");
                if (::connect(d->fd, (struct sockaddr *)&sa, sizeof(sa))) {
                    ::close(d->fd);
                    d->fd = -1;
                    break;
                }
            }
            GDMAuthenticate();
        } else {
            if ((ptr = strchr(dpy, ':')))
                ptr = strchr(ptr, '.');
            snprintf(sa.sun_path, sizeof(sa.sun_path),
                     "%s/dmctl-%.*s/socket",
                     ctl, ptr ? int(ptr - dpy) : 512, dpy);
            if (::connect(d->fd, (struct sockaddr *)&sa, sizeof(sa))) {
                ::close(d->fd);
                d->fd = -1;
            }
        }
        break;
    case OldKDM:
        {
            QString tf(ctl);
            tf.truncate(tf.indexOf(','));
            d->fd = ::open(tf.toLatin1(), O_WRONLY);
        }
        break;
    }
}

KDisplayManager::~KDisplayManager()
{
    delete d;
}

bool
KDisplayManager::exec(const char *cmd)
{
    QByteArray buf;

    return exec(cmd, buf);
}

/**
 * Execute a KDM/GDM remote control command.
 * @param cmd the command to execute. FIXME: undocumented yet.
 * @param buf the result buffer.
 * @return result:
 *  @li If true, the command was successfully executed.
 *   @p ret might contain addional results.
 *  @li If false and @p ret is empty, a communication error occurred
 *   (most probably KDM is not running).
 *  @li If false and @p ret is non-empty, it contains the error message
 *   from KDM.
 */
bool
KDisplayManager::exec(const char *cmd, QByteArray &buf)
{
    bool ret = false;
    int tl;
    int len = 0;

    if (d->fd < 0)
        goto busted;

    tl = strlen(cmd);
    if (::write(d->fd, cmd, tl) != tl) {
      bust:
        ::close(d->fd);
        d->fd = -1;
      busted:
        buf.resize(0);
        return false;
    }
    if (DMType == OldKDM) {
        buf.resize(0);
        return true;
    }
    for (;;) {
        if (buf.size() < 128)
            buf.resize(128);
        else if (buf.size() < len * 2)
            buf.resize(len * 2);
        if ((tl = ::read(d->fd, buf.data() + len, buf.size() - len)) <= 0) {
            if (tl < 0 && errno == EINTR)
                continue;
            goto bust;
        }
        len += tl;
        if (buf[len - 1] == '\n') {
            buf[len - 1] = 0;
            if (len > 2 && (buf[0] == 'o' || buf[0] == 'O') &&
                (buf[1] == 'k' || buf[1] == 'K') && buf[2] <= ' ')
                ret = true;
            break;
        }
    }
    return ret;
}

static bool getCurrentSeat(QDBusObjectPath *currentSession, QDBusObjectPath *currentSeat)
{
    CKManager man;
    QDBusReply<QDBusObjectPath> r = man.call(QLatin1String("GetCurrentSession"));
    if (r.isValid()) {
        CKSession sess(r.value());
        if (sess.isValid()) {
            QDBusReply<QDBusObjectPath> r2 = sess.call(QLatin1String("GetSeatId"));
            if (r2.isValid()) {
                if (currentSession)
                    *currentSession = r.value();
                *currentSeat = r2.value();
                return true;
            }
        }
    }
    return false;
}

static QList<QDBusObjectPath> getSessionsForSeat(const QDBusObjectPath &path)
{
    CKSeat seat(path);
    if (seat.isValid()) {
        QDBusReply<QList<QDBusObjectPath> > r = seat.call(QLatin1String("GetSessions"));
        if (r.isValid()) {
            // This will contain only local sessions:
            // - this is only ever called when isSwitchable() is true => local seat
            // - remote logins into the machine are assigned to other seats
            return r.value();
        }
    }
    return QList<QDBusObjectPath>();
}

static void getSessionLocation(CKSession &lsess, SessEnt &se)
{
    QString tty;
    QDBusReply<QString> r = lsess.call(QLatin1String("GetX11Display"));
    if (r.isValid() && !r.value().isEmpty()) {
        QDBusReply<QString> r2 = lsess.call(QLatin1String("GetX11DisplayDevice"));
        tty = r2.value();
        se.display = r.value();
        se.tty = false;
    } else {
        QDBusReply<QString> r2 = lsess.call(QLatin1String("GetDisplayDevice"));
        tty = r2.value();
        se.display = tty;
        se.tty = true;
    }
    se.vt = tty.mid(strlen("/dev/tty")).toInt();
}

#ifndef KDM_NO_SHUTDOWN
bool
KDisplayManager::canShutdown()
{
    if (DMType == NewGDM || DMType == NoDM || DMType == LightDM) {
        QDBusReply<bool> canStop = CKManager().call(QLatin1String("CanStop"));
        return (canStop.isValid() && canStop.value());
    }

    if (DMType == OldKDM)
        return strstr(ctl, ",maysd") != 0;

    QByteArray re;

    if (DMType == OldGDM)
        return exec("QUERY_LOGOUT_ACTION\n", re) && re.indexOf("HALT") >= 0;

    return exec("caps\n", re) && re.indexOf("\tshutdown") >= 0;
}

void
KDisplayManager::shutdown(KWorkSpace::ShutdownType shutdownType,
                          KWorkSpace::ShutdownMode shutdownMode, /* NOT Default */
                          const QString &bootOption)
{
    if (shutdownType == KWorkSpace::ShutdownTypeNone || shutdownType == KWorkSpace::ShutdownTypeLogout)
        return;

    bool cap_ask;
    if (DMType == NewKDM) {
        QByteArray re;
        cap_ask = exec("caps\n", re) && re.indexOf("\tshutdown ask") >= 0;
    } else {
        if (!bootOption.isEmpty())
            return;

        if (DMType == NewGDM || DMType == NoDM || DMType == LightDM) {
            // FIXME: entirely ignoring shutdownMode
            CKManager().call(QLatin1String(
                    shutdownType == KWorkSpace::ShutdownTypeReboot ? "Restart" : "Stop"));
            return;
        }

        cap_ask = false;
    }
    if (!cap_ask && shutdownMode == KWorkSpace::ShutdownModeInteractive)
        shutdownMode = KWorkSpace::ShutdownModeForceNow;

    QByteArray cmd;
    if (DMType == OldGDM) {
        cmd.append(shutdownMode == KWorkSpace::ShutdownModeForceNow ?
                   "SET_LOGOUT_ACTION " : "SET_SAFE_LOGOUT_ACTION ");
        cmd.append(shutdownType == KWorkSpace::ShutdownTypeReboot ?
                   "REBOOT\n" : "HALT\n");
    } else {
        cmd.append("shutdown\t");
        cmd.append(shutdownType == KWorkSpace::ShutdownTypeReboot ?
                   "reboot\t" : "halt\t");
        if (!bootOption.isEmpty())
            cmd.append("=").append(bootOption.toLocal8Bit()).append("\t");
        cmd.append(shutdownMode == KWorkSpace::ShutdownModeInteractive ?
                   "ask\n" :
                   shutdownMode == KWorkSpace::ShutdownModeForceNow ?
                   "forcenow\n" :
                   shutdownMode == KWorkSpace::ShutdownModeTryNow ?
                   "trynow\n" : "schedule\n");
    }
    exec(cmd.data());
}

bool
KDisplayManager::bootOptions(QStringList &opts, int &defopt, int &current)
{
    if (DMType != NewKDM)
        return false;

    QByteArray re;
    if (!exec("listbootoptions\n", re))
        return false;

    opts = QString::fromLocal8Bit(re.data()).split('\t', QString::SkipEmptyParts);
    if (opts.size() < 4)
        return false;

    bool ok;
    defopt = opts[2].toInt(&ok);
    if (!ok)
        return false;
    current = opts[3].toInt(&ok);
    if (!ok)
        return false;

    opts = opts[1].split(' ', QString::SkipEmptyParts);
    for (QStringList::Iterator it = opts.begin(); it != opts.end(); ++it)
        (*it).replace("\\s", " ");

    return true;
}
#endif // KDM_NO_SHUTDOWN

// This only tells KDM to not auto-re-login upon session crash
void
KDisplayManager::setLock(bool on)
{
    if (DMType == NewKDM || DMType == OldKDM)
        exec(on ? "lock\n" : "unlock\n");
}

bool
KDisplayManager::isSwitchable()
{
    if (DMType == NewGDM || DMType == LightDM) {
        QDBusObjectPath currentSeat;
        if (getCurrentSeat(0, &currentSeat)) {
            CKSeat seat(currentSeat);
            if (seat.isValid()) {
                QDBusReply<bool> r = seat.call(QLatin1String("CanActivateSessions"));
                if (r.isValid())
                    return r.value();
            }
        }
        return false;
    }

    if (DMType == OldKDM)
        return dpy[0] == ':';

    if (DMType == OldGDM)
        return exec("QUERY_VT\n");

    QByteArray re;

    return exec("caps\n", re) && re.indexOf("\tlocal") >= 0;
}

int
KDisplayManager::numReserve()
{
    if (DMType == NewGDM || DMType == OldGDM || DMType == LightDM)
        return 1; /* Bleh */

    if (DMType == OldKDM)
        return strstr(ctl, ",rsvd") ? 1 : -1;

    QByteArray re;
    int p;

    if (!(exec("caps\n", re) && (p = re.indexOf("\treserve ")) >= 0))
        return -1;
    return atoi(re.data() + p + 9);
}

void
KDisplayManager::startReserve()
{
    if (DMType == NewGDM)
        GDMFactory().call(QLatin1String("CreateTransientDisplay"));
    else if (DMType == OldGDM)
        exec("FLEXI_XSERVER\n");
    else if (DMType == LightDM) {
        LightDMDBus lightDM;
        lightDM.call("SwitchToGreeter");
    }
    else
        exec("reserve\n");
}

bool
KDisplayManager::localSessions(SessList &list)
{
    if (DMType == OldKDM)
        return false;

    if (DMType == NewGDM || DMType == LightDM) {
        QDBusObjectPath currentSession, currentSeat;
        if (getCurrentSeat(&currentSession, &currentSeat)) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                CKSession lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    getSessionLocation(lsess, se);
                    // "Warning: we haven't yet defined the allowed values for this property.
                    // It is probably best to avoid this until we do."
                    QDBusReply<QString> r = lsess.call(QLatin1String("GetSessionType"));
                    if (r.value() != QLatin1String("LoginWindow")) {
                        QDBusReply<unsigned> r2 = lsess.call(QLatin1String("GetUnixUser"));
                        se.user = KUser(K_UID(r2.value())).loginName();
                        se.session = "<unknown>";
                    }
                    se.self = (sp == currentSession);
                    list.append(se);
                }
            }
            return true;
        }
        return false;
    }

    QByteArray re;

    if (DMType == OldGDM) {
        if (!exec("CONSOLE_SERVERS\n", re))
            return false;
        const QStringList sess = QString(re.data() +3).split(QChar(';'), QString::SkipEmptyParts);
        for (QStringList::ConstIterator it = sess.constBegin(); it != sess.constEnd(); ++it) {
            QStringList ts = (*it).split(QChar(','));
            SessEnt se;
            se.display = ts[0];
            se.user = ts[1];
            se.vt = ts[2].toInt();
            se.session = "<unknown>";
            se.self = ts[0] == ::getenv("DISPLAY"); /* Bleh */
            se.tty = false;
            list.append(se);
        }
    } else {
        if (!exec("list\talllocal\n", re))
            return false;
        const QStringList sess = QString(re.data() + 3).split(QChar('\t'), QString::SkipEmptyParts);
        for (QStringList::ConstIterator it = sess.constBegin(); it != sess.constEnd(); ++it) {
            QStringList ts = (*it).split(QChar(','));
            SessEnt se;
            se.display = ts[0];
            se.vt = ts[1].mid(2).toInt();
            se.user = ts[2];
            se.session = ts[3];
            se.self = (ts[4].indexOf('*') >= 0);
            se.tty = (ts[4].indexOf('t') >= 0);
            list.append(se);
        }
    }
    return true;
}

void
KDisplayManager::sess2Str2(const SessEnt &se, QString &user, QString &loc)
{
    if (se.tty) {
        user = i18nc("user: ...", "%1: TTY login", se.user);
        loc = se.vt ? QString("vt%1").arg(se.vt) : se.display ;
    } else {
        user =
            se.user.isEmpty() ?
                se.session.isEmpty() ?
                    i18nc("... location (TTY or X display)", "Unused") :
                    se.session == "<remote>" ?
                        i18n("X login on remote host") :
                        i18nc("... host", "X login on %1", se.session) :
                se.session == "<unknown>" ?
                    se.user :
                    i18nc("user: session type", "%1: %2",
                          se.user, se.session);
        loc =
            se.vt ?
                QString("%1, vt%2").arg(se.display).arg(se.vt) :
                se.display;
    }
}

QString
KDisplayManager::sess2Str(const SessEnt &se)
{
    QString user, loc;

    sess2Str2(se, user, loc);
    return i18nc("session (location)", "%1 (%2)", user, loc);
}

bool
KDisplayManager::switchVT(int vt)
{
    if (DMType == NewGDM || DMType == LightDM) {
        QDBusObjectPath currentSeat;
        if (getCurrentSeat(0, &currentSeat)) {
            foreach (const QDBusObjectPath &sp, getSessionsForSeat(currentSeat)) {
                CKSession lsess(sp);
                if (lsess.isValid()) {
                    SessEnt se;
                    getSessionLocation(lsess, se);
                    if (se.vt == vt) {
                        if (se.tty) // ConsoleKit simply ignores these
                            return false;
                        lsess.call(QLatin1String("Activate"));
                        return true;
                    }
                }
            }
        }
        return false;
    }

    if (DMType == OldGDM)
        return exec(QString("SET_VT %1\n").arg(vt).toLatin1());

    return exec(QString("activate\tvt%1\n").arg(vt).toLatin1());
}

void
KDisplayManager::lockSwitchVT(int vt)
{
    // Lock first, otherwise the lock won't be able to kick in until the session is re-activated.
    QDBusInterface screensaver("org.freedesktop.ScreenSaver", "/ScreenSaver", "org.freedesktop.ScreenSaver");
    screensaver.call("Lock");

    switchVT(vt);
}

void
KDisplayManager::GDMAuthenticate()
{
    FILE *fp;
    const char *dpy, *dnum, *dne;
    int dnl;
    Xauth *xau;

    dpy = DisplayString(QX11Info::display());
    if (!dpy) {
        dpy = ::getenv("DISPLAY");
        if (!dpy)
            return;
    }
    dnum = strchr(dpy, ':') + 1;
    dne = strchr(dpy, '.');
    dnl = dne ? dne - dnum : strlen(dnum);

    /* XXX should do locking */
    if (!(fp = fopen(XauFileName(), "r")))
        return;

    while ((xau = XauReadAuth(fp))) {
        if (xau->family == FamilyLocal &&
            xau->number_length == dnl && !memcmp(xau->number, dnum, dnl) &&
            xau->data_length == 16 &&
            xau->name_length == 18 && !memcmp(xau->name, "MIT-MAGIC-COOKIE-1", 18))
        {
            QString cmd("AUTH_LOCAL ");
            for (int i = 0; i < 16; i++)
                cmd += QString::number((uchar)xau->data[i], 16).rightJustified(2, '0');
            cmd += '\n';
            if (exec(cmd.toLatin1())) {
                XauDisposeAuth(xau);
                break;
            }
        }
        XauDisposeAuth(xau);
    }

    fclose (fp);
}

#endif // Q_WS_X11
