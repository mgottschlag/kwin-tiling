/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 1999 Martin R. Jones <mjones@kde.org>
Copyright (C) 2002 Luboš Luňák <l.lunak@kde.org>
Copyright (C) 2003 Oswald Buddenhagen <ossi@kde.org>
Copyright (C) 2008 Chani Armitage <chanika@gmail.com>
Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "lockwindow.h"
#include "autologout.h"
#include "ksldapp.h"
// workspace
#include <kephal/kephal/screens.h>
// KDE
#include <KDE/KApplication>
#include <KDE/KDebug>
#include <KDE/KXErrorHandler>
// Qt
#include <QtCore/QTimer>
#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtGui/QX11Info>
// X11
#include <X11/Xatom.h>
#include <fixx11h.h>

static Window gVRoot = 0;
static Window gVRootData = 0;
static Atom   gXA_VROOT;
static Atom   gXA_SCREENSAVER_VERSION;

//#define CHECK_XSELECTINPUT
#ifdef CHECK_XSELECTINPUT
#include <dlfcn.h>
static bool check_xselectinput = false;
extern "C"
int XSelectInput( Display* dpy, Window w, long e )
{
    typedef int (*ptr)(Display*, Window, long);
    static ptr fun = NULL;
    if( fun == NULL )
        fun = (ptr)dlsym( RTLD_NEXT, "XSelectInput" );
    if( check_xselectinput && w == DefaultRootWindow( dpy ))
        kDebug() << kBacktrace();
    return fun( dpy, w, e );
}
#endif

namespace ScreenLocker
{

LockWindow::LockWindow()
    : QWidget()
    , m_autoLogoutTimer(new QTimer(this))
{
    initialize();
}

LockWindow::~LockWindow()
{
}

void LockWindow::initialize()
{
    kapp->installX11EventFilter(this);

    // Get root window size
    XWindowAttributes rootAttr;
    QX11Info info;
    XGetWindowAttributes(QX11Info::display(), RootWindow(QX11Info::display(),
                                                         info.screen()), &rootAttr);
    kapp->desktop(); // make Qt set its event mask on the root window first
    XSelectInput( QX11Info::display(), QX11Info::appRootWindow(),
                  SubstructureNotifyMask | rootAttr.your_event_mask );
#ifdef CHECK_XSELECTINPUT
    check_xselectinput = true;
#endif

    setGeometry(0, 0, rootAttr.width, rootAttr.height);
    // virtual root property
    gXA_VROOT = XInternAtom (QX11Info::display(), "__SWM_VROOT", False);
    gXA_SCREENSAVER_VERSION = XInternAtom (QX11Info::display(), "_SCREENSAVER_VERSION", False);

        // read the initial information about all toplevel windows
    Window r, p;
    Window* real;
    unsigned nreal;
    if( XQueryTree( x11Info().display(), x11Info().appRootWindow(), &r, &p, &real, &nreal )
        && real != NULL ) {
        KXErrorHandler err; // ignore X errors here
        for( unsigned i = 0; i < nreal; ++i ) {
            XWindowAttributes winAttr;
            if (XGetWindowAttributes(QX11Info::display(), real[ i ], &winAttr)) {
                WindowInfo info;
                info.window = real[ i ];
                info.viewable = ( winAttr.map_state == IsViewable );
                m_windowInfo.append( info ); // ordered bottom to top
            }
        }
        XFree( real );
    }
    m_autoLogoutTimer->setSingleShot(true);
    connect(m_autoLogoutTimer, SIGNAL(timeout()), SLOT(autoLogoutTimeout()));
}

void LockWindow::showLockWindow()
{
    Visual* visual = CopyFromParent;
    int depth = CopyFromParent;
    XSetWindowAttributes attrs;
    int flags = CWOverrideRedirect;
    attrs.override_redirect = 1;
    hide();
    Window w = XCreateWindow( x11Info().display(), RootWindow( x11Info().display(), x11Info().screen()),
        x(), y(), width(), height(), 0, depth, InputOutput, visual, flags, &attrs );

    create( w, false, true );

    // Some xscreensaver hacks check for this property
    const char *version = "KDE 4.0";

    XChangeProperty (QX11Info::display(), winId(),
                     gXA_SCREENSAVER_VERSION, XA_STRING, 8, PropModeReplace,
                     (unsigned char *) version, strlen(version));


    XSetWindowAttributes attr;
    attr.event_mask = KeyPressMask | ButtonPressMask | PointerMotionMask |
                        VisibilityChangeMask | ExposureMask;
    XChangeWindowAttributes(QX11Info::display(), winId(),
                            CWEventMask, &attr);

    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::transparent);
    setPalette(p);
    setAttribute(Qt::WA_PaintOnScreen, true);
    setAttribute(Qt::WA_NoSystemBackground, false);
    setAttribute(Qt::WA_PaintOutsidePaintEvent, true); // for bitBlt in resume()

    kDebug() << "Lock window Id: " << winId();

    move(0, 0);
    XSync(QX11Info::display(), False);

    setVRoot( winId(), winId() );
    if (KSldApp::self()->autoLogoutTimeout()) {
        m_autoLogoutTimer->start(KSldApp::self()->autoLogoutTimeout());
    }
}

//---------------------------------------------------------------------------
//
// Hide the screen locker window
//
void LockWindow::hideLockWindow()
{
  if (m_autoLogoutTimer->isActive()) {
      m_autoLogoutTimer->stop();
  }
  emit userActivity();
  hide();
  lower();
  removeVRoot(winId());
  XDeleteProperty(QX11Info::display(), winId(), gXA_SCREENSAVER_VERSION);
  if ( gVRoot ) {
      unsigned long vroot_data[1] = { gVRootData };
      XChangeProperty(QX11Info::display(), gVRoot, gXA_VROOT, XA_WINDOW, 32,
                      PropModeReplace, (unsigned char *)vroot_data, 1);
      gVRoot = 0;
  }
  XSync(QX11Info::display(), False);
}

//---------------------------------------------------------------------------
static int ignoreXError(Display *, XErrorEvent *)
{
    return 0;
}

//---------------------------------------------------------------------------
//
// Save the current virtual root window
//
void LockWindow::saveVRoot()
{
  Window rootReturn, parentReturn, *children;
  unsigned int numChildren;
  QX11Info info;
  Window root = RootWindowOfScreen(ScreenOfDisplay(QX11Info::display(), info.screen()));

  gVRoot = 0;
  gVRootData = 0;

  int (*oldHandler)(Display *, XErrorEvent *);
  oldHandler = XSetErrorHandler(ignoreXError);

  if (XQueryTree(QX11Info::display(), root, &rootReturn, &parentReturn,
      &children, &numChildren))
  {
    for (unsigned int i = 0; i < numChildren; i++)
    {
      Atom actual_type;
      int actual_format;
      unsigned long nitems, bytesafter;
      unsigned char *newRoot = 0;

      if ((XGetWindowProperty(QX11Info::display(), children[i], gXA_VROOT, 0, 1,
          False, XA_WINDOW, &actual_type, &actual_format, &nitems, &bytesafter,
          &newRoot) == Success) && newRoot)
      {
        gVRoot = children[i];
        Window *dummy = (Window*)newRoot;
        gVRootData = *dummy;
        XFree ((char*) newRoot);
        break;
      }
    }
    if (children)
    {
      XFree((char *)children);
    }
  }

  XSetErrorHandler(oldHandler);
}

//---------------------------------------------------------------------------
//
// Set the virtual root property
//
void LockWindow::setVRoot(Window win, Window vr)
{
    if (gVRoot)
        removeVRoot(gVRoot);

        QX11Info info;
    unsigned long rw = RootWindowOfScreen(ScreenOfDisplay(QX11Info::display(), info.screen()));
    unsigned long vroot_data[1] = { vr };

    Window rootReturn, parentReturn, *children;
    unsigned int numChildren;
    Window top = win;
    while (1) {
        if (!XQueryTree(QX11Info::display(), top , &rootReturn, &parentReturn,
                                 &children, &numChildren))
            return;
        if (children)
            XFree((char *)children);
        if (parentReturn == rw) {
            break;
        } else
            top = parentReturn;
    }

    XChangeProperty(QX11Info::display(), top, gXA_VROOT, XA_WINDOW, 32,
                     PropModeReplace, (unsigned char *)vroot_data, 1);
}

//---------------------------------------------------------------------------
//
// Remove the virtual root property
//
void LockWindow::removeVRoot(Window win)
{
    XDeleteProperty (QX11Info::display(), win, gXA_VROOT);
}
static void fakeFocusIn( WId window )
{
    // We have keyboard grab, so this application will
    // get keyboard events even without having focus.
    // Fake FocusIn to make Qt realize it has the active
    // window, so that it will correctly show cursor in the dialog.
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xfocus.display = QX11Info::display();
    ev.xfocus.type = FocusIn;
    ev.xfocus.window = window;
    ev.xfocus.mode = NotifyNormal;
    ev.xfocus.detail = NotifyAncestor;
    XSendEvent( QX11Info::display(), window, False, NoEventMask, &ev );
}

// Event filter
bool LockWindow::x11Event(XEvent* event)
{
    bool ret = false;
    switch (event->type) {
        case ButtonPress:
        case ButtonRelease:
        case KeyPress:
        case KeyRelease:
        case MotionNotify:
            if (KSldApp::self()->isGraceTime()) {
                KSldApp::self()->unlock();
                return true;
            }
            if (m_autoLogoutTimer->isActive()) {
                m_autoLogoutTimer->start(KSldApp::self()->autoLogoutTimeout());
            }
            emit userActivity();
            if (!m_lockWindows.isEmpty()) {
                XEvent ev2 = *event;
                Window root_return;
                int x_return, y_return;
                unsigned int width_return, height_return, border_width_return, depth_return;
                WId targetWindow = 0;
                KXErrorHandler err; // ignore X errors
                foreach (WId window, m_lockWindows) {
                    if (XGetGeometry(QX11Info::display(), window, &root_return,
                                &x_return, &y_return,
                                &width_return, &height_return,
                                &border_width_return, &depth_return)
                        &&
                        (event->xkey.x>=x_return && event->xkey.x<=x_return+(int)width_return)
                        &&
                        (event->xkey.y>=y_return && event->xkey.y<=y_return+(int)height_return) ) {
                        targetWindow = window;
                        ev2.xkey.window = ev2.xkey.subwindow = targetWindow;
                        ev2.xkey.x = event->xkey.x - x_return;
                        ev2.xkey.y = event->xkey.y - y_return;
                        break;
                    }
                }
                XSendEvent(QX11Info::display(), targetWindow, False, NoEventMask, &ev2);
                ret = true;
            }
            break;
        case ConfigureNotify: // from SubstructureNotifyMask on the root window
            if(event->xconfigure.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xconfigure.window );
                if( index >= 0 ) {
                    int index2 = event->xconfigure.above ? findWindowInfo( event->xconfigure.above ) : 0;
                    if( index2 < 0 )
                        kDebug(1204) << "Unknown above for ConfigureNotify";
                    else { // move just above the other window
                        if( index2 < index )
                            ++index2;
                        m_windowInfo.move( index, index2 );
                    }
                } else
                    kDebug(1204) << "Unknown toplevel for ConfigureNotify";
                //kDebug() << "ConfigureNotify:";
                //the stacking order changed, so let's change the stacking order again to what we want
                stayOnTop();
            }
            break;
        case MapNotify: // from SubstructureNotifyMask on the root window
            if( event->xmap.event == QX11Info::appRootWindow()) {
                kDebug(1204) << "MapNotify:" << event->xmap.window;
                int index = findWindowInfo( event->xmap.window );
                if( index >= 0 )
                    m_windowInfo[ index ].viewable = true;
                else
                    kDebug(1204) << "Unknown toplevel for MapNotify";
                KXErrorHandler err; // ignore X errors here
                if (isLockWindow(event->xmap.window)) {
                    if (m_lockWindows.contains(event->xmap.window)) {
                        kDebug() << "uhoh! duplicate!";
                    } else {
                        if (!isVisible()) {
                            // not yet shown and we have a lock window, so we show our own window
                            show();
                            setCursor(Qt::ArrowCursor);
                        }
                        m_lockWindows.prepend(event->xmap.window);
                        fakeFocusIn(event->xmap.window);
                    }
                }
                stayOnTop();
            }
            break;
        case UnmapNotify:
            if (event->xunmap.event == QX11Info::appRootWindow()) {
                kDebug(1204) << "UnmapNotify:" << event->xunmap.window;
                int index = findWindowInfo( event->xunmap.window );
                if( index >= 0 )
                    m_windowInfo[ index ].viewable = false;
                else
                    kDebug(1204) << "Unknown toplevel for MapNotify";
                m_lockWindows.removeAll(event->xunmap.window);
            }
            break;
        case CreateNotify:
            if (event->xcreatewindow.parent == QX11Info::appRootWindow()) {
                kDebug() << "CreateNotify:" << event->xcreatewindow.window;
                int index = findWindowInfo( event->xcreatewindow.window );
                if( index >= 0 )
                    kDebug() << "Already existing toplevel for CreateNotify";
                else {
                    WindowInfo info;
                    info.window = event->xcreatewindow.window;
                    info.viewable = false;
                    m_windowInfo.append( info );
                }
            }
            break;
        case DestroyNotify:
            if (event->xdestroywindow.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xdestroywindow.window );
                if( index >= 0 )
                    m_windowInfo.removeAt( index );
                else
                    kDebug() << "Unknown toplevel for DestroyNotify";
            }
            break;
        case ReparentNotify:
            if (event->xreparent.event == QX11Info::appRootWindow() && event->xreparent.parent != QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xreparent.window );
                if( index >= 0 )
                    m_windowInfo.removeAt( index );
                else
                    kDebug() << "Unknown toplevel for ReparentNotify away";
            } else if (event->xreparent.parent == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xreparent.window );
                if( index >= 0 )
                    kDebug() << "Already existing toplevel for ReparentNotify";
                else {
                    WindowInfo info;
                    info.window = event->xreparent.window;
                    info.viewable = false;
                    m_windowInfo.append( info );
                }
            }
            break;
        case CirculateNotify:
            if (event->xcirculate.event == QX11Info::appRootWindow()) {
                int index = findWindowInfo( event->xcirculate.window );
                if( index >= 0 ) {
                    m_windowInfo.move( index, event->xcirculate.place == PlaceOnTop ? m_windowInfo.size() - 1 : 0 );
                } else
                    kDebug() << "Unknown toplevel for CirculateNotify";
            }
            break;
    }
    return ret;
}

int LockWindow::findWindowInfo(Window w)
{
    for( int i = 0;
         i < m_windowInfo.size();
         ++i )
        if( m_windowInfo[ i ].window == w )
            return i;
    return -1;
}

void LockWindow::stayOnTop()
{

    // this restacking is written in a way so that
    // if the stacking positions actually don't change,
    // all restacking operations will be no-op,
    // and no ConfigureNotify will be generated,
    // thus avoiding possible infinite loops
    QVector< Window > stack( m_lockWindows.count() + 1 );
    int count = 0;
    foreach( WId w, m_lockWindows )
        stack[ count++ ] = w;
    // finally, the lock window
    stack[ count++ ] = winId();
    // do the actual restacking if needed
    XRaiseWindow( x11Info().display(), stack[ 0 ] );
    if( count > 1 )
        XRestackWindows( x11Info().display(), stack.data(), count );
}

bool LockWindow::isLockWindow(Window id)
{
    Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
    Atom actualType;
    int actualFormat;
    unsigned long nitems, remaining;
    unsigned char *data = 0;
    Display *display = QX11Info::display();

    int result = XGetWindowProperty(display, id, tag, 0, 1, False, tag, &actualType,
            &actualFormat, &nitems, &remaining, &data);

    bool lockWindow = false;
    if (result == Success && actualType == tag) {
        lockWindow = true;
    }
    if (data) {
        XFree(data);
    }
    return lockWindow;
}

void LockWindow::autoLogoutTimeout()
{
    QPointer<AutoLogout> dlg = new AutoLogout(this);
    dlg->adjustSize();

    int screen = Kephal::ScreenUtils::primaryScreenId();
    if (Kephal::ScreenUtils::numScreens() > 1) {
        screen = Kephal::ScreenUtils::screenId(QCursor::pos());
    }

    const QRect screenRect = Kephal::ScreenUtils::screenGeometry(screen);
    QRect rect = dlg->geometry();
    rect.moveCenter(screenRect.center());
    dlg->move(rect.topLeft());
    Atom tag = XInternAtom(QX11Info::display(), "_KDE_SCREEN_LOCKER", False);
    XChangeProperty(QX11Info::display(), dlg->winId(), tag, tag, 32, PropModeReplace, 0, 0);
    dlg->exec();
    delete dlg;
    // start the timer again - only if the window is still shown
    if (isVisible()) {
        m_autoLogoutTimer->start(KSldApp::self()->autoLogoutTimeout());
    }
}

void LockWindow::paintEvent(QPaintEvent* )
{
    QPainter p(this);
    p.setBrush(QBrush(Qt::black));
    p.drawRect(geometry());
}

}
