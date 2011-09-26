/*

Greeter module for xdm

Copyright (C) 1997, 1998 Steffen Hansen <hansen@kde.org>
Copyright (C) 2000-2003 Oswald Buddenhagen <ossi@kde.org>


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

#include "kgapp.h"

#include "kdm_greet.h"
#include "kdmshutdown.h"
#include "kdmconfig.h"
#include "kgreeter.h"
#ifdef XDMCP
# include "kchooser.h"
#endif
#include "themer/kdmthemer.h"
#include "utils.h"

#include <kcrash.h>
#include <kglobalsettings.h>
#include <kcomponentdata.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kstandarddirs.h>

#include <QDesktopWidget>
#include <QPainter>
#include <QX11Info>

#include <stdio.h>
#include <stdlib.h> // free(), exit()
#include <unistd.h> // alarm()
#include <signal.h>
#include <locale.h>
#include <fcntl.h>
#include <sys/stat.h>
#ifdef USE_SYSLOG
# include <syslog.h>
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <fixx11h.h>

extern "C" {

static void
sigAlarm(int)
{
    exit(EX_RESERVER_DPY);
}

}

GreeterApp::GreeterApp(int &argc, char **argv) :
    inherited(argc, argv),
    regrabPtr(false), regrabKbd(false), initalBusy(true), sendInteract(false),
    dragWidget(0)
{
    pingInterval = _isLocal ? 0 : _pingInterval;
    if (pingInterval) {
        struct sigaction sa;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sa.sa_handler = sigAlarm;
        sigaction(SIGALRM, &sa, 0);
        alarm(pingInterval * 70); // sic! give the "proper" pinger enough time
        pingTimerId = startTimer(pingInterval * 60000);
    } else
        pingTimerId = 0;
}

void
GreeterApp::markBusy()
{
    if (initalBusy)
        initalBusy = false;
    else
        setCursor(QX11Info::display(), desktop()->winId(), XC_watch);
    setOverrideCursor(Qt::WaitCursor);
}

void
GreeterApp::markReady()
{
    restoreOverrideCursor();
    setCursor(QX11Info::display(), desktop()->winId(), XC_left_ptr);
}

void
GreeterApp::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == pingTimerId) {
        alarm(0);
        if (!pingServer(QX11Info::display()))
            ::exit(EX_RESERVER_DPY);
        alarm(pingInterval * 70); // sic! give the "proper" pinger enough time
    }
}

bool
GreeterApp::x11EventFilter(XEvent * ev)
{
    KeySym sym;

    if (_grabInput) {
        switch (ev->type) {
        case FocusIn:
        case FocusOut:
            if (ev->xfocus.mode == NotifyUngrab) {
                if (!regrabKbd) {
                    secureKeyboard(QX11Info::display());
                    regrabKbd = true;
                }
            } else {
                regrabKbd = false;
            }
            return false;
        case EnterNotify:
        case LeaveNotify:
            if (ev->xcrossing.mode == NotifyUngrab) {
                if (!regrabPtr) {
                    securePointer(QX11Info::display());
                    regrabPtr = true;
                }
            } else {
                regrabPtr = false;
            }
            return false;
        }
    }
    switch (ev->type) {
    case KeyPress:
        sym = XLookupKeysym(&ev->xkey, 0);
        if (sym != XK_Return && !IsModifierKey(sym))
            emit activity();
        break;
    case ButtonPress:
        emit activity();
        /* fall through */
    case ButtonRelease:
        // Hack to let the RMB work as LMB
        if (ev->xbutton.button == 3)
            ev->xbutton.button = 1;
        /* fall through */
    case MotionNotify:
        if (ev->xbutton.state & Button3Mask)
            ev->xbutton.state = (ev->xbutton.state & ~Button3Mask) | Button1Mask;
        switch (ev->type) {
        case ButtonPress:
            if (((ev->xbutton.state & Mod1Mask) && ev->xbutton.button == 1) ||
                dragWidget)
            {
                if (!dragWidget &&
                    ev->xbutton.window != QX11Info::appRootWindow(_greeterScreen) &&
                    (dragWidget = QWidget::find(ev->xbutton.window)))
                {
                    dragWidget = dragWidget->window();
                    dialogStartPos = dragWidget->geometry().center();
                    mouseStartPos = QPoint(ev->xbutton.x_root, ev->xbutton.y_root);
                    setOverrideCursor(QCursor(Qt::SizeAllCursor));
                }
                return true;
            }
            break;
        case ButtonRelease:
            if (dragWidget) {
                restoreOverrideCursor();
                dragWidget = 0;
                return true;
            }
            break;
        case MotionNotify:
            if (dragWidget) {
                QRect grt(dragWidget->rect());
                grt.moveCenter(dialogStartPos +
                               QPoint(ev->xbutton.x_root, ev->xbutton.y_root) -
                               mouseStartPos);
                FDialog::fitInto(qApp->desktop()->screenGeometry(_greeterScreen), grt);
                dragWidget->setGeometry(grt);
                return true;
            }
            break;
        }
        break;
    default:
        return false;
    }
    if (sendInteract) {
        sendInteract = false;
        // We assume that no asynchronous communication is going on
        // before the first user interaction.
        gSendInt(G_Interact);
    }
    return false;
}

extern "C" {

static int
xIOErr(Display *)
{
    exit(EX_RESERVER_DPY);
    // Bogus return value, notreached
    return 0;
}

static void
sigterm(int n ATTR_UNUSED)
{
    exit(EX_NORMAL);
}

static char *savhome;

static void
cleanup(void)
{
    char buf[128];

    if (strcmp(savhome, getenv("HOME")) || memcmp(savhome, "/tmp/", 5)) {
        logError("Internal error: memory corruption detected\n"); /* no panic: recursion */
    } else {
        sprintf(buf, "rm -rf %s", savhome);
        system(buf);
    }
}

static int
goodLocale(const char *var)
{
    char *l = getenv(var);
    if (!l)
        return False;
    if (*l && strcmp(l, "C") && strcmp(l, "POSIX"))
        return True;
    unsetenv(l);
    return False;
}

} // extern "C"

static uint
qHash(const QSize &sz)
{
    return (sz.width() << 12) ^ sz.height();
}

int
main(int argc ATTR_UNUSED, char **argv)
{
    char *ci;
    int i;
    char qtrc[40];

    if (!(ci = getenv("CONINFO"))) {
        fprintf(stderr, "This program is part of kdm and should not be run manually.\n");
        return 1;
    }
    if (sscanf(ci, "%d %d %d %d", &srfd, &swfd, &mrfd, &mwfd) != 4)
        return 1;
    fcntl(srfd, F_SETFD, FD_CLOEXEC);
    fcntl(swfd, F_SETFD, FD_CLOEXEC);
    fcntl(mrfd, F_SETFD, FD_CLOEXEC);
    fcntl(mwfd, F_SETFD, FD_CLOEXEC);
    gSet(0);

#ifdef USE_SYSLOG
    openlog("kdm_greet", LOG_PID, LOG_DAEMON);
#endif

    if ((debugLevel = gRecvInt()) & DEBUG_WGREET)
        sleep(100);

    signal(SIGTERM, sigterm);

    dname = getenv("DISPLAY");

    initConfig();

    /* for QSettings */
    srand(time(0));
    for (i = 0; i < 10000; i++) {
        sprintf(qtrc, "/tmp/%010d", rand());
        if (!mkdir(qtrc, 0700))
            goto okay;
    }
    logPanic("Cannot create $HOME\n");
  okay:
    if (setenv("HOME", qtrc, 1))
        logPanic("Cannot set $HOME\n");
    if (!(savhome = strdup(qtrc)))
        logPanic("Cannot save $HOME\n");
    atexit(cleanup);

    if (*_language) {
        /*
         * Make reasonably sure the locale is not POSIX. This will still fail
         * if all of the following apply:
         * - LANG, LC_MESSAGES & LC_ALL resolve to POSIX
         * - an abbreviated locale is configured (the kcm does this)
         * - the en_US locale is not installed
         */
        if (!goodLocale("LC_ALL") &&
            !goodLocale("LC_MESSAGES") &&
            !goodLocale("LANG"))
        {
            if (strchr(_language, '_') && setlocale(LC_ALL, _language))
                setenv("LANG", _language, 1);
            else if (setlocale(LC_ALL, "en_US"))
                setenv("LANG", "en_US", 1);
            else
                logError("Cannot set locale. Translations will not work.\n");
        }

        setenv("LANGUAGE", _language, 1);
    }

    // fool qt's platform detection so it loads the kde platform plugin
    setenv("KDE_FULL_SESSION", "true", 1);
    setenv("KDE_SESSION_VERSION", "4", 1);
    setenv("DESKTOP_SESSION", "kde", 1); // for qt 4.6 only
    // fool d-bus, so we get no kbuildsycoca, etc.
    setenv("DBUS_SESSION_BUS_ADDRESS", "fake", 1);

    static char *fakeArgv[] = { (char *)"kdmgreet", 0 };
    static int fakeArgc = as(fakeArgv) - 1;

    KCrash::setFlags(KCrash::KeepFDs | KCrash::SaferDialog | KCrash::AlwaysDirectly);
    KCrash::setApplicationName(QLatin1String(fakeArgv[0]));
    KCrash::setDrKonqiEnabled(true);
    XSetIOErrorHandler(xIOErr);
    KComponentData inst(fakeArgv[0]);
    foreach (const QString &dir, KGlobal::dirs()->resourceDirs("qtplugins"))
        QCoreApplication::addLibraryPath(dir);
    GreeterApp app(fakeArgc, fakeArgv);
    initQAppConfig();
    KGlobalSettings::self()->activate(KGlobalSettings::ApplySettings);

    Display *dpy = QX11Info::display();
    QDesktopWidget *dw = app.desktop();

    if (_greeterScreen < 0)
        _greeterScreen = _greeterScreen == -2 ?
            dw->screenNumber(QPoint(dw->width() - 1, 0)) :
            dw->screenNumber(QPoint(0, 0));

    _colorScheme = KStandardDirs::locate("data", "color-schemes/" + _colorScheme + ".colors");
    if (!_colorScheme.isEmpty()) {
        KSharedConfigPtr config = KSharedConfig::openConfig(_colorScheme, KConfig::SimpleConfig);
        app.setPalette(KGlobalSettings::createApplicationPalette(config));
    }

    KdmThemer *themer;
    if (_useTheme && !_theme.isEmpty()) {
        QMap<QString, bool> showTypes;
        // "config" not implemented
#ifdef XDMCP
        if (_loginMode != LOGIN_LOCAL_ONLY)
            showTypes["chooser"] = true;
#endif
        showTypes["system"] = true;
        if (_allowShutdown != SHUT_NONE) {
            showTypes["halt"] = true;
            showTypes["reboot"] = true;
            // "suspend" not implemented
        }

        themer = new KdmThemer(_theme, showTypes, app.desktop()->screen());
        if (!themer->isOK()) {
            delete themer;
            themer = 0;
        }
    } else {
        themer = 0;
    }

    setupModifiers(dpy, _numLockStatus);
    secureDisplay(dpy);
    KProcess *proc = 0;
    if (!_grabServer) {
        if (_useBackground && !themer) {
            proc = new KProcess;
            *proc << QByteArray(argv[0], strrchr(argv[0], '/') - argv[0] + 1) + "krootimage";
            *proc << _backgroundCfg;
            proc->start();
        }
        gSendInt(G_SetupDpy);
        gRecvInt();
    }

    gSendInt(G_Ready);

    if (themer) {
        // Group by size to avoid rescaling images repeatedly
        QHash<QSize, QList<int> > scrns;
        for (int i = 0; i < dw->numScreens(); i++)
            scrns[dw->screenGeometry(i).size()] << i;
        QPixmap pm(dw->size());
        QPainter p(&pm);
        p.fillRect(dw->rect(), Qt::black);
        QSize gsz = dw->screenGeometry(_greeterScreen).size();
        // Paint these first, as throwing away their images does not hurt
        QHash<QSize, QList<int> >::ConstIterator it;
        for (it = scrns.constBegin(); it != scrns.constEnd(); ++it)
            if (it.key() != gsz)
                foreach (int i, it.value())
                    themer->paintBackground(&p, dw->screenGeometry(i), false);
        // If we are lucky, these will use the same images as the greeter
        foreach (int i, scrns.value(gsz))
            if (i != _greeterScreen)
                themer->paintBackground(&p, dw->screenGeometry(i), false);
        // Paint the greeter background last - it will be re-used.
        themer->paintBackground(&p, dw->screenGeometry(_greeterScreen), true);
        QPalette palette;
        palette.setBrush(dw->backgroundRole(), QBrush(pm));
        dw->setPalette(palette);
        XClearWindow(dpy, dw->winId());
    }

    int rslt = ex_exit;
    for (;;) {
        int cmd = gRecvInt();

        if (cmd == G_ConfShutdown) {
            gSet(1);
            gSendInt(G_QryDpyShutdown);
            int how = gRecvInt(), uid = gRecvInt();
            QString os = qString(gRecvStr());
            gSet(0);
            app.markReady();
            KDMSlimShutdown::externShutdown(how, os, uid, true);
            gSendInt(G_Ready);
            break;
        }

        if (cmd == G_ErrorGreet) {
            app.markReady();
            if (KGVerify::handleFailVerify(qApp->desktop()->screen(_greeterScreen), true))
                break;
            _autoLoginDelay = 0;
            cmd = G_Greet;
        }

        KProcess *proc2 = 0;
        app.markBusy();
        FDialog *dialog;
#ifdef XDMCP
        if (cmd == G_Choose) {
            dialog = new ChooserDlg;
            gSendInt(G_Ready); /* tell chooser to go into async mode */
            gRecvInt(); /* ack */
        } else
#endif
        {
            if ((cmd != G_GreetTimed && !_autoLoginAgain) ||
                    _autoLoginUser.isEmpty())
                _autoLoginDelay = 0;
            if (themer)
                dialog = new KThemedGreeter(themer);
            else
                dialog = new KStdGreeter;
            if (*_preloader) {
                proc2 = new KProcess;
                *proc2 << _preloader;
                proc2->start();
            }
        }
        QObject::connect(dialog, SIGNAL(ready()), &app, SLOT(markReady()));
        app.enableSendInteract();
        debug("entering event loop\n");
        rslt = dialog->exec();
        debug("left event loop\n");
        delete dialog;
        delete proc2;
#ifdef XDMCP
        switch (rslt) {
        case ex_greet:
            gSendInt(G_DGreet);
            continue;
        case ex_choose:
            gSendInt(G_DChoose);
            continue;
        default:
            break;
        }
#endif
        break;
    }

    KGVerify::done();

    delete proc;
    delete themer;

    unsecureDisplay(dpy);
    restoreModifiers();

    if (rslt == ex_login) {
        gSendInt(G_Ready);
        KGVerify::handleFailVerify(qApp->desktop()->screen(_greeterScreen), false);
    }

    return EX_NORMAL;
}

#include "kgapp.moc"
