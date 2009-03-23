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


#include "settings_writer.h"

#include "action_data/action_data_group.h"
#include "action_data/action_data.h"
#include "action_data/command_url_shortcut_action_data.h"
#include "action_data/generic_action_data.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/simple_action_data.h"

#include "conditions/conditions_list.h"

#include "settings.h"
#include "windows_helper/window_selection_list.h"


#include <KDE/KConfig>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>

namespace KHotKeys {

const int CurrentFileVersion = 2;


SettingsWriter::SettingsWriter(const Settings *settings)
    :   _settings(settings)
    {
    }


void SettingsWriter::exportTo(const ActionDataBase *element, KConfigBase &config)
    {
    if (!element)
        {
        Q_ASSERT(element);
        return;
        }

    // Clean the file
    QStringList groups = config.groupList();
    Q_FOREACH (QString name, config.groupList())
        {
        config.deleteGroup(name);
        }

    KConfigGroup mainGroup(&config, "Main");
    mainGroup.writeEntry("Version", CurrentFileVersion);

    // The root group contains nothing but the datacount!
    KConfigGroup dataGroup(&config,  "Data");
    dataGroup.writeEntry("DataCount", 1);

    // The group for the element to export
    KConfigGroup data1Group(&config,  "Data_1");
    element->cfg_write(data1Group);

    const ActionDataGroup *group = dynamic_cast<const ActionDataGroup*>(element);
    if (group)
        {
        write_actions(data1Group, group, true);
        }
    }


void SettingsWriter::visitActionData(const ActionData *data)
    {
    visitActionDataBase(data);

    KConfigGroup *config = _stack.top();

    // Write triggers if available
    if (data->triggers())
        {
        KConfigGroup triggersGroup( config->config(), config->name() + "Triggers" );
        data->triggers()->cfg_write( triggersGroup );
        }

    // Write actions if available
    if (data->actions())
        {
        KConfigGroup actionsGroup( config->config(), config->name() + "Actions" );
        data->actions()->cfg_write( actionsGroup );
        }
    }


void SettingsWriter::visitActionDataBase(const ActionDataBase *base)
    {
    KConfigGroup *config = _stack.top();

    config->writeEntry( "Type",    "ERROR" ); // derived classes should call with their type
    config->writeEntry( "Name",    base->name());
    config->writeEntry( "Comment", base->comment());
    config->writeEntry( "Enabled", base->enabled(true));

    if (base->conditions())
        {
        KConfigGroup conditionsConfig( config->config(), config->name() + "Conditions" );
        base->conditions()->cfg_write( conditionsConfig );
        }
    else
        {
        kDebug() << "No conditions";
        }
    }

void SettingsWriter::visitActionDataGroup(const ActionDataGroup *group)
    {
    visitActionDataBase(group);

    KConfigGroup *config = _stack.top();

    config->writeEntry( "SystemGroup", int(group->system_group()));
    config->writeEntry( "Type", "ACTION_DATA_GROUP" );

    int cnt = 0;
    Q_FOREACH(ActionDataBase *child, group->children())
        {
        ++cnt;
        KConfigGroup childConfig(
                config->config(),
                config->name() + QString("_") + QString::number(cnt));
        _stack.push(&childConfig);
        kDebug() << child->name();
        child->accept(this);
        _stack.pop();
        }
    config->writeEntry( "DataCount", cnt );
    }


void SettingsWriter::visitCommandUrlShortcutActionData(const CommandUrlShortcutActionData *data)
    {
    visitActionData(data);

    KConfigGroup *config = _stack.top();
    config->writeEntry( "Type", "COMMAND_URL_SHORTCUT_ACTION_DATA" );
    }


void SettingsWriter::visitGenericActionData(const Generic_action_data *data)
    {
    visitActionData(data);

    KConfigGroup *config = _stack.top();
    config->writeEntry( "Type", "GENERIC_ACTION_DATA" );
    }


void SettingsWriter::visitMenuentryShortcutActionData(const MenuEntryShortcutActionData *data)
    {
    visitActionData(data);

    KConfigGroup *config = _stack.top();
    config->writeEntry( "Type", "MENUENTRY_SHORTCUT_ACTION_DATA" );
    }


void SettingsWriter::visitSimpleActionData(const SimpleActionData *data)
    {
    visitActionData(data);

    KConfigGroup *config = _stack.top();
    if (dynamic_cast<const Activate_window_shortcut_action_data*>(data))
        config->writeEntry( "Type", "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA" );
    else if (dynamic_cast<const Dbus_shortcut_action_data*>(data))
        config->writeEntry( "Type", "DBUS_SHORTCUT_ACTION_DATA" );
    else if (dynamic_cast<const Keyboard_input_shortcut_action_data*>(data))
        config->writeEntry( "Type", "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA" );
    else
        config->writeEntry( "Type", "SIMPLE_ACTION_DATA" );
    }


void SettingsWriter::writeTo(KConfigBase &config)
    {
    QStringList groups = config.groupList();
    for( QStringList::ConstIterator it = groups.constBegin();
         it != groups.constEnd();
         ++it )
        config.deleteGroup( *it );

    // Write the global settings
    KConfigGroup mainGroup(&config, "Main");
    mainGroup.writeEntry("Version", 2);
    mainGroup.writeEntry("AlreadyImported", _settings->already_imported);
    mainGroup.writeEntry("Disabled", _settings->isDaemonDisabled());

    // Write the actions
    KConfigGroup dataGroup( &config,  "Data" );
    _stack.push(&dataGroup);

    int cnt = 0;
    if (_settings->actions())
        {
        Q_FOREACH(ActionDataBase *child, _settings->actions()->children())
            {
            ++cnt;
            KConfigGroup childConfig(
                    dataGroup.config(),
                    QString("Data_") + QString::number(cnt));
            _stack.push(&childConfig);
            child->accept(this);
            _stack.pop();
            }
        }
    dataGroup.writeEntry( "DataCount", cnt );
    _stack.pop();

    // CHECKME Count still needed????
    // int cnt = write_actions( dataGroup, _settings->actions(), true );
    // mainGroup.writeEntry( "Autostart", cnt != 0 && !_settings->isDaemonDisabled() );

    // Write the gestures configuration
    KConfigGroup gesturesConfig( &config, "Gestures" );
    gesturesConfig.writeEntry( "Disabled", _settings->areGesturesDisabled() );
    gesturesConfig.writeEntry( "MouseButton", _settings->gestureMouseButton() );
    gesturesConfig.writeEntry( "Timeout", _settings->gestureTimeOut() );
    if( _settings->gesturesExclude() != NULL )
        {
        KConfigGroup gesturesExcludeConfig( &config, "GesturesExclude" );
        _settings->gesturesExclude()->cfg_write( gesturesExcludeConfig );
        }
    else
        config.deleteGroup( "GesturesExclude" );
    KConfigGroup voiceConfig( &config, "Voice" );
    voiceConfig.writeEntry("Shortcut" , _settings->voice_shortcut.toString() );
    }


// return value means the number of enabled actions written in the cfg file
// i.e. 'Autostart' for value > 0 should be on
int SettingsWriter::write_actions(KConfigGroup& cfg_P, const ActionDataGroup* parent_P, bool enabled_P)
    {
    int enabled_cnt = 0;
    QString save_cfg_group = cfg_P.name();
    int cnt = 0;
    if( parent_P )
        {
        Q_FOREACH(ActionDataBase *child,parent_P->children())
            {
            ++cnt;
            if( enabled_P && child->enabled( true ))
                ++enabled_cnt;
            KConfigGroup itConfig( cfg_P.config(), save_cfg_group + '_' + QString::number( cnt ));
            child->cfg_write( itConfig );
            ActionDataGroup* grp = dynamic_cast< ActionDataGroup* >(child);
            if( grp != NULL )
                enabled_cnt += write_actions( itConfig, grp, enabled_P && child->enabled( true ));
            }
        }
    cfg_P.writeEntry( "DataCount", cnt );
    return enabled_cnt;
    }




} // namespace KHotKeys

