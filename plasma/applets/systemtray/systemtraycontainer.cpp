/***************************************************************************
 *   systemtraywidget.h                                                    *
 *                                                                         *
 *   Copyright (C) 2007 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

// Own
#include "systemtraycontainer.h"

// KDE
#include <KDebug>
#include <plasma/theme.h>

// Qt
#include <QX11Info>

// Xlib
#include <X11/Xlib.h>

SystemTrayContainer::SystemTrayContainer(QWidget *parent)
    : QX11EmbedContainer(parent)
{
    connect(this, SIGNAL(clientClosed()), SLOT(deleteLater()));
    connect(this, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(handleError(QX11EmbedContainer::Error)));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(updateBackground()));
    updateBackground();

    // Tray icons have a fixed size of 22x22
    setMaximumSize(22, 22);
}

void SystemTrayContainer::embedSystemTrayClient( WId clientId )
{
    kDebug() << "attempting to embed" << clientId;
    if( !prepareFor(clientId)) { // temporary hack, until QX11EmbedContainer gets fixed
         deleteLater();
         return;
    }

    embedClient(clientId);

    // check if we still have a valid clientId since there may cases where we don't any
    // longer after calling embedClient like e.g. if there is already a pidgin-instance
    // running and it got started again. In that case those guniqueapplication starts
    // and fires a SYSTEM_TRAY_REQUEST_DOCK with another clientId up, exists and passes
    // commandline-arguments on to the other running instance and embedClient does fail
    // without emitting a clientClosed() or error() signal.
    XWindowAttributes attr;
    if( !XGetWindowAttributes(QX11Info::display(), clientId, &attr) /*|| attr.map_state == IsUnmapped*/ ) {
        deleteLater();
    }
}

bool SystemTrayContainer::x11Event(XEvent *event)
{
    bool ok = QX11EmbedContainer::x11Event(event);
    if (event->type == ReparentNotify) {
        setMinimumSize(22,22);
    }
    return ok;
}

void SystemTrayContainer::updateBackground()
{
    // Qt's regular quasi-transparent background doesn't work so set it to the
    // theme's background color instead.
    QPalette p = palette();
    p.setBrush(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    setPalette(p);
    setBackgroundRole(QPalette::Window);
}

void SystemTrayContainer::handleError(QX11EmbedContainer::Error error)
{
    Q_UNUSED(error);
    deleteLater();
}

// Temporary hack to change X window used by QX11EmbedContainer so that it matches
// the window embedded into it (#153193).
bool SystemTrayContainer::prepareFor(WId w)
{
    Display* dpy = QX11Info::display();

    XWindowAttributes ga;
    if( !XGetWindowAttributes(dpy, w, &ga))
        return false;

    XSetWindowAttributes sa;
    sa.background_pixel = WhitePixel(dpy, DefaultScreen(dpy));
    sa.border_pixel = BlackPixel(dpy, DefaultScreen(dpy));
    sa.colormap = ga.colormap;

    Window ww = XCreateWindow(dpy, parentWidget() ? parentWidget()->winId() : DefaultRootWindow(dpy),
            0, 0, 22, 22, 0, ga.depth, InputOutput, ga.visual,
            CWBackPixel | CWBorderPixel | CWColormap, &sa);
    create(ww, true, true);

    // repeat everything from QX11EmbedContainer's ctor that might be relevant
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(true);
    setEnabled(false);

    XSelectInput(dpy, ww,
            KeyPressMask | KeyReleaseMask |
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
            KeymapStateMask |
            PointerMotionMask |
            EnterWindowMask | LeaveWindowMask |
            FocusChangeMask |
            ExposureMask |
            StructureNotifyMask |
            SubstructureNotifyMask);
    XFlush(dpy);
    return true;
}
