/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
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
#include <QTimer>
#include <QtDBus/QtDBus>

#include <KAction>
#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KSelectionWatcher>
#include <KWindowSystem>

#include <ksmserver_interface.h>

#include <Plasma/Containment>
#include <Plasma/Theme>

#include "appletbrowser.h"
#include "appadaptor.h"
#include "backgrounddialog.h"
#include "desktopcorona.h"
#include "desktopview.h"
#include "panelview.h"
#include "plasma-shell-desktop.h"

#include <kephal/screens.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;
#endif

void checkComposite()
{
#ifdef Q_WS_X11
    dpy = XOpenDisplay(0); // open default display
    if (!dpy) {
        kError() << "Cannot connect to the X server" << endl;
        return;
    }
    if( qgetenv( "KDE_SKIP_ARGB_VISUALS" ) == "1" )
        return;

    int screen = DefaultScreen(dpy);
    int eventBase, errorBase;

    if (XRenderQueryExtension(dpy, &eventBase, &errorBase)) {
        int nvi;
        XVisualInfo templ;
        templ.screen  = screen;
        templ.depth   = 32;
        templ.c_class = TrueColor;
        XVisualInfo *xvi = XGetVisualInfo(dpy,
                                          VisualScreenMask | VisualDepthMask | VisualClassMask,
                                          &templ, &nvi);
        for (int i = 0; i < nvi; ++i) {
            XRenderPictFormat *format = XRenderFindVisualFormat(dpy, xvi[i].visual);
            if (format->type == PictTypeDirect && format->direct.alphaMask) {
                visual = xvi[i].visual;
                colormap = XCreateColormap(dpy, RootWindow(dpy, screen), visual, AllocNone);
                break;
            }
        }
        XFree(xvi);
    }

    kDebug() << (colormap ? "Plasma has an argb visual" : "Plasma lacks an argb visual") << visual << colormap;
    kDebug() << ((KWindowSystem::compositingActive() && colormap) ? "Plasma can use COMPOSITE for effects"
                                                                    : "Plasma is COMPOSITE-less") << "on" << dpy;
#endif
}

PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        checkComposite();
#ifdef Q_WS_X11
        return new PlasmaApp(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
#else
        return new PlasmaApp(0, 0, 0);
#endif
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap)
#ifdef Q_WS_X11
    : KUniqueApplication(display, visual, colormap),
#else
    : KUniqueApplication(),
#endif
      m_corona(0),
      m_appletBrowser(0),
      m_zoomLevel(Plasma::DesktopZoom),
      m_panelHidden(0)
{
    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasma-shells-common");
    KCrash::setFlags(KCrash::AutoRestart);

    new PlasmaAppAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/App", this);
    notifyStartup(false);

    // Enlarge application pixmap cache
    // Calculate the size required to hold background pixmaps for all screens.
    // Add 10% so that other (smaller) pixmaps can also be cached.
    int cacheSize = 0;
    for (int i = 0; i < Kephal::ScreenUtils::numScreens(); i++) {
        QSize size = Kephal::ScreenUtils::screenSize(i);
        cacheSize += 4 * size.width() * size.height() / 1024;
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

    //TODO: Make the shortcut configurable
    KAction *showAction = new KAction( this );
    showAction->setText( i18n( "Show Dashboard" ) );
    showAction->setObjectName( "Show Dashboard" ); // NO I18N
    showAction->setGlobalShortcut( KShortcut( Qt::CTRL + Qt::Key_F12 ) );
    connect( showAction, SIGNAL( triggered() ), this, SLOT( toggleDashboard() ) );

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    QTimer::singleShot(0, this, SLOT(setupDesktop()));

#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    int screen = DefaultScreen(dpy);
    char net_wm_cm_name[100];
    sprintf(net_wm_cm_name, "_NET_WM_CM_S%d", screen);
    m_compositeWatch = new KSelectionWatcher(net_wm_cm_name, -1, this);
    connect(m_compositeWatch, SIGNAL(newOwner(Window)), this, SLOT(compositingChanged()));
    connect(m_compositeWatch, SIGNAL(lostOwner()), this, SLOT(compositingChanged()));
#endif
}

PlasmaApp::~PlasmaApp()
{
    delete m_appletBrowser;
}

void PlasmaApp::setupDesktop()
{
    // intialize the default theme and set the font
    Plasma::Theme::defaultTheme()->setFont(AppSettings::desktopFont());

    // this line initializes the corona.
    corona();

    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenRemoved(int)), SLOT(screenRemoved(int)));

    // and now, let everyone know we're ready!
    notifyStartup(true);
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

#ifdef Q_WS_X11
    delete m_compositeWatch;
    m_compositeWatch = 0;
#endif

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

    QHash<Plasma::Containment *, BackgroundDialog *> dialogs = m_configDialogs;
    m_configDialogs.clear();
    qDeleteAll(dialogs);

    delete m_corona;

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
    int currentScreen = 0;
    if (Kephal::ScreenUtils::numScreens() > 1) {
        currentScreen = Kephal::ScreenUtils::screenId(QCursor::pos());
    }

    int currentDesktop = -1;
    if (AppSettings::perVirtualDesktopViews()) {
        currentDesktop = KWindowSystem::currentDesktop();
    }

    DesktopView *view = viewForScreen(currentScreen, currentDesktop);
    if (!view) {
        kWarning() << "we don't have a DesktopView for the current screen!" << currentScreen << currentDesktop;
        return;
    }

    view->toggleDashboard();
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

Plasma::ZoomLevel PlasmaApp::desktopZoomLevel() const
{
    return m_zoomLevel;
}

QList<PanelView*> PlasmaApp::panelViews() const
{
    return m_panels;
}

void PlasmaApp::compositingChanged()
{
#ifdef Q_WS_X11
    foreach (PanelView *panel, m_panels) {
        panel->recreateUnhideTrigger();
    }
#endif
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
    if (m_panelHidden && event->xany.send_event != True &&
        (event->type == EnterNotify || event->type == MotionNotify)) {
        PanelView *panel = findPanelForTrigger(event->xcrossing.window);
        if (panel) {
            if (event->type == EnterNotify) {
                panel->hintOrUnhide(QPoint());
            }
            //FIXME: this if it was possible to avoid the polling
            /*else if (event->type == LeaveNotify) {
                panel->unhintHide();
            }*/
            else if (event->type == MotionNotify) {
                XMotionEvent *motion = (XMotionEvent*)event;
                //kDebug() << "motion" << motion->x << motion->y << panel->location();
                panel->hintOrUnhide(QPoint(motion->x_root, motion->y_root));
            }

            return true;
        }
    }

    return KUniqueApplication::x11EventFilter(event);
}
#endif

void PlasmaApp::screenRemoved(int id)
{
    kDebug() << id;
    QMutableListIterator<DesktopView *> it(m_desktops);
    while (it.hasNext()) {
        DesktopView *view = it.next();
        if (view->screen() == id) {
            // the screen was removed, so we'll destroy the
            // corresponding view
            kDebug() << "removing the view for screen" << id;
            view->setContainment(0);
            it.remove();
            delete view;
        }
    }

    /*
    TODO: remove panels when screen goes away.
          first, however, we need to be able to reserve and restore the panelsettings
          even when the view itself goes away
    QMutableListIterator<PanelView*> it(m_panels);
    while (it.hasNext()) {
        PanelView *panel = it.next();
        if (panel->screen() == i) {
            delete panel;
            it.remove();
        }
    }
    */
}

DesktopView* PlasmaApp::viewForScreen(int screen, int desktop) const
{
    foreach (DesktopView *view, m_desktops) {
        //kDebug() << "comparing" << view->screen() << screen;
        if (view->screen() == screen && (desktop < 0 || view->desktop() == desktop)) {
            return view;
        }
    }

    return 0;
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        QTime t;
        t.start();
        DesktopCorona *c = new DesktopCorona(this);
        connect(c, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(containmentAdded(Plasma::Containment*)));
        connect(c, SIGNAL(configSynced()), this, SLOT(syncConfig()));

        foreach (DesktopView *view, m_desktops) {
            connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                    view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }

        c->setItemIndexMethod(QGraphicsScene::NoIndex);
        c->initializeLayout();
        c->checkScreens();
        m_corona = c;
        kDebug() << " ------------------------------------------>" << t.elapsed();
    }

    return m_corona;
}

void PlasmaApp::showAppletBrowser()
{
    Plasma::Containment *containment = dynamic_cast<Plasma::Containment *>(sender());

    if (!containment) {
        return;
    }

    foreach (DesktopView *view, m_desktops) {
        if (view->containment() == containment && view->isDashboardVisible()) {
            // the dashboard will pick this one up!
            return;
        }
    }

    showAppletBrowser(containment);
}

void PlasmaApp::showAppletBrowser(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser();
        m_appletBrowser->setContainment(containment);
        m_appletBrowser->setApplication();
        m_appletBrowser->setAttribute(Qt::WA_DeleteOnClose);
        m_appletBrowser->setWindowTitle(i18n("Add Widgets"));
        m_appletBrowser->setWindowIcon(KIcon("plasmagik"));
        connect(m_appletBrowser, SIGNAL(destroyed()), this, SLOT(appletBrowserDestroyed()));
    } else {
        m_appletBrowser->setContainment(containment);
    }

    KWindowSystem::setOnDesktop(m_appletBrowser->winId(), KWindowSystem::currentDesktop());
    m_appletBrowser->show();
    KWindowSystem::activateWindow(m_appletBrowser->winId());
}

void PlasmaApp::appletBrowserDestroyed()
{
    m_appletBrowser = 0;
}

bool PlasmaApp::hasComposite()
{
//    return true;
#ifdef Q_WS_X11
    return colormap && KWindowSystem::compositingActive();
#else
    return false;
#endif
}

void PlasmaApp::notifyStartup(bool completed)
{
    org::kde::KSMServerInterface ksmserver("org.kde.ksmserver", "/KSMServer", QDBusConnection::sessionBus());

    const QString startupID("workspace desktop");
    if (completed) {
        ksmserver.resumeStartup(startupID);
    } else {
        ksmserver.suspendStartup(startupID);
    }
}

void PlasmaApp::createView(Plasma::Containment *containment)
{
    kDebug() << "Containment name:" << containment->name()
             << "| type" << containment->containmentType()
             <<  "| screen:" << containment->screen()
             <<  "| desktop:" << containment->desktop()
             << "| geometry:" << containment->geometry()
             << "| zValue:" << containment->zValue();

    // find the mapping of View to Containment, if any,
    // so we can restore things on start.
    KConfigGroup viewIds(KGlobal::config(), "ViewIds");
    int id = viewIds.readEntry(QString::number(containment->id()), 0);

    WId viewWindow = 0;

    switch (containment->containmentType()) {
        case Plasma::Containment::PanelContainment: {
            PanelView *panelView = new PanelView(containment, id);
            viewWindow = panelView->winId();
            connect(panelView, SIGNAL(destroyed(QObject*)), this, SLOT(panelRemoved(QObject*)));
            m_panels << panelView;
            panelView->show();
            break;
        }
        default:
            if (containment->screen() > -1 &&
                containment->screen() < Kephal::ScreenUtils::numScreens()) {
                DesktopView *view = viewForScreen(containment->screen(), containment->desktop());
                if (view) {
                    kDebug() << "had a view for" << containment->screen() << containment->desktop();
                    // we already have a view for this screen
                    return;
                }

                kDebug() << "creating a new view for" << containment->screen() << containment->desktop()
                         << "and we have" << Kephal::ScreenUtils::numScreens() << "screens";

                // we have a new screen. neat.
                view = new DesktopView(containment, id, 0);
                viewWindow = view->winId();
                if (m_corona) {
                    connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                            view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
                }

                m_desktops.append(view);
                view->show();
            }
            break;
    }

#ifdef Q_WS_X11
    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argvisuals in a better way
    if (viewWindow) {
        XClassHint classHint;
        classHint.res_name = const_cast<char*>("Plasma");
        classHint.res_class = const_cast<char*>("Plasma");
        XSetClassHint(QX11Info::display(), viewWindow, &classHint);
    }
#endif
}

void PlasmaApp::containmentAdded(Plasma::Containment *containment)
{
    createView(containment);
    disconnect(containment, 0, this, 0);
    connect(containment, SIGNAL(zoomRequested(Plasma::Containment*,Plasma::ZoomDirection)),
            this, SLOT(zoom(Plasma::Containment*,Plasma::ZoomDirection)));
    connect(containment, SIGNAL(showAddWidgetsInterface(QPointF)), this, SLOT(showAppletBrowser()));
    connect(containment, SIGNAL(configureRequested(Plasma::Containment*)),
            this, SLOT(configureContainment(Plasma::Containment*)));

    if (containment->containmentType() != Plasma::Containment::PanelContainment) {
        connect(containment, SIGNAL(addSiblingContainment(Plasma::Containment *)),
                this, SLOT(addContainment(Plasma::Containment *)));
    }
}

void PlasmaApp::configureContainment(Plasma::Containment *containment)
{
    BackgroundDialog *configDialog = 0;

    if (m_configDialogs.contains(containment)) {
        configDialog = m_configDialogs.value(containment);
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

        configDialog = new BackgroundDialog(resolution, containment, view);
        configDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(configDialog, SIGNAL(destroyed(QObject*)),
                this, SLOT(configDialogRemoved(QObject*)));
    }

    configDialog->show();
    KWindowSystem::setOnDesktop(configDialog->winId(), KWindowSystem::currentDesktop());
    KWindowSystem::activateWindow(configDialog->winId());
}

void PlasmaApp::addContainment(Plasma::Containment *fromContainment)
{
    QString plugin = fromContainment ? fromContainment->pluginName() : QString();
    Plasma::Containment *c = m_corona->addContainment(plugin);

    if (c && fromContainment) {
        foreach (DesktopView *view, m_desktops) {
            if (view->containment() == c){
                view->setContainment(c);
                break;
            }
        }
    }
}

void PlasmaApp::zoom(Plasma::Containment *containment, Plasma::ZoomDirection direction)
{
    if (direction == Plasma::ZoomIn) {
        zoomIn(containment);
        foreach (DesktopView *view, m_desktops) {
            view->zoomIn(m_zoomLevel);
        }

        if (m_zoomLevel == Plasma::DesktopZoom) {
            int currentDesktop = -1;
            if (AppSettings::perVirtualDesktopViews()) {
                currentDesktop = KWindowSystem::currentDesktop();
            }

            DesktopView *view = viewForScreen(desktop()->screenNumber(QCursor::pos()), currentDesktop);

            if (view && view->containment() != containment) {
                // zooming in all the way, so lets swap containments about if need be
                view->setContainment(containment);
            }
        }
    } else if (direction == Plasma::ZoomOut) {
        zoomOut(containment);
        foreach (DesktopView *view, m_desktops) {
            view->zoomOut(m_zoomLevel);
        }
    }
}

void PlasmaApp::zoomIn(Plasma::Containment *containment)
{
    bool zoomIn = true;
    bool zoomOut = true;
    bool addSibling = true;
    bool lock = false;
    bool remove = false;

    if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::DesktopZoom;
        containment->closeToolBox();
        addSibling = false;
        zoomIn = false;
        lock = true;
    } else if (m_zoomLevel == Plasma::OverviewZoom) {
        m_zoomLevel = Plasma::GroupZoom;
        remove = true;
    }

    //make sure everybody can zoom out again
    foreach (Plasma::Containment *c, m_corona->containments()) {
        if (c->containmentType() == Plasma::Containment::PanelContainment) {
            continue;
        }

        c->enableAction("zoom in", zoomIn);
        c->enableAction("zoom out", zoomOut);
        c->enableAction("add sibling containment", addSibling);
        c->enableAction("lock widgets", lock);
        c->enableAction("remove", remove && (c->screen() == -1));
        c->enableAction("add widgets", true);
    }
}

void PlasmaApp::zoomOut(Plasma::Containment *)
{
    bool zoomIn = true;
    bool zoomOut = true;
    bool addSibling = true;
    bool lock = false;
    bool addWidgets = true;

    if (m_zoomLevel == Plasma::DesktopZoom) {
        m_zoomLevel = Plasma::GroupZoom;
    } else if (m_zoomLevel == Plasma::GroupZoom) {
        m_zoomLevel = Plasma::OverviewZoom;
        zoomOut = false;
        addWidgets = false;
    }

    //make sure everybody can zoom out again
    foreach (Plasma::Containment *c, m_corona->containments()) {
        if (c->containmentType() == Plasma::Containment::PanelContainment) {
            continue;
        }

        c->enableAction("zoom in", zoomIn);
        c->enableAction("zoom out", zoomOut);
        c->enableAction("add sibling containment", addSibling);
        c->enableAction("lock widgets", lock);
        c->enableAction("remove", c->screen() == -1);
        c->enableAction("add widgets", addWidgets);
    }
}

void PlasmaApp::panelRemoved(QObject* panel)
{
    m_panels.removeAll((PanelView*)panel);
}

void PlasmaApp::configDialogRemoved(QObject* dialog)
{
    QMutableHashIterator<Plasma::Containment *, BackgroundDialog *> it(m_configDialogs);
    while (it.hasNext()) {
        it.next();
        if (it.value() == (BackgroundDialog*)dialog) {
           it.remove();
        }
    }
}

#include "plasmaapp.moc"
