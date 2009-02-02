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

#include "triggers/triggers.h"
#include "action_data/action_data.h"
#include "windows_handler.h"

#include <KDE/KAction>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>

#include "shortcuts_handler.h"

namespace KHotKeys {

ShortcutTrigger::ShortcutTrigger(
        ActionData* data_P,
        const KShortcut& shortcut,
        const QUuid &uuid )
    : Trigger( data_P ), _uuid(uuid), _conditions_met(false)
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
    KAction *act = keyboard_handler->addAction( _uuid, name, shortcut );
    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );

    connect(
        act, SIGNAL(globalShortcutChanged(const QKeySequence&)),
        this, SIGNAL(globalShortcutChanged(const QKeySequence&)));
    }


ShortcutTrigger::ShortcutTrigger(
        KConfigGroup& cfg_P
       ,ActionData* data_P )
    :   Trigger( cfg_P, data_P ),
        _uuid( cfg_P.readEntry( "Uuid", QUuid::createUuid().toString())),
        _conditions_met(true)
    {
    QString shortcutString = cfg_P.readEntry( "Key" );

    // TODO: Check if this is still necessary
    shortcutString.replace("Win+", "Meta+"); // Qt4 doesn't parse Win+, avoid a shortcut without modifier

    KAction *act = keyboard_handler->addAction(
        _uuid,
        data_P->name(),
        KShortcut(shortcutString));

    connect(
        act, SIGNAL(triggered(bool)),
        this, SLOT(trigger()) );

    connect(
        act, SIGNAL(globalShortcutChanged(const QKeySequence&)),
        this, SIGNAL(globalShortcutChanged(const QKeySequence&)));
    }


ShortcutTrigger::~ShortcutTrigger()
    {
    keyboard_handler->removeAction( _uuid );
    }


void ShortcutTrigger::aboutToBeErased()
    {
    kDebug();
    KAction *action = qobject_cast<KAction*>(keyboard_handler->getAction( _uuid ));
    if(action)
        {
        kDebug() << "removing the global shortcut";
        action->forgetGlobalShortcut();
        }
    }


void ShortcutTrigger::activate( bool activate_P )
    {
    kDebug() << activate_P << " and " << khotkeys_active();
    if( activate_P && khotkeys_active())
        {
        _conditions_met = true;
        }
    else
        {
        _conditions_met = false;
        }
    }


void ShortcutTrigger::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Key", shortcut().toString());
    cfg_P.writeEntry( "Type", "SHORTCUT" ); // overwrites value set in base::cfg_write()
    cfg_P.writeEntry( "Uuid", _uuid.toString() );
    }


ShortcutTrigger* ShortcutTrigger::copy( ActionData* data_P ) const
    {
    kDebug() << "Shortcut_trigger::copy()";
    return new ShortcutTrigger( data_P ? data_P : data, shortcut(), QUuid::createUuid());
    }


const QString ShortcutTrigger::description() const
    {
    return i18n( "Shortcut trigger: " ) + shortcut().toString();
    }


void ShortcutTrigger::set_key_sequence( const QKeySequence &seq )
    {
    // Get the action from the keyboard handler
    KAction *action = qobject_cast<KAction*>(keyboard_handler->getAction( _uuid ));
    Q_ASSERT(action);
    if (!action) return;

    // Set our key sequence
    action->setGlobalShortcut( 
        KShortcut(seq),
        KAction::ActiveShortcut,
        KAction::NoAutoloading );
    }


KShortcut ShortcutTrigger::shortcut() const
    {
    // Get the action from the keyboard handler
    KAction *action = qobject_cast<KAction*>(keyboard_handler->getAction( _uuid ));
    Q_ASSERT(action);
    if (!action) 
        return KShortcut();

    return action->globalShortcut();
    }


void ShortcutTrigger::trigger()
    {
    kDebug() << data->name() << " was triggered";
    if (_conditions_met)
        {
        windows_handler->set_action_window( 0 ); // use active window
        data->execute();
        }
    }


} // namespace KHotKeys

