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

#include "triggers.h"
#include "action_data.h"
#include "gestures.h"
#include "windows.h"

#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Gesture_trigger::Gesture_trigger( Action_data* data_P, const QString &gesturecode_P )
    : Trigger( data_P ), _gesturecode( gesturecode_P )
    {
    }

Gesture_trigger::Gesture_trigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P )
    {
    _gesturecode = cfg_P.readEntry( "Gesture" );
    }

Gesture_trigger::~Gesture_trigger()
    {
    gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }

void Gesture_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Gesture", gesturecode());
    cfg_P.writeEntry( "Type", "GESTURE" ); // overwrites value set in base::cfg_write()
    }

Trigger* Gesture_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Gesture_trigger::copy()";
    return new Gesture_trigger( data_P ? data_P : data, gesturecode());
    }

const QString Gesture_trigger::description() const
    {
    return i18n( "Gesture trigger: " ) + gesturecode();
    }

void Gesture_trigger::handle_gesture( const QString &gesture_P, WId window_P )
    {
    if( gesturecode() == gesture_P )
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }

void Gesture_trigger::activate( bool activate_P )
    {
    if( activate_P )
        gesture_handler->register_handler( this, SLOT( handle_gesture( const QString&, WId )));
    else
        gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }

} // namespace KHotKeys

