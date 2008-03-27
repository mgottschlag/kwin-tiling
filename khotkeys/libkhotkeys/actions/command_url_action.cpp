/*
   Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "actions.h"
#include "action_data.h"

#include <KDE/KAuthorized>
#include <KDE/KConfigGroup>
#include <KDE/KRun>
#include <KDE/KService>
#include <KDE/KUriFilter>
#include <kworkspace/kworkspace.h>


namespace KHotKeys {

Command_url_action::Command_url_action( Action_data* data_P, const QString& command_url_P )
    : Action( data_P ), _command_url( command_url_P )
    {
    }


QString Command_url_action::command_url() const
    {
    return _command_url;
    }


Command_url_action::Command_url_action( KConfigGroup& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    _command_url = cfg_P.readEntry( "CommandURL" );
    }


void Command_url_action::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "CommandURL", command_url());
    cfg_P.writeEntry( "Type", "COMMAND_URL" ); // overwrites value set in base::cfg_write()
    }


Action* Command_url_action::copy( Action_data* data_P ) const
    {
    return new Command_url_action( data_P, command_url());
    }


const QString Command_url_action::description() const
    {
    return i18n( "Command/URL : " ) + command_url();
    }


void Command_url_action::execute()
    {
    if( command_url().isEmpty())
        return;
    KUriFilterData uri;
    QString cmd = command_url();
    static bool sm_ready = false;
    if( !sm_ready )
        {
        KWorkSpace::propagateSessionManager();
        sm_ready = true;
        }
//    int space_pos = command_url().find( ' ' );
//    if( command_url()[ 0 ] != '\'' && command_url()[ 0 ] != '"' && space_pos > -1
//        && command_url()[ space_pos - 1 ] != '\\' )
//        cmd = command_url().left( space_pos ); // get first 'word'
    uri.setData( cmd );
    KUriFilter::self()->filterUri( uri );
    if( uri.uri().isLocalFile() && !uri.uri().hasRef() )
        cmd = uri.uri().path();
    else
        cmd = uri.uri().url();
    switch( uri.uriType())
        {
        case KUriFilterData::LocalFile:
        case KUriFilterData::LocalDir:
        case KUriFilterData::NetProtocol:
        case KUriFilterData::Help:
            {
            ( void ) new KRun( uri.uri(),0L);
          break;
            }
        case KUriFilterData::Executable:
            {
            if (!KAuthorized::authorizeKAction("shell_access"))
                return;
            if( !uri.hasArgsAndOptions())
                {
                KService::Ptr service = KService::serviceByDesktopName( cmd );
                if( service )
                    {
                    KRun::run( *service, KUrl::List(), NULL );
                  break;
                    }
                }
            // fall though
            }
        case KUriFilterData::Shell:
            {
            if (!KAuthorized::authorizeKAction("shell_access"))
                return;
            if( !KRun::runCommand(
                cmd + ( uri.hasArgsAndOptions() ? uri.argsAndOptions() : "" ),
                cmd, uri.iconName(), NULL )) {
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


void Command_url_action::set_command_url( const QString &command )
    {
    _command_url = command;
    }

} // namespace KHotKeys
