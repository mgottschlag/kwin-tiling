/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "plasmaapp.h"

#ifdef Q_WS_WIN
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0500
#include <windows.h>
#endif

#include <unistd.h>

#ifndef _SC_PHYS_PAGES
    #ifdef Q_OS_FREEBSD
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #endif

    #ifdef Q_OS_NETBSD
    #include <sys/param.h>
    #include <sys/sysctl.h>
    #endif
#endif

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QVBoxLayout>

#include <KAction>
#include <KAuthorized>
#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KGlobalAccel>
#include <KGlobalSettings>
#include <KNotification>
#include <KRun>
#include <KWindowSystem>
#include <KService>
#include <KIconLoader>

#include <ksmserver_interface.h>

#include <Plasma/AbstractToolBox>
#include <Plasma/AccessAppletJob>
#include <Plasma/AccessManager>
#include <Plasma/AuthorizationManager>
#include <Plasma/Containment>
#include <Plasma/Context>
#include <Plasma/Dialog>
#include <Plasma/Theme>
#include <Plasma/Wallpaper>
#include <Plasma/WindowEffects>

#include <KActivities/Controller>

#include <kephal/screens.h>

#include <plasmagenericshell/backgrounddialog.h>

#include "activity.h"
#include "appadaptor.h"
#include "controllerwindow.h"
#include "checkbox.h"
#include "desktopcorona.h"
#include "desktopview.h"
#include "interactiveconsole.h"
#include "panelshadows.h"
#include "panelview.h"
#include "plasma-shell-desktop.h"
#include "toolbutton.h"
#include "klistconfirmationdialog.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#endif

extern QString plasmaLocale;

PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        return new PlasmaApp();
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp()
    : KUniqueApplication(),
      m_corona(0),
      m_panelHidden(0),
      m_mapper(new QSignalMapper(this)),
      m_startupSuspendWaitCount(0),
      m_ignoreDashboardClosures(false),
      m_pendingFixedDashboard(false),
      m_unlockCorona(false)
{
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "plasma app ctor start" << "(line:" << __LINE__ << ")";
    suspendStartup(true);

    if (KGlobalSettings::isMultiHead()) {
        KGlobal::locale()->setLanguage(plasmaLocale, KGlobal::config().data());
    }

    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasmagenericshell");
    KCrash::setFlags(KCrash::AutoRestart);

    // why is the next line of code here here?
    //
    // plasma-desktop was once plasma. not a big deal, right?
    //
    // well, kglobalaccel has a policy of forever
    // reserving shortcuts. even if the application is not running, it will still
    // defend that application's right to using that global shortcut. this has,
    // at least to me, some very obvious negative impacts on usability, such as
    // making it difficult for the user to switch between applications of the
    // same type and use the same global shortcuts, or when the component changes
    // name as in plasma-desktop.
    //
    // applications can unregister each other's shortcuts, though, and so that's
    // exactly what we do here.
    //
    // i'd love to just rely on the kconf_update script, but kglobalaccel doesn't
    // listen to changes in its config file nor has a dbus interace to tickle it
    // into re-reading it and it starts too early in the start up sequence for
    // kconf_update to beat it to the config file.
    //
    // so we instead deal with a dbus roundtrip with kded
    // (8 context switches at minimum iirc?)
    // at every app start for something that really only needs to be done once
    // but which we can't know for sure when it has been done.
    //
    // if kglobalaccel ignored non-running apps or could be prompted into
    // reloading its config, this would be unecessary.
    //
    // what's kind of funny is that if plasma actually relied on kglobalaccel for
    // the plasmoid shortcuts (which it can't because layouts change too often and
    // can be stored/retreived making kglobalaccel management a non-option) this
    // would be a total disaster. As it is it's "just" potentially losing the user's
    // customization of the Ctrl+F12 default shortcut to bring up the dashboard.
    //
    // this line should be removed when we decide that 4.[012]->4.<current> is
    // no longer a supported upgrade path. sometime after dragons make a comeback
    // and can be seen circling the skies again.
    // - aseigo.
    KGlobalAccel::cleanComponent("plasma");

    m_panelViewCreationTimer.setSingleShot(true);
    m_panelViewCreationTimer.setInterval(0);

    m_desktopViewCreationTimer.setSingleShot(true);
    m_desktopViewCreationTimer.setInterval(0);

    new PlasmaAppAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/App", this);

    // Enlarge application pixmap cache
    // Calculate the size required to hold background pixmaps for all screens.
    // Add 10% so that other (smaller) pixmaps can also be cached.
    int cacheSize = 0;

    if (KGlobalSettings::isMultiHead()) {
        int id = 0;
#ifdef Q_WS_X11
        Display *dpy = XOpenDisplay(NULL);
        if (dpy) {
            id = DefaultScreen(dpy);
            XCloseDisplay(dpy);
        }
#endif
        const QSize size = Kephal::ScreenUtils::screenSize(id);
        cacheSize += 4 * size.width() * size.height() / 1024;
    } else {
        const int numScreens = Kephal::ScreenUtils::numScreens();
        for (int i = 0; i < numScreens; i++) {
            QSize size = Kephal::ScreenUtils::screenSize(i);
            cacheSize += 4 * size.width() * size.height() / 1024;
        }
    }
    cacheSize += cacheSize / 10;

    // Calculate the size of physical system memory; _SC_PHYS_PAGES *
    // _SC_PAGESIZE is documented to be able to overflow 32-bit integers,
    // so apply a 10-bit shift. FreeBSD 6-STABLE doesn't have _SC_PHYS_PAGES
    // (it is documented in FreeBSD 7-STABLE as "Solaris and Linux extension")
    // so use sysctl in those cases.
#if defined(_SC_PHYS_PAGES)
    int memorySize = sysconf(_SC_PHYS_PAGES);
    memorySize *= sysconf(_SC_PAGESIZE) / 1024;
#else
#ifdef Q_OS_FREEBSD
    int sysctlbuf[2];
    size_t size = sizeof(sysctlbuf);
    int memorySize;
    // This could actually use hw.physmem instead, but I can't find
    // reliable documentation on how to read the value (which may
    // not fit in a 32 bit integer).
    if (!sysctlbyname("vm.stats.vm.v_page_size", sysctlbuf, &size, NULL, 0)) {
        memorySize = sysctlbuf[0] / 1024;
        size = sizeof(sysctlbuf);
        if (!sysctlbyname("vm.stats.vm.v_page_count", sysctlbuf, &size, NULL, 0)) {
            memorySize *= sysctlbuf[0];
        }
    }
#endif
#ifdef Q_OS_NETBSD
    size_t memorySize;
    size_t len;
    static int mib[] = { CTL_HW, HW_PHYSMEM };

    len = sizeof(memorySize);
    sysctl(mib, 2, &memorySize, &len, NULL, 0);
    memorySize /= 1024;
#endif
#ifdef Q_WS_WIN
    size_t memorySize;

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof (statex);
    GlobalMemoryStatusEx (&statex);

    memorySize = (statex.ullTotalPhys/1024) + (statex.ullTotalPageFile/1024);
#endif
    // If you have no suitable sysconf() interface and are not FreeBSD,
    // then you are out of luck and get a compile error.
#endif

    // Increase the pixmap cache size to 1% of system memory if it isn't already
    // larger so as to maximize cache usage. 1% of 1GB ~= 10MB.
    if (cacheSize < memorySize / 100) {
        cacheSize = memorySize / 100;
    }

    kDebug() << "Setting the pixmap cache size to" << cacheSize << "kilobytes";
    QPixmapCache::setCacheLimit(cacheSize);

    KAction *showAction = new KAction(this);
    showAction->setText(i18n("Show Dashboard"));
    showAction->setObjectName( QLatin1String("Show Dashboard" )); // NO I18N
    showAction->setGlobalShortcut(KShortcut(Qt::CTRL + Qt::Key_F12));
    connect(showAction, SIGNAL(triggered()), this, SLOT(toggleDashboard()));

    KGlobal::setAllowQuit(true);
    KGlobal::ref();

    connect(m_mapper, SIGNAL(mapped(QString)),
            this, SLOT(addRemotePlasmoid(QString)));
    connect(Plasma::AccessManager::self(),
            SIGNAL(finished(Plasma::AccessAppletJob*)),
            this, SLOT(plasmoidAccessFinished(Plasma::AccessAppletJob*)));
    connect(Plasma::AccessManager::self(),
            SIGNAL(remoteAppletAnnounced(Plasma::PackageMetadata)),
            this, SLOT(remotePlasmoidAdded(Plasma::PackageMetadata)));

    Plasma::AuthorizationManager::self()->setAuthorizationPolicy(Plasma::AuthorizationManager::PinPairing);

    QTimer::singleShot(0, this, SLOT(setupDesktop()));
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "plasma app ctor end" << "(line:" << __LINE__ << ")";
}

PlasmaApp::~PlasmaApp()
{
}

void PlasmaApp::setupDesktop()
{
#ifdef Q_WS_X11
    Atom atoms[5];
    const char * const atomNames[] = {"XdndAware", "XdndEnter", "XdndFinished", "XdndPosition", "XdndStatus"};
    XInternAtoms(QX11Info::display(), const_cast<char **>(atomNames), 5, False, atoms);
    m_XdndAwareAtom = atoms[0];
    m_XdndEnterAtom = atoms[1];
    m_XdndFinishedAtom = atoms[2];
    m_XdndPositionAtom = atoms[3];
    m_XdndStatusAtom = atoms[4];
    const int xdndversion = 5;
    m_XdndVersionAtom = (Atom)xdndversion;
#endif

    // intialize the default theme and set the font
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    theme->setFont(AppSettings::desktopFont());
    m_panelShadows = new PanelShadows();

    // this line initializes the corona.
    corona();

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenRemoved(int)), SLOT(screenRemoved(int)));
    connect(screens, SIGNAL(screenAdded(Kephal::Screen*)), SLOT(screenAdded(Kephal::Screen*)));

    if (AppSettings::perVirtualDesktopViews()) {
        connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)),
                this, SLOT(checkVirtualDesktopViews(int)));
    }

    // free the memory possibly occupied by the background image of the
    // root window - login managers will typically set one
    QPalette palette;
    palette.setColor(desktop()->backgroundRole(), Qt::black);
    desktop()->setPalette(palette);

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Plasma App SetupDesktop()" << "(line:" << __LINE__ << ")";

    // now connect up the creation timers and start them to get the views created
    connect(&m_panelViewCreationTimer, SIGNAL(timeout()), this, SLOT(createWaitingPanels()));
    connect(&m_desktopViewCreationTimer, SIGNAL(timeout()), this, SLOT(createWaitingDesktops()));
    m_panelViewCreationTimer.start();
    m_desktopViewCreationTimer.start();
}

void PlasmaApp::quit()
{
    if (m_corona) {
        cleanup();
        KGlobal::deref();
    }
}

void PlasmaApp::cleanup()
{
    if (!m_corona) {
        return;
    }

    m_corona->saveLayout();

    // save the mapping of Views to Containments at the moment
    // of application exit so we can restore that when we start again.
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    viewIds.deleteGroup();

    foreach (PanelView *v, m_panels) {
        if (v->containment()) {
            viewIds.writeEntry(QString::number(v->containment()->id()), v->id());
        }
    }

    foreach (DesktopView *v, m_desktops) {
        if (v->containment()) {
            viewIds.writeEntry(QString::number(v->containment()->id()), v->id());
        }
    }

    QList<DesktopView*> desktops = m_desktops;
    m_desktops.clear();
    qDeleteAll(desktops);

    QList<PanelView*> panels = m_panels;
    m_panels.clear();
    qDeleteAll(panels);

    delete m_console.data();
    delete m_corona;
    m_corona = 0;

    delete m_panelShadows;
    m_panelShadows = 0;

    //TODO: This manual sync() should not be necessary. Remove it when
    // KConfig was fixed
    KGlobal::config()->sync();
}

void PlasmaApp::syncConfig()
{
    KGlobal::config()->sync();
}

void PlasmaApp::toggleDashboard()
{
    // we don't want to listen to dashboard closure signals when toggling
    // otherwise we get toggleDashboard -> dashboardClosed -> showDashboard
    // and the wrong state of shown dashboards occurs.
    m_ignoreDashboardClosures = true;

    const int currentDesktop = KWindowSystem::currentDesktop() - 1;
    foreach (DesktopView *view, m_desktops) {
        if (AppSettings::perVirtualDesktopViews()) {
            // always hide the dashboard if it isn't on the current desktop
            if (view->desktop() == currentDesktop) {
                view->toggleDashboard();
            }
        } else {
            view->toggleDashboard();
        }
    }

    m_ignoreDashboardClosures = false;
}

void PlasmaApp::showDashboard(bool show)
{
    // we don't want to listen to dashboard closure signals when showing/hiding
    // otherwise we get showDashboard -> dashboardClosed -> showDashboard
    // and that could end up badly :)
    m_ignoreDashboardClosures = true;

    const int currentDesktop = KWindowSystem::currentDesktop() - 1;
    foreach (DesktopView *view, m_desktops) {
        if (AppSettings::perVirtualDesktopViews()) {
            // always hide the dashboard if it isn't on the current desktop
            if (view->desktop() == currentDesktop) {
                view->showDashboard(show);
            }
        } else {
            view->showDashboard(show);
        }
    }

    m_ignoreDashboardClosures = false;
}

void PlasmaApp::dashboardClosed()
{
    if (!m_ignoreDashboardClosures) {
        showDashboard(false);
    }
}

void PlasmaApp::showInteractiveConsole()
{
    if (KGlobal::config()->isImmutable() || !KAuthorized::authorize("plasma-desktop/scripting_console")) {
        return;
    }

    InteractiveConsole *console = m_console.data();
    if (!console) {
        m_console = console = new InteractiveConsole(m_corona);
    }

    KWindowSystem::setOnDesktop(console->winId(), KWindowSystem::currentDesktop());
    console->show();
    console->raise();
    KWindowSystem::forceActiveWindow(console->winId());
}

void PlasmaApp::loadScriptInInteractiveConsole(const QString &script)
{
    showInteractiveConsole();
    if (m_console) {
        m_console.data()->loadScript(script);
    }
}

void PlasmaApp::panelHidden(bool hidden)
{
    if (hidden) {
        ++m_panelHidden;
        //kDebug() << "panel hidden" << m_panelHidden;
    } else {
        --m_panelHidden;
        if (m_panelHidden < 0) {
            kDebug() << "panelHidden(false) called too many times!";
            m_panelHidden = 0;
        }
        //kDebug() << "panel unhidden" << m_panelHidden;
    }
}

QList<PanelView*> PlasmaApp::panelViews() const
{
    return m_panels;
}

PanelShadows *PlasmaApp::panelShadows() const
{
    return m_panelShadows;
}

ControllerWindow *PlasmaApp::showWidgetExplorer(int screen, Plasma::Containment *containment)
{
    return showController(screen, containment, true);
}

void PlasmaApp::toggleActivityManager()
{
    const int currentScreen = m_corona->screenId(QCursor::pos());

    QWeakPointer<ControllerWindow> controllerPtr = m_widgetExplorers.value(currentScreen);
    ControllerWindow *controller = controllerPtr.data();
    if (controller) {
        controller->deleteLater();
        return;
    }

    //try to find the "active" containment
    int currentDesktop = -1;
    if (AppSettings::perVirtualDesktopViews()) {
        currentDesktop = KWindowSystem::currentDesktop() - 1;
    }

    Plasma::Containment *containment = m_corona->containmentForScreen(currentScreen, currentDesktop);

    showController(currentScreen, containment, false);
}

ControllerWindow *PlasmaApp::showController(int screen, Plasma::Containment *containment, bool widgetExplorerMode)
{
    if (!containment) {
        kDebug() << "no containment";
        return 0;
    }

    QWeakPointer<ControllerWindow> controllerPtr = m_widgetExplorers.value(screen);
    ControllerWindow *controller = controllerPtr.data();

    if (!controller) {
        //kDebug() << "controller not found for screen" << screen;
        controllerPtr = controller = new ControllerWindow(0);
        m_widgetExplorers.insert(screen, controllerPtr);
    }

    controller->setContainment(containment);
    if (!containment || containment->screen() != screen) {
        controller->setScreen(screen);
    }

    controller->setLocation(containment->location());

    if (widgetExplorerMode) {
        controller->showWidgetExplorer();
    } else {
        controller->showActivityManager();
    }

    controller->show();
    Plasma::WindowEffects::slideWindow(controller, controller->location());
    KWindowSystem::setOnAllDesktops(controller->winId(), true);
    QTimer::singleShot(0, controller, SLOT(activate()));
    KWindowSystem::setState(controller->winId(), NET::SkipTaskbar | NET::SkipPager | NET::Sticky | NET::KeepAbove);
    return controller;
}

void PlasmaApp::hideController(int screen)
{
    QWeakPointer<ControllerWindow> controller = m_widgetExplorers.value(screen);
    if (controller) {
        controller.data()->hide();
    }
}

#ifdef Q_WS_X11
PanelView *PlasmaApp::findPanelForTrigger(WId trigger) const
{
    foreach (PanelView *panel, m_panels) {
        if (panel->unhideTrigger() == trigger) {
            return panel;
        }
    }

    return 0;
}

bool PlasmaApp::x11EventFilter(XEvent *event)
{
    if (m_panelHidden > 0 &&
        (event->type == ClientMessage ||
         (event->xany.send_event != True && (event->type == EnterNotify ||
                                             event->type == MotionNotify)))) {

        /*
        if (event->type == ClientMessage) {
            kDebug() << "client message with" << event->xclient.message_type << m_XdndEnterAtom << event->xcrossing.window;
        }
        */

        bool dndEnter = false;
        bool dndPosition = false;
        if (event->type == ClientMessage) {
            dndEnter = event->xclient.message_type == m_XdndEnterAtom;
            if (!dndEnter) {
                dndPosition = event->xclient.message_type == m_XdndPositionAtom;
                if (!dndPosition) {
                    //kDebug() << "FAIL!";
                    return KUniqueApplication::x11EventFilter(event);
                }
            } else {
                //kDebug() << "on enter" << event->xclient.data.l[0];
            }
        }

        PanelView *panel = findPanelForTrigger(event->xcrossing.window);
        //kDebug() << "panel?" << panel << ((dndEnter || dndPosition) ? "Drag and drop op" : "Mouse move op");
        if (panel) {
            if (dndEnter || dndPosition) {
                QPoint p;

                const unsigned long *l = (const unsigned long *)event->xclient.data.l;
                if (dndPosition) {
                    p = QPoint((l[2] & 0xffff0000) >> 16, l[2] & 0x0000ffff);
                } else {
                    p = QCursor::pos();
                }

                XClientMessageEvent response;
                response.type = ClientMessage;
                response.window = l[0];
                response.format = 32;
                response.data.l[0] = panel->winId(); //event->xcrossing.window;

                if (panel->hintOrUnhide(p, true)) {
                    response.message_type = m_XdndFinishedAtom;
                    response.data.l[1] = 0; // flags
                    response.data.l[2] = XNone;
                } else {
                    response.message_type = m_XdndStatusAtom;
                    response.data.l[1] = 0; // flags
                    response.data.l[2] = 0; // x, y
                    response.data.l[3] = 0; // w, h
                    response.data.l[4] = 0; // action
                }

                XSendEvent(QX11Info::display(), l[0], False, NoEventMask, (XEvent*)&response);
            } else if (event->type == EnterNotify) {
                panel->hintOrUnhide(QPoint(-1, -1));
                //kDebug() << "entry";
            //FIXME: this if it was possible to avoid the polling
            /*} else if (event->type == LeaveNotify) {
                panel->unhintHide();
            */
            } else if (event->type == MotionNotify) {
                XMotionEvent *motion = (XMotionEvent*)event;
                //kDebug() << "motion" << motion->x << motion->y << panel->location();
                panel->hintOrUnhide(QPoint(motion->x_root, motion->y_root));
            }
        }
    }

    return KUniqueApplication::x11EventFilter(event);
}
#endif

void PlasmaApp::screenRemoved(int id)
{
    kDebug() << "@@@@" << id;
    QMutableListIterator<DesktopView *> it(m_desktops);
    while (it.hasNext()) {
        DesktopView *view = it.next();
        if (view->screen() == id) {
            // the screen was removed, so we'll destroy the
            // corresponding view
            kDebug() << "@@@@removing the view for screen" << id;
            view->setContainment(0);
            it.remove();
            delete view;
        }
    }

#if 1
    /**
    UPDATE: this was linked to kephal events, which are not optimal, but it seems it may well
    have been the panel->migrateTo call due to a bug in libplasma fixed in e2108ed. so let's try
    and re-enable this.
    NOTE: CURRENTLY UNSAFE DUE TO HOW KEPHAL (or rather, it seems, Qt?) PROCESSES EVENTS
          DURING XRANDR EVENTS. REVISIT IN 4.8!
          */
    Kephal::Screen *primary = Kephal::Screens::self()->primaryScreen();
    QList<Kephal::Screen *> screens = Kephal::Screens::self()->screens();
    screens.removeAll(primary);

    // Now we process panels: if there is room on another screen for the panel,
    // we migrate the panel there, otherwise the view is deleted. The primary
    // screen is preferred in all cases.
    QMutableListIterator<PanelView*> pIt(m_panels);
    while (pIt.hasNext()) {
        PanelView *panel = pIt.next();
        if (panel->screen() == id) {
            Kephal::Screen *moveTo = 0;
            if (canRelocatePanel(panel, primary)) {
                moveTo = primary;
            } else {
                foreach (Kephal::Screen *screen, screens) {
                    if (canRelocatePanel(panel, screen)) {
                        moveTo = screen;
                        break;
                    }
                }
            }

            if (moveTo) {
                panel->migrateTo(moveTo->id());
            } else {
                pIt.remove();
                delete panel;
                continue;
            }
        }

        panel->updateStruts();
    }
#else
    QMutableListIterator<PanelView*> pIt(m_panels);
    while (pIt.hasNext()) {
        PanelView *panel = pIt.next();
        if (panel->screen() == id) {
            pIt.remove();
            delete panel;
        }
    }
#endif
}

void PlasmaApp::screenAdded(Kephal::Screen *screen)
{
    foreach (Plasma::Containment *containment, corona()->containments()) {
        if (isPanelContainment(containment) && containment->screen() == screen->id()) {
            m_panelsWaiting << containment;
            m_panelViewCreationTimer.start();
        }
    }

    foreach (PanelView *view, m_panels) {
        if (view->migratedFrom(screen->id())) {
            view->migrateTo(screen->id());
        }
    }
}

bool PlasmaApp::canRelocatePanel(PanelView * view, Kephal::Screen *screen)
{
    if (!screen || !view->containment()) {
        return false;
    }

    QRect newGeom = view->geometry();
    switch (view->location()) {
        case Plasma::TopEdge:
            newGeom.setY(screen->geom().y());
            newGeom.setX(view->offset());
            break;
        case Plasma::BottomEdge:
            newGeom.setY(screen->geom().bottom() - newGeom.height());
            newGeom.setX(view->offset());
            break;
        case Plasma::LeftEdge:
            newGeom.setX(screen->geom().left());
            newGeom.setY(view->offset());
            break;
        case Plasma::RightEdge:
            newGeom.setX(screen->geom().right() - newGeom.width());
            newGeom.setY(view->offset());
            break;
        default:
            break;
    }

    kDebug() << "testing:" << screen->id() << view << view->geometry() << view->location() << newGeom;
    foreach (PanelView *pv, m_panels) {
        kDebug() << pv << pv->screen() << pv->screen() << pv->location() << pv->geometry();
        if (pv != view &&
            pv->screen() == screen->id() &&
            pv->location() == view->location() &&
            pv->geometry().intersects(newGeom)) {
            return false;
        }
    }

    return true;
}


DesktopView* PlasmaApp::viewForScreen(int screen, int desktop) const
{
    foreach (DesktopView *view, m_desktops) {
        if (view->containment()) {
            kDebug() << "comparing" << view->containment()->screen() << screen;
        }
        if (view->containment() && view->containment()->screen() == screen && (desktop < 0 || view->containment()->desktop() == desktop)) {
            return view;
        }
    }

    return 0;
}

DesktopCorona* PlasmaApp::corona(bool createIfMissing)
{
    if (!m_corona && createIfMissing) {
        QTime t;
        t.start();
        DesktopCorona *c = new DesktopCorona(this);
        connect(c, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(containmentAdded(Plasma::Containment*)));
        connect(c, SIGNAL(configSynced()), this, SLOT(syncConfig()));
        connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                this, SLOT(containmentScreenOwnerChanged(int,int,Plasma::Containment*)));

        foreach (DesktopView *view, m_desktops) {
            connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                    view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }

        //actions!
        KAction *activityAction = c->addAction("manage activities");
        connect(activityAction, SIGNAL(triggered()), this, SLOT(toggleActivityManager()));
        activityAction->setText(i18n("Activities..."));
        activityAction->setIcon(KIcon("preferences-activities"));
        activityAction->setData(Plasma::AbstractToolBox::ConfigureTool);
        activityAction->setShortcut(KShortcut("alt+d, alt+a"));
        activityAction->setShortcutContext(Qt::ApplicationShortcut);
        activityAction->setGlobalShortcut(KShortcut(Qt::META + Qt::Key_Q));

        c->updateShortcuts();

        m_corona = c;
        c->setItemIndexMethod(QGraphicsScene::NoIndex);
        c->initializeLayout();
        c->processUpdateScripts();
        c->checkActivities();
        c->checkScreens();
        foreach (Plasma::Containment *containment, c->containments()) {
            if (containment->screen() != -1 && containment->wallpaper()) {
                ++m_startupSuspendWaitCount;
                connect(containment->wallpaper(), SIGNAL(update(QRectF)), this, SLOT(wallpaperCheckedIn()));
            }
        }

        QTimer::singleShot(5000, this, SLOT(wallpaperCheckInTimeout()));
        kDebug() << " ------------------------------------------>" << t.elapsed() << m_startupSuspendWaitCount;
    }

    return m_corona;
}

void PlasmaApp::wallpaperCheckInTimeout()
{
    if (m_startupSuspendWaitCount > 0) {
        m_startupSuspendWaitCount = 0;
        suspendStartup(false);
    }
}

void PlasmaApp::wallpaperCheckedIn()
{
    if (m_startupSuspendWaitCount < 1) {
        return;
    }

    --m_startupSuspendWaitCount;
    if (m_startupSuspendWaitCount < 1) {
        m_startupSuspendWaitCount = 0;
        suspendStartup(false);
    }
}

bool PlasmaApp::hasComposite()
{
//    return true;
#ifdef Q_WS_X11
    return KWindowSystem::compositingActive();
#else
    return false;
#endif
}

void PlasmaApp::suspendStartup(bool suspend)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("workspace desktop");
    if (suspend) {
        ksmserver.suspendStartup(startupID);
    } else {
        ksmserver.resumeStartup(startupID);
    }
}

bool PlasmaApp::isPanelContainment(Plasma::Containment *containment)
{
    if (!containment) {
        return false;
    }

    Plasma::Containment::Type t = containment->containmentType();

    return t == Plasma::Containment::PanelContainment ||
           t == Plasma::Containment::CustomPanelContainment;

}

void PlasmaApp::createView(Plasma::Containment *containment)
{
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Plasma App createView() start" << "(line:" << __LINE__ << ")";
    kDebug() << "Containment name:" << containment->name()
             << "| type" << containment->containmentType()
             <<  "| screen:" << containment->screen()
             <<  "| desktop:" << containment->desktop()
             << "| geometry:" << containment->geometry()
             << "| zValue:" << containment->zValue();

    // find the mapping of View to Containment, if any,
    // so we can restore things on start.

    if (isPanelContainment(containment)) {
        m_panelsWaiting << containment;
        m_panelViewCreationTimer.start();
    } else if (containment->screen() > -1 &&
               containment->screen() < m_corona->numScreens()) {
        if (AppSettings::perVirtualDesktopViews()) {
            if (containment->desktop() < 0 ||
                containment->desktop() > KWindowSystem::numberOfDesktops() - 1) {
                return;
            }
        }

        m_desktopsWaiting.append(containment);
        m_desktopViewCreationTimer.start();
    }
}

void PlasmaApp::setWmClass(WId id)
{
#ifdef Q_WS_X11
    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argvisuals in a better way
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("Plasma");
    classHint.res_class = const_cast<char*>("Plasma");
    XSetClassHint(QX11Info::display(), id, &classHint);
#endif
}

void PlasmaApp::createWaitingPanels()
{
    if (m_panelsWaiting.isEmpty()) {
        return;
    }

    const QList<QWeakPointer<Plasma::Containment> > containments = m_panelsWaiting;
    m_panelsWaiting.clear();

    foreach (QWeakPointer<Plasma::Containment> containmentPtr, containments) {
        Plasma::Containment *containment = containmentPtr.data();
        if (!containment) {
            continue;
        }

        foreach (PanelView *view, m_panels) {
            if (view->containment() == containment) {
                continue;
            }
        }

        if (containment->screen() < 0) {
            continue;
        }

        // try to relocate the panel if it is on a now-non-existent screen
        if (containment->screen() >= m_corona->numScreens()) {
            m_panelRelocationCandidates << containment;
            continue;
        }

        createPanelView(containment);
    }

    if (!m_panelRelocationCandidates.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(relocatePanels()));
    }
}

void PlasmaApp::relocatePanels()
{
    // we go through relocatables last so that all other panels can be set up first,
    // preventing panel creation ordering to trip up the canRelocatePanel algorithm
    Kephal::Screen *primary = Kephal::Screens::self()->primaryScreen();
    QList<Kephal::Screen *> screens = Kephal::Screens::self()->screens();
    screens.removeAll(primary);

    foreach (QWeakPointer<Plasma::Containment> c, m_panelRelocationCandidates) {
        Plasma::Containment *containment = c.data();
        if (!containment) {
            continue;
        }

        Kephal::Screen *moveTo = 0;
        PanelView *panelView = createPanelView(containment);
        if (canRelocatePanel(panelView, primary)) {
            moveTo = primary;
        } else {
            foreach (Kephal::Screen *screen, screens) {
                if (canRelocatePanel(panelView, screen)) {
                    moveTo = screen;
                    break;
                }
            }
        }

        if (moveTo) {
            panelView->migrateTo(moveTo->id());
        } else {
            m_panels.removeAll(panelView);
            delete panelView;
        }
    }

    m_panelRelocationCandidates.clear();
}

PanelView *PlasmaApp::createPanelView(Plasma::Containment *containment)
{
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    const int id = viewIds.readEntry(QString::number(containment->id()), 0);
    PanelView *panelView = new PanelView(containment, id);

    connect(panelView, SIGNAL(destroyed(QObject*)), this, SLOT(panelRemoved(QObject*)));
    m_panels << panelView;
    panelView->show();
    setWmClass(panelView->winId());
    return panelView;
}

void PlasmaApp::createWaitingDesktops()
{
    const QList<QWeakPointer<Plasma::Containment> > containments = m_desktopsWaiting;
    m_desktopsWaiting.clear();

    foreach (QWeakPointer<Plasma::Containment> weakContainment, containments) {
        if (weakContainment) {
            Plasma::Containment *containment = weakContainment.data();
            KConfigGroup viewIds(KGlobal::config(), "ViewIds");
            const int id = viewIds.readEntry(QString::number(containment->id()), 0);

            const int desktop = AppSettings::perVirtualDesktopViews() ? containment->desktop() : -1;
            if (desktop >= KWindowSystem::numberOfDesktops()) {
                kDebug() << "not creating a view on desktop" << desktop << " as it does not exist";
                continue;
            }

            const int screen = containment->screen();
            if (screen >= m_corona->numScreens() || screen < 0) {
                kDebug() << "not creating a view on screen" << screen << "as it does not exist";
                continue;
            }

            DesktopView *view = viewForScreen(screen, desktop);

            if (view) {
                kDebug() << "already had a view for" << containment->screen() << containment->desktop();
                // we already have a view for this screen
                continue;
            }

            kDebug() << "creating a new view for" << containment->screen() << containment->desktop()
                     << "and we have" << m_corona->numScreens() << "screens";

            // we have a new screen. neat.
            view = new DesktopView(containment, id, 0);
            connect(view, SIGNAL(dashboardClosed()), this, SLOT(dashboardClosed()));
            if (m_corona) {
                connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                        view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
            }

            m_desktops.append(view);
            view->show();
            setWmClass(view->winId());
        }
    }
    setFixedDashboard(fixedDashboard());
}

void PlasmaApp::containmentAdded(Plasma::Containment *containment)
{
    if (isPanelContainment(containment)) {
        foreach (PanelView * panel, m_panels) {
            if (panel->containment() == containment) {
                kDebug() << "not creating second PanelView with existing Containment!!";
                return;
            }
        }
    }

    createView(containment);

    disconnect(containment, 0, this, 0);
    connect(containment, SIGNAL(configureRequested(Plasma::Containment*)),
            this, SLOT(configureContainment(Plasma::Containment*)));

    if ((containment->containmentType() == Plasma::Containment::DesktopContainment ||
         containment->containmentType() == Plasma::Containment::CustomContainment)) {
        QAction *a = containment->action("remove");
        delete a; //activities handle removal now
        if (!(m_loadingActivity.isEmpty() || m_corona->offscreenWidgets().contains(containment))) {
            Plasma::Context *context = containment->context();
            if (context->currentActivityId().isEmpty()) {
                //kDebug() << "@#$%@#$%@#$%@#$%#@$#@%@$#^%$&^$^$%#%$";
                //kDebug() << "script->containment->activity";
                Activity *activity = m_corona->activity(m_loadingActivity);
                Q_ASSERT(activity);
                activity->replaceContainment(containment);
            }
        }

        if (containment->containmentType() == Plasma::Containment::DesktopContainment) {
            foreach (QAction *action, m_corona->actions()) {
                containment->addToolBoxAction(action);
            }
        }
    }

    if (!isPanelContainment(containment) && !KAuthorized::authorize("editable_desktop_icons")) {
        containment->setImmutability(Plasma::SystemImmutable);
    }
}

void PlasmaApp::containmentScreenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment *containment)
{
    Q_UNUSED(wasScreen)
    kDebug() << "@@@was" << wasScreen << "is" << isScreen << (QObject*)containment << m_desktops.count();

    if (isScreen < 0) {
        kDebug() << "@@@screen<0";
        return;
    }

    if (isPanelContainment(containment)) {
        kDebug() << "@@@isPanel";
        return;
    }

    bool pvd = AppSettings::perVirtualDesktopViews();
    foreach (DesktopView *view, m_desktops) {
        if (view->screen() == isScreen && (!pvd || view->desktop() == containment->desktop())) {
            kDebug() << "@@@@found view" << view;
            return;
        }
    }

    kDebug() << "@@@@appending";
    m_desktopsWaiting.append(containment);
    m_desktopViewCreationTimer.start();
}

void PlasmaApp::configureContainment(Plasma::Containment *containment)
{
    const QString id = "plasma_containment_settings_" + QString::number(containment->id());
    BackgroundDialog *configDialog = qobject_cast<BackgroundDialog*>(KConfigDialog::exists(id));

    if (configDialog) {
        configDialog->reloadConfig();
    } else {
        const QSize resolution = QApplication::desktop()->screenGeometry(containment->screen()).size();
        Plasma::View *view = viewForScreen(containment->screen(), containment->desktop());

        if (!view) {
            view = viewForScreen(desktop()->screenNumber(QCursor::pos()), containment->desktop());

            if (!view) {
                if (m_desktops.count() < 1) {
                    return;
                }

                view = m_desktops.at(0);
            }

        }

        KConfigSkeleton *nullManager = new KConfigSkeleton(0);
        configDialog = new BackgroundDialog(resolution, containment, view, 0, id, nullManager);
        configDialog->setAttribute(Qt::WA_DeleteOnClose);


        // if our containment is a dashboard containment only, then we don't
        // want to mess with activities OR allow the user to change the containment type
        // doing so causes the dashboard view to lose its containment and renders it useless
        bool isDashboardContainment = fixedDashboard();
        if (isDashboardContainment) {
            bool found = false;
            foreach (DesktopView *view, m_desktops) {
                if (view->dashboardContainment() == containment) {
                    found = true;
                    break;
                }
            }

            isDashboardContainment = found;
        }

        if (isDashboardContainment) {
            configDialog->setLayoutChangeable(false);
        } else {
            Activity *activity = m_corona->activity(containment->context()->currentActivityId());
            Q_ASSERT(activity);
            connect(configDialog, SIGNAL(containmentPluginChanged(Plasma::Containment*)),
                    activity, SLOT(replaceContainment(Plasma::Containment*)));
        }

        connect(configDialog, SIGNAL(destroyed(QObject*)), nullManager, SLOT(deleteLater()));
    }

    configDialog->show();
    KWindowSystem::setOnDesktop(configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(configDialog->winId());
}

void PlasmaApp::cloneCurrentActivity()
{
    KActivities::Controller controller;
    //getting the current activity is *so* much easier than the current containment(s) :) :)
    QString oldId = controller.currentActivity();
    Activity *oldActivity = m_corona->activity(oldId);
    QString newId = controller.addActivity(i18nc("%1 is the activity name", "copy of %1", oldActivity->name()));

    QString file = "activities/";
    file += newId;
    KConfig external(file, KConfig::SimpleConfig, "appdata");

    //copy the old config to the new location
    oldActivity->save(external);
    //kDebug() << "saved to" << file;

    //load the new one
    controller.setCurrentActivity(newId);
}

//TODO accomodate activities
void PlasmaApp::setPerVirtualDesktopViews(bool perDesktopViews)
{
    if (perDesktopViews == perVirtualDesktopViews()) {
        return;
    }
    AppSettings::setPerVirtualDesktopViews(perDesktopViews);
    AppSettings::self()->writeConfig();

    disconnect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)),
               this, SLOT(checkVirtualDesktopViews(int)));

    m_pendingFixedDashboard = fixedDashboard();

    if (perDesktopViews) {
        connect(KWindowSystem::self(), SIGNAL(numberOfDesktopsChanged(int)),
                this, SLOT(checkVirtualDesktopViews(int)));
        checkVirtualDesktopViews(KWindowSystem::numberOfDesktops());
        setFixedDashboard(m_pendingFixedDashboard);
    } else {
        QList<DesktopView *> perScreenViews;
        foreach (DesktopView *view, m_desktops) {
            if (view->containment()) {
                view->containment()->setScreen(-1, -1);
            }

            delete view;
        }

        m_desktops.clear();
        m_corona->checkScreens(true);
    }
}

bool PlasmaApp::perVirtualDesktopViews() const
{
    return AppSettings::perVirtualDesktopViews();
}

void PlasmaApp::checkVirtualDesktopViews(int numDesktops)
{
    kDebug() << numDesktops;
    if (AppSettings::perVirtualDesktopViews()) {
        QMutableListIterator<DesktopView *> it(m_desktops);
        while (it.hasNext()) {
            DesktopView *view = it.next();
            if (!view->containment() || view->desktop() < 0 || view->desktop() >= numDesktops)  {
                delete view;
                it.remove();
            }
        }
    }

    m_corona->checkScreens(true);
}

void PlasmaApp::setFixedDashboard(bool fixedDashboard)
{
    //TODO: should probably have one dashboard containment per screen
    Plasma::Containment *c = 0;
    m_pendingFixedDashboard = fixedDashboard;
    if (fixedDashboard) {
        foreach (Plasma::Containment *possibility, m_corona->containments()) {
            if (possibility->pluginName() == "desktopDashboard") {
                c = possibility;
                break;
            }
        }

        if (!c) {
            //avoid the containmentAdded signal being emitted
            c = m_corona->addContainment("desktopDashboard");
        }

        //everything failed? probably a badly packaged plasma
        if (!c) {
            return;
        }
        m_corona->addOffscreenWidget(c);
    }

    QSize maxViewSize;
    foreach (DesktopView *view, m_desktops) {
        view->setDashboardContainment(c);
        if (view->size().width() > maxViewSize.width() && view->size().height() > maxViewSize.height()) {
            maxViewSize = view->size();
        }
    }

    if (fixedDashboard) {
        c->resize(maxViewSize);
    }

    m_corona->requestConfigSync();
}

bool PlasmaApp::fixedDashboard() const
{
    if (m_desktops.isEmpty()) {
        return m_pendingFixedDashboard;
    }

    foreach (DesktopView *view, m_desktops) {
        if (!view->dashboardFollowsDesktop()) {
            return true;
        }
    }

    return false;
}

void PlasmaApp::panelRemoved(QObject *panel)
{
    m_panels.removeAll((PanelView *)panel);
}

void PlasmaApp::remotePlasmoidAdded(Plasma::PackageMetadata metadata)
{
    //kDebug();
    if (m_desktops.isEmpty()) {
        return;
    }

    if (m_corona->immutability() == Plasma::SystemImmutable) {
        kDebug() << "Corona is system locked";
        return;
    }

    // the notification ptr is automatically delete when the notification is closed
    KNotification *notification = new KNotification("newplasmoid", m_desktops.at(0));
    notification->setText(i18n("A new widget has become available on the network:<br><b>%1</b> - <i>%2</i>",
                               metadata.name(), metadata.description()));

    // setup widget icon
    if (!metadata.icon().isEmpty()) {
        notification->setPixmap(KIcon(metadata.icon()).pixmap(IconSize(KIconLoader::Desktop)));
    }

    // locked, but the user is able to unlock
    if (m_corona->immutability() == Plasma::UserImmutable) {
        m_unlockCorona = true;
        notification->setActions(QStringList(i18n("Unlock and add to current activity")));
    } else {
        // immutability == Plasma::Mutable
        notification->setActions(QStringList(i18n("Add to current activity")));
    }

    m_mapper->setMapping(notification, metadata.remoteLocation().prettyUrl());
    connect(notification, SIGNAL(action1Activated()), m_mapper, SLOT(map()));

    kDebug() << "firing notification";
    notification->sendEvent();
}

void PlasmaApp::addRemotePlasmoid(const QString &location)
{
    if (m_unlockCorona) {
        m_unlockCorona = false;
        m_corona->setImmutability(Plasma::Mutable);
    }

    Plasma::AccessManager::self()->accessRemoteApplet(KUrl(location));
}

void PlasmaApp::plasmoidAccessFinished(Plasma::AccessAppletJob *job)
{
    if (m_desktops.isEmpty()) {
        return;
    }

    Plasma::Containment *c = m_desktops.at(0)->containment();
    if (c) {
        kDebug() << "adding applet";
        c->addApplet(job->applet(), QPointF(-1, -1), false);
    }
}

void PlasmaApp::createActivity(const QString &plugin)
{
    KActivities::Controller controller;
    QString id = controller.addActivity(i18nc("Default name for a new activity", "New Activity"));

    Activity *a = m_corona->activity(id);
    Q_ASSERT(a);
    a->setDefaultPlugin(plugin);

    controller.setCurrentActivity(id);
}

void PlasmaApp::createActivityFromScript(const QString &script, const QString &name, const QString &icon, const QStringList &startupApps)
{
    KActivities::Controller controller;
    m_loadingActivity = controller.addActivity(name);
    Activity *a = m_corona->activity(m_loadingActivity);
    Q_ASSERT(a);
    a->setIcon(icon);

    //kDebug() << "$$$$$$$$$$$$$$$$ begin script for" << m_loadingActivity;
    m_corona->evaluateScripts(QStringList() << script, false);
    //kDebug() << "$$$$$$$$$$$$$$$$ end script for" << m_loadingActivity;

    controller.setCurrentActivity(m_loadingActivity);
    m_loadingActivity.clear();

    KListConfirmationDialog * confirmDialog = new KListConfirmationDialog(
            i18n("Run applications"),
            i18n("This activity template requests to run the following applications"),
            i18n("Run selected"),
            i18n("Run none")
            );
    connect(confirmDialog, SIGNAL(selected(QList<QVariant>)),
            this, SLOT(executeCommands(QList<QVariant>)));

    foreach (const QString & exec, startupApps) {
        QString realExec = exec;

        #define LazyReplace(VAR, VAL) \
            if (realExec.contains(VAR)) realExec = realExec.replace(VAR, VAL);

        LazyReplace("$desktop",   KGlobalSettings::desktopPath());
        LazyReplace("$autostart", KGlobalSettings::autostartPath());
        LazyReplace("$documents", KGlobalSettings::documentPath());
        LazyReplace("$music",     KGlobalSettings::musicPath());
        LazyReplace("$video",     KGlobalSettings::videosPath());
        LazyReplace("$downloads", KGlobalSettings::downloadPath());
        LazyReplace("$pictures",  KGlobalSettings::picturesPath());

        QString name = realExec.split(" ")[0];

        KService::Ptr service = KService::serviceByDesktopName(name);

        if (service) {
            confirmDialog->addItem(KIcon(service->icon()), service->name(),
                    ((realExec == name) ? QString() : realExec), realExec, exec.split(" ").size() <= 2);
        } else {
            confirmDialog->addItem(KIcon("dialog-warning"), name,
                    ((realExec == name) ? QString() : realExec), realExec, false);
        }

        #undef LazyReplace
    }

    confirmDialog->exec();
}

void PlasmaApp::executeCommands(const QList < QVariant > & commands)
{
    foreach (const QVariant & command, commands) {
        KRun::runCommand(command.toString(), 0);
    }
}

#include "plasmaapp.moc"
