/****************************************************************************

 KHotKeys

 Copyright (C) 1999-2001 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#ifndef COMMAND_URL_SHORTCUT_ACTION_DATA_H
#define COMMAND_URL_SHORTCUT_ACTION_DATA_H

#include "simple_action_data.h"


namespace KHotKeys {

class KDE_EXPORT CommandUrlShortcutActionData
    : public Simple_action_data< ShortcutTrigger, CommandUrlAction >
    {
        typedef Simple_action_data< ShortcutTrigger, CommandUrlAction > base;
    public:
        CommandUrlShortcutActionData( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, bool enabled_P = true );
        CommandUrlShortcutActionData( ActionDataGroup* parent_P, const QString& name_P,
            const QString& comment_P, const KShortcut& shortcut_P, const QString& command_url_P,
            bool enabled_P = true );
        CommandUrlShortcutActionData( KConfigGroup& cfg_P, ActionDataGroup* parent_P );
    };


} // namespace KHotKeys

#endif

