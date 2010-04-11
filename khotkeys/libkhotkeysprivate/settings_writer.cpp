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


SettingsWriter::SettingsWriter(
            const Settings *settings,
            ActionState state,
            const QString &id,
            bool allowMerging)
    :   _settings(settings)
        ,_state(state)
        ,_importId(id)
        ,_allowMerging(allowMerging)
        ,_export(false)
    {
    }


void SettingsWriter::exportTo(
        const ActionDataBase *element,
        KConfigBase &config)
    {
    _export = true;

    if (!element)
        {
        Q_ASSERT(element);
        return;
        }

    // Clean the file
    QStringList groups = config.groupList();
    Q_FOREACH (const QString &name, config.groupList())
        {
        config.deleteGroup(name);
        }

    KConfigGroup mainGroup(&config, "Main");
    mainGroup.writeEntry("Version", CurrentFileVersion);
    mainGroup.writeEntry("AllowMerge", _allowMerging);

    if (!_importId.isEmpty()) mainGroup.writeEntry("ImportId", _importId);

    // The root group contains nothing but the datacount!
    KConfigGroup dataGroup(&config,  "Data");
    dataGroup.writeEntry("DataCount", 1);

    // The group for the element to export
    KConfigGroup data1Group(&config,  "Data_1");
    _stack.push(&data1Group);
    element->accept(this);
    _stack.pop();

    _export = false;
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

    switch (_state)
        {
        case KHotKeys::Retain:
            config->writeEntry( "Enabled", base->isEnabled(ActionDataBase::Ignore));
            break;

        case KHotKeys::Enabled:
            config->writeEntry("Enabled", true);
            break;

        case KHotKeys::Disabled:
            config->writeEntry("Enabled", false);
            break;

        default:
            Q_ASSERT(false);
            config->writeEntry("Enabled", false);
        }

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
        child->accept(this);
        _stack.pop();
        }
    config->writeEntry( "DataCount", cnt );

    // We only write those two back if we do not export the settings
    if (!_export)
        {
        // ImportId only if set
        if (!group->importId().isEmpty())
            config->writeEntry("ImportId", group->importId());
        if (group->allowMerging())
            config->writeEntry("AllowMerge", group->allowMerging());
        }

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


} // namespace KHotKeys

