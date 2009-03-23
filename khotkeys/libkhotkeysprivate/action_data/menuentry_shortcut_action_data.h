#ifndef MENUENTRY_SHORTCUT_ACTION_DATA_H
#define MENUENTRY_SHORTCUT_ACTION_DATA_H
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

#include "simple_action_data.h"
#include "triggers/triggers.h"
#include "actions/actions.h"


namespace KHotKeys {

class KDE_EXPORT MenuEntryShortcutActionData
    : public Simple_action_data< ShortcutTrigger, MenuEntryAction >
    {

    typedef Simple_action_data< ShortcutTrigger, MenuEntryAction > base;

    public:

        MenuEntryShortcutActionData(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                bool enabled = true);

        MenuEntryShortcutActionData(
                ActionDataGroup* parent,
                const QString& name,
                const QString& comment,
                const KShortcut& shortcut,
                const QString& command_url,
                bool enabled = true);

        MenuEntryShortcutActionData(
                KConfigGroup& cfg,
                ActionDataGroup* parent);

        /**
         * Visitor pattern
         * @reimp
         */
        virtual void accept(ActionDataVisitor *visitor) const;

    };


} // namespace KHotKeys


#endif /* MENUENTRY_SHORTCUT_ACTION_DATA_H */
