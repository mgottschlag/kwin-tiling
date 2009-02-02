/* Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
   Copyright (C) 2008 Michael Jansen <kde@michael-jansen.biz>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   Version 2 as published by the Free Software Foundation;

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "windows_helper/window_selection_rules.h"
#include "windows_helper/window_selection_rules.h"

#include <KLocale>
#include <KDebug>

namespace KHotKeys {

Windowdef_simple::Windowdef_simple(
        const QString& comment_P,
        const QString& title_P,
        substr_type_t title_type_P,
        const QString& wclass_P,
        substr_type_t wclass_type_P,
        const QString& role_P,
        substr_type_t role_type_P,
        int window_types_P )
    :   Windowdef( comment_P ),
        _title( title_P ),
        _title_match_type( title_type_P ),
        _wclass( wclass_P ),
        _wclass_match_type( wclass_type_P ),
        _role( role_P ),
        _role_match_type( role_type_P ),
        _window_types( window_types_P )
    {}


Windowdef_simple::Windowdef_simple( KConfigGroup& cfg_P )
    : Windowdef( cfg_P )
    {
    _title = cfg_P.readEntry( "Title" );
    _title_match_type = static_cast< substr_type_t >( cfg_P.readEntry( "TitleType",0 ));
    _wclass = cfg_P.readEntry( "Class" );
    _wclass_match_type = static_cast< substr_type_t >( cfg_P.readEntry( "ClassType",0 ));
    _role = cfg_P.readEntry( "Role" );
    _role_match_type = static_cast< substr_type_t >( cfg_P.readEntry( "RoleType", 0 ));
    _window_types = cfg_P.readEntry( "WindowTypes",0 );
    }


void Windowdef_simple::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Title", title());
    cfg_P.writeEntry( "TitleType", int(_title_match_type) );
    cfg_P.writeEntry( "Class", wclass());
    cfg_P.writeEntry( "ClassType", int(_wclass_match_type) );
    cfg_P.writeEntry( "Role", role());
    cfg_P.writeEntry( "RoleType", int(_role_match_type) );
    cfg_P.writeEntry( "WindowTypes", window_types());
    cfg_P.writeEntry( "Type", "SIMPLE" ); // overwrites value set in base::cfg_write()
    }


Windowdef_simple* Windowdef_simple::copy() const
    {
    return new Windowdef_simple( comment(), title(), title_match_type(), wclass(),
        wclass_match_type(), role(), role_match_type(), window_types());
    }


const QString Windowdef_simple::description() const
    {
    return i18n( "Window simple: " ) + comment();
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


bool Windowdef_simple::match( const Window_data& window_P )
    {
    if( !type_match( window_P.type ))
        return false;
    if( !is_substr_match( window_P.title, title(), _title_match_type ))
        return false;
    if( !is_substr_match( window_P.wclass, wclass(), _wclass_match_type ))
        return false;
    if( !is_substr_match( window_P.role, role(), _role_match_type ))
        return false;
    kDebug() << "window match:" << window_P.title << ":OK";
    return true;
    }


const QString& Windowdef_simple::role() const
    {
    return _role;
    }


Windowdef_simple::substr_type_t Windowdef_simple::role_match_type() const
    {
    return _role_match_type;
    }


void Windowdef_simple::set_title(const QString &title)
    {
    _title = title;
    }


void Windowdef_simple::set_title_match_type(const substr_type_t &type)
    {
    _title_match_type = type;
    }


void Windowdef_simple::set_role(const QString &role)
    {
    _role = role;
    }


void Windowdef_simple::set_role_match_type(const substr_type_t &type)
    {
    _role_match_type = type;
    }


void Windowdef_simple::set_window_types(const int types)
    {
    _window_types = types;
    }


void Windowdef_simple::set_wclass(const QString &wclass)
    {
    _wclass = wclass;
    }


void Windowdef_simple::set_wclass_match_type(const substr_type_t &type)
    {
    _wclass_match_type = type;
    }


const QString& Windowdef_simple::title() const
    {
    return _title;
    }


Windowdef_simple::substr_type_t Windowdef_simple::title_match_type() const
    {
    return _title_match_type;
    }


bool Windowdef_simple::type_match( window_type_t type_P ) const
    {
    return window_types() & type_P;
    }


bool Windowdef_simple::type_match( NET::WindowType type_P ) const
    {
    return ( window_types() & ( 1 << type_P ))
        || ( type_P == NET::Unknown && ( window_types() & WINDOW_TYPE_NORMAL ));
        // CHECKME HACK haaaack !
    }


const QString& Windowdef_simple::wclass() const
    {
    return _wclass;
    }


Windowdef_simple::substr_type_t Windowdef_simple::wclass_match_type() const
    {
    return _wclass_match_type;
    }


int Windowdef_simple::window_types() const
    {
    return _window_types;
    }

} // namespace KHotKeys

