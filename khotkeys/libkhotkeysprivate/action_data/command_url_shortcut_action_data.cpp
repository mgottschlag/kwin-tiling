/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#include "command_url_shortcut_action_data.h"

#include "action_data/action_data_visitor.h"
#include "actions/actions.h"
#include "conditions/conditions.h"

#include <kconfiggroup.h>


namespace KHotKeys
{


template<> KDE_EXPORT
void Simple_action_data< ShortcutTrigger, CommandUrlAction >
    ::cfg_write( KConfigGroup& cfg ) const
    {
    base::cfg_write( cfg );
    cfg.writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }


CommandUrlShortcutActionData::CommandUrlShortcutActionData(
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment,
        const KShortcut& shortcut,
        const QString& command_url,
        bool enabled)
    :   Simple_action_data< ShortcutTrigger, CommandUrlAction >(
            parent,
            name,
            comment,
            enabled)
    {
    set_action( new CommandUrlAction( this, command_url ));
    set_trigger( new ShortcutTrigger( this, shortcut ));
    }


void CommandUrlShortcutActionData::accept(ActionDataVisitor *visitor) const
    {
    visitor->visitCommandUrlShortcutActionData(this);
    }


CommandUrlShortcutActionData::CommandUrlShortcutActionData(
        ActionDataGroup* parent,
        const QString& name,
        const QString& comment,
        bool enabled)
    :   Simple_action_data< ShortcutTrigger, CommandUrlAction >(
            parent,
            name,
            comment,
            enabled)
    {}


CommandUrlShortcutActionData::CommandUrlShortcutActionData(
        KConfigGroup& cfg,
        ActionDataGroup* parent)
    :   Simple_action_data< ShortcutTrigger, CommandUrlAction >(cfg, parent)
    {}


} // namespace KHotKeys
