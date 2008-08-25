/*
 *   Copyright 2006-2008 Aaron Seigo <aseigo@kde.org>
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

#include <unistd.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmapCache>
#include <QTimer>
#include <QtDBus/QtDBus>

#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>
#include <KAction>

#include <ksmserver_interface.h>

#include <plasma/containment.h>
#include <plasma/theme.h>

#include "midcorona.h"
#include "midview.h"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;

void checkComposite()
{
#ifdef Q_WS_X11
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
#endif
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
      m_corona(0),
      m_mainView(0),
      m_isDesktop(false)
{
    KGlobal::locale()->insertCatalog("libplasma");

    bool isDesktop = KCmdLineArgs::parsedArgs()->isSet("desktop");
    if (isDesktop) {
        notifyStartup(false);
    }

    // TODO: this same pattern is in KRunner (see workspace/krunner/restartingapplication.h)
    // would be interesting to see if this could be shared.
    if (!KCrash::crashHandler())
    {
        // this means we've most likely crashed once. so let's see if we
        // stay up for more than 10s time, and if so reset the
        // crash handler since the crash isn't a frequent offender
        QTimer::singleShot(10000, this, SLOT(setCrashHandler()));
    } else {
        // See if a crash handler was installed. It was if the -nocrashhandler
        // argument was given, but the app eats the kde options so we can't
        // check that directly. If it wasn't, don't install our handler either.
        setCrashHandler();
    }

    //TODO: decide how to handle the cache size; possibilities:
    //      * % of ram, as in desktop
    //      * fixed size, hardcoded (uck)
    //      * optional size, specified on command line
    //      * optional size, in a config file
    //      * don't do anything special at all
    //QPixmapCache::setCacheLimit(cacheSize);

    KConfigGroup cg(KGlobal::config(), "General");
    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));

    setIsDesktop(isDesktop);

    // this line initializes the corona.
    corona();

    if (isDesktop) {
        notifyStartup(true);
    }

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
}

PlasmaApp::~PlasmaApp()
{
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    delete m_mainView;
    delete m_corona;

    //TODO: This manual sync() should not be necessary?
    syncConfig();
}

void PlasmaApp::syncConfig()
{
    KGlobal::config()->sync();
}

void PlasmaApp::setIsDesktop(bool isDesktop)
{
    m_isDesktop = isDesktop;
    if (m_mainView) {
        m_mainView->setIsDesktop(isDesktop);
    }

    if (isDesktop) {
        connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize(int)));
    } else {
        disconnect(QApplication::desktop(), SIGNAL(resized(int)), this, SLOT(adjustSize(int)));
    }
}

bool PlasmaApp::isDesktop() const
{
    return m_isDesktop;
}

void PlasmaApp::adjustSize(int screen)
{
    Q_UNUSED(screen)

    if (m_mainView) {
        //TODO: change size to be size of screen - the height of the panel m_mainView->adjustSize();
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
    if (!m_corona) {
        MidCorona *c = new MidCorona(this);
        connect(c, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(c, SIGNAL(configSynced()), this, SLOT(syncConfig()));

        c->setItemIndexMethod(QGraphicsScene::NoIndex);
        c->initializeLayout();
        m_corona = c;
    }

    return m_corona;
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

    if (!m_mainView && containment->id() == MidView::defaultId()) {
        m_mainView = new MidView(containment);

        if (m_corona) {
            connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                    m_mainView, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }

        m_mainView->setIsDesktop(m_isDesktop);
        m_mainView->show();

        //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
        //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
        //qapplication will manage apps with argb visuals in a better way
        XClassHint classHint;
        classHint.res_name = const_cast<char*>("Plasma");
        classHint.res_class = const_cast<char*>("Plasma");
        XSetClassHint(QX11Info::display(), m_mainView->winId(), &classHint);
    }
}

#include "plasmaapp.moc"
