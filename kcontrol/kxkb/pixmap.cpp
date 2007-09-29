/*
 *  Copyright (C) 2003-2006 Andriy Rysin (rysin@kde.org)
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

#include <QImage>
#include <QFont>
#include <QPainter>
#include <QRegExp>
#include <QHash>

#include <kstandarddirs.h>
#include <klocale.h>
#include <kdebug.h>

#include "pixmap.h"
#include "rules.h"
#include "kxkbconfig.h"


static const int FLAG_MAX_WIDTH = 21;
static const int FLAG_MAX_HEIGHT = 14;

const QString LayoutIcon::flagTemplate("l10n/%1/flag.png");
const QString& LayoutIcon::ERROR_CODE("error");
LayoutIcon* LayoutIcon::instance;


LayoutIcon& LayoutIcon::getInstance() {
	if( instance == NULL ) {
		instance = new LayoutIcon();
	}
	return *instance;
}

LayoutIcon::LayoutIcon():
		m_pixmapCache(),
		m_labelFont("sans")
{
	m_labelFont.setPixelSize(10);
	m_labelFont.setWeight(QFont::Bold);
}

const QPixmap&
LayoutIcon::findPixmap(const QString& code_, bool showFlag, const QString& displayName_)
{
	QPixmap* pm = NULL;

	if( code_ == ERROR_CODE ) {
		pm = m_pixmapCache[ERROR_CODE];
		if( pm == NULL ) {
			pm = createErrorPixmap();
			m_pixmapCache.insert(ERROR_CODE, pm);
		}
		return *pm;
	}

	QString displayName(displayName_);

	if( displayName.isEmpty() ) {
		displayName = KxkbConfig::getDefaultDisplayName(code_);
	}
	if( displayName.length() > 3 )
		displayName = displayName.left(3);

	const QString pixmapKey( showFlag ? code_ + '.' + displayName : displayName );

	pm = m_pixmapCache[pixmapKey];
	if( pm )
		return *pm;

	QString flag;
	if( showFlag ) {
		QString countryCode = getCountryFromLayoutName( code_ );
		flag = KStandardDirs::locate("locale", flagTemplate.arg(countryCode));
	}

	if( flag.isEmpty() ) {
		pm = new QPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
		pm->fill(Qt::gray);
	}
	else {
		pm = new QPixmap(flag);
		dimPixmap( *pm );

#if 0
		if( pm->height() < FLAG_MAX_HEIGHT ) {
			QPixmap* pix = new QPixmap(FLAG_MAX_WIDTH, FLAG_MAX_HEIGHT);
			pix->fill( Qt::lightGray );
//			pix->fill( QColor(qRgba(127,127,127,255)) );
//			QBitmap mask;
//			mask.fill(1);
//			pix->setMask(mask);

			int dy = (pix->height() - pm->height()) / 2;
			copyBlt( pix, 0, dy, pm, 0, 0, -1, -1 );
//			QPixmap* px = new QPixmap(21, 14);
//			px->convertFromImage(img);*/
			delete pm;
			pm = pix;
		}
#endif
	}

	QPainter p(pm);
	p.setFont(m_labelFont);

	p.setPen(Qt::black);
	p.drawText(1, 1, pm->width(), pm->height()-2, Qt::AlignCenter, displayName);
	p.setPen(Qt::white);
	p.drawText(0, 0, pm->width(), pm->height()-2, Qt::AlignCenter, displayName);

	m_pixmapCache.insert(pixmapKey, pm);

	return *pm;
}

/**
@brief Try to get country code from layout name in xkb before xorg 6.9.0
*/
QString LayoutIcon::getCountryFromLayoutName(const QString& layoutName)
{
	QString flag;

		if( layoutName == "mkd" )
			flag = "mk";
		else
		if( layoutName == "srp" ) {
			QString csFlagFile = KStandardDirs::locate("locale", flagTemplate.arg("cs"));
			flag = csFlagFile.isEmpty() ? "yu" : "cs";
		}
		else
			if( layoutName.endsWith("/jp") )
				flag = "jp";
        else
            if( layoutName == "trq" || layoutName == "trf" || layoutName == "tralt" )
                flag = "tr";
		else
			if( layoutName.length() > 2 )
				flag = "";
		else
				flag = layoutName;

    return flag;
}


void LayoutIcon::dimPixmap(QPixmap& pm)
{
	QImage image = pm.toImage();
	for (int y=0; y<image.height(); y++)
		for(int x=0; x<image.width(); x++)
	{
		QRgb rgb = image.pixel(x,y);
		QRgb dimRgb(qRgb(qRed(rgb)*3/4, qGreen(rgb)*3/4, qBlue(rgb)*3/4));
		image.setPixel(x, y, dimRgb);
	}
	pm = QPixmap::fromImage(image);
}

static const char* ERROR_LABEL = "err";

//private
QPixmap* LayoutIcon::createErrorPixmap()
{
	QPixmap* pm = new QPixmap(21, 14);
	pm->fill(Qt::white);

	QPainter p(pm);

	p.setFont(m_labelFont);
	p.setPen(Qt::red);
	p.drawText(1, 1, pm->width(), pm->height()-2, Qt::AlignCenter, ERROR_LABEL);
	p.setPen(Qt::blue);
	p.drawText(0, 0, pm->width(), pm->height()-2, Qt::AlignCenter, ERROR_LABEL);
	m_pixmapCache.insert(ERROR_CODE, pm);

	return pm;
}
