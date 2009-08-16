/**
 * Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "menuentry_shortcut_action_data.h"

#include "action_data/action_data_visitor.h"
#include "actions/actions.h"
#include "conditions/conditions.h"

#include <kconfiggroup.h>
#include <kdebug.h>

namespace KHotKeys {


MenuEntryShortcutActionData::MenuEntryShortcutActionData( 
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment,
        const KShortcut& shortcut,
        const QString& menuentry)
    :   base(parent, name, comment)
    {
    set_action( new MenuEntryAction( this, menuentry ));
    set_trigger( new ShortcutTrigger( this, shortcut ));
    }


MenuEntryShortcutActionData::MenuEntryShortcutActionData(
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment)
    :   base(parent, name, comment)
    {}


void MenuEntryShortcutActionData::accept(ActionDataVisitor *visitor)
    {
    visitor->visitMenuentryShortcutActionData(this);
    }


void MenuEntryShortcutActionData::accept(ActionDataConstVisitor *visitor) const
    {
    visitor->visitMenuentryShortcutActionData(this);
    }


} // namespace KHotKeys
