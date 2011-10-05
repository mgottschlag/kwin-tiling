/***************************************************************************
 *   x11embedcontainer.cpp                                                 *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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

#include "x11embedcontainer.h"
#include "x11embedpainter.h"
#include "fdoselectionmanager.h"

// KDE
#include <KDebug>

// Qt
#include <QtGui/QPainter>
#include <QtGui/QPaintEngine>
#include <QtGui/QX11Info>

// Xlib
#include <config-X11.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

#ifdef HAVE_XCOMPOSITE
#  include <X11/extensions/Xcomposite.h>
#endif


namespace SystemTray
{

class X11EmbedContainer::Private
{
public:
    Private(X11EmbedContainer *q)
        : q(q),
          picture(None),
          updatesEnabled(true)
    {
    }

    ~Private()
    {
        if (picture) {
           XRenderFreePicture(QX11Info::display(), picture);
        }
    }

    X11EmbedContainer *q;

    XWindowAttributes attr;
    Picture picture;
    bool updatesEnabled;
    QImage oldBackgroundImage;
};


X11EmbedContainer::X11EmbedContainer(QWidget *parent)
    : QX11EmbedContainer(parent),
      d(new Private(this))
{
    connect(this, SIGNAL(clientIsEmbedded()),
            this, SLOT(ensureValidSize()));
}


X11EmbedContainer::~X11EmbedContainer()
{
    FdoSelectionManager::manager()->removeDamageWatch(this);
    delete d;
}


void X11EmbedContainer::embedSystemTrayClient(WId clientId)
{
    Display *display = QX11Info::display();

    if (!XGetWindowAttributes(display, clientId, &d->attr)) {
        emit error(QX11EmbedContainer::Unknown);
        return;
    }

    XSetWindowAttributes sAttr;
    sAttr.background_pixel = BlackPixel(display, DefaultScreen(display));
    sAttr.border_pixel = BlackPixel(display, DefaultScreen(display));
    sAttr.colormap = d->attr.colormap;

    WId parentId = parentWidget() ? parentWidget()->winId() : DefaultRootWindow(display);
    Window winId = XCreateWindow(display, parentId, 0, 0, d->attr.width, d->attr.height,
                                 0, d->attr.depth, InputOutput, d->attr.visual,
                                 CWBackPixel | CWBorderPixel | CWColormap, &sAttr);

    XWindowAttributes attr;
    if (!XGetWindowAttributes(display, winId, &attr)) {
        emit error(QX11EmbedContainer::Unknown);
        return;
    }

    create(winId);

#if defined(HAVE_XCOMPOSITE) && defined(HAVE_XFIXES) && defined(HAVE_XDAMAGE)
    XRenderPictFormat *format = XRenderFindVisualFormat(display, d->attr.visual);
    if (format && format->type == PictTypeDirect && format->direct.alphaMask &&
        FdoSelectionManager::manager()->haveComposite())
    {
        // Redirect ARGB windows to offscreen storage so we can composite them ourselves
        XRenderPictureAttributes attr;
        attr.subwindow_mode = IncludeInferiors;

        d->picture = XRenderCreatePicture(display, clientId, format, CPSubwindowMode, &attr);
        XCompositeRedirectSubwindows(display, winId, CompositeRedirectManual);
        FdoSelectionManager::manager()->addDamageWatch(this, clientId);

        //kDebug() << "Embedded client uses an ARGB visual -> compositing.";
    } else {
        //kDebug() << "Embedded client is not using an ARGB visual.";
    }
#endif

    // repeat everything from QX11EmbedContainer's ctor that might be relevant
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAcceptDrops(true);
    setEnabled(false);

    XSelectInput(display, winId,
                 KeyPressMask | KeyReleaseMask |
                 ButtonPressMask | ButtonReleaseMask | ButtonMotionMask |
                 KeymapStateMask |
                 PointerMotionMask |
                 EnterWindowMask | LeaveWindowMask |
                 FocusChangeMask |
                 ExposureMask |
                 StructureNotifyMask |
                 SubstructureNotifyMask);

    XFlush(display);

    embedClient(clientId);

    // FIXME: This checks that the client is still valid. Qt won't pick it up
    // if the client closes before embedding completes. However, what happens
    // if the close happens after this point? Should checks happen on a timer
    // until embedding completes perhaps?
    if (!XGetWindowAttributes(QX11Info::display(), clientId, &d->attr)) {
        emit error(QX11EmbedContainer::Unknown);
        return;
    }
}


void X11EmbedContainer::ensureValidSize()
{
    QSize s = QSize(qBound(minimumSize().width(), width(), maximumSize().width()),
                    qBound(minimumSize().height(), height(), maximumSize().height()));
    resize(s);
}


void X11EmbedContainer::setUpdatesEnabled(bool enabled)
{
    d->updatesEnabled = enabled;
}


void X11EmbedContainer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (!d->updatesEnabled) {
        return;
    }

    if (!d->picture) {
        FdoSelectionManager::painter()->updateContainer(this);
        return;
    }

    // Taking a detour via a QPixmap is unfortunately the only way we can get
    // the window contents into Qt's backing store.
    QPixmap pixmap(size());
    pixmap = toX11Pixmap(pixmap);
    pixmap.fill(Qt::transparent);
    XRenderComposite(x11Info().display(), PictOpSrc, d->picture, None, pixmap.x11PictureHandle(),
                     0, 0, 0, 0, 0, 0, width(), height());
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
}

void X11EmbedContainer::setBackgroundPixmap(QPixmap background)
{
    if (!clientWinId()) {
        return;
    }

    //Prevent updating the background-image if possible. Updating can cause a very annoying flicker due to the XClearArea, and thus has to be kept to a minimum
    QImage image;
    if (background.paintEngine()->type() != QPaintEngine::X11)
      image = background.toImage(); // With the raster graphics system this call just returns the backing image, so the image data isn't copied.
    else
      image = background.copy().toImage(); //With the X11 graphics engine, we have to create a copy first, else we get a crash

    if(d->oldBackgroundImage == image) {
      return;
    }
    d->oldBackgroundImage = image;

    Display* display = QX11Info::display();
    XSetWindowBackgroundPixmap(display, clientWinId(), toX11Pixmap(background).handle());
    XClearArea(display, clientWinId(), 0, 0, 0, 0, True);
}

// Qt has qt_toX11Pixmap(), but that's sadly not public API. So the only
// option seems to be to create X11-based QPixmap using QPixmap::fromX11Pixmap()
// and draw the non-native pixmap to it.
// NOTE: The alpha-channel is not preserved if it exists, but for X pixmaps it generally should not be needed anyway.
QPixmap X11EmbedContainer::toX11Pixmap(const QPixmap& pix)
{
    if(pix.handle() != 0)   // X11 pixmap
        return pix;
    Pixmap xpix = XCreatePixmap(pix.x11Info().display(), RootWindow(pix.x11Info().display(), pix.x11Info().screen()),
                                pix.width(), pix.height(), QX11Info::appDepth());
    QPixmap wrk = QPixmap::fromX11Pixmap(xpix);
    QPainter paint(&wrk);
    paint.drawPixmap(0, 0, pix);
    paint.end();
    QPixmap ret = wrk.copy();
    wrk = QPixmap(); // reset, so that xpix can be freed (QPixmap does not own it)
    XFreePixmap(pix.x11Info().display(), xpix);
    return ret;
}

}

#include "x11embedcontainer.moc"
