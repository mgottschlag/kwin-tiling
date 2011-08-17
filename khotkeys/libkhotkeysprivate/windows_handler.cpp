/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _WINDOWS_CPP_

#include "windows_handler.h"

#include "windows_helper/window_selection_rules.h"
#include "windows_helper/window_selection_list.h"

#include <QRegExp>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kwindowsystem.h>
#include <klocale.h>

#include "khotkeysglobal.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>

namespace KHotKeys
{

// WindowsHandler

WindowsHandler::WindowsHandler( bool enable_signal_P, QObject* parent_P )
    : QObject( parent_P ), signals_enabled( enable_signal_P ),
        _action_window( 0 )
    {
    if( signals_enabled )
        {
        connect( KWindowSystem::self(), SIGNAL(windowAdded(WId)), SLOT(window_added_slot(WId)));
        connect( KWindowSystem::self(), SIGNAL(windowRemoved(WId)), SLOT(window_removed_slot(WId)));
        connect( KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)),
            SLOT(active_window_changed_slot(WId)));
        }
    }

WindowsHandler::~WindowsHandler()
    {
    }

void WindowsHandler::window_added_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_added( window_P );
    // CHECKME tyhle i dalsi by asi mely jit nastavit, jestli aktivuji vsechny, nebo jen jeden
    // pripojeny slot ( stejne jako u Kdb, kde by to take melo jit nastavit )
    }

void WindowsHandler::window_removed_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_removed( window_P );
    if( window_P == _action_window )
        _action_window = 0;
    }

void WindowsHandler::active_window_changed_slot( WId window_P )
    {
    if( signals_enabled )
        emit active_window_changed( window_P );
    }

void WindowsHandler::window_changed_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_changed( window_P );
    }

void WindowsHandler::window_changed_slot( WId window_P, unsigned int flags_P )
    {
    if( signals_enabled )
        emit window_changed( window_P, flags_P );
    }

QString WindowsHandler::get_window_role( WId id_P )
    {
    // TODO this is probably just a hack
    return KWindowSystem::windowInfo( id_P, 0, NET::WM2WindowRole ).windowRole();
    }

QString WindowsHandler::get_window_class( WId id_P )
    {
    XClassHint hints_ret;
    if( XGetClassHint( QX11Info::display(), id_P, &hints_ret ) == 0 ) // 0 means error
	return "";
    QString ret( hints_ret.res_name );
    ret += ' ';
    ret += hints_ret.res_class;
    XFree( hints_ret.res_name );
    XFree( hints_ret.res_class );
    return ret;
    }

WId WindowsHandler::active_window()
    {
    return KWindowSystem::activeWindow();
    }

WId WindowsHandler::action_window()
    {
    return _action_window;
    }

void WindowsHandler::set_action_window( WId window_P )
    {
    _action_window = window_P;
    }

WId WindowsHandler::find_window( const Windowdef_list* window_P )
    {
    for( QList< WId >::const_iterator it = KWindowSystem::windows().begin();
         it != KWindowSystem::windows().end();
         ++it )
        {
        Window_data tmp( *it );
        if( window_P->match( tmp ))
            return *it;
        }
    return None;
    }

WId WindowsHandler::window_at_position( int x, int y )
    {
    Window child, dummy;
    Window parent = QX11Info::appRootWindow();
    Atom wm_state = XInternAtom( QX11Info::display(), "WM_STATE", False );
    for( int i = 0;
         i < 10;
         ++i )
        {
        int destx, desty;
        // find child at that position
        if( !XTranslateCoordinates( QX11Info::display(), parent, parent, x, y, &destx, &desty, &child )
            || child == None )
            return 0;
        // and now transform coordinates to the child
        if( !XTranslateCoordinates( QX11Info::display(), parent, child, x, y, &destx, &desty, &dummy ))
            return 0;
        x = destx;
        y = desty;
        Atom type;
        int format;
        unsigned long nitems, after;
        unsigned char* prop;
        if( XGetWindowProperty( QX11Info::display(), child, wm_state, 0, 0, False, AnyPropertyType,
	    &type, &format, &nitems, &after, &prop ) == Success )
            {
	    if( prop != NULL )
	        XFree( prop );
	    if( type != None )
	        return child;
            }
        parent = child;
        }
    return 0;

    }

void WindowsHandler::activate_window( WId id_P )
    {
    KWindowSystem::forceActiveWindow( id_P );
    }

// Window_data

Window_data::Window_data( WId id_P )
    : type( NET::Unknown )
    {
    KWindowInfo kwin_info = KWindowSystem::windowInfo( id_P, NET::WMName | NET::WMWindowType ); // TODO optimize
    if( kwin_info.valid())
        {
        title = kwin_info.name();
        role = windows_handler->get_window_role( id_P );
        wclass = windows_handler->get_window_class( id_P );
        type = kwin_info.windowType( SUPPORTED_WINDOW_TYPES_MASK );
        if( type == NET::Override ) // HACK consider non-NETWM fullscreens to be normal too
            type = NET::Normal;
        if( type == NET::Unknown )
            type = NET::Normal;
        }
    }

} // namespace KHotKeys

#include "moc_windows_handler.cpp"

