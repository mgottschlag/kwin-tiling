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
#include "windows.h"

#include <KDE/KAction>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

Shortcut_trigger::Shortcut_trigger(
        Action_data* data_P,
        const QString &text,
        const KShortcut& shortcut_P,
        const QUuid &uuid )
    : Trigger( data_P ), _shortcut( shortcut_P ), _uuid(uuid)
    {
    KAction *act = keyboard_handler->addAction( _uuid, text, _shortcut );
    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );
    }

Shortcut_trigger::Shortcut_trigger(
        KConfigGroup& cfg_P
       ,Action_data* data_P )
    : Trigger( cfg_P, data_P )
     ,_shortcut()
     ,_uuid( cfg_P.readEntry( "Uuid", QUuid::createUuid().toString()))
    {
        QString shortcutString = cfg_P.readEntry( "Key" );
        shortcutString.replace("Win+", "Meta+"); // Qt4 doesn't parse Win+, avoid a shortcut without modifier
        _shortcut = KShortcut(shortcutString);
    KAction *act = keyboard_handler->addAction( _uuid, data_P->name(), _shortcut );
    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );
    }

Shortcut_trigger::~Shortcut_trigger()
    {
    keyboard_handler->removeAction( _uuid );
    }

void Shortcut_trigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", _shortcut.toString());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Uuid", _uuid.toString() );
    }

Shortcut_trigger* Shortcut_trigger::copy( Action_data* data_P ) const
    {
    kDebug( 1217 ) << "Shortcut_trigger::copy()";
    return new Shortcut_trigger( data_P ? data_P : data, i18n("Copy of ") + QString(data_P ? data_P->name() : data->name()), shortcut(), QUuid::createUuid());
    }

const QString Shortcut_trigger::description() const
    {
    // CHECKME vice mods
    return i18n( "Shortcut trigger: " ) + _shortcut.toString();
    // CHECKME i18n pro toString() ?
    }

void Shortcut_trigger::trigger()
    {
    kDebug() << data->name() << " was triggered";
    windows_handler->set_action_window( 0 ); // use active window
    data->execute();
    }

void Shortcut_trigger::activate( bool activate_P )
    {
    if( activate_P && khotkeys_active())
        {
        kDebug() << "TODO implement activate";
        }
    else
        {
        kDebug() << "TODO implement activate";
        }
    }

} // namespace KHotKeys

