/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef MENUENTRY_SHORTCUT_ACTION_DATA_H
#define MENUENTRY_SHORTCUT_ACTION_DATA_H

#include "simple_action_data.h"
#include "triggers.h"
#include "actions.h"


namespace KHotKeys {

class KDE_EXPORT Menuentry_shortcut_action_data
    : public Simple_action_data< Shortcut_trigger, MenuEntryAction >
    {

        typedef Simple_action_data< Shortcut_trigger, MenuEntryAction > base;

    public:

        Menuentry_shortcut_action_data( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );

        Menuentry_shortcut_action_data( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, const KShortcut& shortcut_P, const QString& command_url_P,
            bool enabled_P = true );

        Menuentry_shortcut_action_data( KConfigGroup& cfg_P, ActionDataGroup* parent_P );
    };


} // namespace KHotKeys

#endif
