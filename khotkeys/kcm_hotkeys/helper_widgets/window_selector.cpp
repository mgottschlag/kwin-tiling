/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define WINDOWSELECTOR_CPP

#include "helper_widgets/window_selector.h"

#include <kdebug.h>
#include <kapplication.h>

#include <QtGui/QDesktopWidget>

#include <X11/Xlib.h>
#include <QX11Info>
#include <fixx11h.h>

namespace KHotKeys
{

WindowSelector::WindowSelector( QObject* receiver_P, const char* slot_P )
    {
    connect( this, SIGNAL(selected_signal(WId)), receiver_P, slot_P );
    }

void WindowSelector::select()
    {
    kapp->desktop()->grabMouse( QCursor( Qt::CrossCursor ));
    kapp->installX11EventFilter( this );
    }

bool WindowSelector::x11Event( XEvent* e )
    {
    if( e->type != ButtonPress )
        return false;
    kapp->desktop()->releaseMouse();
    if( e->xbutton.button == Button1 )
        {
        WId window = findRealWindow( e->xbutton.subwindow );
        if( window )
            selected_signal( window );
        }
    delete this;
    return true;
    }

WId WindowSelector::findRealWindow( WId w, int depth )
    {
    if( depth > 5 )
        return None;
    static Atom wm_state = XInternAtom( QX11Info::display(), "WM_STATE", False );
    Atom type;
    int format;
    unsigned long nitems, after;
    unsigned char* prop;
    if( XGetWindowProperty( QX11Info::display(), w, wm_state, 0, 0, False, AnyPropertyType,
        &type, &format, &nitems, &after, &prop ) == Success )
        {
        if( prop != NULL )
            XFree( prop );
        if( type != None )
            return w;
        }
    Window root, parent;
    Window* children;
    unsigned int nchildren;
    Window ret = None;
    if( XQueryTree( QX11Info::display(), w, &root, &parent, &children, &nchildren ) != 0 )
        {
        for( unsigned int i = 0;
             i < nchildren && ret == None;
             ++i )
            ret = findRealWindow( children[ i ], depth + 1 );
        if( children != NULL )
            XFree( children );
        }
    return ret;
}


} // namespace KHotKeys

#include "window_selector.moc"
