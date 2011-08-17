/* 
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
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

#include "krandrpassivepopup.h"

#include <kapplication.h>
#include <QX11Info>
//Added by qt3to4:
#include <QPixmap>
#include <QEvent>

// this class is just like KPassivePopup, but it keeps track of the widget
// it's supposed to be positioned next to, and adjust its position if that
// widgets moves (needed because after a resolution switch Kicker will
// reposition itself, causing normal KPassivePopup to stay at weird places)

KRandrPassivePopup::KRandrPassivePopup( QWidget *parent, Qt::WFlags f )
    : KPassivePopup( parent, f )
    {
    update_timer.setSingleShot( true );
    connect( &update_timer, SIGNAL(timeout()), SLOT(slotPositionSelf()));
    }
    
KRandrPassivePopup* KRandrPassivePopup::message( const QString &caption, const QString &text,
    const QPixmap &icon, QWidget *parent, int timeout )
    {
    KRandrPassivePopup *pop = new KRandrPassivePopup( parent );
    pop->setAutoDelete( true );
    pop->setView( caption, text, icon );
    pop->setTimeout( timeout );
    pop->show();
    pop->startWatchingWidget( parent );
    return pop;
    }

void KRandrPassivePopup::startWatchingWidget( QWidget* widget_P )
    {
    static Atom wm_state = XInternAtom( QX11Info::display() , "WM_STATE", False );
    Window win = widget_P->winId();
    bool x11_events = false;
    for(;;)
	{
	Window root, parent;
	Window* children;
	unsigned int nchildren;
	XQueryTree( QX11Info::display(), win, &root, &parent, &children, &nchildren );
	if( children != NULL )
	    XFree( children );
	if( win == root ) // huh?
	    break;
	win = parent;
	
	QWidget* widget = QWidget::find( win );
	if( widget != NULL )
	    {
	    widget->installEventFilter( this );
	    watched_widgets.append( widget );
	    }
	else
	    {
	    XWindowAttributes attrs;
	    XGetWindowAttributes( QX11Info::display(), win, &attrs );
	    XSelectInput( QX11Info::display(), win, attrs.your_event_mask | StructureNotifyMask );
	    watched_windows.append( win );
	    x11_events = true;
	    }
	Atom type;
	int format;
	unsigned long nitems, after;
	unsigned char* data;
	if( XGetWindowProperty( QX11Info::display(), win, wm_state, 0, 0, False, AnyPropertyType,
	    &type, &format, &nitems, &after, &data ) == Success )
	    {
	    if( data != NULL )
		XFree( data );
	    if( type != None ) // toplevel window
		break;
	    }
	}
    if( x11_events )
	kapp->installX11EventFilter( this );
    }
    	
bool KRandrPassivePopup::eventFilter( QObject* o, QEvent* e )
    {
    if( e->type() == QEvent::Move && o->isWidgetType()
	&& watched_widgets.contains( static_cast< QWidget* >( o )))
        QTimer::singleShot( 0, this, SLOT(slotPositionSelf()));
    return false;
    }

bool KRandrPassivePopup::x11Event( XEvent* e )
    {
    if( e->type == ConfigureNotify && watched_windows.contains( e->xconfigure.window ) )
	{
	if( !update_timer.isActive() )
	    update_timer.start( 10 );
	return false;
	}
    return KPassivePopup::x11Event( e );
    }
        
void KRandrPassivePopup::slotPositionSelf()
    {
    positionSelf();
    }
    
#include "krandrpassivepopup.moc"
