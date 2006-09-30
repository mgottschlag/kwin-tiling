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

#include "kdmitem.h"
#include "kdmlayout.h"
#include "kdmthemer.h"

#include <kdm_greet.h>

#include <kglobal.h>
#include <kdebug.h>

#include <QWidget>
#include <QLayout>
#include <QImage>
#include <QLineEdit>
#include <QPainter>
//Added by qt3to4:
#include <QList>

KdmItem::KdmItem( QObject *parent, const QDomNode &node )
	: QObject( parent )
	, isButton( false )
	, boxManager( 0 )
	, fixedManager( 0 )
	, image( 0 )
	, myWidget( 0 )
{
	// Set default layout for every item
	currentManager = MNone;
	geom.pos.x.type = geom.pos.y.type =
		geom.size.x.type = geom.size.y.type = DTnone;
	geom.minSize.x.type = geom.minSize.y.type =
		geom.maxSize.x.type = geom.maxSize.y.type = DTpixel;
	geom.minSize.x.val = geom.minSize.y.val = 0;
	geom.maxSize.x.val = geom.maxSize.y.val = 1000000;
	geom.anchor = "nw";

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
			parseAttribute( el.attribute( "x", QString() ), geom.pos.x );
			parseAttribute( el.attribute( "y", QString() ), geom.pos.y );
			parseAttribute( el.attribute( "width", QString() ), geom.size.x );
			parseAttribute( el.attribute( "height", QString() ), geom.size.y );
			parseAttribute( el.attribute( "min-width", QString() ), geom.minSize.x );
			parseAttribute( el.attribute( "min-height", QString() ), geom.minSize.y );
			parseAttribute( el.attribute( "max-width", QString() ), geom.maxSize.x );
			parseAttribute( el.attribute( "max-height", QString() ), geom.maxSize.y );
			geom.anchor = el.attribute( "anchor", "nw" );
			QString exp = el.attribute( "expand", "false" ).toLower();
			bool ok;
			geom.expand = exp.toInt( &ok );
			if (!ok)
				geom.expand = exp == "true";
		}
		if (tagName == "buddy")
			buddy = el.attribute( "idref", "" );
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

KdmThemer *
KdmItem::themer()
{
	if (KdmThemer *thm = qobject_cast<KdmThemer *>(parent()))
		return thm;
	if (KdmItem *parentItem = qobject_cast<KdmItem *>( parent() ))
		return parentItem->themer();
	return 0;
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

	emit needPlacement();
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
KdmItem::setGeometry( QStack<QSize> &parentSizes, const QRect &newGeometry, bool force )
{
	kDebug() << " KdmItem::setGeometry " << id << " " << newGeometry << endl;
	// check if already 'in place'
	if (!force && area == newGeometry)
		return;

	area = newGeometry;

	// recurr to all boxed children
	if (boxManager && !boxManager->isEmpty())
		boxManager->update( parentSizes, newGeometry, force );

	// recurr to all fixed children
	if (fixedManager && !fixedManager->isEmpty())
		fixedManager->update( parentSizes, newGeometry, force );

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
		if (debugLevel & DEBUG_THEMING) {
			// Draw bounding rect for this item
			QPen pen( Qt::white );
			pen.setCapStyle( Qt::FlatCap );
			pen.setDashPattern( QVector<qreal>() << 5 << 6 );
			p->setPen( pen );
			p->setBackgroundMode( Qt::OpaqueMode );
			p->setBackground( Qt::black );
			p->drawRect( area.x() + 1, area.y() + 1, area.width() - 1, area.height() - 1 );
			p->setBackgroundMode( Qt::TransparentMode );
		}
	}

	// Dispatch paint events to children
	foreach (KdmItem *itm, m_children)
		itm->paint( p, rect );
}

bool
KdmItem::childrenContain( int x, int y )
{
	foreach (KdmItem *itm, m_children)
		if (!itm->isHidden()) {
			if (itm->area.contains( x, y ))
				return true;
			if (itm->childrenContain( x, y ))
				return true;
		}
	return false;
}

void
KdmItem::activateBuddy()
{
	if (KdmItem *itm = themer()->findNode( buddy ))
		if (itm->myWidget) {
			itm->myWidget->setFocus();
			if (QLineEdit *le = qobject_cast<QLineEdit *>(itm->myWidget))
				le->selectAll();
		}
}

KdmItem *KdmItem::currentActive = 0;

void
KdmItem::mouseEvent( int x, int y, bool pressed, bool released )
{
	if (isHidden())
		return;

	ItemState oldState = state;
	if (area.contains( x, y ) || (isButton && childrenContain( x, y ))) {
		if (released && oldState == Sactive) {
			if (isButton)
				emit activated( id );
			state = Sprelight;
			currentActive = 0;
		} else if (pressed && !buddy.isEmpty())
			activateBuddy();
		else if (pressed || currentActive == this) {
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
	if (oldState != state)
		statusChanged( isButton );

	if (!isButton)
		foreach (KdmItem *itm, m_children)
			itm->mouseEvent( x, y, pressed, released );
}

void
KdmItem::statusChanged( bool descend )
{
	if (descend)
		foreach (KdmItem *o, m_children) {
			o->state = state;
			o->statusChanged( descend );
		}
}

// BEGIN protected inheritable

QSize
KdmItem::sizeHint()
{
	if (myWidget)
		return myWidget->sizeHint();
	return QSize(
		geom.size.x.type == DTpixel ? geom.size.x.val : 0,
		geom.size.y.type == DTpixel ? geom.size.y.val : 0 );
}

const QSize &
KdmItem::ensureHintedSize( QSize &hintedSize )
{
	if (!hintedSize.isValid()) {
		hintedSize = sizeHint();
		kDebug() << " => hintedSize " << hintedSize << endl;
	}
	return hintedSize;	
}

const QSize &
KdmItem::ensureBoxHint( QSize &boxHint, QStack<QSize> &parentSizes, QSize &hintedSize )
{
	if (!boxHint.isValid()) {
		if (myWidget || !boxManager)
			boxHint = ensureHintedSize( hintedSize );
		else
			boxHint = boxManager->sizeHint( parentSizes );
		kDebug() << " => boxHint " << boxHint << endl;
	}
	return boxHint;	
}

static const QSize &
getParentSize( const QStack<QSize> &parentSizes, int levels )
{
	int off = parentSizes.size() - 1 - levels;
	if (off < 0) {
		kError() << "Theme references element below the root." << endl;
		off = 0;
	}
	return parentSizes[off];
}

void
KdmItem::calcSize(
	const DataPair &sz,
	QStack<QSize> &parentSizes, QSize &hintedSize, QSize &boxHint,
	QSize &io )
{
	int w, h;

	if (sz.x.type == DTpixel)
		w = sz.x.val;
	else if (sz.x.type == DTnpixel)
		w = io.width() - sz.x.val;
	else if (sz.x.type == DTpercent)
		w = getParentSize( parentSizes, sz.x.levels ).width() * sz.x.val / 100;
	else if (sz.x.type == DTbox)
		w = ensureBoxHint( boxHint, parentSizes, hintedSize ).width();
	else
		w = ensureHintedSize( hintedSize ).width();

	if (sz.y.type == DTpixel)
		h = sz.y.val;
	else if (sz.y.type == DTnpixel)
		h = io.height() - sz.y.val;
	else if (sz.y.type == DTpercent)
		h = getParentSize( parentSizes, sz.y.levels ).height() * sz.y.val / 100;
	else if (sz.y.type == DTbox)
		h = ensureBoxHint( boxHint, parentSizes, hintedSize ).height();
	else
		h = ensureHintedSize( hintedSize ).height();

	if (sz.x.type == DTscale && h && ensureHintedSize( hintedSize ).height())
		w = w * h / hintedSize.height();
	else if (sz.y.type == DTscale && w && ensureHintedSize( hintedSize ).width())
		h = w * h / hintedSize.width();

	io.setWidth( w );
	io.setHeight( h );
}

void
KdmItem::sizingHint( QStack<QSize> &parentSizes, SizeHint &hint )
{
	kDebug() << "KdmItem::sizingHint " << id
		<< " parentSize=" << parentSizes.top() << endl;

	QSize hintedSize, boxHint;
	hint.min = hint.opt = hint.max = parentSizes.top();
	calcSize( geom.size, parentSizes, hintedSize, boxHint, hint.opt );
	calcSize( geom.minSize, parentSizes, hintedSize, boxHint, hint.min );
	calcSize( geom.maxSize, parentSizes, hintedSize, boxHint, hint.max );
	kDebug() << "size " << hint.opt << " min " << hint.min << " max " << hint.max << endl;

	hint.max = hint.max.expandedTo( hint.min ); // if this triggers, the theme is bust
	hint.opt = hint.opt.boundedTo( hint.max ).expandedTo( hint.min );

	// Note: no clipping to parent because this broke many themes!
}

QRect
KdmItem::placementHint( QStack<QSize> &sizes, const QSize &sz, const QPoint &offset )
{
	const QSize &parentSize = sizes.top();
	int x = offset.x(),
	    y = offset.y(),
	    w = parentSize.width(),
	    h = parentSize.height();

	if (geom.pos.x.type == DTpixel)
		x += geom.pos.x.val;
	else if (geom.pos.x.type == DTnpixel)
		x += w - geom.pos.x.val;
	else if (geom.pos.x.type == DTpercent)
		x += w * geom.pos.x.val / 100;

	if (geom.pos.y.type == DTpixel)
		y += geom.pos.y.val;
	else if (geom.pos.y.type == DTnpixel)
		y += h - geom.pos.y.val;
	else if (geom.pos.y.type == DTpercent)
		y += h * geom.pos.y.val / 100;

	kDebug() << "adjusted size " << sz << endl;
	
	// defaults to center
	int dx = sz.width() / 2, dy = sz.height() / 2;

	// anchor the rect to an edge / corner
	if (geom.anchor.length() > 0 && geom.anchor.length() < 3) {
		if (geom.anchor.indexOf( 'n' ) >= 0)
			dy = 0;
		if (geom.anchor.indexOf( 's' ) >= 0)
			dy = sz.height();
		if (geom.anchor.indexOf( 'w' ) >= 0)
			dx = 0;
		if (geom.anchor.indexOf( 'e' ) >= 0)
			dx = sz.width();
	}
	kDebug() << "KdmItem::placementHint " << id << " size=" << sz
		<< " x=" << x << " dx=" << dx<< " y=" << y << " dy=" << dy << endl;
	y -= dy;
	x -= dx;

	return QRect( x, y, sz.width(), sz.height() );
}

QRect
KdmItem::placementHint( QStack<QSize> &sizes, const QPoint &offset )
{
	SizeHint sh;
	sizingHint( sizes, sh );
	return placementHint( sizes, sh.opt, offset );
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
}

void
KdmItem::parseAttribute( const QString &s, DataPoint &pt )
{
	if (s.isEmpty())
		return;

	int p;
	if (s == "box") {	// box value
		pt.type = DTbox;
		pt.val = 0;
	} else if (s == "scale") {
		pt.type = DTscale;
		pt.val = 0;
	} else if ((p = s.indexOf( '%' )) >= 0) {	// percent value
		pt.type = DTpercent;
		QString sCopy = s;
		sCopy.remove( p, 1 );
		pt.levels = 0;
		while ((p = sCopy.indexOf( '^' )) >= 0) {
			sCopy.remove( p, 1 );
			pt.levels++;
		}
		sCopy.replace( ',', '.' );
		pt.val = (int)sCopy.toDouble();
	} else {		// int value
		pt.type = DTpixel;
		QString sCopy = s;
		if (sCopy.at( 0 ) == '-') {
			sCopy.remove( 0, 1 );
			pt.type = DTnpixel;
		}
		sCopy.replace( ',', '.' );
		pt.val = (int)sCopy.toDouble();
	}
}


static QString
getword( QString &rs )
{
	int splitAt = rs.lastIndexOf( ' ' ) + 1;
	QString s( rs.mid( splitAt ) );
	rs.truncate( splitAt - 1 );
	return s;
}

void
KdmItem::parseFont( const QString &is, QFont &font )
{
	QString rs( is.simplified() );
	QString s( getword( rs ) );
	bool ok;
	if (s.endsWith( "px" )) {
		int ps = s.left( s.length() - 2 ).toInt( &ok );
		if (ok) {
			font.setPixelSize( ps );
			s = getword( rs );
		}
	} else {
		double ps = s.toDouble( &ok );
		if (ok) {
			font.setPointSizeF( ps );
			s = getword( rs );
		}
	}
	forever {
		QString ss( s.toLower() );
		if (ss == "oblique")
			font.setStyle( QFont::StyleOblique );
		else if (ss == "italic")
			font.setStyle( QFont::StyleItalic );
		else if (ss == "ultra-light")
			font.setWeight( 13 );
		else if (ss == "light")
			font.setWeight( QFont::Light );
		else if (ss == "medium")
			font.setWeight( 50 );
		else if (ss == "semi-bold")
			font.setWeight( QFont::DemiBold );
		else if (ss == "bold")
			font.setWeight( QFont::Bold );
		else if (ss == "ultra-bold")
			font.setWeight( QFont::Black );
		else if (ss == "heavy")
			font.setWeight( 99 );
		else if (ss == "ultra-condensed")
			font.setStretch( QFont::UltraCondensed );
		else if (ss == "extra-condensed")
			font.setStretch( QFont::ExtraCondensed );
		else if (ss == "condensed")
			font.setStretch( QFont::Condensed );
		else if (ss == "semi-condensed")
			font.setStretch( QFont::SemiCondensed );
		else if (ss == "semi-expanded")
			font.setStretch( QFont::SemiExpanded );
		else if (ss == "expanded")
			font.setStretch( QFont::Expanded );
		else if (ss == "extra-expanded")
			font.setStretch( QFont::ExtraExpanded );
		else if (ss == "ultra-expanded")
			font.setStretch( QFont::UltraExpanded );
		else if (ss == "normal" || // no-op
		         ss == "small-caps" || // this and following ignored
		         ss == "not-rotated" || ss == "south" || ss == "upside-down" ||
		         ss == "north" ||
		         ss == "rotated-left" || ss == "east" ||
		         ss == "rotated-right" || ss == "west")
		{
		} else
			break;
		s = getword( rs );
	}
	if (!rs.isEmpty())
		rs.append( ' ' ).append( s );
	else
		rs = s;
	QStringList ffs = rs.split( QRegExp( " ?, ?" ), QString::SkipEmptyParts );
	if (!ffs.isEmpty()) {
		foreach (QString ff, ffs) {
			font.setFamily( ff );
			if (font.exactMatch())
				return;
		}
		font.setFamily( ffs.first() );
	}
}

void
KdmItem::parseColor( const QString &s, const QString &a, QColor &color )
{
	if (!s.length() || s.at( 0 ) != '#')
		return;
	bool ok;
	QString sCopy = s;
	uint hexColor = sCopy.remove( 0, 1 ).toUInt( &ok, 16 );
	if (ok) {
		if (sCopy.length() == 8)
			color.setRgba( hexColor );
		else {
			color.setRgb( hexColor );
			if (!a.isNull())
				color.setAlpha( int(a.toFloat() * 255) );
		}
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
