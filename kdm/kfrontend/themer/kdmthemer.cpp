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

#include <kdmconfig.h>
#include <kfdialog.h>

#include <kiconloader.h>
#include <kimageeffect.h>
#include <klocale.h>
#include <ksimpleconfig.h>
#include <kdebug.h>

#include <qfile.h>
#include <qfileinfo.h>
//#include <qtimer.h>		// animation timer - TODO
#include <qobject.h>
#include <qpainter.h>
#include <qwidget.h>
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
    , backBuffer( 0 )
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
		FDialog::box( widget(), errorbox, i18n( "Cannot open theme file %1" ).arg(filename) );
		return;
	}
	if (!domTree.setContent( &opmlFile )) {
		FDialog::box( widget(), errorbox, i18n( "Cannot parse theme file %1" ).arg(filename) );
		return;
	}
	// Set the root (screen) item
	rootItem = new KdmRect( 0, QDomNode(), "kdm root" );
	connect( rootItem, SIGNAL(needUpdate( int, int, int, int )),
	         widget(), SLOT(update( int, int, int, int )) );

	rootItem->setBaseDir( QFileInfo( filename ).absolutePath() );

	// generate all the items defined in the theme
	generateItems( rootItem );

	connect( rootItem, SIGNAL(activated( const QString & )), SIGNAL(activated( const QString & )) );

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
	delete backBuffer;
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
KdmThemer::updateGeometry( bool force )
{
	rootItem->setGeometry( QRect( QPoint(), widget()->size() ), force );
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
		updateGeometry( false );
		showStructure( rootItem );
		break;
	case QEvent::Paint:
		{
			QRect paintRect = static_cast<QPaintEvent *>(e)->rect();
			kdDebug() << "paint on: " << paintRect << endl;

			if (!backBuffer)
				backBuffer = new QPixmap( widget()->size() );
			if (backBuffer->size() != widget()->size())
				backBuffer->resize( widget()->size() );

			QPainter p;
			p.begin( backBuffer );
			rootItem->paint( &p, paintRect );
			p.end();

			bitBlt( widget(), paintRect.topLeft(), backBuffer, paintRect );
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
	if (!parent)
		return;

	QDomNodeList subnodeList;	//List of subnodes of this node

	/*
	 * Get the child nodes
	 */
	if (node.isNull()) {	// It's the first node, get its child nodes
		QDomElement theme = domTree.documentElement();

		// Get its tag, and check it's correct ("greeter")
		if (theme.tagName() != "greeter") {
			kdDebug() << "This does not seem to be a correct theme file." << endl;
			return;
		}
		// Get the list of child nodes
		subnodeList = theme.childNodes();
	} else
		subnodeList = node.childNodes();

	/*
	 * Go through each of the child nodes
	 */
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
				generateItems( newItem, subnode );
				if (el.attribute( "button", "false" ) == "true")
					newItem->inheritFromButton( newItem );
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
		QStringList modeList = QStringList::split( ",", modes );

		// If current mode isn't in this list, do not display item
		if (modeList.find( m_currentMode ) == modeList.end())
			return false;
	}

	QString type = el.attribute( "type" );
	if (type == "timed" || type == "config" || type == "suspend")
		return false;	// not implemented (yet)
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
		kdDebug() << "\n\n<=======  Widget tree =================" << endl;
	if (!wlist.isEmpty()) {
		counter++;
		QListIterator<QObject*> it( wlist );
		QObject *object;

		while (it.hasNext()) {
			object = it.next();
			QString node;
			for (int i = 1; i < counter; i++)
				node += "-";

			if (object->inherits( "KdmItem" )) {
				KdmItem *widget = (KdmItem *)object;
				kdDebug() << node << "|" << widget->type() << " me=" << widget << " " << widget->area << endl;
			}

			showStructure( object );
		}
		counter--;
	}
	if (counter == 0)
		kdDebug() << "\n\n<=======  Widget tree =================\n\n" << endl;
}

#include "kdmthemer.moc"
