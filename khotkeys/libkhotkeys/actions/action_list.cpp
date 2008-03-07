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

#include <kconfiggroup.h>

namespace KHotKeys {

Action_list::Action_list( KConfigGroup& cfg_P, Action_data* data_P )
    : QList< Action* >()
    {
    int cnt = cfg_P.readEntry( "ActionsCount", 0 );
    QString save_cfg_group = cfg_P.name();
    for( int i = 0;
         i < cnt;
         ++i )
        {
        KConfigGroup group( cfg_P.config(), save_cfg_group + QString::number( i ) );
        Action* action = Action::create_cfg_read( group, data_P );
        if( action )
            append( action );
        }
    }


Action_list::~Action_list()
    {
    while (!isEmpty())
        {
        delete takeFirst();
        }
    }


void Action_list::cfg_write( KConfigGroup& cfg_P ) const
    {
    QString save_cfg_group = cfg_P.name();
    int i = 0;
    for( Action_list::ConstIterator it = begin();
         it != end();
         ++it )
        {
        KConfigGroup group( cfg_P.config(), save_cfg_group + QString::number( i ) );
        (*it)->cfg_write( group );
        }
    cfg_P.writeEntry( "ActionsCount", i );
    }

} // namespace KHotKeys
