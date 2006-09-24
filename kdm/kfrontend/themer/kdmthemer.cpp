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

#include "kdmthemer.h"
#include "kdmitem.h"
#include "kdmpixmap.h"
#include "kdmrect.h"
#include "kdmlabel.h"

#include <kdm_greet.h>
#include <kdmconfig.h>
#include <kfdialog.h>

#include <kiconloader.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <QFile>
#include <QFileInfo>
//#include <QTimer>		// animation timer - TODO
#include <QObject>
#include <QPainter>
#include <QWidget>
#include <qregion.h>
//Added by qt3to4:
#include <QPixmap>
#include <QPaintEvent>
#include <QEvent>
#include <QMouseEvent>

#include <unistd.h>

/*
 * KdmThemer. The main theming interface
 */
KdmThemer::KdmThemer( const QString &_filename, const QString &mode, QWidget *parent )
	: QObject( parent )
	, rootItem( 0 )
	, m_geometryOutdated( true )
	, m_geometryInvalid( true )
{
	// Set the mode we're working in
	m_currentMode = mode;

	// read the XML file and create DOM tree
	QString filename = _filename;
	if (!::access( QFile::encodeName( filename + "/GdmGreeterTheme.desktop" ), R_OK )) {
		KSimpleConfig cfg( filename + "/GdmGreeterTheme.desktop" );
		cfg.setGroup( "GdmGreeterTheme" );
		filename += '/' + cfg.readEntry( "Greeter" );
	}
	QFile opmlFile( filename );
	if (!opmlFile.open( QIODevice::ReadOnly )) {
		FDialog::box( widget(), errorbox, i18n( "Cannot open theme file %1" , filename) );
		return;
	}
	if (!domTree.setContent( &opmlFile )) {
		FDialog::box( widget(), errorbox, i18n( "Cannot parse theme file %1" , filename) );
		return;
	}
	// generate all the items defined in the theme
	const QDomElement &theme = domTree.documentElement();
	// Get its tag, and check it's correct ("greeter")
	if (theme.tagName() != "greeter") {
		FDialog::box( widget(), errorbox, i18n( "%1 does not seem to be a correct theme file" , filename) );
		return;
	}

	// Set the root (screen) item
	rootItem = new KdmRect( this, QDomNode() );

	basedir = QFileInfo( filename ).absolutePath();

	generateItems( rootItem, theme );

/*	*TODO*
	// Animation timer
	QTimer *time = new QTimer( this );
	time->start( 500 );
	connect( time, SIGNAL(timeout()), SLOT(update()) )
*/
}

KdmThemer::~KdmThemer()
{
	delete rootItem;
}

inline QWidget *
KdmThemer::widget()
{
	return static_cast<QWidget *>(parent());
}

KdmItem *
KdmThemer::findNode( const QString &item ) const
{
	return rootItem->findNode( item );
}

void
KdmThemer::slotNeedPlacement()
{
	m_geometryOutdated = m_geometryInvalid = true;
	widget()->update();
}

void
KdmThemer::update( int x, int y, int w, int h )
{
	widget()->update( x, y, w, h );
}

// BEGIN other functions

void
KdmThemer::widgetEvent( QEvent *e )
{
	if (!rootItem)
		return;
	switch (e->type()) {
	case QEvent::MouseMove:
		{
			QMouseEvent *me = static_cast<QMouseEvent *>(e);
			rootItem->mouseEvent( me->x(), me->y() );
		}
		break;
	case QEvent::MouseButtonPress:
		{
			QMouseEvent *me = static_cast<QMouseEvent *>(e);
			rootItem->mouseEvent( me->x(), me->y(), true );
		}
		break;
	case QEvent::MouseButtonRelease:
		{
			QMouseEvent *me = static_cast<QMouseEvent *>(e);
			rootItem->mouseEvent( me->x(), me->y(), false, true );
		}
		break;
	case QEvent::Show:
		rootItem->show();
		break;
	case QEvent::Resize:
		m_geometryOutdated = true;
		widget()->update();
		break;
	case QEvent::Paint:
		if (m_geometryOutdated) {
			rootItem->setGeometry( QRect( QPoint(), widget()->size() ), m_geometryInvalid );
			if (debugLevel & DEBUG_THEMING)
				showStructure( rootItem );
			m_geometryOutdated = m_geometryInvalid = false;
		}
		{
			QRect paintRect = static_cast<QPaintEvent *>(e)->rect();
			//kDebug() << "paint on: " << paintRect << endl;

			QPainter p( widget() );
			rootItem->paint( &p, paintRect );
			rootItem->showWidget();
		}
		break;
	default:
		break;
	}
}

/*
void
KdmThemer::pixmap( const QRect &r, QPixmap *px )
{
	bitBlt( px, QPoint( 0, 0 ), widget(), r );
}
*/

void
KdmThemer::generateItems( KdmItem *parent, const QDomNode &node )
{
	/*
	 * Go through each of the child nodes
	 */
	const QDomNodeList &subnodeList = node.childNodes();
	for (int nod = 0; nod < subnodeList.count(); nod++) {
		QDomNode subnode = subnodeList.item( nod );
		QDomElement el = subnode.toElement();
		QString tagName = el.tagName();

		if (tagName == "item") {
			if (!willDisplay( subnode ))
				continue;
			// It's a new item. Draw it
			QString type = el.attribute( "type" );

			KdmItem *newItem = 0;

			if (type == "label")
				newItem = new KdmLabel( parent, subnode );
			else if (type == "pixmap")
				newItem = new KdmPixmap( parent, subnode );
			else if (type == "rect")
				newItem = new KdmRect( parent, subnode );
			else if (type == "entry") {
				newItem = new KdmRect( parent, subnode );
				newItem->setType( type );
			}
			//	newItem = new KdmEntry( parent, subnode );
			//else if (type=="list")
			//	newItem = new KdmList( parent, subnode );
			else if (type == "svg")
				newItem = new KdmPixmap( parent, subnode );
			if (newItem) {
				newItem->setIsButton( el.attribute( "button", "false" ) == "true" );
				connect( newItem, SIGNAL(needUpdate( int, int, int, int )),
						 SLOT(update( int, int, int, int )) );
				connect( newItem, SIGNAL(needPlacement()),
						 SLOT(slotNeedPlacement()) );
				connect( newItem, SIGNAL(activated( const QString & )),
						 SIGNAL(activated( const QString & )) );
				generateItems( newItem, subnode );
			}
		} else if (tagName == "box") {
			if (!willDisplay( subnode ))
				continue;
			// It's a new box. Draw it
			parent->setBoxLayout( subnode );
			generateItems( parent, subnode );
		} else if (tagName == "fixed") {
			if (!willDisplay( subnode ))
				continue;
			// It's a new box. Draw it
			parent->setFixedLayout( subnode );
			generateItems( parent, subnode );
		}
	}
}

bool KdmThemer::willDisplay( const QDomNode &node )
{
	QDomNode showNode = node.namedItem( "show" );

	// No "show" node means this item can be displayed at all times
	if (showNode.isNull())
		return true;

	QDomElement el = showNode.toElement();

	QString modes = el.attribute( "modes" );
	if (!modes.isNull()) {
		QStringList modeList = modes.split( ",", QString::SkipEmptyParts );

		// If current mode isn't in this list, do not display item
		if (!modeList.contains( m_currentMode ))
			return false;
	}

	QString type = el.attribute( "type" );
	if (type == "config" || type == "suspend")
		return false;	// not implemented (yet)
	if (type == "timed")
		return _autoLoginDelay != 0;
	if (type == "chooser")
#ifdef XDMCP
		return _loginMode != LOGIN_LOCAL_ONLY;
#else
		return false;
#endif
	if (type == "halt" || type == "reboot")
		return _allowShutdown != SHUT_NONE;
//	if (type == "system")
//		return true;

	// All tests passed, item will be displayed
	return true;
}

void
KdmThemer::showStructure( QObject *obj )
{

	QObjectList wlist = obj->children();
	static int counter = 0;
	if (counter == 0)
		kDebug() << "\n\n<=======  Widget tree =================" << endl;
	if (!wlist.isEmpty()) {
		counter++;
		foreach (QObject *object, wlist) {
			QString node;
			for (int i = 1; i < counter; i++)
				node += '-';

			if (object->inherits( "KdmItem" )) {
				KdmItem *widget = (KdmItem *)object;
				kDebug() << node << "|" << widget->type() << " me=" << widget->id << " " << widget->area << endl;
			}

			showStructure( object );
		}
		counter--;
	}
	if (counter == 0)
		kDebug() << "\n\n<=======  Widget tree =================\n\n" << endl;
}

#include "kdmthemer.moc"
