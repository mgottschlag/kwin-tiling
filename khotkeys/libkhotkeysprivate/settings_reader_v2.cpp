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
 **/

#include "settings_reader_v2.h"

#include "action_data/action_data_group.h"
#include "action_data/command_url_shortcut_action_data.h"
#include "action_data/generic_action_data.h"
#include "action_data/keyboard_input_gesture_action_data.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/simple_action_data.h"

#include "triggers/triggers.h"

#include "settings.h"

#include <KDE/KConfig>
#include <KDE/KConfigBase>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>


SettingsReaderV2::SettingsReaderV2(KHotKeys::Settings *settings, bool loadAll, bool loadDisabled)
    :   _settings(settings),
        _loadAll(loadAll),
        _disableActions(loadDisabled)
    {}


SettingsReaderV2::~SettingsReaderV2()
    {}


void SettingsReaderV2::read(const KConfigBase &config, KHotKeys::ActionDataGroup *parent)
    {
    KConfigGroup data(&config, "Data");

    // We have to skip the top level group.
    QString configName = data.name();
    int cnt = data.readEntry("DataCount", 0);
    for (int i = 1; i <= cnt; ++i)
        {
        KConfigGroup childConfig(data.config(), configName + '_' + QString::number(i));
        if (_loadAll || KHotKeys::ActionDataBase::cfg_is_enabled(childConfig))
            {
            readAction(childConfig, parent);
            }
        }
    }


KHotKeys::ActionDataGroup *SettingsReaderV2::readGroup(
        const KConfigGroup &config,
        KHotKeys::ActionDataGroup *parent)
    {
    KHotKeys::ActionDataGroup *group = NULL;

    // Check if it is allowed to merge the group. If yes check for a group
    // with the desired name
    if (config.readEntry("AllowMerge", false))
        {
        Q_FOREACH (KHotKeys::ActionDataBase *child, parent->children())
            {
            if (KHotKeys::ActionDataGroup* existing = dynamic_cast< KHotKeys::ActionDataGroup* >(child))
                {
                if (config.readEntry( "Name" ) == existing->name())
                    {
                    group = existing;
                    break;
                    }
                }
            }
        }

    // Do not allow loading a system group if there is already one.
    unsigned int system_group_tmp = config.readEntry( "SystemGroup", 0 );
    if ((system_group_tmp != 0) && (system_group_tmp < KHotKeys::ActionDataGroup::SYSTEM_MAX))
            {
            // It's a valid value. Get the system group and load into it
            group = _settings->get_system_group(
                    static_cast< KHotKeys::ActionDataGroup::system_group_t > (system_group_tmp));
            }

    // if no group was found or merging is disabled create a new group
    if (!group)
        {
        kDebug() << "Creating group " << config.name() << parent->comment();
        group = new KHotKeys::ActionDataGroup(config, parent );

        // We only disable newly created groups
        if (_disableActions)
            {
            group->set_enabled(false);
            }

        }

    Q_ASSERT(group);

    // Now load the children
    QString configName = config.name();
    int cnt = config.readEntry("DataCount", 0);
    for (int i = 1; i <= cnt; ++i)
        {
        KConfigGroup childConfig(config.config(), configName + '_' + QString::number(i));
        if (_loadAll || KHotKeys::ActionDataBase::cfg_is_enabled(childConfig))
            {
            readAction(childConfig, group);
            }
        }

    return group;
    }


KHotKeys::ActionDataBase *SettingsReaderV2::readAction(
        const KConfigGroup &config,
        KHotKeys::ActionDataGroup *parent)
    {
    KHotKeys::ActionDataBase *newObject = NULL;

    QString type = config.readEntry("Type");

    if (type == "ACTION_DATA_GROUP")
        {
        newObject = readGroup(config, parent);
        // Groups take care of disabling themselves.
        return newObject;
        }
    else if (type == "GENERIC_ACTION_DATA")
        {
        newObject = new KHotKeys::Generic_action_data(config, parent);
        }
    else if (type == "MENUENTRY_SHORTCUT_ACTION_DATA")
        {
        // We collect all of those in the system group
        newObject = new KHotKeys::MenuEntryShortcutActionData(
                config,
                _settings->get_system_group(KHotKeys::ActionDataGroup::SYSTEM_MENUENTRIES));
        }
    else if (type == "SIMPLE_ACTION_DATA"
          || type == "COMMAND_URL_SHORTCUT_ACTION_DATA"
          || type == "DCOP_SHORTCUT_ACTION_DATA" || type == "DBUS_SHORTCUT_ACTION_DATA"
          || type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA"
          || type == "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA"
          || type == "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA")
        {
        KHotKeys::SimpleActionData *sa;

        sa = new KHotKeys::SimpleActionData(config, parent);
        if (sa->trigger()->type() == KHotKeys::Trigger::ShortcutTriggerType
                && sa->action()->type() == KHotKeys::Action::MenuEntryActionType)
            {
            delete sa;
            newObject = new KHotKeys::MenuEntryShortcutActionData(config, parent);
            }
        else
            {
            newObject = sa;
            }
        }
    else
        {
        kWarning() << "Unknown ActionDataBase type read from cfg file\n";
        return NULL;
        }

    return newObject;
    }

