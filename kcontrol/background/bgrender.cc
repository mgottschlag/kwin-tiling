/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#include <time.h>
#include <stdlib.h>

#include <kconfig.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qobject.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <qtimer.h>

#include <kapp.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kstddirs.h>
#include <kpixmap.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include <kprocess.h>
#include <kimageio.h>

#include <bgdefaults.h>
#include <bgsettings.h>
#include <bgrender.h>


/**** KBackgroundRenderer ****/


KBackgroundRenderer::KBackgroundRenderer(int desk, KConfig *config)
    : KBackgroundSettings(desk, config)
{
    m_State = 0;
 
    if (config)
	m_pConfig = config;
    else
	m_pConfig = new KConfig("kdesktoprc");
    m_pDirs = KGlobal::dirs();
    m_rSize = m_Size = QApplication::desktop()->size();
    m_pBackground = 0L; m_pImage = 0L;
    m_pProc = 0L;
    m_bPreview = false;
    m_bTile = false;

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), SLOT(render()));
}


KBackgroundRenderer::~KBackgroundRenderer()
{
    delete m_pImage;
}


void KBackgroundRenderer::tile(QImage *dest, QRect rect, QImage *src)
{
    rect &= dest->rect();

    int x, y;
    int h = rect.height(), w = rect.width();
    int offx = rect.x(), offy = rect.y();
    int sw = src->width(), sh = src->height();

    for (y=offy; y<offy+h; y++)
	for (x=offx; x<offx+w; x++)
	    dest->setPixel(x, y, src->pixel(x%sw, y%sh));
}
	

/*
 * Build a command line to run the program.
 */

QString KBackgroundRenderer::buildCommand()
{
    QString num;
    int pos = 0;

    QString cmd;
    if (m_bPreview)
        cmd = previewCommand();
    else
        cmd = command();

    if (cmd.isEmpty())
	return QString();

    while ((pos = cmd.find('%', pos)) != -1) {

        if (pos == (int) (cmd.length() - 1))
            break;

        switch (cmd.at(pos+1).latin1()) {
        case 'f':
            cmd.replace(pos, 2, m_Tempfile);
            pos += m_Tempfile.length() - 2;
            break;

        case 'x':
            num.setNum(m_Size.width());
            cmd.replace(pos, 2, num);
            pos += num.length() - 2;
            break;

        case 'y':
            num.setNum(m_Size.height());
            cmd.replace(pos, 2, num);
            pos += num.length() - 2;
            break;

        case '%':
            cmd.replace(pos, 2, "%");
            pos--;
            break;
        }

    }
    return cmd;
}


/*
 * Create a background tile. If the background mode is `Program', 
 * this is asynchronous.
 */
int KBackgroundRenderer::doBackground(bool quit)
{
    if (m_State & BackgroundDone)
        return Done;

    int wpmode = wallpaperMode();
    int blmode = blendMode();
    int bgmode = backgroundMode();
    
    if ( (blmode == NoBlending) &&
	 ((wpmode == Tiled) ||
	  (wpmode == Scaled) ||
	  (wpmode == CenterTiled)) ) {
      // full screen wallpaper modes: background is not visible
      m_State |= BackgroundDone;
      return Done;
    }
    if (quit) {
	if (bgmode == Program)
	    m_pProc->kill();
        return Done;
    }

    int retval = Done;
    QString file;

    switch (bgmode) {

    case Flat:
	m_pBackground->create(10, 10, 32);
        m_pBackground->fill(colorA().rgb());
        break;

    case Pattern:
    {
        file = m_pDirs->findResource("dtop_pattern", pattern());
        if (file.isEmpty())
            break;

	m_pBackground->load(file);
	if (m_pBackground->isNull())
	    break;
	int w = m_pBackground->width();
	int h = m_pBackground->height();
	if ((w > m_Size.width()) || (h > m_Size.height())) {
	    w = QMIN(w, m_Size.width());
	    h = QMIN(h, m_Size.height());
	    *m_pBackground = m_pBackground->copy(0, 0, w, h);
	}
	KImageEffect::flatten(*m_pBackground, colorA(), colorB(), 0);
	break;
    }
    case Program:
        if (m_State & BackgroundStarted)
            break;
        m_State |= BackgroundStarted;
        m_Tempfile = tmpnam(0L);

	file = buildCommand();
	if (file.isEmpty())
	    break;

        m_pProc = new KShellProcess;
        *m_pProc << file;
        connect(m_pProc, SIGNAL(processExited(KProcess *)),
                SLOT(slotBackgroundDone(KProcess *)));
        m_pProc->start(KShellProcess::NotifyOnExit);
        retval = Wait;
        break;
		
    case HorizontalGradient:
    {
	QSize size = m_Size;
	size.setHeight(30);
	*m_pBackground = KImageEffect::gradient(size, colorA(), colorB(), 
		KImageEffect::HorizontalGradient, 0);
        break;
    }
    case VerticalGradient:
    {
	QSize size = m_Size;
	size.setWidth(30);
        *m_pBackground = KImageEffect::gradient(size, colorA(), colorB(),
		KImageEffect::VerticalGradient, 0);
        break;
    }
    case PyramidGradient:
        *m_pBackground = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::PyramidGradient, 0);
        break;

    case PipeCrossGradient:
        *m_pBackground = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::PipeCrossGradient, 0);
        break;

    case EllipticGradient:
        *m_pBackground = KImageEffect::gradient(m_Size, colorA(), colorB(),
		KImageEffect::EllipticGradient, 0);
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

    int wpmode = wallpaperMode();
    int blmode = blendMode();

    // Tiling is only possible if there's no blending
    bool bTile = m_bTile;
    if ((wpmode != NoWallpaper) && (blmode != NoBlending))
      bTile = false;

    QImage wp;
    if (wpmode != NoWallpaper) {
	if (currentWallpaper().isEmpty()) {
	    wpmode = NoWallpaper;
	    goto wp_out;
	}
	QString file = m_pDirs->findResource("wallpaper", currentWallpaper());
	if (file.isEmpty()) {
	    wpmode = NoWallpaper;
	    goto wp_out;
	}

	wp.load(file);
	if (wp.isNull()) {
	    wpmode = NoWallpaper;
	    goto wp_out;
	}
	wp = wp.convertDepth(32);

	// If we're previewing, scale the wallpaper down to make the preview
	// look more like the real desktop.
	if (m_bPreview) {
	    int xs = wp.width() * m_Size.width() / m_rSize.width();
	    int ys = wp.height() * m_Size.height() / m_rSize.height();
	    if ((xs < 1) || (ys < 1))
	    {
	       xs = ys = 1;
	    }
	    wp = wp.smoothScale(xs, ys);
	}
    }
wp_out:

    if ( m_pBackground->isNull()) {
      m_pBackground->create(10, 10, 32);
      m_pBackground->fill(colorA().rgb());
    }
	
    int ww = wp.width();
    int wh = wp.height();
    int retval = Done;

    switch (wpmode) {
    case NoWallpaper:
    {
	if (bTile)
	    *m_pImage = *m_pBackground;
	else  {
	    m_pImage->create(m_Size, 32);
	    tile(m_pImage, QRect(0, 0, m_Size.width(), m_Size.height()),
		    m_pBackground);
	}
	break;
    }
    case Tiled:
    {
	int w = m_Size.width();
	int h = m_Size.height();
        int y;

	if (bTile && (ww <= w) && (wh <= h))
	    *m_pImage = wp;
	else {
	    m_pImage->create(m_Size, 32);
	    tile(m_pImage, QRect(0, 0, w, QMIN(wh,h)), &wp);

	    if (h > wh)
		for (y=wh; y<h; y++)
		    memcpy(m_pImage->scanLine(y), m_pImage->scanLine(y % wh),
			   m_pImage->bytesPerLine());
	}
        break;
    }

    case CenterTiled:
    {
	QSize size = m_Size;
	if (bTile)
	    size = QSize(QMIN(m_Size.width(), wp.width()), 
		    QMIN(m_Size.height(), wp.height()));
	m_pImage->create(size, 32);

	int w = size.width();
	int h = size.height();
        int xa = ww - ((m_Size.width() - ww) / 2) % ww;
        int ya = wh - ((m_Size.height() - wh) / 2) % wh;
        int x, y;

        for (y=0; y < QMIN(wh,h); y++)
            for (x=0; x < w; x++)
                m_pImage->setPixel(x, y, wp.pixel((xa + x) % ww, (ya + y) % wh));

        if (h > wh)
            for (y=wh; y < h; y++)
                memcpy(m_pImage->scanLine(y), m_pImage->scanLine(y % wh),
                       m_pImage->bytesPerLine());
        break;
    }

    case Scaled:
        *m_pImage = wp.smoothScale(m_Size.width(), m_Size.height());
        break;

    case Centred:
    {
	int w = m_Size.width();
	int h = m_Size.height();
        int xa = (w - ww) / 2;
        int ya = (h - wh) / 2;
        int y, offx, offy;

        // Check if anchor point is not outside the screen
        if (xa <= 0) {
            offx = -xa; ww = w; xa = 0;
        } else
            offx = 0;
        if (ya <= 0) {
            offy = -ya; wh = h; ya = 0;
        } else
            offy = 0;
	
	// Background is no tile?
	if (m_pBackground->size() == m_Size) {

	  // if we blend, we need a deep copy
	  if (blmode != NoBlending)
	    *m_pImage = m_pBackground->copy();
	  else
	    *m_pImage = *m_pBackground;
	  if (m_pImage->depth()<32)
	    *m_pImage = m_pImage->convertDepth(32);
	}
	else {
	    int tw = w, th = h;
	    int bw = m_pBackground->width(), bh = m_pBackground->height();
	    if (bTile) {
		tw = QMIN(bw * ((xa + ww + bw - 1) / bw), w);
		th = QMIN(bh * ((ya + wh + bh - 1) / bh), h);
	    }
	    m_pImage->create(tw, th, 32);

	    // Fill the rectangles not covered by the centred image.
	    if (ya) {
		tile(m_pImage, QRect(0, 0, tw, ya), m_pBackground);
		tile(m_pImage, QRect(0, ya+wh, tw, th-ya-wh), m_pBackground);
	    }
	    if (xa) {
		tile(m_pImage, QRect(0, ya, xa, wh), m_pBackground);
		tile(m_pImage, QRect(xa+ww, ya, tw-xa-ww, wh), m_pBackground);
	    }
	}

	// And copy the centred image		
        for (y=0; y<wh; y++) {
	    if (m_pImage->scanLine(ya+y) && wp.scanLine(y+offy))
            memcpy(m_pImage->scanLine(ya+y) + xa * sizeof(QRgb),
                   wp.scanLine(y+offy) + offx * sizeof(QRgb),
                   ww * sizeof(QRgb));
	}
        break;
    }

    case CentredMaxpect:
    {
	int w = m_Size.width();
	int h = m_Size.height();

        double sx = (double) w / ww;
        double sy = (double) h / wh;
        if (sx > sy) {
            ww = (int) (sy*ww);
            wh = h;
        } else {
            ww = w;
            wh = (int) (sx*wh);
        }
        wp = wp.smoothScale(ww, wh);

        int xa = (w - ww) / 2;
        int ya = (h - wh) / 2;
        int y;

	if (m_pBackground->size() == m_Size) {

	  // if we blend, we need a deep copy
	  if (blmode != NoBlending)
	    *m_pImage = m_pBackground->copy();
	  else
	    *m_pImage = *m_pBackground;
	    
	  if (m_pImage->depth()<32)
	    *m_pImage = m_pImage->convertDepth(32);

	}
	else {
	    int tw = w, th = h;
	    int bw = m_pBackground->width(), bh = m_pBackground->height();
	    if (bTile) {
		tw = QMIN(bw * ((xa + ww + bw - 1) / bw), w);
		th = QMIN(bh * ((ya + wh + bh - 1) / bh), h);
	    }
	    m_pImage->create(tw, th, 32);

	    if (ya) {
		tile(m_pImage, QRect(0, 0, tw, ya), m_pBackground);
		tile(m_pImage, QRect(0, ya+wh, tw, th-ya-wh), m_pBackground);
	    } else {
		tile(m_pImage, QRect(0, 0, xa, th), m_pBackground);
		tile(m_pImage, QRect(xa+ww, 0, tw-xa-ww, th), m_pBackground);
	    }
	}

        for (y=0; y<wh; y++)
            memcpy(m_pImage->scanLine(ya+y) + xa * sizeof(QRgb),
                   wp.scanLine(y), wp.bytesPerLine());
        break;
    }

    }

    if (wpmode != NoWallpaper) {
      int bal = blendBalance();

      switch( blmode ) {
      case HorizontalBlending:
	KImageEffect::blend( *m_pImage, *m_pBackground,
			     KImageEffect::HorizontalGradient,
			     bal, 100 );
	break;

      case VerticalBlending:
	KImageEffect::blend( *m_pImage, *m_pBackground,
			     KImageEffect::VerticalGradient, 
			     100, bal );
	break;

      case PyramidBlending:
	KImageEffect::blend( *m_pImage, *m_pBackground,
			     KImageEffect::PyramidGradient, 
			     bal, bal );
	break;

      case PipeCrossBlending:
	KImageEffect::blend( *m_pImage, *m_pBackground,
			     KImageEffect::PipeCrossGradient,
			     bal, bal );
	break;

      case EllipticBlending:
	KImageEffect::blend( *m_pImage, *m_pBackground,
			     KImageEffect::EllipticGradient, 
			     bal, bal );
	break;

      case IntensityBlending:
	KImageEffect::modulate( *m_pImage, *m_pBackground, reverseBlending(),
		    KImageEffect::Intensity, bal, KImageEffect::All );
	break;

      case SaturateBlending:
	KImageEffect::modulate( *m_pImage, *m_pBackground, reverseBlending(),
		    KImageEffect::Saturation, bal, KImageEffect::Gray );
	break;

      case ContrastBlending:
	KImageEffect::modulate( *m_pImage, *m_pBackground, reverseBlending(),
		    KImageEffect::Contrast, bal, KImageEffect::All );
	break;

      case HueShiftBlending:
	KImageEffect::modulate( *m_pImage, *m_pBackground, reverseBlending(),
		    KImageEffect::HueShift, bal, KImageEffect::Gray );
	break;
      }
    }

    if (retval == Done)
        m_State |= WallpaperDone;

    return retval;
}


void KBackgroundRenderer::slotBackgroundDone(KProcess *)
{
    kdDebug() << "slotBackgroundDone" << endl;
    m_State |= BackgroundDone;

    if (m_pProc->normalExit() && !m_pProc->exitStatus())
        m_pBackground->load(m_Tempfile);

    unlink(m_Tempfile.latin1());
    m_pTimer->start(0, true);
}



/*
 * Starts the rendering process.
 */
void KBackgroundRenderer::start()
{
    if (m_pBackground == 0L)
	m_pBackground = new QImage();
    if (m_pImage == 0L)
	m_pImage = new QImage();

    m_State = Rendering;
    m_pTimer->start(0, true);
}


/*
 * This slot is connected to a timer event. It is called repeatedly until
 * the rendering is done.
 */
void KBackgroundRenderer::render()
{
    if (!(m_State & Rendering))
        return;

    int ret;

    if (!(m_State & BackgroundDone)) {
        ret = doBackground();
        if (ret != Wait)
	    m_pTimer->start(0, true);
	return;
    }

    // No async wallpaper
    doWallpaper();

    done();
}


/*
 * Rendering is finished.
 */
void KBackgroundRenderer::done()
{
    m_State |= AllDone;
    emit imageDone(desk());
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
    delete m_pBackground; m_pBackground = 0L;
    delete m_pImage; m_pImage = 0L;
    delete m_pProc; m_pProc = 0L;
    m_State = 0;
}


void KBackgroundRenderer::setPreview(QSize size)
{
    if (size.isNull())
        m_bPreview = false;
    else {
        m_bPreview = true;
        m_Size = size;
    }
}
	

void KBackgroundRenderer::setTile(bool tile)
{
    m_bTile = tile;
}


QImage *KBackgroundRenderer::image()
{
    if (m_State & AllDone)
        return m_pImage;
    return 0L;
}


void KBackgroundRenderer::load(int desk)
{
    if (m_State & Rendering)
        stop();
    cleanup();
    m_bPreview = false;
    m_bTile = false;
    m_Size = m_rSize;

    KBackgroundSettings::load(desk);
}


#include "bgrender.moc"
