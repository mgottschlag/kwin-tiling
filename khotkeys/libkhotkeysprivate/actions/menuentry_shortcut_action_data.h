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

class KDE_EXPORT MenuEntryShortcutActionData
    : public Simple_action_data< ShortcutTrigger, MenuEntryAction >
    {

        typedef Simple_action_data< ShortcutTrigger, MenuEntryAction > base;

    public:

        MenuEntryShortcutActionData( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );

        MenuEntryShortcutActionData( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, const KShortcut& shortcut_P, const QString& command_url_P,
            bool enabled_P = true );

        MenuEntryShortcutActionData( KConfigGroup& cfg_P, ActionDataGroup* parent_P );
    };


} // namespace KHotKeys

#endif
