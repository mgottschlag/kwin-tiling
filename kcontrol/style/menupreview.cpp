/*
 * Menu Transparency Preview Widget
 * Copyright (C) 2002 Karol Szwed <gallium@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "menupreview.h"

#include <QPainter>
#include <QImage>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>

#include <KApplication>
#include <kpixmapeffect.h>
#include <klocale.h>
#include <kimageeffect.h>
#include <kiconloader.h>


MenuPreview::MenuPreview( QWidget* parent, int opacity, PreviewMode pvm )
	: QWidget( parent, Qt::WStyle_Customize | Qt::WNoAutoErase ),
	pixBackground(NULL), pixOverlay(NULL), pixBlended(NULL)
{
	setFixedSize(150, 150);
	setFocusPolicy( Qt::NoFocus );

	mode = pvm;
	if (opacity < 0)   opacity = 0;
	if (opacity > 100) opacity = 100;
	menuOpacity = opacity/100.0;

	pixBackground = new QPixmap();
	pixOverlay = new QPixmap();
	pixBlended = new QPixmap();

	createPixmaps();
	blendPixmaps();
}

MenuPreview::~MenuPreview()
{
	delete pixBackground;
	delete pixOverlay;
	delete pixBlended;
}

void MenuPreview::createPixmaps()
{
	int w = width()-2;
	int h = height()-2;

	if (pixBackground)
		*pixBackground = QPixmap( w, h );
	if (pixOverlay)
		*pixOverlay = QPixmap( w, h );
	if (pixBlended)
		*pixBlended = QPixmap( w, h );

	QColorGroup cg = QColorGroup( palette() );
	QColor c1 = cg.color( backgroundRole() );
	QColor c2 = cg.mid();

	if (pixBackground) {
		// Paint checkerboard
		QPainter p;
		p.begin(pixBackground);
		for(int x=0; x < pixBackground->width(); x+=5)
			for(int y=0; y < pixBackground->height(); y+=5)
				p.fillRect( x, y, 5, 5,
								 (x % 2) ?
								((y % 2) ?  c2 : c1  ) : 	// See the grid? ;-)
								((y % 2) ?  c1 : c2  ) );
		KIconLoader* icl = KIconLoader::global();
		QPixmap pix = icl->loadIcon("go", K3Icon::Desktop, K3Icon::SizeLarge, K3Icon::ActiveState);
		p.drawPixmap( (width()-2-pix.width())/2, (height()-2-pix.height())/2, pix );
	}

	if (pixOverlay) {
                c1 = cg.color(QPalette::Button).light(110);
		c2 = cg.color(QPalette::Button).dark(110);
		KPixmapEffect::gradient( *pixOverlay, c1, c2, KPixmapEffect::VerticalGradient );
	}
}

void MenuPreview::blendPixmaps()
{
	// Rebuild pixmaps, and repaint
	if (pixBlended && pixBackground)
	{
		if (mode == Blend && pixOverlay) {
			QImage src = pixOverlay->toImage();
			QImage dst = pixBackground->toImage();
			KImageEffect::blend(src, dst, menuOpacity);
			*pixBlended = QPixmap::fromImage( dst );
		} else if (mode == Tint) {
                        QColor clr = palette().color( QPalette::Button );
			QImage dst = pixBackground->toImage();
			KImageEffect::blend(clr, dst, menuOpacity);
			*pixBlended = QPixmap::fromImage( dst );
		}
	}
}

void MenuPreview::setOpacity( int opacity )
{
	if (opacity < 0 || opacity > 100)
		return;

	if ((int)(menuOpacity*100) != opacity) {
		menuOpacity = opacity/100.0;
		blendPixmaps();
		repaint( );
	}
}

void MenuPreview::setPreviewMode( PreviewMode pvm )
{
	if (mode != pvm) {
		mode = pvm;
		blendPixmaps();
		repaint( );
	}
}

void MenuPreview::paintEvent( QPaintEvent* /* pe */ )
{
	// Paint the frame and blended pixmap
	int x2 = width()-1;
	int y2 = height()-1;

	QPainter p(this);
	p.setPen(palette().color( QPalette::Active, QPalette::Dark ));
	p.drawLine(0, 0, x2, 0);
	p.drawLine(0, 0, 0, y2);
	p.setPen(palette().color( QPalette::Active, QPalette::Light));
	p.drawLine(1, y2, x2, y2);
	p.drawLine(x2, 1, x2, y2);

	if (mode == NoEffect)
            p.fillRect(1, 1, --x2, --y2, palette().color( QPalette::Active, QPalette::Button ) );
	else if (mode != NoEffect && pixBlended)
		p.drawPixmap(1, 1, *pixBlended, 0, 0, --x2, --y2);

	QRect r = rect();
	r.translate(6,3);
	p.setPen( palette().color( QPalette::Active, QPalette::Text ) );
	p.drawText( r, Qt::AlignTop | Qt::AlignLeft, QString::number((int)(menuOpacity*100))+i18n("%") );
}

#include "menupreview.moc"

// vim: set noet ts=4:

