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

#include <KDE/KConfigGroup>
#include <KDE/KDebug>
#include <KDE/KProcess>

namespace KHotKeys {

Dbus_action::Dbus_action( KConfigGroup& cfg_P, Action_data* data_P )
    : Action( cfg_P, data_P )
    {
    app = cfg_P.readEntry( "RemoteApp" );
    obj = cfg_P.readEntry( "RemoteObj" );
    call = cfg_P.readEntry( "Call" );
    args = cfg_P.readEntry( "Arguments" );
    }

void Dbus_action::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "DBUS" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "RemoteApp", app );
    cfg_P.writeEntry( "RemoteObj", obj );
    cfg_P.writeEntry( "Call", call );
    cfg_P.writeEntry( "Arguments", args );
    }

void Dbus_action::execute()
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
    kDebug( 1217 ) << "D-Bus call:" << app << ":" << obj << ":" << call << ":" << args_list;
    KProcess proc;
    proc << "qdbus" << app << obj << call << args_list;
    proc.startDetached();
    }

const QString Dbus_action::description() const
    {
    return i18n( "D-Bus: " ) + remote_application() + "::" + remote_object() + "::"
        + called_function();
    }

Action* Dbus_action::copy( Action_data* data_P ) const
    {
    return new Dbus_action( data_P, remote_application(), remote_object(),
        called_function(), arguments());
    }

} // namespace KHotKeys

