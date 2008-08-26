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
#include <QVBoxLayout>
#include <QtDBus/QtDBus>

#include <KAction>
#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KStandardAction>
#include <KWindowSystem>

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

static const int CONTROL_BAR_HEIGHT = 22;

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
                                                                  : "Plasma is COMPOSITE-less")
             << "on" << dpy;

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
      m_window(0),
      m_controlBar(0),
      m_mainView(0),
      m_isDesktop(false)
{
    KGlobal::locale()->insertCatalog("libplasma");

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    bool isDesktop = args->isSet("desktop");
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

    m_window = new QWidget;

    //FIXME: if argb visuals enabled Qt will always set WM_CLASS as "qt-subapplication" no matter what
    //the application name is we set the proper XClassHint here, hopefully won't be necessary anymore when
    //qapplication will manage apps with argb visuals in a better way
    XClassHint classHint;
    classHint.res_name = const_cast<char*>("Plasma");
    classHint.res_class = const_cast<char*>("Plasma");
    XSetClassHint(QX11Info::display(), m_window->winId(), &classHint);

    QVBoxLayout *layout = new QVBoxLayout(m_window);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_controlBar = new MidView(0, m_window);
    m_controlBar->setFixedHeight(CONTROL_BAR_HEIGHT);
    m_controlBar->setBackgroundBrush(Qt::red);
    m_mainView = new MidView(0, m_window);

    layout->addWidget(m_controlBar);
    layout->addWidget(m_mainView);

    int width = 400;
    int height = 200;
    if (isDesktop) {
        //TODO: reserve a WM strut for m_controlBar
        QRect rect = desktop()->screenGeometry(0);
        width = rect.width();
        height = rect.height();
    } else {
        QAction *action = KStandardAction::quit(qApp, SLOT(quit()), m_window);
        m_window->addAction(action);

        QString geom = args->getOption("screen");
        int x = geom.indexOf('x');

        if (x > 0)  {
            width = qMax(width, geom.left(x).toInt());
            height = qMax(height, geom.right(geom.length() - x - 1).toInt());
        }
    }

    m_window->setFixedSize(width, height);
    m_mainView->setFixedSize(width, height - CONTROL_BAR_HEIGHT);

    // this line initializes the corona.
    corona();
    setIsDesktop(isDesktop);
    reserveStruts();

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

    delete m_window;
    m_window = 0;

    delete m_corona;
    m_corona = 0;

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

    if (isDesktop) {
        m_window->setWindowFlags(m_window->windowFlags() | Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_window->winId(), true);
        KWindowSystem::setType(m_window->winId(), NET::Desktop);
        m_window->lower();
        connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize(int)));
    } else {
        m_window->setWindowFlags(m_window->windowFlags() & ~Qt::FramelessWindowHint);
        KWindowSystem::setOnAllDesktops(m_window->winId(), false);
        KWindowSystem::setType(m_window->winId(), NET::Normal);
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

    QRect rect = desktop()->screenGeometry(0);
    int width = rect.width();
    int height = rect.height();
    m_window->setFixedSize(width, height);
    m_mainView->setFixedSize(width, height - CONTROL_BAR_HEIGHT);
    reserveStruts();
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

void PlasmaApp::reserveStruts()
{
    NETExtendedStrut strut;
    if (isDesktop()) {
        strut.top_width = m_controlBar->height();
        strut.top_start = m_window->x();
        strut.top_end = m_window->x() + m_window->width() - 1;
    }

    KWindowSystem::setExtendedStrut(m_window->winId(),
                                    strut.left_width, strut.left_start, strut.left_end,
                                    strut.right_width, strut.right_start, strut.right_end,
                                    strut.top_width, strut.top_start, strut.top_end,
                                    strut.bottom_width, strut.bottom_start, strut.bottom_end);
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new MidCorona(this);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), this, SLOT(syncConfig()));


        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        m_window->show();

        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                m_mainView, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));

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
    if (m_mainView && containment->id() == MidView::defaultId()) {
        kDebug() << "setting the mainview containment!";
        m_mainView->setContainment(containment);
    }
}

#include "plasmaapp.moc"
