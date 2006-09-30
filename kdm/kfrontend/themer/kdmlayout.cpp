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
KdmLayoutFixed::update( QStack<QSize> &parentSizes, const QRect &parentGeometry, bool force )
{
	kDebug() << "KdmLayoutFixed::update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (parentGeometry.width() < 0 || parentGeometry.height() < 0) {
		kDebug() << "invalid\n";
		return;
	}
	// For each child in list I ask their hinted size and set it!
	parentSizes.push( parentGeometry.size() );
	foreach (KdmItem *itm, m_children)
		itm->setGeometry( parentSizes, itm->placementHint( parentSizes, parentGeometry.topLeft() ), force );
	parentSizes.pop();
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
KdmLayoutBox::update( QStack<QSize> &parentSizes, const QRect &parentGeometry, bool force )
{
	kDebug() << this << " update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (!parentGeometry.isValid() || parentGeometry.isEmpty())
		return;

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
		foreach (KdmItem *itm, m_children) {
			if (itm->isExplicitlyHidden())
				continue;
			QRect temp = childrenRect;
			if (box.isVertical) {
				int height = (temp.height() - (ccnt - 1) * box.spacing) / ccnt;
				temp.setHeight( height );
				childrenRect.setTop( childrenRect.top() + height + box.spacing );
			} else {
				int width = (temp.width() - (ccnt - 1) * box.spacing) / ccnt;
				temp.setWidth( width );
				childrenRect.setLeft( childrenRect.left() + width + box.spacing );
			}
			parentSizes.push( temp.size() );
			QRect itemRect = itm->placementHint( parentSizes, temp.topLeft() );
			parentSizes.pop();
			parentSizes.push( parentGeometry.size() );
			itm->setGeometry( parentSizes, itemRect, force );
			parentSizes.pop();
			ccnt--;
		}
	} else {
		foreach (KdmItem *itm, m_children) {
			if (itm->isExplicitlyHidden())
				continue;
			parentSizes.push( QSize( 0, 0 ) );
			QSize itemSize = itm->sizingHint( parentSizes );
			parentSizes.pop();
			QRect temp = childrenRect;
			if (box.isVertical) {
				temp.setHeight( itemSize.height() );
				childrenRect.setTop( childrenRect.top() + itemSize.height() + box.spacing );
			} else {
				temp.setWidth( itemSize.width() );
				childrenRect.setLeft( childrenRect.left() + itemSize.width() + box.spacing );
			}
			parentSizes.push( temp.size() );
			QRect itemRect = itm->placementHint( parentSizes, temp.topLeft() );
			parentSizes.pop();
			kDebug() << this << " placementHint for " << itm << " temp " << temp << " final " << itemRect << " childrenRect now " << childrenRect << endl;
			parentSizes.push( parentGeometry.size() );
			itm->setGeometry( parentSizes, itemRect, force );
			parentSizes.pop();
		}
	}
}

//FIXME truly experimental (is so close to greeter_geometry.c)
QSize
KdmLayoutBox::sizeHint( QStack<QSize> &parentSizes )
{
	int ccnt = 0;
	QSize bounds( 0, 0 ), sum( 0, 0 );

	// Sum up area taken by children
	QSize parentSize = parentSizes.pop();
	parentSizes.push( QSize( 0, 0 ) );
	foreach (KdmItem *itm, m_children) {
		if (itm->isExplicitlyHidden())
			continue;
		QSize s = itm->sizingHint( parentSizes );
		bounds = bounds.expandedTo( s );
		sum += s;
		ccnt++;
	}
	parentSizes.pop();
	parentSizes.push( parentSize );

	if (box.homogeneous) {
		if (box.isVertical)
			bounds.rheight() *= ccnt;
		else
			bounds.rwidth() *= ccnt;
	} else {
		if (box.isVertical)
			bounds.rheight() = sum.height();
		else
			bounds.rwidth() = sum.width();
	}
	
	// Add padding and items spacing
	int totspc = box.spacing * (ccnt - 1);
	if (box.isVertical)
		bounds.rheight() += totspc;
	else
		bounds.rwidth() += totspc;
	bounds += QSize( 2 * box.xpadding, 2 * box.ypadding );

	// Make hint at least equal to minimum size (if set)
	return bounds.expandedTo( QSize( box.minwidth, box.minheight ) );
}
