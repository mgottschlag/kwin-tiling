/* vi: ts=8 sts=4 sw=4
 * kate: space-indent on; tab-width 8; indent-width 4; indent-mode cstyle;
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */
#include "bgrender.h"

#include <fixx11h.h>
#include <config-workspace.h>

#include <time.h>
#include <stdlib.h>
#include <utime.h>

#include <QTimer>
#include <QPainter>
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <QDesktopWidget>
#include <QPaintEngine>
#include <QHash>
#include <QPixmap>
#include <QSvgRenderer>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <qimageblitz.h>
#include <ktemporaryfile.h>
#include <kcursor.h>
#include <kfilemetainfo.h>
#include <kconfiggroup.h>
#include <kmacroexpander.h>

#include "bgdefaults.h"

#include <X11/Xlib.h>

#include <QX11Info>

/**** KBackgroundRenderer ****/


KBackgroundRenderer::KBackgroundRenderer(int screen, bool drawBackgroundPerScreen, const KSharedConfigPtr &config)
    : KBackgroundSettings(screen, drawBackgroundPerScreen, config)
{
    m_State = 0;
    m_isBusyCursor = false;
    m_enableBusyCursor = false;
    m_pDirs = KGlobal::dirs();
    m_rSize = m_Size = drawBackgroundPerScreen ?
            QApplication::desktop()->screenGeometry(screen).size() : QApplication::desktop()->size();
    m_pProc = 0L;
    m_Tempfile = 0L;
    m_bPreview = false;
    m_Cached = false;
    m_TilingEnabled = false;

    m_pTimer = new QTimer(this);
    m_pTimer->setSingleShot(true);
    connect(m_pTimer, SIGNAL(timeout()), SLOT(render()));
}


KBackgroundRenderer::~KBackgroundRenderer()
{
    cleanup();
    delete m_Tempfile;
    m_Tempfile = 0;
}


void KBackgroundRenderer::setSize(const QSize &size)
{
    m_rSize = m_Size = size;
}

/*
 * Re-configure because the desktop has been resized.
 */
void KBackgroundRenderer::desktopResized()
{
    m_State = 0;
    m_rSize = drawBackgroundPerScreen() ?
            QApplication::desktop()->screenGeometry(screen()).size() : QApplication::desktop()->size();
    if (!m_bPreview)
        m_Size = m_rSize;
}


void KBackgroundRenderer::tile(QImage &dest, const QRect &_rect, const QImage &src)
{
    QRect rect = _rect;
    rect &= dest.rect();

    int x, y;
    int h = rect.height(), w = rect.width();
    int offx = rect.x(), offy = rect.y();
    int sw = src.width(), sh = src.height();

    for (y = offy; y < offy + h; y++)
        for (x = offx; x < offx + w; x++)
            dest.setPixel(x, y, src.pixel(x % sw, y % sh));
}


/*
 * Build a command line to run the program.
 */

QString KBackgroundRenderer::buildCommand()
{
    QString cmd;
    if (m_bPreview)
        cmd = previewCommand();
    else
        cmd = command();

    if (cmd.isEmpty())
        return QString();

    QHash<QChar, QString> map;
    map.insert('f', m_Tempfile->fileName());
    map.insert('x', QString::number(m_Size.width()));
    map.insert('y', QString::number(m_Size.height()));
    return KMacroExpander::expandMacrosShellQuote(cmd, map);
}


/*
 * Create a background tile. If the background mode is `Program',
 * this is asynchronous.
 */
int KBackgroundRenderer::doBackground(bool quit)
{
    if (m_State & BackgroundDone)
        return Done;
    int bgmode = backgroundMode();

    if (!enabled())
        bgmode = Flat;

    if (quit) {
        if (bgmode == Program && m_pProc)
            m_pProc->terminate();
        return Done;
    }

    int retval = Done;
    QString file;

    static unsigned int tileWidth = 0;
    static unsigned int tileHeight = 0;
    if (tileWidth == 0) {
        int tile_val = QPixmap::defaultDepth() >= 24 ? 1 : 2;
        // some dithering may be needed even with bpb==15/16, so don't use tileWidth==1
        // for them
        // with tileWidth>2, repainting the desktop causes nasty effect (XFree86 4.1.0)
        if (XQueryBestTile(QX11Info::display(), QX11Info::appRootWindow(), tile_val, tile_val,
                           &tileWidth, &tileHeight) != Success)
            tileWidth = tileHeight = tile_val; // some defaults
    }
    switch (bgmode) {

    case Flat:
        // this can be tiled correctly without problems
        m_Background = QImage(tileWidth, tileHeight, QImage::Format_RGB32);
        m_Background.fill(colorA().rgb());
        break;

    case Pattern: {
        if (pattern().isEmpty())
            break;
        file = m_pDirs->findResource("dtop_pattern", pattern());
        if (file.isEmpty())
            break;

        m_Background.load(file);
        if (m_Background.isNull())
            break;
        int w = m_Background.width();
        int h = m_Background.height();
        if ((w > m_Size.width()) || (h > m_Size.height())) {
            w = qMin(w, m_Size.width());
            h = qMin(h, m_Size.height());
            m_Background = m_Background.copy(0, 0, w, h);
        }
        Blitz::flatten(m_Background, colorA(), colorB());
        break;
    }
    case Program:
        if (m_State & BackgroundStarted)
            break;
        m_State |= BackgroundStarted;
        createTempFile();

        file = buildCommand();
        if (file.isEmpty())
            break;

        delete m_pProc;
        m_pProc = new KProcess;
        m_pProc->setShellCommand(file);
        connect(m_pProc,
                SIGNAL(finished(int, QProcess::ExitStatus)),
                SLOT(slotBackgroundDone(int, QProcess::ExitStatus)));
        m_pProc->start();
        retval = Wait;
        break;

    case HorizontalGradient: {
        QSize size = m_Size;
        // on <16bpp displays the gradient sucks when tiled because of dithering
        if (canTile())
            size.setHeight(tileHeight);
        m_Background = Blitz::gradient(size, colorA(), colorB(),
                                       Blitz::HorizontalGradient);
        break;
    }
    case VerticalGradient: {
        QSize size = m_Size;
        // on <16bpp displays the gradient sucks when tiled because of dithering
        if (canTile())
            size.setWidth(tileWidth);
        m_Background = Blitz::gradient(size, colorA(), colorB(),
                                       Blitz::VerticalGradient);
        break;
    }
    case PyramidGradient:
        m_Background = Blitz::gradient(m_Size, colorA(), colorB(),
                                       Blitz::PyramidGradient);
        break;

    case PipeCrossGradient:
        m_Background = Blitz::gradient(m_Size, colorA(), colorB(),
                                       Blitz::PipeCrossGradient);
        break;

    case EllipticGradient:
        m_Background = Blitz::gradient(m_Size, colorA(), colorB(),
                                       Blitz::EllipticGradient);
        break;
    }

    if (retval == Done)
        m_State |= BackgroundDone;

    return retval;
}


int KBackgroundRenderer::doWallpaper(bool quit)
{
    if (m_State & WallpaperDone)
        return Done;

    if (quit)
        // currently no asynch. wallpapers
        return Done;

    int wpmode = enabled() ? wallpaperMode() : NoWallpaper;

    m_Wallpaper = QImage();
    if (wpmode != NoWallpaper) {
wp_load:
        if (currentWallpaper().isEmpty()) {
            wpmode = NoWallpaper;
            goto wp_out;
        }
        QString file = m_pDirs->findResource("wallpaper", currentWallpaper());
        if (file.isEmpty()) {
            wpmode = NoWallpaper;
            goto wp_out;
        }

        // _Don't_ use KMimeType, as it relies on ksycoca which we really
        // don't want in krootimage (kdm context).
        //if (KMimeType::findByPath(file)->is("image/svg+xml")) {
        if (file.endsWith(".svg") || file.endsWith(".svgz")) {

            // Special stuff for SVG icons

            //FIXME
            //ksvgiconloader doesn't seem to let us find out the
            //ratio of width to height so for the most part we just
            //assume it's a square
            int svgWidth;
            int svgHeight;
            switch (wpmode) {
            case Centred:
            case CentredAutoFit:
                svgHeight = (int)(m_Size.height() * 0.8);
                svgWidth = svgHeight;
                break;
            case Tiled:
            case CenterTiled:
                svgHeight = (int)(m_Size.height() * 0.5);
                svgWidth = svgHeight;
                break;
            case Scaled:
                svgHeight = m_Size.height();
                svgWidth = m_Size.width();
                break;
            case CentredMaxpect:
            case ScaleAndCrop:
            case TiledMaxpect:
                svgHeight = m_Size.height();
                svgWidth = svgHeight;
                break;
            case NoWallpaper:
            default:
                kWarning() << "unknown diagram type" ;
                svgHeight = m_Size.height();
                svgWidth = svgHeight;
                break;
            }
            //FIXME hack due to strangeness with
            //background control modules
            if (svgHeight < 200) {
                svgHeight *= 6;
                svgWidth *= 6;
            }

            QSvgRenderer renderer(file);
            if (renderer.isValid()) {
                m_Wallpaper = QImage(svgWidth, svgHeight, QImage::Format_ARGB32_Premultiplied);
                m_Wallpaper.fill(0);
                QPainter p(&m_Wallpaper);
                renderer.render(&p);
            }
        } else {
            m_Wallpaper.load(file);
        }
        if (m_Wallpaper.isNull()) {
            kWarning() << "failed to load wallpaper " << file ;
            if (discardCurrentWallpaper())
                goto wp_load;
            wpmode = NoWallpaper;
            goto wp_out;
        }
        m_Wallpaper = m_Wallpaper.convertToFormat(QImage::Format_ARGB32_Premultiplied, Qt::DiffuseAlphaDither);

        // If we're previewing, scale the wallpaper down to make the preview
        // look more like the real desktop.
        if (m_bPreview) {
            int xs = m_Wallpaper.width() * m_Size.width() / m_rSize.width();
            int ys = m_Wallpaper.height() * m_Size.height() / m_rSize.height();
            if ((xs < 1) || (ys < 1)) {
                xs = ys = 1;
            }
            if (m_WallpaperRect.size() != QSize(xs, ys))
                m_Wallpaper = m_Wallpaper.scaled(xs, ys, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        }

#if 0
        // HACK: Use KFileMetaInfo only when we have a KApplication
        // KFileMetaInfo needs ksycoca and so on, but this code is
        // used also in krootimage (which in turn is used by kdm).
        if (kapp) {
            KFileMetaInfo metaInfo(file);
            if (metaInfo.isValid() && metaInfo.item("Orientation").isValid()) {
                switch (metaInfo.item("Orientation").value().toInt()) {
                case 2:
                    // Flipped horizontally
                    m_Wallpaper.mirrored(true, false);
                    break;
                case 3:
                    // Rotated 180 degrees
                    m_Wallpaper = m_Wallpaper.transformed(QMatrix().rotate(180));
                    break;
                case 4:
                    // Flipped vertically
                    m_Wallpaper = m_Wallpaper.mirrored(false, true);
                    break;
                case 5:
                    // Rotated 90 degrees & flipped horizontally
                    m_Wallpaper = m_Wallpaper.transformed(QMatrix().rotate(90)).mirrored(true, false);
                    break;
                case 6:
                    // Rotated 90 degrees
                    m_Wallpaper = m_Wallpaper.transformed(QMatrix().rotate(90));
                    break;
                case 7:
                    // Rotated 90 degrees & flipped vertically
                    m_Wallpaper = m_Wallpaper.transformed(QMatrix().rotate(90)).mirrored(false, true);
                    break;
                case 8:
                    // Rotated 270 degrees
                    m_Wallpaper = m_Wallpaper.transformed(QMatrix().rotate(270));
                    break;
                case 1:
                default:
                    // Normal or invalid orientation
                    break;
                }
            }
        }
#endif
    }
wp_out:

    if (m_Background.isNull()) {
        m_Background = QImage(8, 8, QImage::Format_RGB32);
        m_Background.fill(colorA().rgb());
    }

    int retval = Done;

    // desktop width/height
    int w = m_Size.width();
    int h = m_Size.height();

    // wallpaper width/height
    int ww = m_Wallpaper.width();
    int wh = m_Wallpaper.height();

    // to be filled destination rectangle; may exceed desktop!
    m_WallpaperRect = QRect();

    switch (wpmode) {
    case NoWallpaper:
        break;
    case Centred:
        m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
        break;
    case Tiled:
        m_WallpaperRect.setRect(0, 0, w, h);
        break;
    case CenterTiled:
        m_WallpaperRect.setCoords(-ww + ((w - ww) / 2) % ww, -wh + ((h - wh) / 2) % wh, w - 1, h - 1);
        break;
    case Scaled:
        ww = w;
        wh = h;
        if (m_WallpaperRect.size() != QSize(w, h))
            m_Wallpaper = m_Wallpaper.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_WallpaperRect.setRect(0, 0, w, h);
        break;
    case CentredAutoFit:
        if (ww <= w && wh <= h) {
            m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh); // like Centred
            break;
        }
        // fall through
    case CentredMaxpect: {
        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            ww = (int)(sy * ww);
            wh = h;
        } else {
            wh = (int)(sx * wh);
            ww = w;
        }
        if (m_WallpaperRect.size() != QSize(ww, wh))
            m_Wallpaper = m_Wallpaper.scaled(ww, wh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, ww, wh);
        break;
    }
    case TiledMaxpect: {
        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            ww = (int)(sy * ww);
            wh = h;
        } else {
            wh = (int)(sx * wh);
            ww = w;
        }
        if (m_WallpaperRect.size() != QSize(ww, wh))
            m_Wallpaper = m_Wallpaper.scaled(ww, wh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_WallpaperRect.setRect(0, 0, w, h);
        break;
    }
    case ScaleAndCrop: {
        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            //Case 1: x needs bigger scaling. Lets increase x and leave part of y offscreen
            ww = w;
            wh = (int)(sx * wh);
        } else {
            //Case 2: y needs bigger scaling. Lets increase y and leave part of x offscreen
            wh = h;
            ww = (int)(sy * ww);
        }
        if (m_WallpaperRect.size() != QSize(ww, wh))
            m_Wallpaper = m_Wallpaper.scaled(ww, wh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_WallpaperRect.setRect((w - ww) / 2, (h - wh) / 2, w, h);
        break;
    }
    }

    wallpaperBlend();

    if (retval == Done)
        m_State |= WallpaperDone;

    return retval;
}

bool KBackgroundRenderer::canTile() const
{
    return m_TilingEnabled && optimize();
}

void KBackgroundRenderer::wallpaperBlend()
{
    if (!enabled() || wallpaperMode() == NoWallpaper
            || (blendMode() == NoBlending &&
                (QApplication::desktop()->paintEngine()->hasFeature(QPaintEngine::Antialiasing)
                 || !m_Wallpaper.hasAlphaChannel()))) {
        fastWallpaperBlend();
    } else {
        fullWallpaperBlend();
    }
}

// works only for NoBlending and no alpha in wallpaper
// but is much faster than QImage fidling
void KBackgroundRenderer::fastWallpaperBlend()
{
    m_Image = QImage();
    // copy background to m_pPixmap
    if (!enabled() || (wallpaperMode() == NoWallpaper && canTile())) {
        // if there's no wallpaper, no need to tile the pixmap to the size of desktop, as X does
        // that automatically and using a smaller pixmap should save some memory
        m_Pixmap = QPixmap::fromImage(m_Background);
        return;
    } else if (wallpaperMode() == Tiled && !m_Wallpaper.hasAlphaChannel() && canTile() && !m_bPreview) {
        // tiles will be tiled by X automatically
        m_Pixmap = QPixmap::fromImage(m_Wallpaper);
        return;
    } else if (m_WallpaperRect.contains(QRect(QPoint(0, 0), m_Size))
               && !m_Wallpaper.hasAlphaChannel()) // wallpaper covers all and no blending
        m_Pixmap = QPixmap(m_Size);
    else if (m_Background.size() == m_Size)
        m_Pixmap = QPixmap::fromImage(m_Background);
    else {
        m_Pixmap = QPixmap(m_Size);
        QPainter p(&m_Pixmap);
        QPixmap pm = QPixmap::fromImage(m_Background);
        p.drawTiledPixmap(0, 0, m_Size.width(), m_Size.height(), pm);
    }

    // paint/alpha-blend wallpaper to destination rectangle of m_pPixmap
    if (m_WallpaperRect.isValid()) {
        QPixmap wp_pixmap = QPixmap::fromImage(m_Wallpaper);
        QPainter pa(&m_Pixmap);
        int ww = m_Wallpaper.width();
        int wh = m_Wallpaper.height();
        for (int y = m_WallpaperRect.top(); y < m_WallpaperRect.bottom(); y += wh) {
            for (int x = m_WallpaperRect.left(); x < m_WallpaperRect.right(); x += ww) {
                pa.drawPixmap(x, y, wp_pixmap);
            }
        }
    }
}


void KBackgroundRenderer::fullWallpaperBlend()
{
    m_Pixmap = QPixmap();

    // desktop width/height
    int w = m_Size.width();
    int h = m_Size.height();

    // copy background to m_pImage
    if (m_Background.size() == m_Size) {
        m_Image = m_Background.copy();

        if (m_Image.depth() < 32) {
            m_Image = m_Image.convertToFormat(QImage::Format_ARGB32_Premultiplied, Qt::DiffuseAlphaDither);
        }
    } else {
        m_Image = QImage(w, h, QImage::Format_RGB32);
        tile(m_Image, QRect(0, 0, w, h), m_Background);
    }

    // blend wallpaper to destination rectangle of m_pImage
    if (m_WallpaperRect.isValid()) {
        int blendFactor = 100;
        if (blendMode() == FlatBlending)
            blendFactor = (blendBalance() + 200) / 4;
        int ww = m_Wallpaper.width();
        int wh = m_Wallpaper.height();
        for (int y = m_WallpaperRect.top(); y < m_WallpaperRect.bottom(); y += wh) {
            for (int x = m_WallpaperRect.left(); x < m_WallpaperRect.right(); x += ww) {
                blend(m_Image, QRect(x, y, ww, wh), m_Wallpaper,
                      QPoint(-qMin(x, 0), -qMin(y, 0)), blendFactor);
            }
        }
    }


    // blend whole desktop
    if (wallpaperMode() != NoWallpaper) {
        int bal = blendBalance();

        switch (blendMode()) {
            /* TODO disabled for now
                  case HorizontalBlending:
                    Blitz::blend(m_Image, m_Background,
                        Blitz::HorizontalGradient,
                               bal, 100);
                    break;

                  case VerticalBlending:
                    Blitz::blend(m_Image, m_Background,
                        Blitz::VerticalGradient,
                               100, bal);
                    break;

                  case PyramidBlending:
                    Blitz::blend(m_Image, m_Background,
                        Blitz::PyramidGradient,
                               bal, bal);
                    break;

                  case PipeCrossBlending:
                    Blitz::blend(m_Image, m_Background,
                        Blitz::PipeCrossGradient,
                               bal, bal);
                    break;

                  case EllipticBlending:
                    Blitz::blend(m_Image, m_Background,
                        Blitz::EllipticGradient,
                               bal, bal);
                    break;
            */

        case IntensityBlending:
            Blitz::modulate(m_Image, m_Background, reverseBlending(),
                            Blitz::Intensity, bal, Blitz::All);
            break;

        case SaturateBlending:
            Blitz::modulate(m_Image, m_Background, reverseBlending(),
                            Blitz::Saturation, bal, Blitz::Brightness);
            break;

        case ContrastBlending:
            Blitz::modulate(m_Image, m_Background, reverseBlending(),
                            Blitz::Contrast, bal, Blitz::All);
            break;

        case HueShiftBlending:
            Blitz::modulate(m_Image, m_Background, reverseBlending(),
                            Blitz::HueShift, bal, Blitz::Brightness);
            break;

        case FlatBlending:
            // Already handled
            break;
        }
    }
}

/* Alpha blend an area from <src> with offset <soffs> to rectangle <dr> of <dst>
 * Default offset is QPoint(0, 0).
 * blendfactor = [0, 100%]
 */
void KBackgroundRenderer::blend(QImage &dst, const QRect &_dr, const QImage &src, const QPoint &soffs, int blendFactor)
{
    QRect dr = _dr;
    int x, y, a;
    dr &= dst.rect();

    for (y = 0; y < dr.height(); y++) {
        if (dst.scanLine(dr.y() + y) && src.scanLine(soffs.y() + y)) {
            QRgb *b;
            const QRgb *d;
            for (x = 0; x < dr.width(); x++) {
                b = reinterpret_cast<QRgb *>(dst.scanLine(dr.y() + y)
                                             + (dr.x() + x) * sizeof(QRgb));
                d = reinterpret_cast<const QRgb *>(src.scanLine(soffs.y() + y)
                                                   + (soffs.x() + x) * sizeof(QRgb));
                a = (qAlpha(*d) * blendFactor) / 100;
                *b = qRgb(qRed(*b) - (((qRed(*b) - qRed(*d)) * a) >> 8),
                          qGreen(*b) - (((qGreen(*b) - qGreen(*d)) * a) >> 8),
                          qBlue(*b) - (((qBlue(*b) - qBlue(*d)) * a) >> 8));
            }
        }
    }
}



void KBackgroundRenderer::slotBackgroundDone(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_State |= BackgroundDone;

    if (exitStatus == QProcess::NormalExit && !exitCode) {
        m_Background.load(m_Tempfile->fileName());
        m_State |= BackgroundDone;
    }

    delete m_Tempfile;
    m_Tempfile = 0;
    m_pTimer->start(0);
    setBusyCursor(false);
}



/*
 * Starts the rendering process.
 */
void KBackgroundRenderer::start(bool enableBusyCursor)
{
    m_enableBusyCursor = enableBusyCursor;
    setBusyCursor(true);

    m_Cached = false;

    m_State = Rendering;
    m_pTimer->start(0);
}


/*
 * This slot is connected to a timer event. It is called repeatedly until
 * the rendering is done.
 */
void KBackgroundRenderer::render()
{
    setBusyCursor(true);
    if (!(m_State & Rendering))
        return;

    if (!(m_State & InitCheck)) {
        QString f = cacheFileName();
        if (useCacheFile()) {
            QString w = m_pDirs->findResource("wallpaper", currentWallpaper());
            QFileInfo wi(w);
            QFileInfo fi(f);
            if (wi.lastModified().isValid() && fi.lastModified().isValid()
                    && wi.lastModified() < fi.lastModified()) {
                QImage im;
                if (im.load(f, "PNG")) {
                    m_Image = im;
                    m_Pixmap = QPixmap::fromImage(m_Image);
                    m_Cached = true;
                    m_State |= InitCheck | BackgroundDone | WallpaperDone;
                }
            }
        }
        m_pTimer->start(0);
        m_State |= InitCheck;
        return;
    }

    int ret;

    if (!(m_State & BackgroundDone)) {
        ret = doBackground();
        if (ret != Wait)
            m_pTimer->start(0);
        return;
    }

    // No async wallpaper
    doWallpaper();

    done();
    setBusyCursor(false);
}


/*
 * Rendering is finished.
 */
void KBackgroundRenderer::done()
{
    setBusyCursor(false);
    m_State |= AllDone;
    emit imageDone(screen());
    if (backgroundMode() == Program && m_pProc &&
            m_pProc->exitStatus() != QProcess::NormalExit) {
        emit programFailure(-1);
    } else if (backgroundMode() == Program && m_pProc &&
               m_pProc->exitCode()) {
        emit programFailure(m_pProc->exitStatus());
    } else if (backgroundMode() == Program) {
        emit programSuccess();
    }
}

/*
 * This function toggles a busy cursor on and off, for use in rendering.
 * It is useful because of the ASYNC nature of the rendering - it is hard
 * to make sure we don't set the busy cursor twice, but only restore
 * once.
 */
void KBackgroundRenderer::setBusyCursor(bool isBusy)
{
    if (m_isBusyCursor == isBusy)
        return;
    if (isBusy && !m_enableBusyCursor)
        return;
    m_isBusyCursor = isBusy;
    if (isBusy)
        QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    else
        QApplication::restoreOverrideCursor();
}

/*
 * Stop the rendering.
 */
void KBackgroundRenderer::stop()
{
    if (!(m_State & Rendering))
        return;

    doBackground(true);
    doWallpaper(true);
    m_State = 0;
}


/*
 * Cleanup after rendering.
 */
void KBackgroundRenderer::cleanup()
{
    setBusyCursor(false);
    m_Background = QImage();
    m_Image = QImage();
    m_Pixmap = QPixmap();
    m_Wallpaper = QImage();
    delete m_pProc;
    m_pProc = 0L;
    m_State = 0;
    m_WallpaperRect = QRect();
    m_Cached = false;
}


void KBackgroundRenderer::setPreview(const QSize &size)
{
    if (size.isNull())
        m_bPreview = false;
    else {
        m_bPreview = true;
        m_Size = size;
    }
}


QPixmap KBackgroundRenderer::pixmap()
{
    if (m_State & AllDone) {
        if (m_Pixmap.isNull())
            m_Pixmap = QPixmap::fromImage(m_Image);
        return m_Pixmap;
    }
    return QPixmap();
}

QImage KBackgroundRenderer::image()
{
    if (m_State & AllDone) {
        if (m_Image.isNull())
            fullWallpaperBlend(); // create from m_Pixmap
        return m_Image;
    }
    return QImage();
}


void KBackgroundRenderer::load(int screen, bool drawBackgroundPerScreen, bool reparseConfig)
{
    if (m_State & Rendering)
        stop();

    cleanup();
    m_bPreview = false;
    m_Size = m_rSize;

    KBackgroundSettings::load(screen, drawBackgroundPerScreen, reparseConfig);
}

void KBackgroundRenderer::createTempFile()
{
    if (!m_Tempfile) {
        m_Tempfile = new KTemporaryFile();
        m_Tempfile->open();
    }
}

QString KBackgroundRenderer::cacheFileName()
{
    QString f = fingerprint();
    f.replace(':', '_'); // avoid characters that shouldn't be in filenames
    f.replace('/', '#');
    f = KStandardDirs::locateLocal("cache", QString("background/%1x%2_%3.png")
                                   .arg(m_Size.width()).arg(m_Size.height()).arg(f));
    return f;
}

bool KBackgroundRenderer::useCacheFile() const
{
    if (!enabled())
        return false;
    if (backgroundMode() == Program)
        return false; // don't cache these at all
    if (wallpaperMode() == NoWallpaper)
        return false; // generating only background patterns should be always faster
    QString file = currentWallpaper();
    if (file.endsWith(".svg") || file.endsWith(".svgz"))
        return true; // cache these, they can be bloody slow
    switch (backgroundMode()) {
    case NoWallpaper:
    case Centred:
    case Tiled:
    case CenterTiled:
        return false; // these don't need scaling
    case CentredMaxpect:
    case TiledMaxpect:
    case Scaled:
    case CentredAutoFit:
    case ScaleAndCrop:
    default:
        return true;
    }
}

void KBackgroundRenderer::saveCacheFile()
{
    if (!(m_State & AllDone))
        return;
    if (!useCacheFile())
        return;
    if (m_Image.isNull())
        fullWallpaperBlend(); // generate from m_Pixmap
    QString f = cacheFileName();
    if (KStandardDirs::exists(f) || m_Cached)
        utime(QFile::encodeName(f), NULL);
    else {
        m_Image.save(f, "PNG");
        // remove old entries from the cache
        QDir dir(KStandardDirs::locateLocal("cache", "background/"));
        const QFileInfoList list = dir.entryInfoList(QStringList() << "*.png", QDir::Files, QDir::Time | QDir::Reversed);
        if (!list.isEmpty()) {
            int size = 0;
            foreach (const QFileInfo &info, list)
                size += info.size();
            foreach (const QFileInfo &info, list) {
                if (size < 8 * 1024 * 1024)
                    break;
                // keep everything newer than 10 minutes if the total size is less than 50M (just in case)
                if (size < 50 * 1024 * 1024
                        && (time_t) info.lastModified().toTime_t() >= time(NULL) - 10 * 60)
                    break;
                size -= info.size();
                QFile::remove(info.absoluteFilePath());
            }
        }
    }
}

#include "bgrender.moc"
