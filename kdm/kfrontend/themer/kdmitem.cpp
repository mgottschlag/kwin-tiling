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

/*
 * Generic Kdm Item
 */

//#define DRAW_OUTLINE 1	// for debugging only

#include "kdmitem.h"
#include "kdmlayout.h"

#include <kglobal.h>
#include <kdebug.h>

#include <QWidget>
#include <QLayout>
#include <QImage>
#ifdef DRAW_OUTLINE
# include <qpainter.h>
//Added by qt3to4:
#include <QList>
#endif

KdmItem::KdmItem( QObject *parent, const QDomNode &node )
	: QObject( parent )
	, boxManager( 0 )
	, fixedManager( 0 )
	, image( 0 )
	, myWidget( 0 )
	, buttonParent( 0 )
{
	// Set default layout for every item
	currentManager = MNone;
	pos.x = pos.y = 0;
	pos.width = pos.height = 1;
	pos.xType = pos.yType = pos.wType = pos.hType = DTnone;
	pos.anchor = "nw";

	isShown = InitialHidden;

	// Set defaults for derived item's properties
	properties.incrementalPaint = false;
	state = Snormal;

	// The "toplevel" node (the screen) is really just like a fixed node
	KdmItem *parentItem = qobject_cast<KdmItem *>( parent );
	if (!parentItem) {
		setFixedLayout();
		return;
	}

	// Read the mandatory Pos tag. Other tags such as normal, prelighted,
	// etc.. are read under specific implementations.
	QDomNodeList childList = node.childNodes();
	for (int nod = 0; nod < childList.count(); nod++) {
		QDomNode child = childList.item( nod );
		QDomElement el = child.toElement();
		QString tagName = el.tagName(), attr;

		if (tagName == "pos") {
			parseAttribute( el.attribute( "x", QString() ), pos.x, pos.xType );
			parseAttribute( el.attribute( "y", QString() ), pos.y, pos.yType );
			parseAttribute( el.attribute( "width", QString() ), pos.width, pos.wType );
			parseAttribute( el.attribute( "height", QString() ), pos.height, pos.hType );
			pos.anchor = el.attribute( "anchor", "nw" );
		}
	}

	QDomNode tnode = node;
	id = tnode.toElement().attribute( "id", QString::number( (ulong)this, 16 ) );

	// Tell 'parent' to add 'me' to its children
	parentItem->addChildItem( this );
}

KdmItem::~KdmItem()
{
	delete boxManager;
	delete fixedManager;
	delete image;
}

void
KdmItem::update()
{
	foreach (KdmItem *itm, m_children)
		itm->update();
}

void
KdmItem::needUpdate()
{
	emit needUpdate( area.x(), area.y(), area.width(), area.height() );
}

void
KdmItem::show( bool force )
{
	if (isShown != InitialHidden && !force)
		return;

	foreach (KdmItem *itm, m_children)
		itm->show();

	isShown = Shown;

	emit needPlacement();
}

void
KdmItem::hide( bool force )
{
	if (isShown == ExplicitlyHidden)
		return;

	if (isShown == InitialHidden && force) {
		isShown = ExplicitlyHidden;
		return;		// no need for further action
	}

	foreach (KdmItem *itm, m_children)
		itm->hide();

	isShown = force ? ExplicitlyHidden : InitialHidden;

	if (myWidget)
		myWidget->hide();

	emit needPlacement();
}

void
KdmItem::inheritFromButton( KdmItem *button )
{
	if (button)
		buttonParent = button;

	foreach (KdmItem *itm, m_children)
		itm->inheritFromButton( button );
}

KdmItem *
KdmItem::findNode( const QString &_id ) const
{
	if (id == _id)
		return const_cast<KdmItem *>( this );

	foreach (KdmItem *itm, m_children)
		if (KdmItem *t = itm->findNode( _id ))
			return t;

	return 0;
}

void
KdmItem::setWidget( QWidget *widget )
{
//	delete myWidget;	-- we *never* own the widget

	if ((myWidget = widget)) {
		myWidget->hide(); // yes, really
		connect( myWidget, SIGNAL(destroyed()), SLOT(widgetGone()) );
	}

	emit needPlacement();
}

void
KdmItem::widgetGone()
{
	myWidget = 0;
}

void
KdmItem::showWidget()
{
	if (isShown != Shown)
		return;
	if (myWidget) {
		myWidget->setGeometry( area );
		myWidget->show();
	}
	foreach (KdmItem *itm, m_children)
		itm->showWidget();
}

/* This is called as a result of KdmLayout::update, and directly on the root */
void
KdmItem::setGeometry( const QRect &newGeometry, bool force )
{
	kDebug() << " KdmItem::setGeometry " << id << newGeometry << endl;
	// check if already 'in place'
	if (!force && area == newGeometry)
		return;

	area = newGeometry;

	// recurr to all boxed children
	if (boxManager && !boxManager->isEmpty())
		boxManager->update( newGeometry, force );

	// recurr to all fixed children
	if (fixedManager && !fixedManager->isEmpty())
		fixedManager->update( newGeometry, force );

	// TODO send *selective* repaint signal
}

void
KdmItem::paint( QPainter *p, const QRect &rect )
{
	if (isHidden())
		return;

	if (myWidget)
		return;

	if (area.intersects( rect )) {
		QRect contentsRect = area.intersect( rect );
		contentsRect.translate( qMin( 0, -area.x() ), qMin( 0, -area.y() ) );
		drawContents( p, contentsRect );
	}

#ifdef DRAW_OUTLINE
	// Draw bounding rect for this item
	p->setPen( Qt::white );
	p->drawRect( area );
#endif

	// Dispatch paint events to children
	foreach (KdmItem *itm, m_children)
		itm->paint( p, rect );
}

KdmItem *KdmItem::currentActive = 0;

void
KdmItem::mouseEvent( int x, int y, bool pressed, bool released )
{
	if (buttonParent && buttonParent != this) {
		buttonParent->mouseEvent( x, y, pressed, released );
		return;
	}

	ItemState oldState = state;
	if (area.contains( x, y )) {
		if (released && oldState == Sactive) {
			if (buttonParent)
				emit activated( id );
			state = Sprelight;
			currentActive = 0;
		} else if (pressed || currentActive == this) {
			state = Sactive;
			currentActive = this;
		} else if (!currentActive)
			state = Sprelight;
		else
			state = Snormal;
	} else {
		if (released)
			currentActive = 0;
		if (currentActive == this)
			state = Sprelight;
		else
			state = Snormal;
	}

	if (!buttonParent)
		foreach (KdmItem *itm, m_children)
			itm->mouseEvent( x, y, pressed, released );

	if (oldState != state)
		statusChanged();
}

void
KdmItem::statusChanged()
{
	if (buttonParent == this)
		foreach (KdmItem *o, m_children) {
			o->state = state;
			o->statusChanged();
		}
}

// BEGIN protected inheritable

QSize
KdmItem::sizeHint()
{
	if (myWidget)
		return myWidget->sizeHint();
	int w = pos.wType == DTpixel ? qAbs( pos.width ) : -1,
	    h = pos.hType == DTpixel ? qAbs( pos.height ) : -1;
	return QSize( w, h );
}

QRect
KdmItem::placementHint( const QRect &parentRect )
{
	QSize hintedSize = sizeHint();
	QSize boxHint;

	int x = parentRect.left(),
	    y = parentRect.top(),
	    w = parentRect.width(),
	    h = parentRect.height();

	kDebug() << "KdmItem::placementHint parentRect=" << id << parentRect << " hintedSize=" << hintedSize << endl;
	// check if width or height are set to "box"
	if (pos.wType == DTbox || pos.hType == DTbox) {
		if (myWidget)
			boxHint = hintedSize;
		else {
			if (!boxManager)
				return parentRect;
			boxHint = boxManager->sizeHint();
		}
		kDebug() << " => boxHint " << boxHint << endl;
	}

	if (pos.xType == DTpixel)
		x += pos.x;
	else if (pos.xType == DTnpixel)
		x = parentRect.right() - pos.x;
	else if (pos.xType == DTpercent)
		x += int( parentRect.width() / 100.0 * pos.x );

	if (pos.yType == DTpixel)
		y += pos.y;
	else if (pos.yType == DTnpixel)
		y = parentRect.bottom() - pos.y;
	else if (pos.yType == DTpercent)
		y += int( parentRect.height() / 100.0 * pos.y );

	if (pos.wType == DTpixel)
		w = pos.width;
	else if (pos.wType == DTnpixel)
		w -= pos.width;
	else if (pos.wType == DTpercent)
		w = int( parentRect.width() / 100.0 * pos.width );
	else if (pos.wType == DTbox)
		w = boxHint.width();
	else if (hintedSize.width() > 0)
		w = hintedSize.width();
	else
		w = 0;

	if (pos.hType == DTpixel)
		h = pos.height;
	else if (pos.hType == DTnpixel)
		h -= pos.height;
	else if (pos.hType == DTpercent)
		h = int( parentRect.height() / 100.0 * pos.height );
	else if (pos.hType == DTbox)
		h = boxHint.height();
	else if (hintedSize.height() > 0)
		h = hintedSize.height();
	else
		h = 0;

	// defaults to center
	int dx = -w / 2, dy = -h / 2;

	// anchor the rect to an edge / corner
	if (pos.anchor.length() > 0 && pos.anchor.length() < 3) {
		if (pos.anchor.indexOf( 'n' ) >= 0)
			dy = 0;
		if (pos.anchor.indexOf( 's' ) >= 0)
			dy = -h;
		if (pos.anchor.indexOf( 'w' ) >= 0)
			dx = 0;
		if (pos.anchor.indexOf( 'e' ) >= 0)
			dx = -w;
	}
	// KdmItem *p = static_cast<KdmItem*>( parent() );
	kDebug() << "KdmItem::placementHint " << id << " x=" << x << " dx=" << dx << " w=" << w << " y=" << y << " dy=" << dy << " h=" << h << " " << parentRect << endl;
	y += dy;
	x += dx;

	// Note: no clipping to parent because this broke many themes!
	return QRect( x, y, w, h );
}

// END protected inheritable


void
KdmItem::addChildItem( KdmItem *item )
{
	m_children.append( item );
	switch (currentManager) {
	case MNone:		// fallback to the 'fixed' case
		setFixedLayout();
	case MFixed:
		fixedManager->addItem( item );
		break;
	case MBox:
		boxManager->addItem( item );
		break;
	}

	// signal bounce from child to parent
	connect( item, SIGNAL(needUpdate( int, int, int, int )), SIGNAL(needUpdate( int, int, int, int )) );
	connect( item, SIGNAL(needPlacement()), SIGNAL(needPlacement()) );
	connect( item, SIGNAL(activated( const QString & )), SIGNAL(activated( const QString & )) );
}

void
KdmItem::parseAttribute( const QString &s, int &val, enum DataType &dType )
{
	if (s.isEmpty())
		return;

	int p;
	if (s == "box") {	// box value
		dType = DTbox;
		val = 0;
	} else if ((p = s.indexOf( '%' )) >= 0) {	// percent value
		dType = DTpercent;
		QString sCopy = s;
		sCopy.remove( p, 1 );
		sCopy.replace( ',', '.' );
		val = (int)sCopy.toDouble();
	} else {		// int value
		dType = DTpixel;
		QString sCopy = s;
		if (sCopy.at( 0 ) == '-') {
			sCopy.remove( 0, 1 );
			dType = DTnpixel;
		}
		sCopy.replace( ',', '.' );
		val = (int)sCopy.toDouble();
	}
}

void
KdmItem::parseFont( const QString &s, QFont &font )
{
	int splitAt = s.lastIndexOf( ' ' );
	if (splitAt < 1)
		return;
	font.setFamily( s.left( splitAt ) );
	int fontSize = s.mid( splitAt + 1 ).toInt();
	if (fontSize > 1)
		font.setPointSize( fontSize );
}

void
KdmItem::parseColor( const QString &s, const QString &a, QColor &color )
{
	if (s.at( 0 ) != '#')
		return;
	bool ok;
	QString sCopy = s;
	int hexColor = sCopy.remove( 0, 1 ).toInt( &ok, 16 );
	if (ok) {
		color.setRgb( hexColor );
		if (!a.isNull())
			color.setAlpha( int(a.toFloat() * 255) );
	}
}

void
KdmItem::setBoxLayout( const QDomNode &node )
{
	if (!boxManager)
		boxManager = new KdmLayoutBox( node );
	currentManager = MBox;
}

void
KdmItem::setFixedLayout( const QDomNode &node )
{
	if (!fixedManager)
		fixedManager = new KdmLayoutFixed( node );
	currentManager = MFixed;
}

#include "kdmitem.moc"
