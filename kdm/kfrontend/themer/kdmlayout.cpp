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

#include "kdmlayout.h"
#include "kdmitem.h"

#include <kdebug.h>

#include <qdom.h>
#include <QRect>
//Added by qt3to4:
#include <QList>

KdmLayoutFixed::KdmLayoutFixed( const QDomNode &/*node*/ )
{
	//Parsing FIXED parameters on 'node' [NONE!]
}

void
KdmLayoutFixed::update( QStack<QRect> &parentGeometries, bool force )
{
	const QRect &parentGeometry = parentGeometries.top();
	kDebug() << "KdmLayoutFixed::update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (parentGeometry.width() < 0 || parentGeometry.height() < 0) {
		kDebug() << "invalid\n";
		return;
	}
	// For each child in list I ask their hinted size and set it!
	foreach (KdmItem *itm, m_children) {
		parentGeometries.push( itm->placementHint( parentGeometries ) );
		itm->setGeometry( parentGeometries, force );
		parentGeometries.pop();
	}
}

KdmLayoutBox::KdmLayoutBox( const QDomNode &node )
{
	//Parsing BOX parameters
	QDomNode n = node;
	QDomElement el = n.toElement();
	box.isVertical = el.attribute( "orientation", "vertical" ) != "horizontal";
	box.xpadding = el.attribute( "xpadding", "0" ).toInt();
	box.ypadding = el.attribute( "ypadding", "0" ).toInt();
	box.spacing = el.attribute( "spacing", "0" ).toInt();
	box.minwidth = el.attribute( "min-width", "0" ).toInt();
	box.minheight = el.attribute( "min-height", "0" ).toInt();
	box.homogeneous = el.attribute( "homogeneous", "false" ) == "true";
}

void
KdmLayoutBox::update( QStack<QRect> &parentGeometries, bool force )
{
	QRect parentGeometry = parentGeometries.pop();
	kDebug() << this << " update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (!parentGeometry.isValid() || parentGeometry.isEmpty()) {
		parentGeometries.push( parentGeometry );
		return;
	}

	// Check if box size was computed. If not compute it
	// TODO check if this prevents updating changing items
//	if (!hintedSize.isValid())
//		sizeHint();

//	kDebug() << this << " hintedSize " << hintedSize << endl;

	//XXX why was this asymmetric? it broke things big time.
	QRect childrenRect = /*box.isVertical ? QRect( parentGeometry.topLeft(), hintedSize ) :*/ parentGeometry;
	// Begin cutting the parent rectangle to attach children on the right place
	childrenRect.adjust( box.xpadding, box.ypadding, -box.xpadding, -box.ypadding );

	kDebug() << this << " childrenRect " << childrenRect << endl;

	// For each child in list ...
	if (box.homogeneous) {
		int ccnt = 0;
		foreach (KdmItem *itm, m_children)
			if (!itm->isExplicitlyHidden())
				ccnt++;
		int height = (childrenRect.height() - (ccnt - 1) * box.spacing) / ccnt;
		int width = (childrenRect.width() - (ccnt - 1) * box.spacing) / ccnt;

		foreach (KdmItem *itm, m_children) {
			if (itm->isExplicitlyHidden())
				continue;
			QRect temp = childrenRect, itemRect;
			if (box.isVertical) {
				temp.setHeight( height );
				childrenRect.setTop( childrenRect.top() + height + box.spacing );
			} else {
				temp.setWidth( width );
				childrenRect.setLeft( childrenRect.left() + width + box.spacing );
			}
			parentGeometries.push( temp );
			itemRect = itm->placementHint( parentGeometries );
			parentGeometries.pop();
			parentGeometries.push( parentGeometry );
			parentGeometries.push( itemRect );
			itm->setGeometry( parentGeometries, force );
			parentGeometries.pop();
			parentGeometries.pop();
		}
	} else {
		foreach (KdmItem *itm, m_children) {
			if (itm->isExplicitlyHidden())
				continue;
			QRect temp = childrenRect, itemRect;
			if (box.isVertical) {
				temp.setHeight( 0 );
				parentGeometries.push( temp );
				itemRect = itm->placementHint( parentGeometries );
				parentGeometries.pop();
				temp.setHeight( itemRect.height() );
				childrenRect.setTop( childrenRect.top() + itemRect.height() + box.spacing );
			} else {
				temp.setWidth( 0 );
				parentGeometries.push( temp );
				itemRect = itm->placementHint( parentGeometries );
				parentGeometries.pop();
				temp.setWidth( itemRect.width() );
				childrenRect.setLeft( childrenRect.left() + itemRect.width() + box.spacing );
			}
			parentGeometries.push( temp );
			itemRect = itm->placementHint( parentGeometries );
			parentGeometries.pop();
			kDebug() << this << " placementHint for " << itm << " temp " << temp << " final " << itemRect << " childrenRect now " << childrenRect << endl;
			parentGeometries.push( parentGeometry );
			parentGeometries.push( itemRect );
			itm->setGeometry( parentGeometries, force );
			parentGeometries.pop();
			parentGeometries.pop();
		}
	}
	
	parentGeometries.push( parentGeometry );
}

//FIXME truly experimental (is so close to greeter_geometry.c)
QSize
KdmLayoutBox::sizeHint( QStack<QRect> &parentGeometries )
{
	// Sum up area taken by children
	QRect parentGeometry = parentGeometries.pop();
	parentGeometries.push( QRect() );
	int w = 0, h = 0;
	foreach (KdmItem *itm, m_children) {
		QSize s = itm->placementHint( parentGeometries ).size();
		if (box.isVertical) {
			if (s.width() > w)
				w = s.width();
			h += s.height();
		} else {
			if (s.height() > h)
				h = s.height();
			w += s.width();
		}
	}
	parentGeometries.pop();
	parentGeometries.push( parentGeometry );

	// Add padding and items spacing
	w += 2 * box.xpadding;
	h += 2 * box.ypadding;
	if (box.isVertical)
		h += box.spacing * (m_children.count() - 1);
	else
		w += box.spacing * (m_children.count() - 1);

	// Make hint at least equal to minimum size (if set)
	return QSize( w < box.minwidth ? box.minwidth : w,
	              h < box.minheight ? box.minheight : h );
}
