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
void Simple_action_data< Shortcut_trigger, Command_url_action >
    ::cfg_write( KConfigGroup& cfg_P ) const
    {
    base::cfg_write( cfg_P );
    cfg_P.writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }


Command_url_shortcut_action_data::Command_url_shortcut_action_data( 
        Action_data_group* parent_P,
        const QString& name_P,
        const QString& comment_P,
        const KShortcut& shortcut_P,
        const QString& command_url_P,
        bool enabled_P )
    : Simple_action_data< Shortcut_trigger, Command_url_action >( parent_P, name_P,
        comment_P, enabled_P )
    {
    set_action( new Command_url_action( this, command_url_P ));
    set_trigger( new Shortcut_trigger( this, shortcut_P ));
    }


Command_url_shortcut_action_data::Command_url_shortcut_action_data( Action_data_group* parent_P,
    const QString& name_P, const QString& comment_P, bool enabled_P )
    : Simple_action_data< Shortcut_trigger, Command_url_action >( parent_P, name_P,
        comment_P, enabled_P )
    {
    }


Command_url_shortcut_action_data::Command_url_shortcut_action_data( 
        KConfigGroup& cfg_P,
        Action_data_group* parent_P )
    : Simple_action_data< Shortcut_trigger, Command_url_action >( cfg_P, parent_P )
    {
    }


} // namespace KHotKeys
