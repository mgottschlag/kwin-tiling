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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kdmrect.h"
#include "kdmthemer.h"

#include <kimageeffect.h>
#include <kdebug.h>

#include <qimage.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QPixmap>

KdmRect::KdmRect( KdmItem *parent, const QDomNode &node, const char *name )
    : KdmItem( parent, node, name )
{
	itemType = "rect";

	// Set default values for rect (note: strings are already Null)
	rect.normal.alpha = 1;
	rect.active.present = false;
	rect.prelight.present = false;
	rect.hasBorder = false;

	// A rect can have no properties (defaults to parent ones)
	if (node.isNull())
		return;

	// Read RECT ID
	QDomNode n = node;
	QDomElement elRect = n.toElement();

	// Read RECT TAGS
	QDomNodeList childList = node.childNodes();
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName();

		if (tagName == "normal") {
			parseColor( el.attribute( "color", QString::null ), rect.normal.color );
			rect.normal.alpha = el.attribute( "alpha", "1.0" ).toFloat();
			parseFont( el.attribute( "font", "Sans 14" ), rect.normal.font );
		} else if (tagName == "active") {
			rect.active.present = true;
			parseColor( el.attribute( "color", QString::null ), rect.active.color );
			rect.active.alpha = el.attribute( "alpha", "1.0" ).toFloat();
			parseFont( el.attribute( "font", "Sans 14" ), rect.active.font );
		} else if (tagName == "prelight") {
			rect.prelight.present = true;
			parseColor( el.attribute( "color", QString::null ), rect.prelight.color );
			rect.prelight.alpha = el.attribute( "alpha", "1.0" ).toFloat();
			parseFont( el.attribute( "font", "Sans 14" ), rect.prelight.font );
		} else if (tagName == "border")
			rect.hasBorder = true;
	}
}

void
KdmRect::drawContents( QPainter *p, const QRect &r )
{
	// choose the correct rect class
	RectStruct::RectClass *rClass = &rect.normal;
	if (state == Sactive && rect.active.present)
		rClass = &rect.active;
	if (state == Sprelight && rect.prelight.present)
		rClass = &rect.prelight;

	if (rClass->alpha <= 0 || !rClass->color.isValid())
		return;

	if (rClass->alpha == 1)
		p->fillRect( area, QBrush( rClass->color ) );
	else {
		QRect backRect = r;
		backRect.moveBy( area.x(), area.y() );
		QPixmap backPixmap( backRect.size() );
		bitBlt( &backPixmap, QPoint( 0, 0 ), p->device(), backRect );
		QImage backImage = backPixmap.convertToImage();
		KImageEffect::blend( rClass->color, backImage, rClass->alpha );
		p->drawImage( backRect.x(), backRect.y(), backImage );
		//  area.moveBy(1,1);
	}
}

void
KdmRect::statusChanged()
{
	KdmItem::statusChanged();
	if (!rect.active.present && !rect.prelight.present)
		return;
	if ((state == Sprelight && !rect.prelight.present) ||
	    (state == Sactive && !rect.active.present))
		return;
	needUpdate( area.x(), area.y(), area.width(), area.height() );
}

/*
void
KdmRect::setAttribs( QWidget *widget )
{
	widget->setFont( rect.normal.font );
}

void
KdmRect::recursiveSetAttribs( QLayoutItem *li )
{
    QWidget *w;
    QLayout *l;

    if ((w = li->widget()))
	setAttribs( w );
    else if ((l = li->layout())) {
	QLayoutIterator it = l->iterator();
	for (QLayoutItem *itm = it.current(); itm; itm = ++it)
	     recursiveSetAttribs( itm );
    }
}

void
KdmRect::setWidget( QWidget *widget )
{
	KdmItem::setWidget( widget );
	setAttribs( widget );
}

void
KdmRect::setLayoutItem( QLayoutItem *item )
{
	KdmItem::setLayoutItem( item );
	recursiveSetAttribs( item );
}
*/

#include "kdmrect.moc"
