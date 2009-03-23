/**
 * Copyright (C) 2009 Michael Jansen <kde@michael-jansen.biz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "settings_reader_v2.h"

#include "action_data/action_data_group.h"
#include "action_data/simple_action_data.h"
#include "action_data/generic_action_data.h"

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
    readGroup(data, parent);
    }


void SettingsReaderV2::readGroup(const KConfigGroup &group, KHotKeys::ActionDataGroup *parent)
    {
    QString groupName = group.name();

    int cnt = group.readEntry("DataCount", 0);
    for (int i = 1; i <= cnt; ++i)
        {
        KConfigGroup dataGroup(group.config(), groupName + '_' + QString::number(i));
        if (_loadAll || KHotKeys::ActionDataBase::cfg_is_enabled(dataGroup))
            {

            KHotKeys::ActionDataBase* new_action = readAction(dataGroup, parent);

            if (_disableActions)
                {
                new_action->set_enabled(false);
                }

            KHotKeys::ActionDataGroup* grp = dynamic_cast< KHotKeys::ActionDataGroup* >( new_action );

            if (grp)
                {
                readGroup(dataGroup, grp);
                }

            }
        }
    }


KHotKeys::ActionDataBase *SettingsReaderV2::readAction(
        const KConfigGroup &actionGroup,
        KHotKeys::ActionDataGroup *parent)
    {
    QString type = actionGroup.readEntry( "Type" );

    if (type == "ACTION_DATA_GROUP")
        {
        if (actionGroup.readEntry("AllowMerge", false))
            {
            Q_FOREACH (KHotKeys::ActionDataBase *child,parent->children())
                {
                if (KHotKeys::ActionDataGroup* existing = dynamic_cast< KHotKeys::ActionDataGroup* >(child))
                    {
                    if (actionGroup.readEntry( "Name" ) == existing->name())
                        {
                        return existing;
                        }
                    }
                }
            }
        return new KHotKeys::ActionDataGroup( actionGroup, parent );
        }
    else if (type == "GENERIC_ACTION_DATA")
        {
        return new KHotKeys::Generic_action_data( actionGroup, parent );
        }
#if 0
    // TODO: Remove KEYBOARD_INPUT_GESTURE_ACTION_DATA
    else if( type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA" )
        {
        return new Keyboard_input_gesture_action_data( actionGroup, parent );
        }
#endif
    else if (type == "SIMPLE_ACTION_DATA"
          || type == "DCOP_SHORTCUT_ACTION_DATA" || type == "DBUS_SHORTCUT_ACTION_DATA"
          || type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA"
          || type == "MENUENTRY_SHORTCUT_ACTION_DATA"
          || type == "COMMAND_URL_SHORTCUT_ACTION_DATA"
          || type == "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA"
          || type == "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA")
        {
        return new KHotKeys::SimpleActionData( actionGroup, parent );
        }
    else
        {
        kWarning() << "Unknown ActionDataBase type read from cfg file\n";
        return 0;
        }
    }

