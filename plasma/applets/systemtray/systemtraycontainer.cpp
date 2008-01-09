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

// Qt
#include <QX11Info>

#include <plasma/theme.h>

// Xlib
#include <X11/Xlib.h>

SystemTrayContainer::SystemTrayContainer(WId clientId, QWidget *parent)
    : KX11EmbedContainer(parent)
{
    prepareFor( clientId ); // temporary hack, until QX11EmbedContainer gets fixed
    connect(this, SIGNAL(clientClosed()), SLOT(deleteLater()));
    connect(this, SIGNAL(error(QX11EmbedContainer::Error)), SLOT(handleError(QX11EmbedContainer::Error)));

    // Tray icons have a fixed size of 22x22
    setMinimumSize(22, 22);

    // Qt's regular quasi-transparent background doesn't work so set it to the
    // theme's background color instead.
    QPalette p = palette();
    p.setBrush(QPalette::Window, Plasma::Theme::self()->backgroundColor());
    setPalette(p);
    setBackgroundRole(QPalette::Window);

    kDebug() << "attempting to embed" << clientId;
    embedClient(clientId);

#if 0
    // BUG: error() sometimes return Unknown even on success
    if (error() == Unknown || error() == InvalidWindowID) {
        kDebug() << "embedding failed for" << clientId;
        deleteLater();
    }
#endif
}

void SystemTrayContainer::handleError(QX11EmbedContainer::Error error)
{
    Q_UNUSED(error);
    deleteLater();
}


// Temporary hack to change X window used by QX11EmbedContainer so that it matches
// the window embedded into it (#153193).
void KX11EmbedContainer::prepareFor( WId w )
{
    Display* dpy = QX11Info::display();
    XWindowAttributes ga;
    XGetWindowAttributes( dpy, w, &ga );
    XSetWindowAttributes sa;
    sa.background_pixel = WhitePixel( dpy, DefaultScreen( dpy ));
    sa.border_pixel = BlackPixel( dpy, DefaultScreen( dpy ));
    sa.colormap = ga.colormap;
    Window ww = XCreateWindow( dpy, parentWidget() ? parentWidget()->winId() : DefaultRootWindow( dpy ),
        1, 1, 1, 1, 0, ga.depth, InputOutput, ga.visual,
        CWBackPixel | CWBorderPixel | CWColormap, &sa );
    create( ww, true, true );
    // repeat everything from QX11EmbedContainer's ctor that might be relevant
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(true);
    setEnabled(false);
    XSelectInput( dpy, ww,
                 KeyPressMask | KeyReleaseMask
                 | ButtonPressMask | ButtonReleaseMask | ButtonMotionMask
                 | KeymapStateMask
                 | PointerMotionMask
                 | EnterWindowMask | LeaveWindowMask
                 | FocusChangeMask
                 | ExposureMask
                 | StructureNotifyMask
                 | SubstructureNotifyMask);
    XFlush( dpy );
}
