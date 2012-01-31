/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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
#include <QtDBus/QtDBus>

//#include <KCrash>
#include <KDebug>
#include <KCmdLineArgs>
#include <KWindowSystem>

//#include <ksmserver_interface.h>

#include <Plasma/Containment>
#include <Plasma/Theme>
#include <Plasma/Dialog>

#include "appadaptor.h"
#include "savercorona.h"
#include "saverview.h"
#include "backgrounddialog.h"


#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <fixx11h.h>

Atom tag; //FIXME should this be a member var or what?
Atom tag2;
const unsigned char DIALOG = 1; //FIXME this is really bad code
const unsigned char VIEW = 2;

Display* dpy = 0;
Colormap colormap = 0;
Visual *visual = 0;
bool composite = false;

void checkComposite()
{
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

    composite = KWindowSystem::compositingActive() && colormap;

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
      m_corona(0),
      m_configDialog(0)
{
    //load translations for libplasma
    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasmagenericshell");

    new AppAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/App", this);

    //FIXME this is probably totally invalid
    // Enlarge application pixmap cache
    // Calculate the size required to hold background pixmaps for all screens.
    // Add 10% so that other (smaller) pixmaps can also be cached.
    int cacheSize = 0;
    QDesktopWidget *desktop = QApplication::desktop();
    int numScreens = desktop->numScreens();
    for (int i = 0; i < numScreens; i++) {
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
    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));
    m_activeOpacity = cg.readEntry("activeOpacity", 1.0);
    m_idleOpacity = cg.readEntry("idleOpacity", 1.0);

    if (cg.readEntry("forceNoComposite", false)) {
        composite = false;
    }

    //we have to keep an eye on created windows
    tag = XInternAtom(QX11Info::display(), "_KDE_SCREENSAVER_OVERRIDE", False);
    tag2 = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
    qApp->installEventFilter(this);

    // this line initializes the corona.
    corona();

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

    setup(KCmdLineArgs::parsedArgs()->isSet("setup"));
    
    m_viewCreationTimer.setSingleShot(true);
    m_viewCreationTimer.setInterval(0);
    connect(&m_viewCreationTimer, SIGNAL(timeout()), this, SLOT(createWaitingViews()));
}

PlasmaApp::~PlasmaApp()
{
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    qDeleteAll(m_views);
    delete m_corona;
    m_corona = 0;

    KGlobal::config()->sync();
}

void PlasmaApp::setActiveOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, m_activeOpacity)) {
        return;
    }
    m_activeOpacity = opacity;
    emit setViewOpacity(opacity);
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("activeOpacity", opacity);
    m_corona->requestConfigSync();
}

void PlasmaApp::createWaitingViews()
{
    const QList<QWeakPointer<Plasma::Containment> > containments = m_viewsWaiting;
    m_viewsWaiting.clear();
    foreach(QWeakPointer<Plasma::Containment> weakContainment, containments) {
        if (weakContainment) {
            Plasma::Containment *containment = weakContainment.data();
            
            KConfigGroup viewIds(KGlobal::config(), "ViewIds");
            
            // we have a new screen. neat.
            SaverView *view = viewForScreen(containment->screen());
            if (view) {
                return;
            }
            
            view = new SaverView(containment, 0);
            if (m_corona) {
                connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                        view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
                connect(m_corona, SIGNAL(shortcutsChanged()), view, SLOT(updateShortcuts()));
            }
            view->setGeometry(QApplication::desktop()->screenGeometry(containment->screen()));

            //FIXME why do I get BadWindow?
            //unsigned char data = VIEW;
            //XChangeProperty(QX11Info::display(), view->effectiveWinId(), tag, tag, 8, PropModeReplace, &data, 1);

            connect(containment, SIGNAL(configureRequested(Plasma::Containment*)),
                    this, SLOT(configureContainment(Plasma::Containment*)));

            //a hack to make sure the keyboard shortcut works
            view->addAction(corona()->action("unlock desktop"));
            view->addAction(corona()->action("unlock widgets"));
            m_views.append(view);
            connect(view, SIGNAL(hidden()), SLOT(lock()));
            connect(view, SIGNAL(hidden()), SIGNAL(hidden()));
            connect(this, SIGNAL(showViews()), view, SLOT(show()));
            connect(this, SIGNAL(hideViews()), view, SLOT(hide()));
            connect(this, SIGNAL(setViewOpacity(qreal)), view, SLOT(setOpacity(qreal)));
            connect(this, SIGNAL(enableSetupMode()), view, SLOT(disableSetupMode()));
            connect(this, SIGNAL(disableSetupMode()), view, SLOT(disableSetupMode()));
            connect(this, SIGNAL(openToolBox()), view, SLOT(openToolBox()));
            connect(this, SIGNAL(closeToolBox()), view, SLOT(closeToolBox()));
            connect(QApplication::desktop(), SIGNAL(resized(int)), view, SLOT(adjustSize(int)));
            emit(openToolBox());
            kDebug() << "view created";
        }
    }
    //activate the new views (yes, this is a lazy way to do it)
    setActive(m_active);
}

void PlasmaApp::setIdleOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, m_idleOpacity)) {
        return;
    }
    m_idleOpacity = opacity;
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("idleOpacity", opacity);
    m_corona->requestConfigSync();
}

qreal PlasmaApp::activeOpacity() const
{
    return m_activeOpacity;
}

qreal PlasmaApp::idleOpacity() const
{
    return m_idleOpacity;
}


void PlasmaApp::setActive(bool activate)
{
    m_active = activate;
    //note: allow this to run even if the value isn't changed,
    //because some views may need updating.
    if (activate) {
        emit setViewOpacity(m_activeOpacity);
        emit showViews();
        emit openToolBox();
    } else {
        if (qFuzzyCompare(m_idleOpacity + qreal(1.0), qreal(1.0))) {
            //opacity is 0
            emit hideViews();
        } else {
            lock();
            emit setViewOpacity(m_idleOpacity);
            emit showViews();
            emit closeToolBox();
        }
    }
}

void PlasmaApp::syncConfig()
{
    KGlobal::config()->sync();
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new SaverCorona(this);
        connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                this, SLOT(containmentScreenOwnerChanged(int,int,Plasma::Containment*)));
        connect(m_corona, SIGNAL(configSynced()), SLOT(syncConfig()));
        //kDebug() << "connected to containmentAdded";
        /*
        foreach (DesktopView *view, m_desktops) {
            connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                            view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }*/

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        //we want this *after* init so that we ignore any lock/unlock spasms that might happen then
        connect(m_corona, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)), this, SLOT(immutabilityChanged(Plasma::ImmutabilityType)));

        //kDebug() << "layout should exist";
        //c->checkScreens();
    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return composite;
}

void PlasmaApp::containmentScreenOwnerChanged(int wasScreen, int isScreen, Plasma::Containment *containment)
{
    Q_UNUSED(wasScreen);
    if (isScreen < 0)
        return;
    m_viewsWaiting.append(containment);
    m_viewCreationTimer.start();
}

void PlasmaApp::setup(bool setupMode)
{
    kDebug() << "setup mode:" << setupMode;

    if (setupMode) {
        emit enableSetupMode();
        if (m_corona->immutability() == Plasma::UserImmutable) {
            m_corona->setImmutability(Plasma::Mutable);
        }
        setActive(true);
    } else {
        kDebug() << "checking lockprocess is still around";
        QDBusInterface lockprocess("org.kde.screenlocker", "/LockProcess",
                "org.kde.screenlocker.LockProcess", QDBusConnection::sessionBus(), this);
        if (lockprocess.isValid()) {
            kDebug() << "success!";
            setActive(false);
        } else {
            kDebug() << "bailing out";
            qApp->quit(); //this failed once. why?
        }
    }
}

bool PlasmaApp::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        //apparently this means we created a new window
        //so, add a tag to prove it's our window
        //FIXME using the show event means we tag on every show, not just the first.
        //harmless but kinda wasteful.
        QWidget *widget = qobject_cast<QWidget*>(obj);
        if (widget && widget->isWindow() && !(qobject_cast<QDesktopWidget*>(widget) ||
                    widget->testAttribute(Qt::WA_DontShowOnScreen))) {
            unsigned char data = 0;
            if (qobject_cast<SaverView*>(widget)) {
                data = VIEW;
            } else if (m_dialogs.contains(widget)) {
                data = DIALOG;
            } else {
                Qt::WindowFlags oldFlags = widget->windowFlags();
                Qt::WindowFlags newFlags = oldFlags | Qt::X11BypassWindowManagerHint;
                if (oldFlags != newFlags) {
                    //now we're *really* fucking with things
                    //we force-disable window management and frames to cut off access to wm-y stuff
                    //and to make it easy to check the tag (frames are a pain)
                    kDebug() << "!!!!!!!setting flags on!!!!!" << widget;
                    QDesktopWidget *desktop = QApplication::desktop();
                    if (qobject_cast<Plasma::Dialog*>(widget)) {
                        //this is a terrible horrible hack that breaks extenders but it mostly works
                        //weird thing is, it sometimes makes the calendar popup too small.
                        newFlags = Qt::Popup;
                    } else {
                        //plasmadialogs can't handle direct input
                        //but configdialogs need it
                        m_dialogs.append(widget);
                        connect(widget, SIGNAL(destroyed(QObject*)), SLOT(dialogDestroyed(QObject*)));
                        connect(this, SIGNAL(showDialogs()), widget, SLOT(show()));
                        connect(this, SIGNAL(hideDialogs()), widget, SLOT(hide()));
                    }
                    widget->setWindowFlags(newFlags);
                    //we do not know the screen this widget should appear on
                    QRect availableGeometry = desktop->availableGeometry();
                    //move to the default screen
                    widget->move(availableGeometry.x(), availableGeometry.y());
                    widget->show(); //setting the flags hid it :(
                    //qApp->setActiveWindow(widget); //gives kbd but not mouse events
                    //kDebug() << "parent" << widget->parentWidget();
                    //FIXME why can I only activate these dialogs from this exact line?
                    widget->activateWindow(); //gives keyboard focus
                    return false; //we'll be back when we get the new show event
                } else {
                    widget->activateWindow(); //gives keyboard focus
                }
            }

            XChangeProperty(QX11Info::display(), widget->effectiveWinId(), tag, tag, 8, PropModeReplace, &data, 1);
            XChangeProperty(QX11Info::display(), widget->effectiveWinId(), tag2, tag2, 32, PropModeReplace, 0, 0);
            kDebug() << "tagged" << widget << widget->effectiveWinId() << "as" << data;
        }
    }
    return false;
}

void PlasmaApp::dialogDestroyed(QObject *obj)
{
    m_dialogs.removeAll(qobject_cast<QWidget*>(obj));
    //if (m_dialogs.isEmpty()) {
        //FIXME multiview
        //if (m_view) {
            //this makes qactions work again
            //m_view->activateWindow();
        //}
    /*} else { failed attempt to fix kbd input after a subdialog closes
        QWidget *top = m_dialogs.last();
        top->activateWindow();
        kDebug() << top;*/
    //}
}

void PlasmaApp::configureContainment(Plasma::Containment *containment)
{
//     SaverView *view = viewForScreen(containment->screen());
//     if (!view) {
//         return;
//     }

    if (m_configDialog) {
        m_configDialog->reloadConfig();
    } else {
        const QSize resolution = QApplication::desktop()->screenGeometry(containment->screen()).size();

        m_configDialog = new BackgroundDialog(resolution, containment);
        m_configDialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    m_configDialog->show();
}

void PlasmaApp::lock()
{
    kDebug() << "lock";
    if (corona() && corona()->immutability() == Plasma::Mutable) {
        corona()->setImmutability(Plasma::UserImmutable);
    }
}

void PlasmaApp::quit()
{
    qApp->quit();
}

void PlasmaApp::immutabilityChanged(Plasma::ImmutabilityType immutability)
{
    if (immutability == Plasma::Mutable) {
        emit showDialogs();
    } else {
        emit hideDialogs();
        emit hideWidgetExplorer();
        emit disableSetupMode();
    }
}

SaverView *PlasmaApp::viewForScreen(int screen)
{
    foreach(SaverView *view, m_views) {
        if (view->screen() == screen)
            return view;
    }
    return 0;
}

#include "plasmaapp.moc"
