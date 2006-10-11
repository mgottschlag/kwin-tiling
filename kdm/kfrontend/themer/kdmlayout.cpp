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

KdmLayoutFixed::KdmLayoutFixed( const QDomNode &/*node*/ )
{
	//Parsing FIXED parameters on 'node' [NONE!]
}

void
KdmLayoutFixed::update( QStack<QSize> &parentSizes, const QRect &parentGeometry, bool force )
{
	enter("Fixed::update") << parentGeometry << " depth " << parentSizes.size() << endl;

	// I can't layout children if the parent rectangle is not valid
	if (parentGeometry.width() < 0 || parentGeometry.height() < 0) {
		leave() << "invalid geometry" << endl;
		return;
	}
	// For each child in list I ask their hinted size and set it!
	parentSizes.push( parentGeometry.size() );
	forEachChild (itm)
		itm->setGeometry( parentSizes, itm->placementHint( parentSizes, parentGeometry.topLeft() ), force );
	parentSizes.pop();
	leave() << "done" << endl;
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

struct LayoutHint {
	int min, opt, max, set;
	bool done;

	LayoutHint() : done( false )
	{
	}
};

void
KdmLayoutBox::update( QStack<QSize> &parentSizes, const QRect &parentGeometry, bool force )
{
	enter("Box::update") << parentGeometry << " depth " << parentSizes.size() << endl;

	// I can't layout children if the parent rectangle is not valid
	if (!parentGeometry.isValid() || parentGeometry.isEmpty()) {
		leave() << "invalid geometry" << endl;
		return;
	}

	QRect childrenRect = parentGeometry;
	// Begin cutting the parent rectangle to attach children on the right place
	childrenRect.adjust( box.xpadding, box.ypadding, -box.xpadding, -box.ypadding );

	debug() << "childrenRect " << childrenRect << endl;

	// For each child in list ...
	if (box.homogeneous) {
		int ccnt = 0;
		forEachVisibleChild (itm)
			ccnt++;
		forEachVisibleChild (itm) {
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
		QVector<LayoutHint> lhs;
		int ccnt = 0, mintot = 0, opttot = 0, esum = 0;
		parentSizes.push( QSize( 0, 0 ) );
		forEachVisibleChild (itm) {
			SizeHint sh;
			itm->sizingHint( parentSizes, sh );
			lhs.resize( ccnt + 1 );
			if (box.isVertical) {
				lhs[ccnt].min = sh.min.height();
				lhs[ccnt].opt = sh.opt.height();
				lhs[ccnt].max = sh.max.height();
			} else {
				lhs[ccnt].min = sh.min.width();
				lhs[ccnt].opt = sh.opt.width();
				lhs[ccnt].max = sh.max.width();
			}
			mintot += lhs[ccnt].min;
			opttot += lhs[ccnt].opt;
			if (itm->geom.expand)
				esum += itm->geom.expand;
			else {
				lhs[ccnt].done = true;
				lhs[ccnt].set = lhs[ccnt].opt;
			}
			ccnt++;
		}
		parentSizes.pop();
		int havetot = box.isVertical ? childrenRect.size().height() : childrenRect.size().width();
		int spacing;
		if (havetot < opttot) {
			// fix your theme, dude
			if (havetot < mintot) {
				for (int i = 0; i < ccnt; i++) {
					lhs[i].set = lhs[i].min * havetot / mintot;
					havetot -= lhs[i].set;
					mintot -= lhs[i].min;
				}
			} else {
				for (int i = 0; i < ccnt; i++) {
					lhs[i].set = lhs[i].opt * havetot / opttot;
					havetot -= lhs[i].set;
					opttot -= lhs[i].opt;
				}
			}
			spacing = 0;
		} else {
			spacing = box.spacing;
			if (havetot < opttot + (ccnt - 1) * spacing)
				spacing = (havetot - opttot) / (ccnt - 1);
			int extra = havetot - opttot - (ccnt - 1) * spacing;
			int tesum, wesum, textra, wextra;
			do {
				tesum = wesum = esum;
				textra = wextra = extra;
				int idx = 0;
				forEachVisibleChild (itm) {
					if (!lhs[idx].done) {
						int mex = itm->geom.expand * wextra / wesum;
						wextra -= mex;
						wesum -= itm->geom.expand;
						if (lhs[idx].opt + mex > lhs[idx].max) {
							lhs[idx].set = lhs[idx].max;
							lhs[idx].done = true;
							esum -= itm->geom.expand;
							extra -= lhs[idx].opt;
						} else
							lhs[idx].set = lhs[idx].opt + mex;
					}
					idx++;
				}
			} while (tesum != esum);
		}
		int idx = 0;
		forEachVisibleChild (itm) {
			QRect temp = childrenRect;
			if (box.isVertical) {
				temp.setHeight( lhs[idx].set );
				childrenRect.setTop( childrenRect.top() + lhs[idx].set + spacing );
			} else {
				temp.setWidth( lhs[idx].set );
				childrenRect.setLeft( childrenRect.left() + lhs[idx].set + spacing );
			}
			parentSizes.push( temp.size() );
			QRect itemRect = itm->placementHint( parentSizes, temp.topLeft() );
			parentSizes.pop();
			debug() << "placementHint for " << itm << " temp " << temp << " final "
				<< itemRect << " childrenRect now " << childrenRect << endl;
			parentSizes.push( parentGeometry.size() );
			itm->setGeometry( parentSizes, itemRect, force );
			parentSizes.pop();
			idx++;
		}
	}
	leave() << "done" << endl;
}

QSize
KdmLayoutBox::sizeHint( QStack<QSize> &parentSizes )
{
	enter("Box::sizeHint") << "parentSize #" << parentSizes.size() << " "
		<< parentSizes.top() << endl;

	int ccnt = 0;
	QSize bounds( 0, 0 ), sum( 0, 0 );

	// Sum up area taken by children
	parentSizes.push( QSize( 0, 0 ) );
	forEachVisibleChild (itm) {
		SizeHint sh;
		itm->sizingHint( parentSizes, sh );
		bounds = bounds.expandedTo( sh.opt );
		sum += sh.opt;
		ccnt++;
	}
	parentSizes.pop();

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

	leave() << "bounds " << bounds << endl;
	
	// Make hint at least equal to minimum size (if set)
	return bounds.expandedTo( QSize( box.minwidth, box.minheight ) );
}
