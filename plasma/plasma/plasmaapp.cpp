/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 Matt Broadstone <mbroadst@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

// plasma.loadEngine("hardware")
// LineGraph graph
// plasma.connect(graph, "hardware", "cpu");

#include "plasmaapp.h"

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

#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>

#include <ksmserver_interface.h>

#include "plasma/appletbrowser.h"
#include <plasma/corona.h>
#include <plasma/containment.h>
#include <plasma/theme.h>

#include "appadaptor.h"
#include "rootwidget.h"
#include "desktopview.h"
#include "panelview.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;

void checkComposite()
{
    dpy = XOpenDisplay(0); // open default display
    if (!dpy) {
        kError() << "Cannot connect to the X server" << endl;
        return;
    }

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
    }

    kDebug() << (colormap ? "Plasma has an argb visual" : "Plasma lacks an argb visual") << visual << colormap;
    kDebug() << ((KWindowSystem::compositingActive() && colormap) ? "Plasma can use COMPOSITE for effects"
                                                                    : "Plasma is COMPOSITE-less") << "on" << dpy;
}

PlasmaApp* PlasmaApp::self()
{
    if (!kapp) {
        checkComposite();
        return new PlasmaApp(dpy, visual ? Qt::HANDLE(visual) : 0, colormap ? Qt::HANDLE(colormap) : 0);
    }

    return qobject_cast<PlasmaApp*>(kapp);
}

PlasmaApp::PlasmaApp(Display* display, Qt::HANDLE visual, Qt::HANDLE colormap)
    : KUniqueApplication(display, visual, colormap),
      m_root(0),
      m_corona(0),
      m_appletBrowser(0)
{
    KGlobal::locale()->insertCatalog("libplasma");

    new AppAdaptor(this); 
    QDBusConnection::sessionBus().registerObject("/App", this);
    notifyStartup(false);

    // TODO: this same pattern is in KRunner (see workspace/krunner/restartingapplication.h)
    // would be interesting to see if this could be shared.
    if (!KCrash::crashHandler())
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 10s time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(10000, this, SLOT(setCrashHandler()));
    }
    else
    {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }

    // Enlarge application pixmap cache
    // Calculate the size required to hold background pixmaps for all screens.
    // Add 10% so that other (smaller) pixmaps can also be cached.
    int cacheSize = 0;
    QDesktopWidget *desktop = QApplication::desktop();
    for (int i = 0; i < desktop->numScreens(); i++) {
        QRect geometry = desktop->screenGeometry(i);
        cacheSize += 4 * geometry.width() * geometry.height() / 1024;
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

    KConfigGroup cg(KGlobal::config(), "General");
    Plasma::Theme::self()->setFont(cg.readEntry("desktopFont", font()));

    m_root = new RootWidget();
    m_root->setAsDesktop(KCmdLineArgs::parsedArgs()->isSet("desktop"));

    // this line initializes the corona.
    corona();

    notifyStartup(true);

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
    m_root->show();
}

PlasmaApp::~PlasmaApp()
{
    delete m_appletBrowser;
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveApplets();
    }

    delete m_root;
    m_root = 0;
    QList<PanelView*> panels = m_panels;
    m_panels.clear();
    qDeleteAll(panels);
    delete m_corona;
}

void PlasmaApp::initializeWallpaper()
{
    if (!m_root) {
        return;
    }

    //FIXME this has moved to Containment, so .....
    //m_root->desktop()->initializeWallpaper();
}

void PlasmaApp::toggleDashboard()
{
    if (m_root) {
        m_root->toggleDashboard();
    }
}

void PlasmaApp::setCrashHandler()
{
    KCrash::setEmergencySaveFunction(PlasmaApp::crashHandler);
}

void PlasmaApp::crashHandler(int signal)
{
    Q_UNUSED(signal);

    fprintf(stderr, "Plasma crashed, attempting to automatically recover\n");

    sleep(1);
    system("plasma --nocrashhandler &"); // try to restart
}

Plasma::Corona* PlasmaApp::corona()
{
    Q_ASSERT(m_root);

    if (!m_corona) {
        m_corona = new Plasma::Corona(this);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                m_root, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->loadApplets();
    }

    return m_corona;
}

void PlasmaApp::showAppletBrowser(Plasma::Containment *containment)
{
    if (!containment) {
        return;
    }

    if (!m_appletBrowser) {
        m_appletBrowser = new Plasma::AppletBrowser(containment);
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
    return colormap && KWindowSystem::compositingActive();
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
             << "| geometry:" << containment->geometry()
             << "| zValue:" << containment->zValue();

    switch (containment->containmentType()) {
        case Plasma::Containment::PanelContainment: {
            PanelView *panelView = new PanelView(containment);
            connect(panelView, SIGNAL(destroyed(QObject*)), this, SLOT(panelRemoved(QObject*)));
            m_panels << panelView;
            panelView->show();
            break;
        }
        default:
            if (containment->screen() > -1) {
                m_root->createDesktopView(containment->screen());
            }
            break;
    }
}

void PlasmaApp::panelRemoved(QObject* panel)
{
    m_panels.removeAll((PanelView*)panel);
}

#include "plasmaapp.moc"
