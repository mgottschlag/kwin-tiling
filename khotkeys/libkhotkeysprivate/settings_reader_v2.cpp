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
#include "action_data/generic_action_data.h"
#include "action_data/menuentry_shortcut_action_data.h"
#include "action_data/simple_action_data.h"

#include "conditions/conditions_list.h"

#include "triggers/triggers.h"

#include "windows_helper/window_selection_list.h"


#include "settings.h"

#include <KDE/KConfig>
#include <KDE/KConfigBase>
#include <KDE/KConfigGroup>
#include <KDE/KDebug>


SettingsReaderV2::SettingsReaderV2(
        KHotKeys::Settings *settings,
        bool loadAll,
        KHotKeys::ActionState stateStrategy,
        const QString &importId)
    :   _settings(settings),
        _loadAll(loadAll),
        _stateStrategy(stateStrategy),
        _importId(importId)
    {
#ifdef KHOTKEYS_TRACE
    kDebug() << "Created SettingsReader with disableActions(" << _stateStrategy << ")";
#endif
    }


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
            KHotKeys::ActionDataBase *elem = readActionData(childConfig, parent);
            if (!_importId.isEmpty())
                elem->setImportId(_importId);
            }
        }
    }


KHotKeys::ActionDataGroup *SettingsReaderV2::readGroup(
        const KConfigGroup &config,
        KHotKeys::ActionDataGroup *parent)
    {
#ifdef KHOTKEYS_TRACE
    kDebug() << "Reading group" << config.readEntry( "Name" );
#endif
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
        group = new KHotKeys::ActionDataGroup(parent);
        _config = &config;
        group->accept(this);
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
            readActionData(childConfig, group);
            }
        }

    // The object is complete. Activate it if needed
    switch (_stateStrategy)
        {
        case KHotKeys::Retain:
            config.readEntry("Enabled", false)
                ? group->enable()
                : group->disable();
            break;

        case KHotKeys::Disabled:
            // Its disabled by default
            break;

        case KHotKeys::Enabled:
            group->enable();
            break;

        default:
            kWarning() << "Unkown stateStrategy";
            Q_ASSERT(false);
            break;
        };

    return group;
    }


KHotKeys::ActionDataBase *SettingsReaderV2::readActionData(
        const KConfigGroup &config,
        KHotKeys::ActionDataGroup *parent)
    {
    QString type = config.readEntry("Type");

    if (type == "ACTION_DATA_GROUP")
        {
        return readGroup(config, parent);
        }

    KHotKeys::ActionData *newObject = NULL;

    if (type == "GENERIC_ACTION_DATA")
        {
        newObject = new KHotKeys::Generic_action_data(parent);
        }
    else if (type == "MENUENTRY_SHORTCUT_ACTION_DATA")
        {
        // We collect all of those in the system group
        newObject = new KHotKeys::MenuEntryShortcutActionData(
                _settings->get_system_group(KHotKeys::ActionDataGroup::SYSTEM_MENUENTRIES));
        }
    else if (type == "SIMPLE_ACTION_DATA"
          || type == "COMMAND_URL_SHORTCUT_ACTION_DATA"
          || type == "DCOP_SHORTCUT_ACTION_DATA" || type == "DBUS_SHORTCUT_ACTION_DATA"
          || type == "KEYBOARD_INPUT_GESTURE_ACTION_DATA"
          || type == "KEYBOARD_INPUT_SHORTCUT_ACTION_DATA"
          || type == "ACTIVATE_WINDOW_SHORTCUT_ACTION_DATA")
        {
        newObject = new KHotKeys::SimpleActionData(parent);
        }
    else
        {
        kWarning() << "Unknown ActionDataBase type read from cfg file\n";
        return NULL;
        }

    _config = &config;
    newObject->accept(this);

    // And the actions
    readActionList(config, newObject);

    // Now load the triggers
    readTriggerList(config, newObject);

    // The object is complete. Activate it if needed
    switch (_stateStrategy)
        {
        case KHotKeys::Retain:
            config.readEntry("Enabled", false)
                ? newObject->enable()
                : newObject->disable();
            break;

        case KHotKeys::Disabled:
            // Its disabled by default
            break;

        case KHotKeys::Enabled:
            newObject->enable();
            break;

        default:
            kWarning() << "Unkown stateStrategy";
            Q_ASSERT(false);
            break;
        };

#ifdef KHOTKEYS_TRACE
    kDebug() << newObject->name() << "loaded into" << newObject->isEnabled(KHotKeys::ActionDataBase::Ignore) << "state";
#endif

    return newObject;
    }


KHotKeys::ActionList *SettingsReaderV2::readActionList(
        const KConfigGroup &config,
        KHotKeys::ActionData *parent)
    {
    KConfigGroup actionsGroup(config.config(), config.name() + "Actions");

    int cnt = actionsGroup.readEntry( "ActionsCount", 0 );
    QString save_cfg_group = actionsGroup.name();
    for (int i=0; i<cnt; ++i)
        {
        KConfigGroup group(actionsGroup.config(), save_cfg_group + QString::number(i));
        KHotKeys::Action* action;

        QString type = group.readEntry( "Type" );

        if (type == "COMMAND_URL")
            action = new KHotKeys::CommandUrlAction(parent);
        else if (type == "MENUENTRY")
            action = new KHotKeys::MenuEntryAction(parent);
        else if (type == "DCOP" || type == "DBUS")
            action = new KHotKeys::DBusAction(parent);
        else if (type == "KEYBOARD_INPUT")
            action = new KHotKeys::KeyboardInputAction(parent);
        else if (type == "ACTIVATE_WINDOW")
            action = new KHotKeys::ActivateWindowAction(parent);
        else
            {
            kWarning() << "Unknown Action type read from cfg file\n";
            return NULL;
            }

        _config = &group;
        action->accept(*this);

        parent->add_action( action );
        }

    return NULL;
    }


KHotKeys::Trigger_list *SettingsReaderV2::readTriggerList(
        const KConfigGroup &config,
        KHotKeys::ActionData *parent)
    {
    KConfigGroup triggersGroup(config.config(), config.name() + "Triggers");
    KHotKeys::Trigger_list *list = parent->triggers();

    if (!triggersGroup.exists()) return list;
    Q_ASSERT(list);

    list->set_comment(triggersGroup.readEntry("Comment"));

    KHotKeys::Trigger *trigger = NULL;

    int cnt = triggersGroup.readEntry( "TriggersCount", 0 );
    for (int i = 0; i < cnt; ++i)
        {
        KConfigGroup triggerConfig( triggersGroup.config(), triggersGroup.name() + QString::number( i ));
        QString type = triggerConfig.readEntry("Type");
        QUuid uuid = triggerConfig.readEntry( "Uuid" );
        if( uuid.isNull())
            uuid = QUuid::createUuid();

        if (type == "SHORTCUT" || type == "SINGLE_SHORTCUT")
            trigger = new KHotKeys::ShortcutTrigger(parent, KShortcut(), uuid);
        else if (type == "WINDOW")
            trigger = new KHotKeys::WindowTrigger(parent);
        else if (type == "GESTURE")
            trigger = new KHotKeys::GestureTrigger(parent);
        else
            {
            kWarning() << "khotkeys: Unknown trigger type" << type;
            return NULL;
            }

        _config = & triggerConfig;
        trigger->accept(*this);

        if (trigger) parent->add_trigger( trigger );
        }

    return list;
    }


void SettingsReaderV2::visit(KHotKeys::ActivateWindowAction& action)
    {
    QString save_cfg_group = _config->name();
    KConfigGroup windowGroup(_config->config(), save_cfg_group + "Window" );
    action.set_window_list(new KHotKeys::Windowdef_list(windowGroup));
    }


void SettingsReaderV2::visit(KHotKeys::CommandUrlAction& action)
    {
    action.set_command_url(_config->readEntry("CommandURL"));
    }


void SettingsReaderV2::visit(KHotKeys::DBusAction& action)
    {
    action.set_remote_application(_config->readEntry( "RemoteApp" ));
    action.set_remote_object(_config->readEntry( "RemoteObj" ));
    action.set_called_function(_config->readEntry( "Call" ));
    action.set_arguments(_config->readEntry( "Arguments" ));
    }


void SettingsReaderV2::visit(KHotKeys::GestureTrigger& trigger)
    {
    if (_config->hasKey("Gesture"))
        trigger.setKDE3Gesture(_config->readEntry("Gesture"));
    else
        trigger.setPointData(_config->readEntry("GesturePointData", QStringList()));
    }

void SettingsReaderV2::visit(KHotKeys::KeyboardInputAction& action)
    {
    action.setInput(_config->readEntry("Input"));

    KHotKeys::Windowdef_list *window_list = NULL;
    KHotKeys::KeyboardInputAction::DestinationWindow destWindow;

    // Try the new format with DestinationWindow
    int destination = _config->readEntry( "DestinationWindow", -1);


    switch (destination)
        {
        case KHotKeys::KeyboardInputAction::SpecificWindow:
            {
            KConfigGroup windowGroup( _config->config(), _config->name() + "DestinationWindow" );
            window_list = new KHotKeys::Windowdef_list( windowGroup );
            destWindow = KHotKeys::KeyboardInputAction::SpecificWindow;
            }
            break;

        case KHotKeys::KeyboardInputAction::ActionWindow:
            destWindow = KHotKeys::KeyboardInputAction::ActionWindow;
            break;

        case KHotKeys::KeyboardInputAction::ActiveWindow:
            destWindow = KHotKeys::KeyboardInputAction::ActiveWindow;
            break;

        case -1:
            {
            // Old format
            if(_config->readEntry( "IsDestinationWindow" , false))
                {
                KConfigGroup windowGroup( _config->config(), _config->name() + "DestinationWindow" );
                window_list = new KHotKeys::Windowdef_list( windowGroup );
                destWindow = KHotKeys::KeyboardInputAction::SpecificWindow;
                }
            else
                {
                if (_config->readEntry( "ActiveWindow" , false)) destWindow = KHotKeys::KeyboardInputAction::ActiveWindow;
                else destWindow = KHotKeys::KeyboardInputAction::ActionWindow;
                }
            }
            break;

        default:
            Q_ASSERT(false);
            destWindow = KHotKeys::KeyboardInputAction::ActionWindow;
        }

    if (!window_list) window_list = new KHotKeys::Windowdef_list;

    action.setDestination(destWindow);
    action.setDestinationWindowRules(window_list);
    }



void SettingsReaderV2::visit(KHotKeys::MenuEntryAction& action)
    {
    visit( * static_cast<KHotKeys::CommandUrlAction*>(&action));
    }


void SettingsReaderV2::visit(KHotKeys::ShortcutTrigger &trigger)
    {
    QString shortcutString = _config->readEntry( "Key" );

    // TODO: Check if this is still necessary
    shortcutString.replace("Win+", "Meta+"); // Qt4 doesn't parse Win+, avoid a shortcut without modifier

    trigger.set_key_sequence(shortcutString);
    }


void SettingsReaderV2::visit(KHotKeys::WindowTrigger &trigger)
    {
    KConfigGroup windowsConfig( _config->config(), _config->name() + "Windows" );
    trigger.set_window_rules(new KHotKeys::Windowdef_list(windowsConfig));
    trigger.setOnWindowEvents(KHotKeys::WindowTrigger::WindowEvents(_config->readEntry( "WindowActions", 0 )));
    }



void SettingsReaderV2::visitActionDataBase(
        KHotKeys::ActionDataBase *object)
    {
    object->set_name(_config->readEntry("Name"));
    object->set_comment(_config->readEntry("Comment"));
    object->setImportId(_config->readEntry("ImportId"));
    object->setAllowMerging(_config->readEntry("AllowMerge", false));

    KConfigGroup conditionsConfig( _config->config(), _config->name() + "Conditions" );

    // Load the conditions if they exist
    if ( conditionsConfig.exists() )
        {
        object->set_conditions(new KHotKeys::Condition_list(conditionsConfig, object));
        }
    else
        {
        object->set_conditions(new KHotKeys::Condition_list(QString(), object));
        }
    }


void SettingsReaderV2::visitActionData(
        KHotKeys::ActionData *object)
    {
    visitActionDataBase(object);
    }


void SettingsReaderV2::visitActionDataGroup(
        KHotKeys::ActionDataGroup *object)
    {
    unsigned int system_group_tmp = _config->readEntry( "SystemGroup", 0 );

    // Correct wrong values
    if(system_group_tmp >= KHotKeys::ActionDataGroup::SYSTEM_MAX)
        {
        system_group_tmp = 0;
        }

    object->set_system_group(static_cast< KHotKeys::ActionDataGroup::system_group_t >(system_group_tmp));

    visitActionDataBase(object);
    }


void SettingsReaderV2::visitGenericActionData(
        KHotKeys::Generic_action_data *object)
    {
    visitActionData(object);
    }


void SettingsReaderV2::visitMenuentryShortcutActionData(
        KHotKeys::MenuEntryShortcutActionData *object)
    {
    visitSimpleActionData(object);
    }


void SettingsReaderV2::visitSimpleActionData(
        KHotKeys::SimpleActionData *object)
    {
    visitActionData(object);
    }
