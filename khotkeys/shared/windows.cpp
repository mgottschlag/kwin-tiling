/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _WINDOWS_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "windows.h"

#include <assert.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3PtrList>

#include <kconfig.h>
#include <kdebug.h>
#include <kwinmodule.h>
#include <kwin.h>
#include <klocale.h>

#include "khotkeysglobal.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>

namespace KHotKeys
{

// Windows

Windows::Windows( bool enable_signal_P, QObject* parent_P )
    : QObject( parent_P ), signals_enabled( enable_signal_P ),
        kwin_module( new KWinModule( this )), _action_window( 0 )
    {
    assert( windows_handler == NULL );
    windows_handler = this;
    if( signals_enabled )
        {
        connect( kwin_module, SIGNAL( windowAdded( WId )), SLOT( window_added_slot( WId )));
        connect( kwin_module, SIGNAL( windowRemoved( WId )), SLOT( window_removed_slot( WId )));
        connect( kwin_module, SIGNAL( activeWindowChanged( WId )),
            SLOT( active_window_changed_slot( WId )));
        }
    }

Windows::~Windows()
    {
    windows_handler = NULL;
    }

void Windows::window_added_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_added( window_P );
    // CHECKME tyhle i dalsi by asi mely jit nastavit, jestli aktivuji vsechny, nebo jen jeden
    // pripojeny slot ( stejne jako u Kdb, kde by to take melo jit nastavit )
    }

void Windows::window_removed_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_removed( window_P );
    if( window_P == _action_window )
        _action_window = 0;
    }

void Windows::active_window_changed_slot( WId window_P )
    {
    if( signals_enabled )
        emit active_window_changed( window_P );
    }

void Windows::window_changed_slot( WId window_P )
    {
    if( signals_enabled )
        emit window_changed( window_P );
    }

void Windows::window_changed_slot( WId window_P, unsigned int flags_P )
    {
    if( signals_enabled )
        emit window_changed( window_P, flags_P );
    }

QString Windows::get_window_role( WId id_P )
    {
    // TODO this is probably just a hack
    return KWin::windowInfo( id_P, 0, NET::WM2WindowRole ).windowRole();
    }

QString Windows::get_window_class( WId id_P )
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

WId Windows::active_window()
    {
    return kwin_module->activeWindow();
    }

WId Windows::action_window()
    {
    return _action_window;
    }

void Windows::set_action_window( WId window_P )
    {
    _action_window = window_P;
    }

WId Windows::find_window( const Windowdef_list* window_P )
    {
    for( QList< WId >::const_iterator it = kwin_module->windows().begin();
         it != kwin_module->windows().end();
         ++it )
        {
        Window_data tmp( *it );
        if( window_P->match( tmp ))
            return *it;
        }
    return None;
    }

WId Windows::window_at_position( int x, int y )
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

void Windows::activate_window( WId id_P )
    {
    KWin::forceActiveWindow( id_P );
    }

// Window_data

Window_data::Window_data( WId id_P )
    : type( NET::Unknown )
    {
    KWin::WindowInfo kwin_info = KWin::windowInfo( id_P, NET::WMName | NET::WMWindowType ); // TODO optimize
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

// Windowdef

void Windowdef::cfg_write( KConfig& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    cfg_P.writeEntry( "Comment", comment());
    }

Windowdef::Windowdef( KConfig& cfg_P )
    {
    _comment = cfg_P.readEntry( "Comment" );
    }

Windowdef* Windowdef::create_cfg_read( KConfig& cfg_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "SIMPLE" )
        return new Windowdef_simple( cfg_P );
    kWarning( 1217 ) << "Unknown Windowdef type read from cfg file\n";
    return NULL;
    }

// Windowdef_list

Windowdef_list::Windowdef_list( KConfig& cfg_P )
    : Q3PtrList< Windowdef >()
    {
    setAutoDelete( true );
    QString save_cfg_group = cfg_P.group();
    _comment = cfg_P.readEntry( "Comment" );
    int cnt = cfg_P.readEntry( "WindowsCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        cfg_P.setGroup( save_cfg_group + QString::number( i ));
        Windowdef* window = Windowdef::create_cfg_read( cfg_P );
        if( window )
            append( window );
        }
    cfg_P.setGroup( save_cfg_group );
    }

void Windowdef_list::cfg_write( KConfig& cfg_P ) const
    {
    QString save_cfg_group = cfg_P.group();
    int i = 0;
    for( Iterator it( *this );
         it;
         ++it, ++i )
        {
        cfg_P.setGroup( save_cfg_group + QString::number( i ));
        it.current()->cfg_write( cfg_P );
        }
    cfg_P.setGroup( save_cfg_group );
    cfg_P.writeEntry( "WindowsCount", i );
    cfg_P.writeEntry( "Comment", comment());
    }

Windowdef_list* Windowdef_list::copy() const
    {
    Windowdef_list* ret = new Windowdef_list( comment());
    for( Iterator it( *this );
         it;
         ++it )
        ret->append( it.current()->copy());
    return ret;
    }


bool Windowdef_list::match( const Window_data& window_P ) const
    {
    if( count() == 0 ) // CHECKME no windows to match => ok
        return true;
    for( Iterator it( *this );
         it;
         ++it )
        if( it.current()->match( window_P ))
            return true;
    return false;
    }

// Windowdef_simple

Windowdef_simple::Windowdef_simple( const QString& comment_P, const QString& title_P,
    substr_type_t title_type_P, const QString& wclass_P, substr_type_t wclass_type_P,
    const QString& role_P, substr_type_t role_type_P, int window_types_P )
    : Windowdef( comment_P ), _title( title_P ), title_type( title_type_P ),
    _wclass( wclass_P ), wclass_type( wclass_type_P ), _role( role_P ),
    role_type( role_type_P ), _window_types( window_types_P )
    {
    }

Windowdef_simple::Windowdef_simple( KConfig& cfg_P )
    : Windowdef( cfg_P )
    {
    _title = cfg_P.readEntry( "Title" );
    title_type = static_cast< substr_type_t >( cfg_P.readEntry( "TitleType",0 ));
    _wclass = cfg_P.readEntry( "Class" );
    wclass_type = static_cast< substr_type_t >( cfg_P.readEntry( "ClassType",0 ));
    _role = cfg_P.readEntry( "Role" );
    role_type = static_cast< substr_type_t >( cfg_P.readEntry( "RoleType", 0 ));
    _window_types = cfg_P.readEntry( "WindowTypes",0 );
    }

void Windowdef_simple::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Title", title());
    cfg_P.writeEntry( "TitleType", int(title_type) );
    cfg_P.writeEntry( "Class", wclass());
    cfg_P.writeEntry( "ClassType", int(wclass_type) );
    cfg_P.writeEntry( "Role", role());
    cfg_P.writeEntry( "RoleType", int(role_type) );
    cfg_P.writeEntry( "WindowTypes", window_types());
    cfg_P.writeEntry( "Type", "SIMPLE" ); // overwrites value set in base::cfg_write()
    }

bool Windowdef_simple::match( const Window_data& window_P )
    {
    if( !type_match( window_P.type ))
        return false;
    if( !is_substr_match( window_P.title, title(), title_type ))
        return false;
    if( !is_substr_match( window_P.wclass, wclass(), wclass_type ))
        return false;
    if( !is_substr_match( window_P.role, role(), role_type ))
        return false;
    kDebug( 1217 ) << "window match:" << window_P.title << ":OK" << endl;
    return true;
    }

bool Windowdef_simple::is_substr_match( const QString& str1_P, const QString& str2_P,
    substr_type_t type_P )
    {
    switch( type_P )
        {
        case NOT_IMPORTANT :
          return true;
        case CONTAINS :
          return str1_P.contains( str2_P ) > 0;
        case IS :
          return str1_P == str2_P;
        case REGEXP :
            {
            QRegExp rg( str2_P );
          return rg.indexIn( str1_P ) >= 0;
            }
        case CONTAINS_NOT :
          return str1_P.contains( str2_P ) == 0;
        case IS_NOT :
          return str1_P != str2_P;
        case REGEXP_NOT :
            {
            QRegExp rg( str2_P );
          return rg.indexIn( str1_P ) < 0;
            }
        }
    return false;
    }

Windowdef* Windowdef_simple::copy() const
    {
    return new Windowdef_simple( comment(), title(), title_match_type(), wclass(),
        wclass_match_type(), role(), role_match_type(), window_types());
    }

const QString Windowdef_simple::description() const
    {
    return i18n( "Window simple: " ) + comment();
    }

} // namespace KHotKeys

#include "windows.moc"
