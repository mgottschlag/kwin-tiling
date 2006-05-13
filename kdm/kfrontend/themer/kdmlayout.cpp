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
KdmLayoutFixed::update( const QRect &parentGeometry, bool force )
{
	kDebug() << "KdmLayoutFixed::update " << parentGeometry << endl;

	// I can't layout children if the parent rectangle is not valid
	if (parentGeometry.width() < 0 || parentGeometry.height() < 0) {
		kDebug() << "invalid\n";
		return;
	}
	// For each child in list I ask their hinted size and set it!
	for (QList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it)
		(*it)->setGeometry( (*it)->placementHint( parentGeometry ), force );
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
KdmLayoutBox::update( const QRect &parentGeometry, bool force )
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
		for (QList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it)
			if (!(*it)->isExplicitlyHidden())
				ccnt++;
		int height = (childrenRect.height() - (ccnt - 1) * box.spacing) / ccnt;
		int width = (childrenRect.width() - (ccnt - 1) * box.spacing) / ccnt;

		for (QList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
			if ((*it)->isExplicitlyHidden())
				continue;
			if (box.isVertical) {
				QRect temp( childrenRect.left(), childrenRect.top(), childrenRect.width(), height );
				(*it)->setGeometry( temp, force );
				childrenRect.setTop( childrenRect.top() + height + box.spacing );
			} else {
				QRect temp( childrenRect.left(), childrenRect.top(), width, childrenRect.height() );
				kDebug() << "placement " << *it << " " << temp << " " << (*it)->placementHint( temp ) << endl;
				temp = (*it)->placementHint( temp );
				(*it)->setGeometry( temp, force );
				childrenRect.setLeft( childrenRect.left() + width + box.spacing );
			}
		}
	} else {
		for (QList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
			if ((*it)->isExplicitlyHidden())
				continue;

			QRect temp = childrenRect, itemRect;
			if (box.isVertical) {
				temp.setHeight( 0 );
				itemRect = (*it)->placementHint( temp );
				temp.setHeight( itemRect.height() );
				childrenRect.setTop( childrenRect.top() + itemRect.size().height() + box.spacing );
			} else {
				temp.setWidth( 0 );
				itemRect = (*it)->placementHint( temp );
				kDebug() << this << " placementHint " << *it << " " << temp << " " << itemRect << endl;
				temp.setWidth( itemRect.width() );
				childrenRect.setLeft( childrenRect.left() + itemRect.size().width() + box.spacing );
				kDebug() << "childrenRect after " << *it << " " << childrenRect << endl;
			}
			itemRect = (*it)->placementHint( temp );
			kDebug() << this << " placementHint2 " << *it << " " << temp << " " << itemRect << endl;
			(*it)->setGeometry( itemRect, force );
		}
	}
}

//FIXME truly experimental (is so close to greeter_geometry.c)
QSize
KdmLayoutBox::sizeHint()
{
	// Sum up area taken by children
	int w = 0, h = 0;
	for (QList<KdmItem *>::ConstIterator it = m_children.begin(); it != m_children.end(); ++it) {
		QSize s = (*it)->placementHint( QRect() ).size();
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
