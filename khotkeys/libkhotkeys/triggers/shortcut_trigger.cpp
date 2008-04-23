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

ShortcutTrigger::ShortcutTrigger( 
        ActionData* data_P,
        const KShortcut& shortcut_P,
        const QUuid &uuid )
    : Trigger( data_P ), _shortcut( shortcut_P ), _uuid(uuid)
    {
    QString name;
    if (data_P)
        {
        name = data_P->name();
        }
    else
        {
        name = "TODO";
        }
    KAction *act = keyboard_handler->addAction( _uuid, name, _shortcut );
    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );
    }


ShortcutTrigger::ShortcutTrigger( 
        KConfigGroup& cfg_P
       ,ActionData* data_P )
    : Trigger( cfg_P, data_P )
     ,_shortcut( cfg_P.readEntry( "Key", QString() ))
     ,_uuid( cfg_P.readEntry( "Uuid", QUuid::createUuid().toString()))
    {
    KAction *act = keyboard_handler->addAction( _uuid, data_P->name(), _shortcut );
    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );
    }


ShortcutTrigger::~ShortcutTrigger()
    {
    keyboard_handler->removeAction( _uuid );
    }


void ShortcutTrigger::activate( bool activate_P )
    {
    kDebug() << activate_P << " and " << khotkeys_active();
    if( activate_P && khotkeys_active())
        {
        kDebug() << "TODO implement activate";
        }
    else
        {
        kDebug() << "TODO implement activate";
        }
    }


void ShortcutTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", _shortcut.toString());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Uuid", _uuid.toString() );
    }


ShortcutTrigger* ShortcutTrigger::copy( ActionData* data_P ) const
    {
    kDebug( 1217 ) << "ShortcutTrigger::copy()";
    return new ShortcutTrigger( data_P ? data_P : data, shortcut(), QUuid::createUuid());
    }


const QString ShortcutTrigger::description() const
    {
    // CHECKME vice mods
    return i18n( "Shortcut trigger: " ) + _shortcut.toString();
    // CHECKME i18n pro toString() ?
    }


void ShortcutTrigger::set_key_sequence( const QKeySequence &seq )
    {
    Q_ASSERT( &_shortcut != 0 );
    _shortcut.setPrimary( seq );

    KAction *action = qobject_cast<KAction*>(keyboard_handler->getAction( _uuid ));
    Q_ASSERT(action);
    if (!action) return;

    action->setGlobalShortcut( _shortcut, KAction::DefaultShortcut | KAction::ActiveShortcut, KAction::NoAutoloading );
    }


const KShortcut& ShortcutTrigger::shortcut() const
    {
    return _shortcut;
    }


void ShortcutTrigger::trigger()
    {
    kDebug() << data->name() << " was triggered";
    windows_handler->set_action_window( 0 ); // use active window
    data->execute();
    }




} // namespace KHotKeys

