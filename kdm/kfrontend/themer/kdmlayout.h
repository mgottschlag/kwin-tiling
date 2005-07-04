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

#ifndef KDMLAYOUT_H
#define KDMLAYOUT_H

/**
 * this is a container for a lot of other stuff
 * but can be treated like a usual widget
 */

#include <qvaluelist.h>
#include <qsize.h>

class KdmItem;

class QDomNode;
class QRect;

class KdmLayout {

public:
//	virtual ~KdmLayout() {};

	// Adds an item that will be managed
	void addItem( KdmItem *item ) { m_children.append( item ); }

	// Return false if any item are managed by this layouter
	bool isEmpty() { return m_children.isEmpty(); }

	// Updates the layout of all items knowing that the parent
	// has the @p parentGeometry geometry
//	virtual void update( const QRect &parentGeometry ) = 0;

protected:
	QValueList<KdmItem *> m_children;
};

class KdmLayoutFixed : public KdmLayout {

public:
	KdmLayoutFixed( const QDomNode &node );

	// Updates the layout of all boxed items knowing that the parent
	// has the @p parentGeometry geometry
	void update( const QRect &parentGeometry, bool force );
};

/**
 * this is a container for a lot of other stuff
 * but can be treated like a usual widget
 */

class KdmLayoutBox : public KdmLayout {

public:
	KdmLayoutBox( const QDomNode &node );

	// Updates the layout of all boxed items knowing that they
	// should fit into @p parentGeometry container
	void update( const QRect &parentGeometry, bool force );

	// Computes the size hint of the box, telling which is the
	// smallest size inside which boxed items will fit
	QSize sizeHint();

private:
	struct {
		bool isVertical;
		int spacing;
		int xpadding;
		int ypadding;
		int minwidth;
		int minheight;
		bool homogeneous;
	} box;
//	QSize hintedSize;
};

#endif
