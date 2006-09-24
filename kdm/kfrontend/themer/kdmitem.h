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

#ifndef KDMITEM_H
#define KDMITEM_H

#include <QObject>
#include <QRect>
#include <qdom.h>
#include <QFont>

class KdmItem;
class KdmLayoutBox;
class KdmLayoutFixed;
class KdmThemer;

class QPainter;

/** class KdmItem
 * @short Base class for every kdmthemes' element.
 *
 * This class provides methods for arranging it and its children to the
 * screen (see note below), painting the whole area or a sub-region using
 * an opened painter, handling mouse events or events in general dispatching
 * them to children and sending some signals to the root (for example on
 * mouse click).
 *
 * KdmItem sits in a hierarchical top to bottom tree with signals that
 * traverse the tree back from leafs (or inner nodes) to the root.
 *
 * To implement a KdmItem only a few virtual protected methods must be
 * reimplemented, other virtual functions are there for convenience only -
 * the default implementation should satisfy your needs.
 */

/**
 * A note on layouting - how does it work?
 *  - setgeometry is called by parent (passing the new geometry)
 *    - item changes its geometry
 *    - if item embeds a widget, reposition it too
 *    - call children's box manager. box->update( my geom )
 *      - sum up the whole space taken by children (via *hint calls) if
 *        needed for box width / height computation. note that the computed
 *        geometry should be equal or similar to parent's geometry.
 *      - pad the rectangle bounding box' contents
 *      - for every child
 *        - if vertical
 *          ( use a top-to-bottom insertion, spacing insertion lines by
 *            children's individual height )
 *          - set up a zero height Parent (placed at the insertion line's
 *            position) and get Geom = child->placementHint( p )
 *          - set up child's Size using Parent's width and Geom's height.
 *          - call to child->setGeometry( Parent.topLeft, Size )
 *        - if horizontal
 *          - flows like the vertical one but uses a left-to-right insertion
 *            and insertion entry points are vertical lines
 *    - call to children's fix manager. fixed->update( my geom )
 *      - for every child
 *          - S = get child's geometry hint (and we'll give item the whole
 *            space it needs, without constraints)
 *          - call to child->setGeometry( S )
 *    - TODO: send a selective redraw signal also merging children's areas
 */

class KdmItem : public QObject {
	Q_OBJECT

	friend class KdmThemer;

public:
	/**
	 * Item constructor and destructor
	 */
	KdmItem( QObject *parent, const QDomNode &node );
	virtual ~KdmItem();

	/**
	 * Fixup the geometry of an item and its children (even if fixed
	 * or boxed ones). Note that this will generate repaint signals
	 * when needed. The default implementation should fit all needs.
	 */
	virtual void setGeometry( const QRect &newGeometry, bool force );

	/**
	 * Paint the item and its children using the given painter.
	 * This is the compositing core function. It buffers paint operations
	 * to speed up rendering of dynamic objects.
	 */
	void paint( QPainter *painter, const QRect &boundaries );

	/**
	 * Update representation of contents and repaint.
	 */
	virtual void update();

	/**
	 * Handle mouse motion and dispatch events to children. This
	 * leads to items prelighting, activation() on click and more..
	 */
	void mouseEvent( int x, int y, bool pressed = false, bool released = false );

	/**
	 * Similar to sizeHint(..), this returns the area of the item
	 * given the @p parentGeometry. The default implementation
	 * takes into account geometric constraints and layoutings.
	 * @param parentGeometry the geometry of the caller item or a
	 * null rect if the geometry of the parent is not yet defined.
	 */
	virtual QRect placementHint( const QRect &parentGeometry );

	/**
	 * Create the box layout manager; next children will be
	 * managed by the box layouter
	 */
	void setBoxLayout( const QDomNode &node = QDomNode() );

	/**
	 * Create the fixed layout manager; next children will be
	 * in fixed position relative to this item
	 */
	void setFixedLayout( const QDomNode &node = QDomNode() );

	QString type() const { return itemType; }
	void setType( const QString &t ) { itemType = t; }
	void setIsButton( bool on ) { isButton = on; }

	KdmItem *findNode( const QString &id ) const;
	virtual void setWidget( QWidget *widget );
	void showWidget();

	void hide( bool force = false );
	void show( bool force = false );

	bool isHidden() const { return isShown != Shown; }
	bool isExplicitlyHidden() const { return isShown == ExplicitlyHidden; }
	QRect rect() const { return area; }

Q_SIGNALS:
	void needUpdate( int x, int y, int w, int h );
	void needPlacement();
	void activated( const QString &id );

protected Q_SLOTS:
	void widgetGone();

protected:
	KdmThemer *themer();

	/**
	 * Returns the optimal/minimal size for this item.
	 * This should be reimplemented in items like label and pixmap.
	 * @return (-1,-1) if no size can be determined (so it should
	 * default to parent's size).
	 */
	virtual QSize sizeHint();

	/**
	 * Low level graphical function to paint the item.
	 * All items must reimplement this function to draw themeselves
	 * (or a part of) into the @p image keeping inside the @p rect .
	 * Try to do this as fast as possible.
	 * @param painter the painter to draw the item with
	 * @param region the part of the the image to render
	 */
	virtual void drawContents( QPainter *painter, const QRect &region ) = 0;

	/**
	 * Called when item changes its 'state' variable. This must
	 * handle item's repaint.
	 */
	virtual void statusChanged( bool descend );

	/**
	 * emits needUpdate( int, int, int, int ) with the full widget area.
	 */
	void needUpdate();

	// This enum identifies in which state the item is
	enum ItemState { Snormal, Sactive, Sprelight } state;

	static KdmItem *currentActive;

	// This struct can be filled in by derived items
	struct {
		bool incrementalPaint;
	} properties;

	// This is the placement of the item
	QRect area;

	QString buddy;
	bool isButton;

	// This struct is filled in by KdmItem base class
	enum DataType { DTnone, DTpixel, DTnpixel, DTpercent, DTbox };
	struct DataPoint {
		int val;
		DataType type;
	};
	struct DataPair {
		DataPoint x, y;
	};
	struct {
		DataPair pos, minSize, size, maxSize;
		QString anchor;
	} geom;

	static void calcSize(
		const DataPair &,
		const QRect &, const QSize &, const QSize &,
		int &, int & );

	/* For internal use ONLY
	 * Add a child item. This function is called automatically
	 * when constructing an @p item with this as the parent.
	 */
	void addChildItem( KdmItem *item );

	bool childrenContain( int x, int y );

	/* For internal use ONLY
	 * Parse type and value of an attribute (pos tag), a font or a
	 * color.
	 */
	void parseAttribute( const QString &, DataPoint & );
	void parseFont( const QString &, QFont & );
	void parseColor( const QString &col, const QString &a, QColor & );

	QString itemType, id;
	QList<KdmItem *> m_children;

	// Layouting related variables
	enum { MNone = 0, MFixed = 1, MBox = 2 } currentManager;
	KdmLayoutBox *boxManager;
	KdmLayoutFixed *fixedManager;

	// Compositing related variables
	QImage *image;

	QWidget *myWidget;

	enum { InitialHidden, ExplicitlyHidden, Shown } isShown;
};

#endif
