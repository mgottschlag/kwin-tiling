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


GestureTrigger::GestureTrigger( Action_data* data_P, const QString &gesturecode_P )
    : Trigger( data_P ), _gesturecode( gesturecode_P )
    {
    }


GestureTrigger::GestureTrigger( KConfigGroup& cfg_P, Action_data* data_P )
    : Trigger( cfg_P, data_P )
    {
    _gesturecode = cfg_P.readEntry( "Gesture" );
    }


GestureTrigger::~GestureTrigger()
    {
    gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }


void GestureTrigger::activate( bool activate_P )
    {
    if( activate_P )
        gesture_handler->register_handler( this, SLOT( handle_gesture( const QString&, WId )));
    else
        gesture_handler->unregister_handler( this, SLOT( handle_gesture( const QString&, WId )));
    }


void GestureTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Gesture", gesturecode());
    cfg_P.writeEntry( "Type", "GESTURE" ); // overwrites value set in base::cfg_write()
    }


Trigger* GestureTrigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "GestureTrigger::copy()";
    return new GestureTrigger( data_P ? data_P : data, gesturecode());
    }


const QString GestureTrigger::description() const
    {
    return i18n( "Gesture trigger: " ) + gesturecode();
    }


const QString& GestureTrigger::gesturecode() const
    {
    return _gesturecode;
    }


void GestureTrigger::handle_gesture( const QString &gesture_P, WId window_P )
    {
    if( gesturecode() == gesture_P )
        {
        windows_handler->set_action_window( window_P );
        data->execute();
        }
    }


} // namespace KHotKeys

