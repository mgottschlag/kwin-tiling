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

Atom tag; //FIXME should this be a member var or what?
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
      m_view(0),
      m_configDialog(0)
{
    //load translations for libplasma
    KGlobal::locale()->insertCatalog("libplasma");
    KGlobal::locale()->insertCatalog("plasma-shells-common");

    new AppAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/App", this);

    //FIXME this is probably totally invalid
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
    Plasma::Theme::defaultTheme()->setFont(cg.readEntry("desktopFont", font()));
    m_activeOpacity = cg.readEntry("activeOpacity", 1.0);
    m_idleOpacity = cg.readEntry("idleOpacity", 1.0);

    if (cg.readEntry("forceNoComposite", false)) {
        composite = false;
    }

    //we have to keep an eye on created windows
    tag = XInternAtom(QX11Info::display(), "_KDE_SCREENSAVER_OVERRIDE", False);
    qApp->installEventFilter(this);

    // this line initializes the corona.
    corona();

    connect(QApplication::desktop(), SIGNAL(resized(int)), SLOT(adjustSize(int)));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

    setup(KCmdLineArgs::parsedArgs()->isSet("setup"));
}

PlasmaApp::~PlasmaApp()
{
    //TODO: This manual sync() should not be necessary. Remove it when
    // KConfig was fixed
    KGlobal::config()->sync();
}

void PlasmaApp::cleanup()
{
    if (m_corona) {
        m_corona->saveLayout();
    }

    delete m_view;
    delete m_corona;
}

void PlasmaApp::setActiveOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, m_activeOpacity)) {
        return;
    }
    m_activeOpacity = opacity;
    if (m_view) {
        //assume it's active, since things are happening
        m_view->setWindowOpacity(opacity);
    }
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("activeOpacity", opacity); //TODO trigger a save
}

void PlasmaApp::setIdleOpacity(qreal opacity)
{
    if (qFuzzyCompare(opacity, m_idleOpacity)) {
        return;
    }
    m_idleOpacity = opacity;
    KConfigGroup cg(KGlobal::config(), "General");
    cg.writeEntry("idleOpacity", opacity); //TODO trigger a save
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
    if (!m_view) {
        return;
    }

    if (activate) {
        m_view->setWindowOpacity(m_activeOpacity);
        m_view->showView();
        m_view->containment()->openToolBox();
    } else if (m_view->isVisible()) {
        if (qFuzzyCompare(m_idleOpacity + qreal(1.0), qreal(1.0))) {
            //opacity is 0
            m_view->hideView();
        } else {
            lock();
            m_view->setWindowOpacity(m_idleOpacity);
            m_view->containment()->closeToolBox();
        }
    } else {
        if (m_idleOpacity > 0) {
            m_view->setWindowOpacity(m_idleOpacity);
            m_view->showView();
        }
        lock();
    }
}

void PlasmaApp::adjustSize(int screen)
{
    if (! m_view) {
        return;
    }
    //FIXME someone needs to tell us what size to use if we've got >1 screen
    QDesktopWidget *desktop = QApplication::desktop();
    QRect geom = desktop->screenGeometry(0);
    m_view->setGeometry(geom);
}

Plasma::Corona* PlasmaApp::corona()
{
    if (!m_corona) {
        m_corona = new SaverCorona(this);
        connect(m_corona, SIGNAL(containmentAdded(Plasma::Containment*)),
                this, SLOT(createView(Plasma::Containment*)));
        //kDebug() << "connected to containmentAdded";
        /*
        foreach (DesktopView *view, m_desktops) {
            connect(c, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                            view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
        }*/

        m_corona->setItemIndexMethod(QGraphicsScene::NoIndex);
        m_corona->initializeLayout();

        //kDebug() << "layout should exist";
        //c->checkScreens();
    }

    return m_corona;
}

bool PlasmaApp::hasComposite()
{
    return composite;
}

//I think we need this for when the corona loads the default setup
//but maybe something simpler would suffice
void PlasmaApp::createView(Plasma::Containment *containment)
{
    kDebug() << "Containment name:" << containment->name()
             << "| type" << containment->containmentType()
             <<  "| screen:" << containment->screen()
             << "| geometry:" << containment->geometry()
             << "| zValue:" << containment->zValue();

    if (m_view) {
        // we already have a view for this screen
        return;
    }

    kDebug() << "creating a view for" << containment->screen() << "and we have"
        << QApplication::desktop()->numScreens() << "screens";

    // we have a new screen. neat.
    m_view = new SaverView(containment, 0);
                /*if (m_corona) {
                    connect(m_corona, SIGNAL(screenOwnerChanged(int,int,Plasma::Containment*)),
                            view, SLOT(screenOwnerChanged(int,int,Plasma::Containment*)));
                }*/
    //FIXME is this the right geometry for multi-screen?
    m_view->setGeometry(QApplication::desktop()->screenGeometry(containment->screen()));

    //FIXME why do I get BadWindow?
    //unsigned char data = VIEW;
    //XChangeProperty(QX11Info::display(), m_view->effectiveWinId(), tag, tag, 8, PropModeReplace, &data, 1);

    connect(containment, SIGNAL(locked()), SLOT(hideDialogs()));
    connect(containment, SIGNAL(locked()), m_view, SLOT(disableSetupMode()));
    connect(containment, SIGNAL(unlocked()), SLOT(showDialogs()));
    connect(containment, SIGNAL(configureRequested(Plasma::Containment*)),
            this, SLOT(configureContainment(Plasma::Containment*)));

    connect(m_view, SIGNAL(hidden()), SLOT(lock()));
    connect(m_view, SIGNAL(hidden()), SIGNAL(hidden()));

    kDebug() << "view created";
}

void PlasmaApp::setup(bool setupMode)
{
    kDebug() << setupMode;
    if (! m_view) {
        kDebug() << "too soon!!";
        return;
    }

    if (setupMode) {
        m_view->enableSetupMode();
        if (m_corona->immutability() == Plasma::UserImmutable) {
            m_corona->setImmutability(Plasma::Mutable);
        }
        setActive(true);
    } else {
        kDebug() << "checking lockprocess is still around";
        QDBusInterface lockprocess("org.kde.krunner_lock", "/LockProcess",
                "org.kde.krunner_lock.LockProcess", QDBusConnection::sessionBus(), this);
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
                    if (qobject_cast<Plasma::Dialog*>(widget)) {
                        //this is a terrible horrible hack that breaks extenders but it mostly works
                        //weird thing is, it sometimes makes the calendar popup too small.
                        newFlags = Qt::Popup;
                    } else {
                        //plasmadialogs can't handle direct input
                        //but configdialogs need it
                        m_dialogs.append(widget);
                        connect(widget, SIGNAL(destroyed(QObject*)), SLOT(dialogDestroyed(QObject*)));
                    }
                    widget->setWindowFlags(newFlags);
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
            kDebug() << "tagged" << widget << widget->effectiveWinId() << "as" << data;
        }
    }
    return false;
}

void PlasmaApp::dialogDestroyed(QObject *obj)
{
    m_dialogs.removeAll(qobject_cast<QWidget*>(obj));
    if (m_dialogs.isEmpty()) {
        if (m_view) {
            //this makes qactions work again
            m_view->activateWindow();
        }
    /*} else { failed attempt to fix kbd input after a subdialog closes
        QWidget *top = m_dialogs.last();
        top->activateWindow();
        kDebug() << top;*/
    }
}

void PlasmaApp::hideDialogs()
{
    foreach (QWidget *w, m_dialogs) {
        w->hide();
    }
    if (m_view) {
        m_view->hideAppletBrowser();
    }
    //FIXME where does the focus go?
}

void PlasmaApp::showDialogs()
{
    foreach (QWidget *w, m_dialogs) {
        w->show();
    }
    //FIXME where does the focus go?
}

void PlasmaApp::configureContainment(Plasma::Containment *containment)
{
    if (!m_view) {
        return;
    }

    if (m_configDialog) {
        m_configDialog->reloadConfig();
    } else {
        const QSize resolution = QApplication::desktop()->screenGeometry(containment->screen()).size();

        m_configDialog = new BackgroundDialog(resolution, containment, m_view);
        m_configDialog->setAttribute(Qt::WA_DeleteOnClose);
        connect(m_configDialog, SIGNAL(destroyed(QObject*)),
                this, SLOT(configDialogRemoved(QObject*)));
    }

    m_configDialog->show();
}

void PlasmaApp::configDialogRemoved(QObject* dialog)
{
    m_configDialog = 0;
}

void PlasmaApp::lock()
{
    if (corona() && corona()->immutability() == Plasma::Mutable) {
        hideDialogs();
        if (m_view) {
            m_view->disableSetupMode();
        }
        corona()->setImmutability(Plasma::UserImmutable);
    }
}

void PlasmaApp::quit()
{
    qApp->quit();
}

#include "plasmaapp.moc"
