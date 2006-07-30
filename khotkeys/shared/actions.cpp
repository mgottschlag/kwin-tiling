/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#define _ACTIONS_CPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "actions.h"

#include <krun.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kurifilter.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kapplication.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <kservice.h>
#include <ksharedptr.h>
#include <kprocess.h>

#include "windows.h"
#include "action_data.h"

#include <X11/X.h>
#include <QX11Info>
//Added by qt3to4:
#include <Q3PtrList>
#include <kauthorized.h>

namespace KHotKeys
{

// Action
    
Action* Action::create_cfg_read( KConfig& cfg_P, Action_data* data_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "COMMAND_URL" )
        return new Command_url_action( cfg_P, data_P );
    if( type == "MENUENTRY" )
        return new Menuentry_action( cfg_P, data_P );
    if( type == "DCOP" )
        return new Dcop_action( cfg_P, data_P );
    if( type == "KEYBOARD_INPUT" )
        return new Keyboard_input_action( cfg_P, data_P );
    if( type == "ACTIVATE_WINDOW" )
        return new Activate_window_action( cfg_P, data_P );
    kWarning( 1217 ) << "Unknown Action type read from cfg file\n";
    return NULL;
    }

void Action::cfg_write( KConfig& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" ); // derived classes should call with their type
    }

// Action_list

Action_list::Action_list( KConfig& cfg_P, Action_data* data_P )
    : Q3PtrList< Action >()
    {
    setAutoDelete( true );
    QString save_cfg_group = cfg_P.group();
    int cnt = cfg_P.readEntry( "ActionsCount", 0 );
    for( int i = 0;
         i < cnt;
         ++i )
        {
        cfg_P.setGroup( save_cfg_group + QString::number( i ));
        Action* action = Action::create_cfg_read( cfg_P, data_P );
        if( action )
            append( action );
        }
    cfg_P.setGroup( save_cfg_group );
    }
    
void Action_list::cfg_write( KConfig& cfg_P ) const
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
    cfg_P.writeEntry( "ActionsCount", i );     
    }

// Command_url_action

Command_url_action::Command_url_action( KConfig& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    _command_url = cfg_P.readEntry( "CommandURL" );
    }

void Command_url_action::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "CommandURL", command_url());    
    cfg_P.writeEntry( "Type", "COMMAND_URL" ); // overwrites value set in base::cfg_write()
    }

void Command_url_action::execute()
    {
    if( command_url().isEmpty())
        return;
    KURIFilterData uri;
    QString cmd = command_url();
//    int space_pos = command_url().find( ' ' );
//    if( command_url()[ 0 ] != '\'' && command_url()[ 0 ] != '"' && space_pos > -1
//        && command_url()[ space_pos - 1 ] != '\\' )
//        cmd = command_url().left( space_pos ); // get first 'word'
    uri.setData( cmd );
    KURIFilter::self()->filterURI( uri );
    if( uri.uri().isLocalFile() && !uri.uri().hasRef() )
        cmd = uri.uri().path();
    else
        cmd = uri.uri().url();
    switch( uri.uriType())
        {
        case KURIFilterData::LOCAL_FILE:
        case KURIFilterData::LOCAL_DIR:
        case KURIFilterData::NET_PROTOCOL:
        case KURIFilterData::HELP:
            {
            ( void ) new KRun( uri.uri(),0L);
          break;
            }
        case KURIFilterData::EXECUTABLE:
            {
            if (!KAuthorized::authorizeKAction("shell_access"))
		return;
            if( !uri.hasArgsAndOptions())
                {
                KService::Ptr service = KService::serviceByDesktopName( cmd );
                if( service )
                    {
                    KRun::run( *service, KUrl::List(), 0 );
                  break;
                    }
                }
            // fall though
            }
        case KURIFilterData::SHELL:
            {
            if (!KAuthorized::authorizeKAction("shell_access"))
		return;
            if( !KRun::runCommand(
                cmd + ( uri.hasArgsAndOptions() ? uri.argsAndOptions() : "" ),
                cmd, uri.iconName())) {
                // CHECKME ?
             }
          break;
            }
        default: // error
          return;
        }
    timeout.setSingleShot( true );
    timeout.start( 1000 ); // 1sec timeout

    }

const QString Command_url_action::description() const
    {
    return i18n( "Command/URL : " ) + command_url();
    }

Action* Command_url_action::copy( Action_data* data_P ) const
    {
    return new Command_url_action( data_P, command_url());
    }

// Menuentry_action

void Menuentry_action::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "MENUENTRY" ); // overwrites value set in base::cfg_write()
    }

KService::Ptr Menuentry_action::service() const
    {
    if (!_service)
    {
        const_cast<Menuentry_action *>(this)->_service = KService::serviceByStorageId(command_url());
    }
    return _service;
    }

void Menuentry_action::execute()
    {
    (void) service();
    if (!_service)
        return;
    KRun::run( *_service, KUrl::List(), 0 );
    timeout.setSingleShot( true );
    timeout.start( 1000 ); // 1sec timeout
    }

const QString Menuentry_action::description() const
    {
    (void) service();
    return i18n( "Menuentry : " ) + (_service ? _service->name() : QString());
    }

Action* Menuentry_action::copy( Action_data* data_P ) const
    {
    return new Menuentry_action( data_P, command_url());
    }

// Dcop_action

Dcop_action::Dcop_action( KConfig& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    app = cfg_P.readEntry( "RemoteApp" );
    obj = cfg_P.readEntry( "RemoteObj" );
    call = cfg_P.readEntry( "Call" );
    args = cfg_P.readEntry( "Arguments" );
    }
    
void Dcop_action::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "DCOP" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "RemoteApp", app );
    cfg_P.writeEntry( "RemoteObj", obj );
    cfg_P.writeEntry( "Call", call );
    cfg_P.writeEntry( "Arguments", args );
    }

void Dcop_action::execute()
    {
    if( app.isEmpty() || obj.isEmpty() || call.isEmpty())
        return;
    QStringList args_list;
    QString args_str = args;
    while( !args_str.isEmpty())
        {
        int pos = 0;
        while( args_str[ pos ] == ' ' )
            ++pos;
        if( args_str[ pos ] == '\"' || args_str[ pos ] == '\'' )
            {
            QString val = "";
            QChar sep = args_str[ pos ];
            bool skip = false;
            ++pos;
            for(;
                 pos < args_str.length();
                 ++pos )
                {
                if( args_str[ pos ] == '\\' )
                    {
                    skip = true;
                    continue;
                    }
                if( !skip && args_str[ pos ] == sep )
                    break;
                skip = false;
                val += args_str[ pos ];
                }
            if( pos >= args_str.length())
                return;
            ++pos;
            args_str = args_str.mid( pos );
            args_list.append( val );
            }
        else
            {
            // one word
            if( pos != 0 )
                args_str = args_str.mid( pos );
            int nxt_pos = args_str.indexOf( ' ' );
            args_list.append( args_str.left( nxt_pos )); // should be ok if nxt_pos is -1
            args_str = nxt_pos >= 0 ? args_str.mid( nxt_pos ) : "";
            }
        }
    kDebug( 1217 ) << "DCOP call:" << app << ":" << obj << ":" << call << ":" << args_list << endl;
    KProcess proc;
    proc << "dcop" << app << obj << call << args_list;
    proc.start( KProcess::DontCare );
    }
    
const QString Dcop_action::description() const
    {
    return i18n( "DCOP : " ) + remote_application() + "::" + remote_object() + "::"
        + called_function();
    }

Action* Dcop_action::copy( Action_data* data_P ) const
    {
    return new Dcop_action( data_P, remote_application(), remote_object(),
        called_function(), arguments());
    }

// Keyboard_input_action

Keyboard_input_action::Keyboard_input_action( KConfig& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    _input = cfg_P.readEntry( "Input" );
    if( cfg_P.readEntry( "IsDestinationWindow" , QVariant(false)).toBool())
        {
        QString save_cfg_group = cfg_P.group();
        cfg_P.setGroup( save_cfg_group + "DestinationWindow" );
        _dest_window = new Windowdef_list( cfg_P );
        _active_window = false; // ignored with _dest_window set anyway
        cfg_P.setGroup( save_cfg_group );
        }
    else
        {
        _dest_window = NULL;
        _active_window = cfg_P.readEntry( "ActiveWindow" , QVariant(false)).toBool();
        }
    }

Keyboard_input_action::~Keyboard_input_action()
    {
    delete _dest_window;
    }
    
void Keyboard_input_action::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "KEYBOARD_INPUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Input", input());
    if( dest_window() != NULL )
        {
        cfg_P.writeEntry( "IsDestinationWindow", true );
        QString save_cfg_group = cfg_P.group();
        cfg_P.setGroup( save_cfg_group + "DestinationWindow" );
        dest_window()->cfg_write( cfg_P );
        cfg_P.setGroup( save_cfg_group );
        }
    else
        cfg_P.writeEntry( "IsDestinationWindow", false );
    cfg_P.writeEntry( "ActiveWindow", _active_window );
    }
    
void Keyboard_input_action::execute()
    {
    if( input().isEmpty())
        return;
    Window w = InputFocus;
    if( dest_window() != NULL )
        {
        w = windows_handler->find_window( dest_window());
        if( w == None )
            w = InputFocus;
        }
    else
        {
        if( !_active_window )
            w = windows_handler->action_window();
        if( w == None )
            w = InputFocus;
        }
    int last_index = -1, start = 0;
    while(( last_index = input().indexOf( ':', last_index + 1 )) != -1 ) // find next ';'
	{
        QString key = input().mid( start, last_index - start ).trimmed();
        QKeySequence ks( key );
        if( key == "Enter" && ks.isEmpty() )
            key = "Return"; // CHECKE hack
	keyboard_handler->send_macro_key( ks.isEmpty() ? 0 : ks[0], w );
	start = last_index + 1;
	}
    // and the last one
    QString key = input().mid( start, input().length()).trimmed();
    QKeySequence ks( key );
    if( key == "Enter" && ks.isEmpty() )
        key = "Return";
    keyboard_handler->send_macro_key( ks.isEmpty() ? 0 : ks[0], w ); // the rest
    XFlush( QX11Info::display());
    }
    
const QString Keyboard_input_action::description() const
    {
    QString tmp = input();
    tmp.replace( '\n', ' ' );
    tmp.truncate( 30 );
    return i18n( "Keyboard input : " ) + tmp;
    }

Action* Keyboard_input_action::copy( Action_data* data_P ) const
    {
    return new Keyboard_input_action( data_P, input(),
        dest_window() ? dest_window()->copy() : NULL, _active_window );
    }

// Activate_window_action

Activate_window_action::Activate_window_action( KConfig& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    _window = new Windowdef_list( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    }

Activate_window_action::~Activate_window_action()
    {
    delete _window;
    }
    
void Activate_window_action::cfg_write( KConfig& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "ACTIVATE_WINDOW" ); // overwrites value set in base::cfg_write()
    QString save_cfg_group = cfg_P.group();
    cfg_P.setGroup( save_cfg_group + "Window" );
    window()->cfg_write( cfg_P );
    cfg_P.setGroup( save_cfg_group );
    }
    
void Activate_window_action::execute()
    {
    if( window()->match( windows_handler->active_window()))
        return; // is already active
    WId win_id = windows_handler->find_window( window());
    if( win_id != None )
        windows_handler->activate_window( win_id );
    }
    
const QString Activate_window_action::description() const
    {
    return i18n( "Activate window : " ) + window()->comment();
    }

Action* Activate_window_action::copy( Action_data* data_P ) const
    {
    return new Activate_window_action( data_P, window()->copy());
    }

} // namespace KHotKeys
