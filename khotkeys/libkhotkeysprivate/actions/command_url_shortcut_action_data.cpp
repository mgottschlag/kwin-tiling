/****************************************************************************

 KHotKeys
 
 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.
 
****************************************************************************/

#include "command_url_shortcut_action_data.h"
#include "actions.h"
#include "conditions.h"

#include <kconfiggroup.h>


namespace KHotKeys
{


template<> KDE_EXPORT
void Simple_action_data< ShortcutTrigger, CommandUrlAction >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }


CommandUrlShortcutActionData::CommandUrlShortcutActionData( 
        ActionDataGroup* parent_P,
        const QString& name_P,
        const QString& comment_P,
        const KShortcut& shortcut_P,
        const QString& command_url_P,
        bool enabled_P )
    : Simple_action_data< ShortcutTrigger, CommandUrlAction >( parent_P, name_P,
        comment_P, enabled_P )
    {
    set_action( new CommandUrlAction( this, command_url_P ));
    set_trigger( new ShortcutTrigger( this, shortcut_P ));
    }


CommandUrlShortcutActionData::CommandUrlShortcutActionData( ActionDataGroup* parent_P,
    const QString& name_P, const QString& comment_P, bool enabled_P )
    : Simple_action_data< ShortcutTrigger, CommandUrlAction >( parent_P, name_P,
        comment_P, enabled_P )
    {
    }


CommandUrlShortcutActionData::CommandUrlShortcutActionData( 
        KConfigGroup& cfg_P,
        ActionDataGroup* parent_P )
    : Simple_action_data< ShortcutTrigger, CommandUrlAction >( cfg_P, parent_P )
    {
    }


} // namespace KHotKeys
