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

#include "window_selection_rules.h"

#include <KDebug>


namespace KHotKeys {

Windowdef::Windowdef( const QString& comment_P )
    : _comment( comment_P )
    {
    }


Windowdef::Windowdef( KConfigGroup& cfg_P )
    {
    _comment = cfg_P.readEntry( "Comment" );
    }


Windowdef::~Windowdef()
    {
    }


void Windowdef::cfg_write( KConfigGroup& cfg_P ) const
    {
    cfg_P.writeEntry( "Type", "ERROR" );
    cfg_P.writeEntry( "Comment", comment());
    }


const QString& Windowdef::comment() const
    {
    return _comment;
    }


Windowdef* Windowdef::create_cfg_read( KConfigGroup& cfg_P )
    {
    QString type = cfg_P.readEntry( "Type" );
    if( type == "SIMPLE" )
        return new Windowdef_simple( cfg_P );
    kWarning() << "Unknown Windowdef type read from cfg file\n";
    return NULL;
    }


void Windowdef::set_comment(const QString &comment)
    {
    _comment = comment;
    }



} // namespace KHotKeys
