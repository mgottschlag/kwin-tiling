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

        CommandUrlShortcutActionData(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                bool enabled = true);

        CommandUrlShortcutActionData(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                const KShortcut& shortcut,
                const QString& command_url,
                bool enabled = true);

        CommandUrlShortcutActionData(
                KConfigGroup& cfg,
                ActionDataGroup* parent);

        /**
         * Visitor pattern
         * @reimp
         */
        virtual void accept(ActionDataVisitor *visitor) const;

    };


} // namespace KHotKeys

#endif

