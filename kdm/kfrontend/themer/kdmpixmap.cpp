/*
 *  Copyright (C) 2003 by Unai Garro <ugarro@users.sourceforge.net>
 *  Copyright (C) 2004 by Enrico Ros <rosenric@dei.unipd.it>
 *  Copyright (C) 2004 by Stephan Kulow <coolo@kde.org>
 *  Copyright (C) 2004 by Oswald Buddenhagen <ossi@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <config.h>

#include "kdmpixmap.h"

#include <kimageeffect.h>
#ifdef HAVE_LIBAGG
#include <ksvgiconengine.h>
#endif

#include <kdebug.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qimage.h>

KdmPixmap::KdmPixmap( KdmItem *parent, const QDomNode &node, const char *name )
    : KdmItem( parent, node, name )
{
	itemType = "pixmap";

	// Set default values for pixmap (note: strings are already Null)
	pixmap.normal.tint.setRgb( 0xFFFFFF );
	pixmap.normal.alpha = 1.0;
	pixmap.active.present = false;
	pixmap.prelight.present = false;

	// Read PIXMAP ID
	// it rarely happens that a pixmap can be a button too!
	QDomNode n = node;
	QDomElement elPix = n.toElement();

	// Read PIXMAP TAGS
	QDomNodeList childList = node.childNodes();
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "normal") {
			loadPixmap( el.attribute( "file", "" ), pixmap.normal.pixmap, pixmap.normal.fullpath );
			parseColor( el.attribute( "tint", "#ffffff" ), pixmap.normal.tint );
			pixmap.normal.alpha = el.attribute( "alpha", "1.0" ).toFloat();
		} else if (tagName == "active") {
			pixmap.active.present = true;
			loadPixmap( el.attribute( "file", "" ), pixmap.active.pixmap, pixmap.active.fullpath );
			parseColor( el.attribute( "tint", "#ffffff" ), pixmap.active.tint );
			pixmap.active.alpha = el.attribute( "alpha", "1.0" ).toFloat();
		} else if (tagName == "prelight") {
			pixmap.prelight.present = true;
			loadPixmap( el.attribute( "file", "" ), pixmap.prelight.pixmap, pixmap.prelight.fullpath );
			parseColor( el.attribute( "tint", "#ffffff" ), pixmap.prelight.tint );
			pixmap.prelight.alpha = el.attribute( "alpha", "1.0" ).toFloat();
		}
	}
}

QSize
KdmPixmap::sizeHint()
{
	// choose the correct pixmap class
	PixmapStruct::PixmapClass * pClass = &pixmap.normal;
	if (state == Sactive && pixmap.active.present)
		pClass = &pixmap.active;
	if (state == Sprelight && pixmap.prelight.present)
		pClass = &pixmap.prelight;
	// use the pixmap size as the size hint
	if (!pClass->pixmap.isNull())
		return pClass->pixmap.size();
	return KdmItem::sizeHint();
}

void
KdmPixmap::setGeometry( const QRect &newGeometry, bool force )
{
	KdmItem::setGeometry( newGeometry, force );
	pixmap.active.readyPixmap.resize( 0, 0 );
	pixmap.prelight.readyPixmap.resize( 0, 0 );
	pixmap.normal.readyPixmap.resize( 0, 0 );
}


void
KdmPixmap::loadPixmap( const QString &fileName, QPixmap &map, QString &fullName )
{
	if (fileName.isEmpty())
		return;

	fullName = fileName;
	if (fullName.at( 0 ) != '/')
		fullName = baseDir() + "/" + fileName;

	if (!fullName.endsWith( ".svg" ))	// we delay it for svgs
		if (!map.load( fullName ))
			fullName = QString::null;
}

void
KdmPixmap::renderSvg( PixmapStruct::PixmapClass *pClass, const QRect &area )
{
#ifdef HAVE_LIBAGG
	// Special stuff for SVG icons
	KSVGIconEngine *svgEngine = new KSVGIconEngine();

	if (svgEngine->load( area.width(), area.height(), pClass->fullpath )) {
		QImage *t = svgEngine->image();
		pClass->pixmap = *t;
		pClass->readyPixmap.resize( 0, 0 );
	} else {
		kdWarning() << "failed to load " << pClass->fullpath << endl;
		pClass->fullpath = QString::null;
	}

	delete svgEngine;
#else
        Q_UNUSED(pClass);
        Q_UNUSED(area);
#endif
}

void
KdmPixmap::drawContents( QPainter *p, const QRect &r )
{
	// choose the correct pixmap class
	PixmapStruct::PixmapClass *pClass = &pixmap.normal;
	if (state == Sactive && pixmap.active.present)
		pClass = &pixmap.active;
	if (state == Sprelight && pixmap.prelight.present)
		pClass = &pixmap.prelight;

	if (pClass->pixmap.isNull()) {
		if (pClass->fullpath.isEmpty())	// if neither is set, we're empty
			return;

		kdDebug() << "renderSVG\n";
		renderSvg( pClass, area );
	}

	int px = area.left() + r.left();
	int py = area.top() + r.top();
	int sx = r.x();
	int sy = r.y();
	int sw = r.width();
	int sh = r.height();
	if (px < 0) {
		px *= -1;
		sx += px;
		px = 0;
	}
	if (py < 0) {
		py *= -1;
		sy += py;
		py = 0;
	}


	if (pClass->readyPixmap.isNull()) {
		QImage scaledImage;

		// use the loaded pixmap or a scaled version if needed

		if (area.size() != pClass->pixmap.size()) {
			if (pClass->fullpath.endsWith( ".svg" )) {
				kdDebug() << "renderSVG\n";
				renderSvg( pClass, area );
				scaledImage = pClass->pixmap.convertToImage();
			} else {
				kdDebug() << "convertFromImage\n";
				QImage tempImage = pClass->pixmap.convertToImage();
				scaledImage = tempImage.smoothScale( area.width(), area.height() );
			}
		} else
			scaledImage = pClass->pixmap.convertToImage();

		bool haveTint = pClass->tint.rgb() != 0xFFFFFF;
		bool haveAlpha = pClass->alpha < 1.0;

		if (haveTint || haveAlpha) {
			// blend image(pix) with the given tint

			int w = scaledImage.width();
			int h = scaledImage.height();
			float tint_red = float( pClass->tint.red() ) / 255;
			float tint_green = float( pClass->tint.green() ) / 255;
			float tint_blue = float( pClass->tint.blue() ) / 255;
			float tint_alpha = pClass->alpha;

			for (int y = 0; y < h; ++y) {
				QRgb *ls = (QRgb *)scaledImage.scanLine( y );
				for (int x = 0; x < w; ++x) {
					QRgb l = ls[x];
					int r = int( qRed( l ) * tint_red );
					int g = int( qGreen( l ) * tint_green );
					int b = int( qBlue( l ) * tint_blue );
					int a = int( qAlpha( l ) * tint_alpha );
					ls[x] = qRgba( r, g, b, a );
				}
			}

		}

		pClass->readyPixmap.convertFromImage( scaledImage );
	}
	// kdDebug() << "Pixmap::drawContents " << pClass->readyPixmap.size() << " " << px << " " << py << " " << sx << " " << sy << " " << sw << " " << sh << endl;
	p->drawPixmap( px, py, pClass->readyPixmap, sx, sy, sw, sh );
}

void
KdmPixmap::statusChanged()
{
	KdmItem::statusChanged();
	if (!pixmap.active.present && !pixmap.prelight.present)
		return;
	if ((state == Sprelight && !pixmap.prelight.present) ||
	    (state == Sactive && !pixmap.active.present))
		return;
	needUpdate();
}

#include "kdmpixmap.moc"
