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
#include <QtCore/QTime>
#include <QtCore/QTimer>
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
    if (pixmap.paintEngine()->type() != QPaintEngine::X11) {
#if defined(HAVE_XCOMPOSITE)
        // If we're using the raster or OpenGL graphics systems, a QPixmap isn't an X pixmap,
        // so we have to get the window contents into a QImage and then draw that.
        Display *dpy = x11Info().display();

        // XXX We really should keep a cached copy of the image client side, and only
        //     update it in response to a damage event.
        Pixmap pixmap = XCompositeNameWindowPixmap(dpy, clientWinId());
        XImage *ximage = XGetImage(dpy, pixmap, 0, 0, width(), height(), AllPlanes, ZPixmap);
        XFreePixmap(dpy, pixmap);
        // We actually check if we get the image from X11 since clientWinId can be any arbiter window (with crazy XWindowAttribute and the pixmap associated is bad)
        if (!ximage)
            return;
        // This is safe to do since we only composite ARGB32 windows, and PictStandardARGB32
        // matches QImage::Format_ARGB32_Premultiplied.
        QImage image((const uchar*)ximage->data, ximage->width, ximage->height, ximage->bytes_per_line,
                     QImage::Format_ARGB32_Premultiplied);

        QPainter p(this);
        p.drawImage(0, 0, image);

        XDestroyImage(ximage);
#endif
    } else {
        pixmap.fill(Qt::transparent);

        XRenderComposite(x11Info().display(), PictOpSrc, d->picture, None, pixmap.x11PictureHandle(),
                         0, 0, 0, 0, 0, 0, width(), height());

        QPainter p(this);
        p.drawPixmap(0, 0, pixmap);
    }
}

void X11EmbedContainer::setBackgroundPixmap(QPixmap background)
{
    if (!clientWinId()) {
        return;
    }

    Display *display = QX11Info::display();
    Pixmap bg = XCreatePixmap(display, clientWinId(), width(), height(), d->attr.depth);

    XRenderPictFormat *format = XRenderFindVisualFormat(display, d->attr.visual);
    Picture picture = XRenderCreatePicture(display, bg, format, 0, 0);

    //Prevent updating the background-image if possible. Updating can cause a very annoying flicker due to the XClearArea, and thus has to be kept to a minimum
    QImage image;
    if (background.paintEngine()->type() != QPaintEngine::X11)
      image = background.toImage(); // With the raster graphics system this call just returns the backing image, so the image data isn't copied.
    else
      image = background.copy().toImage(); //With the X11 graphics engine, we have to create a copy first, else we get a crash

    if(d->oldBackgroundImage == image) {
      XFreePixmap(display, bg);
      XRenderFreePicture(display, picture);
      return;
    }
    d->oldBackgroundImage = image;

    if (background.paintEngine()->type() != QPaintEngine::X11) {

        XRenderPictFormat *format = 0;
        int depth = 0;
        int bpp = 0;

        if (image.format() == QImage::Format_ARGB32_Premultiplied) {
            format = XRenderFindStandardFormat(display, PictStandardARGB32);
            depth = 32;
            bpp = 32;
        } else if (image.format() == QImage::Format_RGB32) {
            format = XRenderFindStandardFormat(display, PictStandardRGB24);
            depth = 24;
            bpp = 32;
        } else if (image.format() == QImage::Format_RGB16) {
            bpp = 16;
            depth = 16;

            // Try to find a picture format that matches the image format.
            // The Render spec doesn't require the X server to support 16bpp formats,
            // so this call can fail.
            XRenderPictFormat templ;
            templ.type             = PictTypeDirect;
            templ.direct.alpha     = 0;
            templ.direct.alphaMask = 0;
            templ.depth            = 16;
            templ.direct.red       = 11;
            templ.direct.redMask   = 0x1f;
            templ.direct.green     = 5;
            templ.direct.greenMask = 0x3f;
            templ.direct.blue      = 0;
            templ.direct.blueMask  = 0x1f;
            format = XRenderFindFormat(display, PictFormatType | PictFormatDepth | PictFormatAlpha |
                                       PictFormatAlphaMask | PictFormatRed | PictFormatRedMask |
                                       PictFormatGreen | PictFormatGreenMask | PictFormatBlue |
                                       PictFormatBlueMask, &templ, 0);
        }

        if (format == 0)
        {
            // Convert the image to a standard format.
            if (image.hasAlphaChannel()) {
                image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
                format = XRenderFindStandardFormat(display, PictStandardARGB32);
                depth = 32;
            } else { 
                image = image.convertToFormat(QImage::Format_RGB32);
                format = XRenderFindStandardFormat(display, PictStandardRGB24);
                depth = 24;
            }
            bpp = 32;
        }

        if (image.format() == QImage::Format_RGB32) {
            // Make sure the would be alpha bits are set to 1.
            quint32 * const pixels = (quint32*)(const_cast<const QImage*>(&image)->bits());
            for (int i = 0; i < image.width() * image.height(); i++) {
                pixels[i] |= 0xff000000;
            }
        }

        Q_ASSERT(format != 0);

        // Get the image data into a pixmap
        XImage ximage;
        ximage.width            = image.width(); 
        ximage.height           = image.height();
        ximage.xoffset          = 0; 
        ximage.format           = ZPixmap;
        // This is a hack to prevent the image data from detaching
        ximage.data             = (char*) const_cast<const QImage*>(&image)->bits();
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        ximage.byte_order       = MSBFirst;
#else
        ximage.byte_order       = LSBFirst;
#endif
        ximage.bitmap_unit      = bpp;
        ximage.bitmap_bit_order = ximage.byte_order;
        ximage.bitmap_pad       = bpp;
        ximage.depth            = depth;
        ximage.bytes_per_line   = image.bytesPerLine();
        ximage.bits_per_pixel   = bpp;
        if (depth > 16) {
            ximage.red_mask     = 0x00ff0000;
            ximage.green_mask   = 0x0000ff00;
            ximage.blue_mask    = 0x000000ff;
        } else {
            // r5g6b5
            ximage.red_mask     = 0xf800;
            ximage.green_mask   = 0x07e0;
            ximage.blue_mask    = 0x001f;
        }
        ximage.obdata           = 0;
        if (XInitImage(&ximage) == 0) {
            XRenderFreePicture(display, picture);
            XFreePixmap(display, bg);
            return;
        }

        Pixmap pm = XCreatePixmap(display, clientWinId(), width(), height(), ximage.depth);
        GC gc = XCreateGC(display, pm, 0, 0);
        XPutImage(display, pm, gc, &ximage, 0, 0, 0, 0, width(), height());
        XFreeGC(display, gc);

        Picture pict = XRenderCreatePicture(display, pm, format, 0, 0);
        XRenderComposite(display, PictOpSrc, pict, None, picture,
                         0, 0, 0, 0, 0, 0, width(), height());
        XRenderFreePicture(display, pict);
        XFreePixmap(display, pm);
    } else {
        XRenderComposite(display, PictOpSrc, background.x11PictureHandle(),
                         None, picture, 0, 0, 0, 0, 0, 0, width(), height());
    }

    XSetWindowBackgroundPixmap(display, clientWinId(), bg);

    XRenderFreePicture(display, picture);
    XFreePixmap(display, bg);

    XClearArea(display, clientWinId(), 0, 0, 0, 0, True);
}

}

#include "x11embedcontainer.moc"
